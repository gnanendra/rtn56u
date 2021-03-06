/*
  File: getfacl.c
  (Linux Access Control List Management)

  Copyright (C) 1999-2002
  Andreas Gruenbacher, <a.gruenbacher@computer.org>
 	
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <errno.h>
#include <sys/acl.h>
#include <acl/libacl.h>

#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <getopt.h>
#include <ftw.h>
#include <locale.h>
#include "config.h"
#include "user_group.h"
#include "misc.h"

#define POSIXLY_CORRECT_STR "POSIXLY_CORRECT"

#if !POSIXLY_CORRECT
#  define CMD_LINE_OPTIONS "dRLP"
#endif
#define POSIXLY_CMD_LINE_OPTIONS "d"

struct option long_options[] = {
#if !POSIXLY_CORRECT
	{ "access",	0, 0, 'a' },
	{ "omit-header",	0, 0, 'c' },
	{ "all-effective",	0, 0, 'e' },
	{ "no-effective",	0, 0, 'E' },
	{ "skip-base",	0, 0, 's' },
	{ "recursive",	0, 0, 'R' },
	{ "logical",	0, 0, 'L' },
	{ "physical",	0, 0, 'P' },
	{ "tabular",	0, 0, 't' },
	{ "absolute-names",	0, 0, 'p' },
	{ "numeric",	0, 0, 'n' },
#endif
	{ "default",	0, 0, 'd' },
	{ "version",	0, 0, 'v' },
	{ "help",	0, 0, 'h' },
	{ NULL,		0, 0, 0   }
};

const char *progname;
const char *cmd_line_options;

int opt_recursive;  /* recurse into sub-directories? */
int opt_walk_logical;  /* always follow symbolic links */
int opt_walk_physical;  /* never follow symbolic links */
int opt_print_acl = 0;
int opt_print_default_acl = 0;
int opt_strip_leading_slash = 1;
int opt_comments = 1;  /* include comments */
int opt_skip_base = 0;  /* skip files that only have the base entries */
int opt_tabular = 0;  /* tabular output format (alias `showacl') */
#if POSIXLY_CORRECT
const int posixly_correct = 1;  /* Posix compatible behavior! */
#else
int posixly_correct = 0;  /* Posix compatible behavior? */
#endif
int had_errors = 0;
int absolute_warning = 0;  /* Absolute path warning was issued */
int print_options = TEXT_SOME_EFFECTIVE;
int opt_numeric = 0;  /* don't convert id's to symbolic names */


static const char *xquote(const char *str)
{
	const char *q = quote(str);
	if (q == NULL) {
		fprintf(stderr, "%s: %s\n", progname, strerror(errno));
		exit(1);
	}
	return q;
}

struct name_list {
	struct name_list *next;
	char name[0];
};

void free_list(struct name_list *names)
{
	struct name_list *next;

	while (names) {
		next = names->next;
		free(names);
		names = next;
	}
}

struct name_list *get_list(const struct stat *st, acl_t acl)
{
	struct name_list *first = NULL, *last = NULL;
	acl_entry_t ent;
	int ret = 0;

	if (acl != NULL)
		ret = acl_get_entry(acl, ACL_FIRST_ENTRY, &ent);
	if (ret != 1)
		return NULL;
	while (ret > 0) {
		acl_tag_t e_type;
		const id_t *id_p;
		const char *name = "";
		int len;

		acl_get_tag_type(ent, &e_type);
		switch(e_type) {
			case ACL_USER_OBJ:
				name = user_name(st->st_uid, opt_numeric);
				break;

			case ACL_USER:
				id_p = acl_get_qualifier(ent);
				if (id_p != NULL)
					name = user_name(*id_p, opt_numeric);
				break;

			case ACL_GROUP_OBJ:
				name = group_name(st->st_gid, opt_numeric);
				break;

			case ACL_GROUP:
				id_p = acl_get_qualifier(ent);
				if (id_p != NULL)
					name = group_name(*id_p, opt_numeric);
				break;
		}
		name = xquote(name);
		len = strlen(name);
		if (last == NULL) {
			first = last = (struct name_list *)
				malloc(sizeof(struct name_list) + len + 1);
		} else {
			last->next = (struct name_list *)
				malloc(sizeof(struct name_list) + len + 1);
			last = last->next;
		}
		if (last == NULL) {
			free_list(first);
			return NULL;
		}
		last->next = NULL;
		strcpy(last->name, name);

		ret = acl_get_entry(acl, ACL_NEXT_ENTRY, &ent);
	}
	return first;
}

int max_name_length(struct name_list *names)
{
	int max_len = 0;
	while (names != NULL) {
		struct name_list *next = names->next;
		int len = strlen(names->name);

		if (len > max_len)
			max_len = len;
		names = next;
	}
	return max_len;
}

int names_width;

struct acl_perm_def {
	acl_tag_t	tag;
	char		c;
};

struct acl_perm_def acl_perm_defs[] = {
	{ ACL_READ,	'r' },
	{ ACL_WRITE,	'w' },
	{ ACL_EXECUTE,	'x' },
	{ 0, 0 }
};

#define ACL_PERMS (sizeof(acl_perm_defs) / sizeof(struct acl_perm_def) - 1)

void acl_perm_str(acl_entry_t entry, char *str)
{
	acl_permset_t permset;
	int n;

	acl_get_permset(entry, &permset);
	for (n = 0; n < (int) ACL_PERMS; n++) {
		str[n] = (acl_get_perm(permset, acl_perm_defs[n].tag) ?
		          acl_perm_defs[n].c : '-');
	}
	str[n] = '\0';
}

void acl_mask_perm_str(acl_t acl, char *str)
{
	acl_entry_t entry;

	str[0] = '\0';
	if (acl_get_entry(acl, ACL_FIRST_ENTRY, &entry) != 1)
		return;
	for(;;) {
		acl_tag_t tag;

		acl_get_tag_type(entry, &tag);
		if (tag == ACL_MASK) {
			acl_perm_str(entry, str);
			return;
		}
		if (acl_get_entry(acl, ACL_NEXT_ENTRY, &entry) != 1)
			return;
	}
}

void apply_mask(char *perm, const char *mask)
{
	while (*perm) {
		if (*mask == '-' && *perm >= 'a' && *perm <= 'z')
			*perm = *perm - 'a' + 'A';
		perm++;
		if (*mask)
			mask++;
	}
}

int show_line(FILE *stream, struct name_list **acl_names,  acl_t acl,
              acl_entry_t *acl_ent, const char *acl_mask,
              struct name_list **dacl_names, acl_t dacl,
	      acl_entry_t *dacl_ent, const char *dacl_mask)
{
	acl_tag_t tag_type;
	const char *tag, *name;
	char acl_perm[ACL_PERMS+1], dacl_perm[ACL_PERMS+1];

	if (acl) {
		acl_get_tag_type(*acl_ent, &tag_type);
		name = (*acl_names)->name;
	} else {
		acl_get_tag_type(*dacl_ent, &tag_type);
		name = (*dacl_names)->name;
	}

	switch(tag_type) {
		case ACL_USER_OBJ:
			tag = "USER";
			break;
		case ACL_USER:
			tag = "user";
			break;
		case ACL_GROUP_OBJ:
			tag = "GROUP";
			break;
		case ACL_GROUP:
			tag = "group";
			break;
		case ACL_MASK:
			tag = "mask";
			break;
		case ACL_OTHER:
			tag = "other";
			break;
		default:
			return -1;
	}

	memset(acl_perm, ' ', ACL_PERMS);
	acl_perm[ACL_PERMS] = '\0';
	if (acl_ent) {
		acl_perm_str(*acl_ent, acl_perm);
		if (tag_type != ACL_USER_OBJ && tag_type != ACL_OTHER &&
		    tag_type != ACL_MASK)
			apply_mask(acl_perm, acl_mask);
	}
	memset(dacl_perm, ' ', ACL_PERMS);
	dacl_perm[ACL_PERMS] = '\0';
	if (dacl_ent) {
		acl_perm_str(*dacl_ent, dacl_perm);
		if (tag_type != ACL_USER_OBJ && tag_type != ACL_OTHER &&
		    tag_type != ACL_MASK)
			apply_mask(dacl_perm, dacl_mask);
	}

	fprintf(stream, "%-5s  %*s  %*s  %*s\n",
	        tag, -names_width, name,
	        -(int)ACL_PERMS, acl_perm,
		-(int)ACL_PERMS, dacl_perm);

	if (acl_names) {
		acl_get_entry(acl, ACL_NEXT_ENTRY, acl_ent);
		(*acl_names) = (*acl_names)->next;
	}
	if (dacl_names) {
		acl_get_entry(dacl, ACL_NEXT_ENTRY, dacl_ent);
		(*dacl_names) = (*dacl_names)->next;
	}
	return 0;
}

int do_show(FILE *stream, const char *path_p, const struct stat *st,
            acl_t acl, acl_t dacl)
{
	struct name_list *acl_names = get_list(st, acl),
	                 *first_acl_name = acl_names;
	struct name_list *dacl_names = get_list(st, dacl),
	                 *first_dacl_name = dacl_names;
	
	int acl_names_width = max_name_length(acl_names);
	int dacl_names_width = max_name_length(dacl_names);
	acl_entry_t acl_ent;
	acl_entry_t dacl_ent;
	char acl_mask[ACL_PERMS+1], dacl_mask[ACL_PERMS+1];
	int ret;

	names_width = 8;
	if (acl_names_width > names_width)
		names_width = acl_names_width;
	if (dacl_names_width > names_width)
		names_width = dacl_names_width;

	acl_mask[0] = '\0';
	if (acl) {
		acl_mask_perm_str(acl, acl_mask);
		ret = acl_get_entry(acl, ACL_FIRST_ENTRY, &acl_ent);
		if (ret == 0)
			acl = NULL;
		if (ret < 0)
			return ret;
	}
	dacl_mask[0] = '\0';
	if (dacl) {
		acl_mask_perm_str(dacl, dacl_mask);
		ret = acl_get_entry(dacl, ACL_FIRST_ENTRY, &dacl_ent);
		if (ret == 0)
			dacl = NULL;
		if (ret < 0)
			return ret;
	}
	fprintf(stream, "# file: %s\n", xquote(path_p));
	while (acl_names != NULL || dacl_names != NULL) {
		acl_tag_t acl_tag, dacl_tag;

		if (acl)
			acl_get_tag_type(acl_ent, &acl_tag);
		if (dacl)
			acl_get_tag_type(dacl_ent, &dacl_tag);

		if (acl && (!dacl || acl_tag < dacl_tag)) {
			show_line(stream, &acl_names, acl, &acl_ent, acl_mask,
			          NULL, NULL, NULL, NULL);
			continue;
		} else if (dacl && (!acl || dacl_tag < acl_tag)) {
			show_line(stream, NULL, NULL, NULL, NULL,
			          &dacl_names, dacl, &dacl_ent, dacl_mask);
			continue;
		} else {
			if (acl_tag == ACL_USER || acl_tag == ACL_GROUP) {
				id_t  *acl_id_p = NULL, *dacl_id_p = NULL;
				if (acl_ent)
					acl_id_p = acl_get_qualifier(acl_ent);
				if (dacl_ent)
					dacl_id_p = acl_get_qualifier(dacl_ent);
				
				if (acl && (!dacl || *acl_id_p < *dacl_id_p)) {
					show_line(stream, &acl_names, acl,
					          &acl_ent, acl_mask,
						  NULL, NULL, NULL, NULL);
					continue;
				} else if (dacl &&
					(!acl || *dacl_id_p < *acl_id_p)) {
					show_line(stream, NULL, NULL, NULL,
					          NULL, &dacl_names, dacl,
						  &dacl_ent, dacl_mask);
					continue;
				}
			}
			show_line(stream, &acl_names,  acl,  &acl_ent, acl_mask,
				  &dacl_names, dacl, &dacl_ent, dacl_mask);
		}
	}

	free_list(first_acl_name);
	free_list(first_dacl_name);

	return 0;
}

/*
 * Create an ACL from the file permission bits
 * of the file PATH_P.
 */
static acl_t
acl_get_file_mode(const char *path_p)
{
	struct stat st;

	if (stat(path_p, &st) != 0)
		return NULL;
	return acl_from_mode(st.st_mode);
}

int do_print(const char *path_p, const struct stat *st)
{
	const char *default_prefix = NULL;
	acl_t acl = NULL, default_acl = NULL;
	int error = 0;

	if (opt_print_acl) {
		acl = acl_get_file(path_p, ACL_TYPE_ACCESS);
		if (acl == NULL && (errno == ENOSYS || errno == ENOTSUP))
			acl = acl_get_file_mode(path_p);
		if (acl == NULL)
			goto fail;
	}

	if (opt_print_default_acl && S_ISDIR(st->st_mode)) {
		default_acl = acl_get_file(path_p, ACL_TYPE_DEFAULT);
		if (default_acl == NULL) {
			if (errno != ENOSYS && errno != ENOTSUP)
				goto fail;
		} else if (acl_entries(default_acl) == 0) {
			acl_free(default_acl);
			default_acl = NULL;
		}
	}

	if (opt_skip_base &&
	    (!acl || acl_equiv_mode(acl, NULL) == 0) && !default_acl)
		return 0;

	if (opt_print_acl && opt_print_default_acl)
		default_prefix = "default:";

	if (opt_strip_leading_slash) {
		if (*path_p == '/') {
			if (!absolute_warning) {
				fprintf(stderr, _("%s: Removing leading "
					"'/' from absolute path names\n"),
				        progname);
				absolute_warning = 1;
			}
			while (*path_p == '/')
				path_p++;
		} else if (*path_p == '.' && *(path_p+1) == '/')
			while (*++path_p == '/')
				/* nothing */ ;
		if (*path_p == '\0')
			path_p = ".";
	}

	if (opt_tabular)  {
		if (do_show(stdout, path_p, st, acl, default_acl) != 0)
			goto fail;
	} else {
		if (opt_comments) {
			printf("# file: %s\n", xquote(path_p));
			printf("# owner: %s\n",
			       xquote(user_name(st->st_uid, opt_numeric)));
			printf("# group: %s\n",
			       xquote(group_name(st->st_gid, opt_numeric)));
		}
		if (acl != NULL) {
			char *acl_text = acl_to_any_text(acl, NULL, '\n',
							 print_options);
			if (!acl_text)
				goto fail;
			if (puts(acl_text) < 0) {
				acl_free(acl_text);
				goto fail;
			}
			acl_free(acl_text);
		}
		if (default_acl != NULL) {
			char *acl_text = acl_to_any_text(default_acl, 
							 default_prefix, '\n',
							 print_options);
			if (!acl_text)
				goto fail;
			if (puts(acl_text) < 0) {
				acl_free(acl_text);
				goto fail;
			}
			acl_free(acl_text);
		}
	}
	if (acl || default_acl || opt_comments)
		printf("\n");

cleanup:
	if (acl)
		acl_free(acl);
	if (default_acl)
		acl_free(default_acl);
	return error;

fail:
	fprintf(stderr, "%s: %s: %s\n", progname, xquote(path_p),
		strerror(errno));
	error = -1;
	goto cleanup;
}


void help(void)
{
	printf(_("%s %s -- get file access control lists\n"),
	       progname, VERSION);
	printf(_("Usage: %s [-%s] file ...\n"),
	         progname, cmd_line_options);
#if !POSIXLY_CORRECT
	if (posixly_correct) {
#endif
		printf(_(
"  -d, --default           display the default access control list\n"));
#if !POSIXLY_CORRECT
	} else {
		printf(_(
"      --access            display the file access control list only\n"
"  -d, --default           display the default access control list only\n"
"      --omit-header       do not display the comment header\n"
"      --all-effective     print all effective rights\n"
"      --no-effective      print no effective rights\n"
"      --skip-base         skip files that only have the base entries\n"
"  -R, --recursive         recurse into subdirectories\n"
"  -L, --logical           logical walk, follow symbolic links\n"
"  -P  --physical          physical walk, do not follow symbolic links\n"
"      --tabular           use tabular output format\n"
"      --numeric           print numeric user/group identifiers\n"
"      --absolute-names    don't strip leading '/' in pathnames\n"));
	}
#endif
	printf(_(
"      --version           print version and exit\n"
"      --help              this help text\n"));
}


static int __errors;
int __do_print(const char *file, const struct stat *stat,
               int flag, struct FTW *ftw)
{
	if (flag & FTW_DNR) {
		/* Item is a directory which can't be read. */
		fprintf(stderr, "%s: %s: %s\n",
			progname, file, strerror(errno));
		return 0;
	}

	/* Process the target of a symbolic link, and traverse the link,
           only if doing a logical walk, or if the symbolic link was
           specified on the command line. Always skip symbolic links if
           doing a physical walk. */

	if (S_ISLNK(stat->st_mode) &&
	    (opt_walk_physical || (ftw->level > 0 && !opt_walk_logical)))
		return 0;

	if (do_print(file, stat))
		__errors++;

	/* We also get here in non-recursive mode. In that case,
	   return something != 0 to abort nftw. */

	if (!opt_recursive)
		return 1;

	return 0;
}

int walk_tree(const char *file)
{
	__errors = 0;
	if (nftw(file, __do_print, 0, opt_walk_physical * FTW_PHYS) < 0) {
		fprintf(stderr, "%s: %s: %s\n", progname, xquote(file),
			strerror(errno));
		__errors++;
	}
	return __errors;
}

char *next_line(FILE *file)
{
	static char line[_POSIX_PATH_MAX], *c;
	if (!fgets(line, sizeof(line), file))
		return NULL;

	c = strrchr(line, '\0');
	while (c > line && (*(c-1) == '\n' ||
			   *(c-1) == '\r')) {
		c--;
		*c = '\0';
	}
	return line;
}


int main(int argc, char *argv[])
{
	int opt;
	char *line;

	progname = basename(argv[0]);

#if POSIXLY_CORRECT
	cmd_line_options = POSIXLY_CMD_LINE_OPTIONS;
#else
	if (getenv(POSIXLY_CORRECT_STR))
		posixly_correct = 1;
	if (!posixly_correct)
		cmd_line_options = CMD_LINE_OPTIONS;
	else
		cmd_line_options = POSIXLY_CMD_LINE_OPTIONS;
#endif

	setlocale(LC_CTYPE, "");
	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	/* Align `#effective:' comments to column 40 for tty's */
	if (!posixly_correct && isatty(fileno(stdout)))
		print_options |= TEXT_SMART_INDENT;

	while ((opt = getopt_long(argc, argv, cmd_line_options,
		                 long_options, NULL)) != -1) {
		switch (opt) {
			case 'a':  /* acl only */
				if (posixly_correct)
					goto synopsis;
				opt_print_acl = 1;
				break;

			case 'd':  /* default acl only */
				opt_print_default_acl = 1;
				break;

			case 'c':  /* no comments */
				if (posixly_correct)
					goto synopsis;
				opt_comments = 0;
				break;

			case 'e':  /* all #effective comments */
				if (posixly_correct)
					goto synopsis;
				print_options |= TEXT_ALL_EFFECTIVE;
				break;

			case 'E':  /* no #effective comments */
				if (posixly_correct)
					goto synopsis;
				print_options &= ~(TEXT_SOME_EFFECTIVE |
				                   TEXT_ALL_EFFECTIVE);
				break;

			case 'R':  /* recursive */
				if (posixly_correct)
					goto synopsis;
				opt_recursive = 1;
				break;

			case 'L':  /* follow all symlinks */
				if (posixly_correct)
					goto synopsis;
				opt_walk_logical = 1;
				opt_walk_physical = 0;
				break;

			case 'P':  /* skip all symlinks */
				if (posixly_correct)
					goto synopsis;
				opt_walk_logical = 0;
				opt_walk_physical = 1;
				break;

			case 's':  /* skip files with only base entries */
				if (posixly_correct)
					goto synopsis;
				opt_skip_base = 1;
				break;

			case 'p':
				if (posixly_correct)
					goto synopsis;
				opt_strip_leading_slash = 0;
				break;

			case 't':
				if (posixly_correct)
					goto synopsis;
				opt_tabular = 1;
				break;

			case 'n':  /* numeric */
				opt_numeric = 1;
				print_options |= TEXT_NUMERIC_IDS;
				break;

			case 'v':  /* print version */
				printf("%s " VERSION "\n", progname);
				return 0;

			case 'h':  /* help */
				help();
				return 0;

			case ':':  /* option missing */
			case '?':  /* unknown option */
			default:
				goto synopsis;
		}
	}

	if (!(opt_print_acl || opt_print_default_acl)) {
		opt_print_acl = 1;
		if (!posixly_correct)
			opt_print_default_acl = 1;
	}
		
	if ((optind == argc) && !posixly_correct)
		goto synopsis;

	do {
		if (optind == argc ||
		    strcmp(argv[optind], "-") == 0) {
			while ((line = next_line(stdin)) != NULL) {
				if (*line == '\0')
					continue;

				had_errors += walk_tree(line);
			}
			if (ferror(stdin)) {
				fprintf(stderr, _("%s: Standard input: %s\n"),
				        progname, strerror(errno));
				had_errors++;
			}
		} else
			had_errors += walk_tree(argv[optind]);
		optind++;
	} while (optind < argc);

	return had_errors ? 1 : 0;

synopsis:
	fprintf(stderr, _("Usage: %s [-%s] file ...\n"),
	        progname, cmd_line_options);
	fprintf(stderr, _("Try `%s --help' for more information.\n"),
		progname);
	return 2;
}

