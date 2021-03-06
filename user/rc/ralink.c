/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <fcntl.h>		//	for restore175C() from Ralink src
#include <flash_ioctl.h>	//	for restore175C() from Ralink src
#include <netconf.h>
#include <nvram/bcmnvram.h>
#include <unistd.h>
#include <string.h>

#include <ralink.h>
#include "iwlib.h"
#include "stapriv.h"
#include <wps.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;

#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if_arp.h>
#include <shutils.h>
#include <rc.h>
#include "ra_ioctl.h"
#include <sys/signal.h>
#include <semaphore_mfp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <flash_mtd.h>

#define MAX_FRW 64
#define WSCD_PIDFILE_5G "/var/run/wscd.pid." WIF
#define WSCD_PIDFILE_2G "/var/run/wscd.pid." WIF2G

int g_wsc_configured = 0;
int g_isEnrollee = 0;

#define IMG_SIZE        8388608

int
getIMG()
{
        unsigned char buffer[2000];
        memset(buffer, 0, sizeof(buffer));
        int fd, offs, len;

        printf("getIMG...\n");  // tmp test

        if(((fd = open("/tmp/img.trx", O_RDWR|O_CREAT)) < 0))
        {
                perror("open tmp img err");
                return -1;
        }
        offs = 0;
        while(offs < IMG_SIZE)
        {
                memset(buffer, 0, sizeof(buffer));
                if((len = IMG_SIZE - offs) > 2000)
                        len = 2000;

                if(FRead((unsigned int *)buffer, offs, len) < 0)
                {
                        fprintf(stderr, "READ IMG error: Out of scope\n");
                        break;
                }
                else
                {
                        printf("## write img from  %x to  %x\n", offs, offs+len);
                        offs+=len;
                        write(fd, buffer, len);
                }
        }
        close(fd);

        printf("\ndone.\n");
        return 0;
}

int
getMAC()
{
	unsigned char buffer[6];
	char macaddr[18];
	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));

	if (FRead((unsigned int *)buffer, OFFSET_MAC_ADDR, 6)<0)
		dbg("READ MAC address: Out of scope\n");
	else
	{
		ether_etoa(buffer, macaddr);
		puts(macaddr);
	}
	return 0;
}

int
getMAC_2G()
{
	unsigned char buffer[6];
	char macaddr[18];
	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));

	if (FRead((unsigned int *)buffer, OFFSET_MAC_ADDR_2G, 6)<0)
		dbg("READ MAC address 2G: Out of scope\n");
	else
	{
		ether_etoa(buffer, macaddr);
		puts(macaddr);
	}
	return 0;
}

int
setMAC(const char *mac)
{
	char ea[ETHER_ADDR_LEN];
	if (ether_atoe(mac, ea))
	{
		FWrite(ea, OFFSET_MAC_ADDR, 6);
		FWrite(ea, OFFSET_MAC_GMAC0, 6);
		getMAC();
	}
	return 0;
}

int
setMAC_2G(const char *mac)
{
	char ea[ETHER_ADDR_LEN];
	if (ether_atoe(mac, ea))
	{
		FWrite(ea, OFFSET_MAC_ADDR_2G, 6);
		FWrite(ea, OFFSET_MAC_GMAC2, 6);
		getMAC_2G();
	}
	return 0;
}

int
getCountryCode()
{
	unsigned char CC[3];
	memset(CC, 0, sizeof(CC));
	FRead(CC, OFFSET_COUNTRY_CODE, 2);
	if (CC[0] == 0xff && CC[1] == 0xff)	// 0xffff is default
		;
	else
		puts(CC);
	return 0;
}

int
setCountryCode(const char *cc)
{
	char CC[3];

	if (strlen(cc)!=2)
		return 0;
	/* Please refer to ISO3166 code list for other countries and can be found at
	 * http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html#sz
	 */
	else if (!strcasecmp(cc, "DB")) ;
	else if (!strcasecmp(cc, "AL")) ;
	else if (!strcasecmp(cc, "DZ")) ;
	else if (!strcasecmp(cc, "AR")) ;
	else if (!strcasecmp(cc, "AM")) ;
	else if (!strcasecmp(cc, "AU")) ;
	else if (!strcasecmp(cc, "AT")) ;
	else if (!strcasecmp(cc, "AZ")) ;
	else if (!strcasecmp(cc, "BH")) ;
	else if (!strcasecmp(cc, "BY")) ;
	else if (!strcasecmp(cc, "BE")) ;
	else if (!strcasecmp(cc, "BZ")) ;
	else if (!strcasecmp(cc, "BO")) ;
	else if (!strcasecmp(cc, "BR")) ;
	else if (!strcasecmp(cc, "BN")) ;
	else if (!strcasecmp(cc, "BG")) ;
	else if (!strcasecmp(cc, "CA")) ;
	else if (!strcasecmp(cc, "CL")) ;
	else if (!strcasecmp(cc, "CN")) ;
	else if (!strcasecmp(cc, "CO")) ;
	else if (!strcasecmp(cc, "CR")) ;
	else if (!strcasecmp(cc, "HR")) ;
	else if (!strcasecmp(cc, "CY")) ;
	else if (!strcasecmp(cc, "CZ")) ;
	else if (!strcasecmp(cc, "DK")) ;
	else if (!strcasecmp(cc, "DO")) ;
	else if (!strcasecmp(cc, "EC")) ;
	else if (!strcasecmp(cc, "EG")) ;
	else if (!strcasecmp(cc, "SV")) ;
	else if (!strcasecmp(cc, "EE")) ;
	else if (!strcasecmp(cc, "FI")) ;
	else if (!strcasecmp(cc, "FR")) ;
	else if (!strcasecmp(cc, "GE")) ;
	else if (!strcasecmp(cc, "DE")) ;
	else if (!strcasecmp(cc, "GR")) ;
	else if (!strcasecmp(cc, "GT")) ;
	else if (!strcasecmp(cc, "HN")) ;
	else if (!strcasecmp(cc, "HK")) ;
	else if (!strcasecmp(cc, "HU")) ;
	else if (!strcasecmp(cc, "IS")) ;
	else if (!strcasecmp(cc, "IN")) ;
	else if (!strcasecmp(cc, "ID")) ;
	else if (!strcasecmp(cc, "IR")) ;
	else if (!strcasecmp(cc, "IE")) ;
	else if (!strcasecmp(cc, "IL")) ;
	else if (!strcasecmp(cc, "IT")) ;
	else if (!strcasecmp(cc, "JP")) ;
	else if (!strcasecmp(cc, "JO")) ;
	else if (!strcasecmp(cc, "KZ")) ;
	else if (!strcasecmp(cc, "KP")) ;
	else if (!strcasecmp(cc, "KR")) ;
	else if (!strcasecmp(cc, "KW")) ;
	else if (!strcasecmp(cc, "LV")) ;
	else if (!strcasecmp(cc, "LB")) ;
	else if (!strcasecmp(cc, "LI")) ;
	else if (!strcasecmp(cc, "LT")) ;
	else if (!strcasecmp(cc, "LU")) ;
	else if (!strcasecmp(cc, "MO")) ;
	else if (!strcasecmp(cc, "MK")) ;
	else if (!strcasecmp(cc, "MY")) ;
	else if (!strcasecmp(cc, "MX")) ;
	else if (!strcasecmp(cc, "MC")) ;
	else if (!strcasecmp(cc, "MA")) ;
	else if (!strcasecmp(cc, "NL")) ;
	else if (!strcasecmp(cc, "NZ")) ;
	else if (!strcasecmp(cc, "NO")) ;
	else if (!strcasecmp(cc, "OM")) ;
	else if (!strcasecmp(cc, "PK")) ;
	else if (!strcasecmp(cc, "PA")) ;
	else if (!strcasecmp(cc, "PE")) ;
	else if (!strcasecmp(cc, "PH")) ;
	else if (!strcasecmp(cc, "PL")) ;
	else if (!strcasecmp(cc, "PT")) ;
	else if (!strcasecmp(cc, "PR")) ;
	else if (!strcasecmp(cc, "QA")) ;
	else if (!strcasecmp(cc, "RO")) ;
	else if (!strcasecmp(cc, "RU")) ;
	else if (!strcasecmp(cc, "SA")) ;
	else if (!strcasecmp(cc, "SG")) ;
	else if (!strcasecmp(cc, "SK")) ;
	else if (!strcasecmp(cc, "SI")) ;
	else if (!strcasecmp(cc, "ZA")) ;
	else if (!strcasecmp(cc, "ES")) ;
	else if (!strcasecmp(cc, "SE")) ;
	else if (!strcasecmp(cc, "CH")) ;
	else if (!strcasecmp(cc, "SY")) ;
	else if (!strcasecmp(cc, "TW")) ;
	else if (!strcasecmp(cc, "TH")) ;
	else if (!strcasecmp(cc, "TT")) ;
	else if (!strcasecmp(cc, "TN")) ;
	else if (!strcasecmp(cc, "TR")) ;
	else if (!strcasecmp(cc, "UA")) ;
	else if (!strcasecmp(cc, "AE")) ;
	else if (!strcasecmp(cc, "GB")) ;
	else if (!strcasecmp(cc, "US")) ;
	else if (!strcasecmp(cc, "UY")) ;
	else if (!strcasecmp(cc, "UZ")) ;
	else if (!strcasecmp(cc, "VE")) ;
	else if (!strcasecmp(cc, "VN")) ;
	else if (!strcasecmp(cc, "YE")) ;
	else if (!strcasecmp(cc, "ZW")) ;
	else
	{
		return 0;
	}

	memset(&CC[0], toupper(cc[0]), 1);
	memset(&CC[1], toupper(cc[1]), 1);
	memset(&CC[2], 0, 1);
	FWrite(CC, OFFSET_COUNTRY_CODE, 2);
	puts(CC);
	return 0;
}

int
atoh(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, MAX_FRW);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == MAX_FRW)
			break;
	}
	return i;
}

int
htoa(const unsigned char *e, char *a, int len)
{
	char *c = a;
	int i;

	for (i = 0; i < len; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

int
FREAD(unsigned int addr_sa, int len)
{
	unsigned char buffer[MAX_FRW];
	char buffer_h[128];
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_h, 0, sizeof(buffer_h));

	if (FRead((unsigned int *)buffer, addr_sa, len)<0)
		dbg("FREAD: Out of scope\n");
	else
	{
		if (len > MAX_FRW)
			len = MAX_FRW;
		htoa(buffer, buffer_h, len);
		puts(buffer_h);
	}
	return 0;
}

/*
 * 	write str_hex to offset da
 *	console input:	FWRITE 0x45000 00:11:22:33:44:55:66:77
 *	console output:	00:11:22:33:44:55:66:77
 *
 */
int
FWRITE(char *da, char* str_hex)
{
	unsigned char ee[MAX_FRW];
	unsigned int addr_da;
	int len;

	addr_da = strtoul(da, NULL, 16);
	if (addr_da && (len = atoh(str_hex, ee)))
	{
		FWrite(ee, addr_da, len);
		FREAD(addr_da, len);
	}
	return 0;
}

int
pincheck(const char *a)
{
	unsigned char *c = (char *) a;
	int i = 0;

	for (;;) {
		if (*c>0x39 || *c<0x30)
			break;
		else
			i++;
		if (!*c++ || i == 8)
			break;
	}
	return (i == 8);
}

int
pinvalidate(char *pin_string)
{
	unsigned long PIN = strtoul(pin_string, NULL, 10);
	unsigned long int accum = 0;
	unsigned int len = strlen(pin_string);

	if (len != 4 && len != 8)
		return  -1;

	if (len == 8) {
		accum += 3 * ((PIN / 10000000) % 10);
		accum += 1 * ((PIN / 1000000) % 10);
		accum += 3 * ((PIN / 100000) % 10);
		accum += 1 * ((PIN / 10000) % 10);
		accum += 3 * ((PIN / 1000) % 10);
		accum += 1 * ((PIN / 100) % 10);
		accum += 3 * ((PIN / 10) % 10);
		accum += 1 * ((PIN / 1) % 10);

		if (0 == (accum % 10))
			return 0;
	}
	else if (len == 4)
		return 0;

	return -1;
}

int
setPIN(const char *pin)
{
	if (pincheck(pin) && !pinvalidate(pin))
	{
		FWrite(pin, OFFSET_PIN_CODE, 8);
		char PIN[9];
		memset(PIN, 0, 9);
		memcpy(PIN, pin, 8);
		puts(PIN);
	}
	return 0;	
}

getBootVer()
{
	unsigned char btv[5];
	memset(btv, 0, sizeof(btv));
	FRead(btv, OFFSET_BOOT_VER, 4);
	puts(btv);

	return 0;
}

getPIN()
{
	unsigned char PIN[9];
	memset(PIN, 0, sizeof(PIN));
	FRead(PIN, OFFSET_PIN_CODE, 8);
	if (PIN[0]!=0xff)
		puts(PIN);
	return 0;
}

int gen_ralink_config()
{
	FILE *fp;
	char *str = NULL;
	char *str2 = NULL;
	int  i;
//	int ssid_num = 1;
	char wmm_enable[8];
	char wmm_noack[8];
	char macbuf[36];
	char list[2048];
	int flag_8021x = 0;
	int wsc_configure = 0;
	int warning = 0;
	int num;

	printf("gen ralink config\n");
	system("mkdir -p /etc/Wireless/RT2860");
	if (!(fp=fopen("/etc/Wireless/RT2860/RT2860.dat", "w+")))
		return 0;

	fprintf(fp, "#The word of \"Default\" must not be removed\n");
	fprintf(fp, "Default\n");

	//CountryRegion
	str = nvram_safe_get("wl_country_code");
	if (str)
	{
		if (    (strcasecmp(str, "CA") == 0) || (strcasecmp(str, "CO") == 0) ||
			(strcasecmp(str, "DO") == 0) || (strcasecmp(str, "GT") == 0) ||
			(strcasecmp(str, "MX") == 0) || (strcasecmp(str, "NO") == 0) ||
			(strcasecmp(str, "PA") == 0) || (strcasecmp(str, "PR") == 0) ||
			(strcasecmp(str, "TW") == 0) || (strcasecmp(str, "US") == 0) ||
			(strcasecmp(str, "UZ") == 0))
			fprintf(fp, "CountryRegion=%d\n", 0);   // channel 1-11
		else if (strcasecmp(str, "DB") == 0  || strcasecmp(str, "") == 0)
			fprintf(fp, "CountryRegion=%d\n", 5);   // channel 1-14
		else
			fprintf(fp, "CountryRegion=%d\n", 1);   // channel 1-13
	}
	else
	{
		warning = 1;
		fprintf(fp, "CountryRegion=%d\n", 5);
	}

	//CountryRegion for A band
	str = nvram_safe_get("wl_country_code");
	if (str)
	{
		if (		(!strcasecmp(str, "AL")) ||
				(!strcasecmp(str, "DZ")) ||
				(!strcasecmp(str, "AU")) ||
				(!strcasecmp(str, "BH")) ||
				(!strcasecmp(str, "BY")) ||
				(!strcasecmp(str, "CA")) ||
				(!strcasecmp(str, "CL")) ||
				(!strcasecmp(str, "CO")) ||
				(!strcasecmp(str, "CR")) ||
				(!strcasecmp(str, "DO")) ||
				(!strcasecmp(str, "EC")) ||
				(!strcasecmp(str, "SV")) ||
				(!strcasecmp(str, "GT")) ||
				(!strcasecmp(str, "HN")) ||
				(!strcasecmp(str, "HK")) ||
				(!strcasecmp(str, "IN")) ||
				(!strcasecmp(str, "IL")) ||
				(!strcasecmp(str, "JO")) ||
				(!strcasecmp(str, "KZ")) ||
				(!strcasecmp(str, "KW")) ||
				(!strcasecmp(str, "LB")) ||
				(!strcasecmp(str, "MO")) ||
				(!strcasecmp(str, "MK")) ||
				(!strcasecmp(str, "MY")) ||
				(!strcasecmp(str, "MX")) ||
				(!strcasecmp(str, "MA")) ||
				(!strcasecmp(str, "NZ")) ||
				(!strcasecmp(str, "NO")) ||
				(!strcasecmp(str, "OM")) ||
				(!strcasecmp(str, "PK")) ||
				(!strcasecmp(str, "PA")) ||
				(!strcasecmp(str, "PR")) ||
				(!strcasecmp(str, "QA")) ||
				(!strcasecmp(str, "RO")) ||
				(!strcasecmp(str, "RU")) ||
				(!strcasecmp(str, "SA")) ||
				(!strcasecmp(str, "SG")) ||
				(!strcasecmp(str, "SY")) ||
				(!strcasecmp(str, "TH")) ||
				(!strcasecmp(str, "UA")) ||
				(!strcasecmp(str, "AE")) ||
				(!strcasecmp(str, "US")) ||
				(!strcasecmp(str, "VN")) ||
				(!strcasecmp(str, "YE")) ||
				(!strcasecmp(str, "ZW"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 0);
		}
		else if (	(!strcasecmp(str, "AT")) ||
				(!strcasecmp(str, "BE")) ||
				(!strcasecmp(str, "BR")) ||
				(!strcasecmp(str, "BG")) ||
				(!strcasecmp(str, "CY")) ||
				(!strcasecmp(str, "DK")) ||
				(!strcasecmp(str, "EE")) ||
				(!strcasecmp(str, "FI")) ||
				(!strcasecmp(str, "DE")) ||
				(!strcasecmp(str, "GR")) ||
				(!strcasecmp(str, "HU")) ||
				(!strcasecmp(str, "IS")) ||
				(!strcasecmp(str, "IE")) ||
				(!strcasecmp(str, "IT")) ||
				(!strcasecmp(str, "LV")) ||
				(!strcasecmp(str, "LI")) ||
				(!strcasecmp(str, "LT")) ||
				(!strcasecmp(str, "LU")) ||
				(!strcasecmp(str, "NL")) ||
				(!strcasecmp(str, "PL")) ||
				(!strcasecmp(str, "PT")) ||
				(!strcasecmp(str, "SK")) ||
				(!strcasecmp(str, "SI")) ||
				(!strcasecmp(str, "ZA")) ||
				(!strcasecmp(str, "ES")) ||
				(!strcasecmp(str, "SE")) ||
				(!strcasecmp(str, "CH")) ||
				(!strcasecmp(str, "GB")) ||
				(!strcasecmp(str, "UZ"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 1);
		}
		else if (	(!strcasecmp(str, "AM")) ||
				(!strcasecmp(str, "AZ")) ||
				(!strcasecmp(str, "HR")) ||
				(!strcasecmp(str, "CZ")) ||
				(!strcasecmp(str, "EG")) ||
				(!strcasecmp(str, "FR")) ||
				(!strcasecmp(str, "GE")) ||
				(!strcasecmp(str, "MC")) ||
				(!strcasecmp(str, "TT")) ||
				(!strcasecmp(str, "TN")) ||
				(!strcasecmp(str, "TR"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 2);
		}
		else if (	(!strcasecmp(str, "AR")) ||
				(!strcasecmp(str, "TW"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 3);
		}
		else if (	(!strcasecmp(str, "BZ")) ||
				(!strcasecmp(str, "BO")) ||
				(!strcasecmp(str, "BN")) ||
				(!strcasecmp(str, "CN")) ||
				(!strcasecmp(str, "ID")) ||
				(!strcasecmp(str, "IR")) ||
				(!strcasecmp(str, "PE")) ||
				(!strcasecmp(str, "PH"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 4);
		}
		else if (	(!strcasecmp(str, "KP")) ||
				(!strcasecmp(str, "KR")) ||
				(!strcasecmp(str, "UY")) ||
				(!strcasecmp(str, "VE"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 5);
		}
		else if (!strcasecmp(str, "DB"))
		{
			fprintf(fp, "CountryRegionABand=%d\n", 7);
		}
		else if (!strcasecmp(str, "JP"))
		{
			fprintf(fp, "CountryRegionABand=%d\n", 9);
		}
		else
		{
			warning = 2;
			fprintf(fp, "CountryRegionABand=%d\n", 7);
		}
	}
	else
	{
		warning = 2;
		fprintf(fp, "CountryRegionABand=%d\n", 7);
	}

	//CountryCode
	str = nvram_safe_get("wl_country_code");
	if (str)
	{
		if (strcmp(str, "") == 0)
			fprintf(fp, "CountryCode=DB\n");
		else
			fprintf(fp, "CountryCode=%s\n", str);
	}
	else
	{
		warning = 3;
		fprintf(fp, "CountryCode=DB\n");
	}

	//SSID Num. [MSSID Only]
	fprintf(fp, "BssidNum=%d\n", 1);

	//SSID
	str = nvram_safe_get("wl_ssid");
	if (str)
		fprintf(fp, "SSID1=%s\n", str);
	else
	{
		warning = 4;
		fprintf(fp, "SSID1=%s\n", "default");
	}

	fprintf(fp, "SSID2=\n");
	fprintf(fp, "SSID3=\n");
	fprintf(fp, "SSID4=\n");
	fprintf(fp, "SSID5=\n");
	fprintf(fp, "SSID6=\n");
	fprintf(fp, "SSID7=\n");
	fprintf(fp, "SSID8=\n");

	//Network Mode
	str = nvram_safe_get("wl_gmode");
	if (str)
	{
		if (atoi(str) == 2)       // A,N
			fprintf(fp, "WirelessMode=%d\n", 8);
		else if (atoi(str) == 1)  // N
			fprintf(fp, "WirelessMode=%d\n", 11);
		else if (atoi(str) == 0)  // A
			fprintf(fp, "WirelessMode=%d\n", 2);
		else			// A,N
			fprintf(fp, "WirelessMode=%d\n", 8);
	}
	else
	{
		warning = 5;		// A,N
		fprintf(fp, "WirelessMode=%d\n", 8);
	}

	fprintf(fp, "FixedTxMode=\n");

	fprintf(fp, "TxRate=%d\n", 0);

	//Channel
//	if (!nvram_match("sw_mode_ex", "2") && !nvram_match("wl_channel", "0"))
	{
		str = nvram_safe_get("wl_channel");

		if (str)
			fprintf(fp, "Channel=%d\n", atoi(str));
		else
		{
			warning = 6;
			fprintf(fp, "Channel=%d\n", 0);
		}
	}
/*
 * not supported in 5G mode
 *
	//BasicRate
	str = nvram_safe_get("wl_rateset");
	if (str)
	{
		if (!strcmp(str, "default"))	// 1, 2, 5.5, 11
			fprintf(fp, "BasicRate=%d\n", 15);
		else if (!strcmp(str, "all"))	// 1, 2, 5.5, 6, 11, 12, 24
			fprintf(fp, "BasicRate=%d\n", 351);
		else if (!strcmp(str, "12"))	// 1, 2
			fprintf(fp, "BasicRate=%d\n", 3);
		else
			fprintf(fp, "BasicRate=%d\n", 15);
	}
	else
	{
		warning = 7;
		fprintf(fp, "BasicRate=%d\n", 15);
	}
*/
	//BeaconPeriod
	str = nvram_safe_get("wl_bcn");
	if (str)
	{
		if (atoi(str) > 1000 || atoi(str) < 20)
		{
			nvram_set("wl_bcn", "100");
			fprintf(fp, "BeaconPeriod=%d\n", 100);
		}
		else
			fprintf(fp, "BeaconPeriod=%d\n", atoi(str));
	}
	else
	{
		warning = 8;
		fprintf(fp, "BeaconPeriod=%d\n", 100);
	}

	//DTIM Period
	str = nvram_safe_get("wl_dtim");
	if (str)
		fprintf(fp, "DtimPeriod=%d\n", atoi(str));
	else
	{
		warning = 9;
		fprintf(fp, "DtimPeriod=%d\n", 1);
	}

	//TxPower
	str = nvram_safe_get("TxPower");
	if (nvram_match("wl_radio_x", "0"))
		fprintf(fp, "TxPower=%d\n", 0);
	else if (str)
		fprintf(fp, "TxPower=%d\n", atoi(str));
	else
	{
		warning = 10;
		fprintf(fp, "TxPower=%d\n", 100);
	}

	//DisableOLBC
	fprintf(fp, "DisableOLBC=%d\n", 0);

	//BGProtection
	str = nvram_safe_get("wl_gmode_protection");
	if (str)
	{
		if (!strcmp(str, "auto"))
			fprintf(fp, "BGProtection=%d\n", 0);
		else
			fprintf(fp, "BGProtection=%d\n", 2);
	}
	else
	{
		warning = 11;
		fprintf(fp, "BGProtection=%d\n", 0);
	}

	//TxAntenna
	fprintf(fp, "TxAntenna=\n");

	//RxAntenna
	fprintf(fp, "RxAntenna=\n");

	//TxPreamble
	fprintf(fp, "TxPreamble=%d\n", 0);

	//RTSThreshold  Default=2347
	str = nvram_safe_get("wl_rts");
	if (str)
		fprintf(fp, "RTSThreshold=%d\n", atoi(str));
	else
	{
		warning = 12;
		fprintf(fp, "RTSThreshold=%d\n", 2347);
	}

	//FragThreshold  Default=2346
	str = nvram_safe_get("wl_frag");
	if (str)
		fprintf(fp, "FragThreshold=%d\n", atoi(str));
	else
	{
		warning = 13;
		fprintf(fp, "FragThreshold=%d\n", 2346);
	}

	//TxBurst
	str = nvram_safe_get("TxBurst");
	if (str)
		fprintf(fp, "TxBurst=%d\n", atoi(str));
	else
	{
		warning = 14;
		fprintf(fp, "TxBurst=%d\n", 1);
	}

	//PktAggregate
	str = nvram_safe_get("PktAggregate");
	if (str)
		fprintf(fp, "PktAggregate=%d\n", atoi(str));
	else
	{
		warning = 15;
		fprintf(fp, "PktAggregate=%d\n", 1);
	}

	fprintf(fp, "FreqDelta=%d\n", 0);
	fprintf(fp, "TurboRate=%d\n", 0);

	//WmmCapable
	bzero(wmm_enable, sizeof(char)*8);
//	for (i = 0; i < ssid_num; i++)
	{
		str = nvram_safe_get("wl_gmode");
		if (str && atoi(str) == 3)	// always enable WMM in N only mode
			sprintf(wmm_enable+strlen(wmm_enable), "%d", 1);
		else
			sprintf(wmm_enable+strlen(wmm_enable), "%d", atoi(nvram_safe_get("wl_wme")));
//		sprintf(wmm_enable+strlen(wmm_enable), "%c", ';');
	}
//	wmm_enable[strlen(wmm_enable) - 1] = '\0';
	wmm_enable[1] = '\0';
	fprintf(fp, "WmmCapable=%s\n", wmm_enable);

	fprintf(fp, "APAifsn=3;7;1;1\n");
	fprintf(fp, "APCwmin=4;4;3;2\n");
	fprintf(fp, "APCwmax=6;10;4;3\n");
	fprintf(fp, "APTxop=0;0;94;47\n");
	fprintf(fp, "APACM=0;0;0;0\n");
	fprintf(fp, "BSSAifsn=3;7;2;2\n");
	fprintf(fp, "BSSCwmin=4;4;3;2\n");
	fprintf(fp, "BSSCwmax=10;10;4;3\n");
	fprintf(fp, "BSSTxop=0;0;94;47\n");
	fprintf(fp, "BSSACM=0;0;0;0\n");

	//AckPolicy
	bzero(wmm_noack, sizeof(char)*8);
	for (i = 0; i < 4; i++)
	{
		sprintf(wmm_noack+strlen(wmm_noack), "%d", strcmp(nvram_safe_get("wl_wme_no_ack"), "on")? 0 : 1);
		sprintf(wmm_noack+strlen(wmm_noack), "%c", ';');
	}
	wmm_noack[strlen(wmm_noack) - 1] = '\0';
	fprintf(fp, "AckPolicy=%s\n", wmm_noack);

	//APSDCapable
	str = nvram_safe_get("APSDCapable");
	if (str)
		fprintf(fp, "APSDCapable=%d\n", atoi(str));
	else
	{
		warning = 18;
		fprintf(fp, "APSDCapable=%d\n", 1);
	}

	//DLSDCapable
	str = nvram_safe_get("DLSCapable");
	if (str)
		fprintf(fp, "DLSCapable=%d\n", atoi(str));
	else
	{
		warning = 19;
		fprintf(fp, "DLSCapable=%d\n", 0);
	}

	//NoForwarding pre SSID & NoForwardingBTNBSSID
	str = nvram_safe_get("wl_ap_isolate");
	if (str)
	{
		fprintf(fp, "NoForwarding=%d\n", atoi(str));
		fprintf(fp, "NoForwardingBTNBSSID=%d\n", atoi(str));
	}
	else
	{
		warning = 20;
		fprintf(fp, "NoForwarding=%d\n", 0);
		fprintf(fp, "NoForwardingBTNBSSID=%d\n", 0);
	}

	//HideSSID
	fprintf(fp, "HideSSID=%s\n", nvram_safe_get("wl_closed"));

	//ShortSlot
	fprintf(fp, "ShortSlot=%d\n", 1);


	//AutoChannelSelect
	{
		str = nvram_safe_get("wl_channel");
/*
		if (nvram_match("sw_mode_ex", "2"))
			fprintf(fp, "AutoChannelSelect=%d\n", 1);
		else */if (str)
		{
			if (atoi(str) == 0)
				fprintf(fp, "AutoChannelSelect=%d\n", 2);
			else
				fprintf(fp, "AutoChannelSelect=%d\n", 0);
		}
		else
		{
			warning = 21;
			fprintf(fp, "AutoChannelSelect=%d\n", 2);
		}
	}

	//IEEE8021X
	str = nvram_safe_get("wl_auth_mode");
	if (str && !strcmp(str, "radius"))
		fprintf(fp, "IEEE8021X=%d\n", 1);
	else
		fprintf(fp, "IEEE8021X=%d\n", 0);

	if (str)
	{
		if (	!strcmp(str, "radius") ||
			!strcmp(str, "wpa") ||
			!strcmp(str, "wpa2")	)
			flag_8021x = 1;
	}

	fprintf(fp, "IEEE80211H=%s\n", nvram_safe_get("IEEE80211H"));
	fprintf(fp, "CarrierDetect=%s\n", nvram_safe_get("CarrierDetect"));
	fprintf(fp, "ChannelGeography=%s\n", nvram_safe_get("ChannelGeography"));
//	fprintf(fp, "ITxBfEn=%d\n", 0);
	fprintf(fp, "PreAntSwitch=\n");
	fprintf(fp, "PhyRateLimit=%d\n", 0);
	fprintf(fp, "DebugFlags=%d\n", 0);
//	fprintf(fp, "ETxBfEnCond=%d\n", 0);
//	fprintf(fp, "ITxBfTimeout=%d\n", 0);
	fprintf(fp, "FineAGC=%d\n", 0);
	fprintf(fp, "StreamMode=%d\n", 0);
	fprintf(fp, "StreamModeMac0=\n");
	fprintf(fp, "StreamModeMac1=\n");
	fprintf(fp, "StreamModeMac2=\n");
	fprintf(fp, "StreamModeMac3=\n");
	fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
	fprintf(fp, "RDRegion=%s\n", nvram_safe_get("RDRegion"));
	fprintf(fp, "StationKeepAlive=%d\n", 0);
	fprintf(fp, "DfsLowerLimit=%d\n", 0);
	fprintf(fp, "DfsUpperLimit=%d\n", 0);
	fprintf(fp, "DfsIndoor=%d\n", 0);
	fprintf(fp, "DFSParamFromConfig=%d\n", 0);
	fprintf(fp, "FCCParamCh0=\n");
	fprintf(fp, "FCCParamCh1=\n");
	fprintf(fp, "FCCParamCh2=\n");
	fprintf(fp, "FCCParamCh3=\n");
	fprintf(fp, "CEParamCh0=\n");
	fprintf(fp, "CEParamCh1=\n");
	fprintf(fp, "CEParamCh2=\n");
	fprintf(fp, "CEParamCh3=\n");
	fprintf(fp, "JAPParamCh0=\n");
	fprintf(fp, "JAPParamCh1=\n");
	fprintf(fp, "JAPParamCh2=\n");
	fprintf(fp, "JAPParamCh3=\n");
	fprintf(fp, "JAPW53ParamCh0=\n");
	fprintf(fp, "JAPW53ParamCh1=\n");
	fprintf(fp, "JAPW53ParamCh2=\n");
	fprintf(fp, "JAPW53ParamCh3=\n");
	fprintf(fp, "FixDfsLimit=%d\n", 0);
	fprintf(fp, "LongPulseRadarTh=%d\n", 0);
	fprintf(fp, "AvgRssiReq=%d\n", 0);
	fprintf(fp, "DFS_R66=%d\n", 0);
	fprintf(fp, "BlockCh=\n");

	//GreenAP
	str = nvram_safe_get("GreenAP");
/*
	if (nvram_match("sw_mode_ex", "2"))
		fprintf(fp, "GreenAP=%d\n", 0);
	else */if (str)
		fprintf(fp, "GreenAP=%d\n", atoi(str));
	else
	{
		warning = 23;
		fprintf(fp, "GreenAP=%d\n", 0);
	}

	//PreAuth
	fprintf(fp, "PreAuth=0\n");

	//AuthMode
	str = nvram_safe_get("wl_auth_mode");
	if (str)
	{
		if (!strcmp(str, "open"))
		{
			fprintf(fp, "AuthMode=%s\n", "OPEN");
		}
		else if (!strcmp(str, "shared"))
		{
			fprintf(fp, "AuthMode=%s\n", "SHARED");
		}
		else if (!strcmp(str, "psk"))
		{
			if (nvram_match("wl_wpa_mode", "0"))
			{
/*
				if (nvram_match("wl_crypto", "tkip"))
					fprintf(fp, "AuthMode=%s\n", "WPAPSK");
				else if (nvram_match("wl_crypto", "aes"))
					fprintf(fp, "AuthMode=%s\n", "WPA2PSK");
				else
*/
				fprintf(fp, "AuthMode=%s\n", "WPAPSKWPA2PSK");
			}
			else if (nvram_match("wl_wpa_mode", "1"))
				fprintf(fp, "AuthMode=%s\n", "WPAPSK");
			else if (nvram_match("wl_wpa_mode", "2"))
				fprintf(fp, "AuthMode=%s\n", "WPA2PSK");
		}
		else if (!strcmp(str, "wpa"))
		{
//			if (nvram_match("wl_wpa_mode", "0")) //2008.11 magic
			if (nvram_match("wl_wpa_mode", "4"))
			{
/*
				if (nvram_match("wl_crypto", "tkip"))
					fprintf(fp, "AuthMode=%s\n", "WPA");
				else if (nvram_match("wl_crypto", "aes"))
					fprintf(fp, "AuthMode=%s\n", "WPA2");
				else
*/
					fprintf(fp, "AuthMode=%s\n", "WPA1WPA2");
			}
		//2008.11 magic for new UI{
//			else if (nvram_match("wl_wpa_mode", "1"))
//				fprintf(fp, "AuthMode=%s\n", "WPA");
			else if (nvram_match("wl_wpa_mode", "3"))
				fprintf(fp, "AuthMode=%s\n", "WPA");
//			else if (nvram_match("wl_wpa_mode", "2"))
//				fprintf(fp, "AuthMode=%s\n", "WPA2");
			//2008.11 magic for new UI}
		}
		//2008.11 magic for new UI{
		else if (!strcmp(str, "wpa2"))
		{
			fprintf(fp, "AuthMode=%s\n", "WPA2");	
		}
		//2008.11 magic for new UI}
		else if (!strcmp(str, "radius"))
		{
			fprintf(fp, "AuthMode=%s\n", "OPEN");
		}
		else
		{
			fprintf(fp, "AuthMode=%s\n", "OPEN");
		}
	}
	else
	{
		warning = 24;
		fprintf(fp, "AuthMode=%s\n", "OPEN");
	}

	//EncrypType
	if (	(nvram_match("wl_auth_mode", "open") && nvram_match("wl_wep_x", "0")) /*||
		(nvram_match("wl_auth_mode", "radius") && nvram_match("wl_wep_x", "0"))*/
	)
		fprintf(fp, "EncrypType=%s\n", "NONE");
	else if (       (nvram_match("wl_auth_mode", "open") && !nvram_match("wl_wep_x", "0")) ||
			nvram_match("wl_auth_mode", "shared") ||
			nvram_match("wl_auth_mode", "radius")/* ||
			(nvram_match("wl_auth_mode", "radius") && !nvram_match("wl_wep_x", "0"))*/
	)
		fprintf(fp, "EncrypType=%s\n", "WEP");
	else if (nvram_match("wl_crypto", "tkip"))
		fprintf(fp, "EncrypType=%s\n", "TKIP");
	else if (nvram_match("wl_crypto", "aes"))
		fprintf(fp, "EncrypType=%s\n", "AES");
	else if (nvram_match("wl_crypto", "tkip+aes"))
		fprintf(fp, "EncrypType=%s\n", "TKIPAES");
	else
		fprintf(fp, "EncrypType=%s\n", "NONE");

	fprintf(fp, "WapiPsk1=\n");
	fprintf(fp, "WapiPsk2=\n");
	fprintf(fp, "WapiPsk3=\n");
	fprintf(fp, "WapiPsk4=\n");
	fprintf(fp, "WapiPsk5=\n");
	fprintf(fp, "WapiPsk6=\n");
	fprintf(fp, "WapiPsk7=\n");
	fprintf(fp, "WapiPsk8=\n");
	fprintf(fp, "WapiPskType=\n");
	fprintf(fp, "Wapiifname=\n");
	fprintf(fp, "WapiAsCertPath=\n");
	fprintf(fp, "WapiUserCertPath=\n");
	fprintf(fp, "WapiAsIpAddr=\n");
	fprintf(fp, "WapiAsPort=\n");

	//RekeyInterval
	str = nvram_safe_get("wl_wpa_gtk_rekey");
	if (str)
	{
		if (atol(str) == 0)
			fprintf(fp, "RekeyMethod=%s\n", "DISABLE");
		else
			fprintf(fp, "RekeyMethod=TIME\n");

		fprintf(fp, "RekeyInterval=%d\n", atol(str));
	}
	else
	{
		warning = 25;
		fprintf(fp, "RekeyMethod=%s\n", "DISABLE");
		fprintf(fp, "RekeyInterval=%d\n", 0);
	}

	//PMKCachePeriod
	fprintf(fp, "PMKCachePeriod=%d\n", 10);

	fprintf(fp, "MeshAutoLink=%d\n", 0);
	fprintf(fp, "MeshAuthMode=\n");
	fprintf(fp, "MeshEncrypType=\n");
	fprintf(fp, "MeshDefaultkey=%d\n", 0);
	fprintf(fp, "MeshWEPKEY=\n");
	fprintf(fp, "MeshWPAKEY=\n");
	fprintf(fp, "MeshId=\n");

	//WPAPSK
	fprintf(fp, "WPAPSK1=%s\n", nvram_safe_get("wl_wpa_psk"));

	fprintf(fp, "WPAPSK2=\n");
	fprintf(fp, "WPAPSK3=\n");
	fprintf(fp, "WPAPSK4=\n");
	fprintf(fp, "WPAPSK5=\n");
	fprintf(fp, "WPAPSK6=\n");
	fprintf(fp, "WPAPSK7=\n");
	fprintf(fp, "WPAPSK8=\n");

	//DefaultKeyID
	fprintf(fp, "DefaultKeyID=%s\n", nvram_safe_get("wl_key"));

	sprintf(list, "wl_key%s", nvram_safe_get("wl_key"));
	if ((strlen(nvram_safe_get(list)) == 5) || (strlen(nvram_safe_get(list)) == 13))
	{
		nvram_set("wl_key_type", "1");
		warning = 261;
	}
	else if ((strlen(nvram_safe_get(list)) == 10) || (strlen(nvram_safe_get(list)) == 26))
	{
		nvram_set("wl_key_type", "0");
		warning = 262;
	}
	else if ((strlen(nvram_safe_get(list)) != 0))
		warning = 263;

	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key1Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key1Str
	fprintf(fp, "Key1Str1=%s\n", nvram_safe_get("wl_key1"));
	fprintf(fp, "Key1Str2=\n");
	fprintf(fp, "Key1Str3=\n");
	fprintf(fp, "Key1Str4=\n");
	fprintf(fp, "Key1Str5=\n");
	fprintf(fp, "Key1Str6=\n");
	fprintf(fp, "Key1Str7=\n");
	fprintf(fp, "Key1Str8=\n");

	//Key2Type
	fprintf(fp, "Key2Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key2Str
	fprintf(fp, "Key2Str1=%s\n", nvram_safe_get("wl_key2"));
	fprintf(fp, "Key2Str2=\n");
	fprintf(fp, "Key2Str3=\n");
	fprintf(fp, "Key2Str4=\n");
	fprintf(fp, "Key2Str5=\n");
	fprintf(fp, "Key2Str6=\n");
	fprintf(fp, "Key2Str7=\n");
	fprintf(fp, "Key2Str8=\n");

	//Key3Type
	fprintf(fp, "Key3Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key3Str
	fprintf(fp, "Key3Str1=%s\n", nvram_safe_get("wl_key3"));
	fprintf(fp, "Key3Str2=\n");
	fprintf(fp, "Key3Str3=\n");
	fprintf(fp, "Key3Str4=\n");
	fprintf(fp, "Key3Str5=\n");
	fprintf(fp, "Key3Str6=\n");
	fprintf(fp, "Key3Str7=\n");
	fprintf(fp, "Key3Str8=\n");

	//Key4Type
	fprintf(fp, "Key4Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key4Str
	fprintf(fp, "Key4Str1=%s\n", nvram_safe_get("wl_key4"));
	fprintf(fp, "Key4Str2=\n");
	fprintf(fp, "Key4Str3=\n");
	fprintf(fp, "Key4Str4=\n");
	fprintf(fp, "Key4Str5=\n");
	fprintf(fp, "Key4Str6=\n");
	fprintf(fp, "Key4Str7=\n");
	fprintf(fp, "Key4Str8=\n");
/*
	fprintf(fp, "SecurityMode=%d\n", 0);
	fprintf(fp, "VLANEnable=%d\n", 0);
	fprintf(fp, "VLANName=\n");
	fprintf(fp, "VLANID=%d\n", 0);
	fprintf(fp, "VLANPriority=%d\n", 0);
*/
	fprintf(fp, "HSCounter=%d\n", 0);

	//HT_HTC
	str = nvram_safe_get("HT_HTC");
	if (str)
		fprintf(fp, "HT_HTC=%d\n", atoi(str));
	else
	{
		warning = 27;
		fprintf(fp, "HT_HTC=%d\n", 1);
	}

	//HT_RDG
	str = nvram_safe_get("HT_RDG");
	if (str)
		fprintf(fp, "HT_RDG=%d\n", atoi(str));
	else
	{
		warning = 28;
		fprintf(fp, "HT_RDG=%d\n", 0);
	}

	//HT_LinkAdapt
	str = nvram_safe_get("HT_LinkAdapt");
	if (str)
		fprintf(fp, "HT_LinkAdapt=%d\n", atoi(str));
	else
	{
		warning = 29;
		fprintf(fp, "HT_LinkAdapt=%d\n", 1);
	}

	//HT_OpMode
	str = nvram_safe_get("HT_OpMode");
	if (str)
		fprintf(fp, "HT_OpMode=%d\n", atoi(str));
	else
	{
		warning = 30;
		fprintf(fp, "HT_OpMode=%d\n", 0);
	}

	//HT_MpduDensity
	str = nvram_safe_get("HT_MpduDensity");
	if (str)
		fprintf(fp, "HT_MpduDensity=%d\n", atoi(str));
	else
	{
		warning = 31;
		fprintf(fp, "HT_MpduDensity=%d\n", 5);
	}

	int Channel = atoi(nvram_safe_get("wl_channel"));
	int EXTCHA;
	int HTBW_MAX = 1;

	if (Channel != 0)
	{
		if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
		    (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
		{
			EXTCHA = 1;
		}
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
				(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
		{
			EXTCHA = 0;
		}
		else
		{
			HTBW_MAX = 0;
		}
	}

	//HT_EXTCHA
#if 0
//	if (!nvram_match("sw_mode_ex", "2") && !nvram_match("wl_channel", "0"))
	{
		str = nvram_safe_get("HT_EXTCHA");
		if (str)
			fprintf(fp, "HT_EXTCHA=%d\n", atoi(str));
		else
		{
			warning = 32;
			fprintf(fp, "HT_EXTCHA=%d\n", 1);
		}
	}
#else
//	if (!nvram_match("wl_channel", "0"))
		fprintf(fp, "HT_EXTCHA=%d\n", EXTCHA);
#endif

	//HT_BW
	str = nvram_safe_get("HT_BW");
/*
	if (nvram_match("sw_mode_ex", "2"))
		fprintf(fp, "HT_BW=%d\n", 1);
	else */if ((atoi(str) > 0) && (HTBW_MAX == 1))
		fprintf(fp, "HT_BW=%d\n", 1);
	else
	{
//		warning = 33;
		fprintf(fp, "HT_BW=%d\n", 0);
	}

	//HT_AutoBA
	str = nvram_safe_get("HT_AutoBA");
	if (str)
		fprintf(fp, "HT_AutoBA=%d\n", atoi(str));
	else
	{
		warning = 34;
		fprintf(fp, "HT_AutoBA=%d\n", 1);
	}

	//HT_BADecline
	str = nvram_safe_get("HT_BADecline");
	if (str)
		fprintf(fp, "HT_BADecline=%d\n", atoi(str));
	else
	{
		warning = 35;
		fprintf(fp, "HT_BADecline=%d\n", 0);
	}

	//HT_AMSDU
	str = nvram_safe_get("HT_AMSDU");
	if (str)
		fprintf(fp, "HT_AMSDU=%d\n", atoi(str));
	else
	{
		warning = 36;
		fprintf(fp, "HT_AMSDU=%d\n", 0);
	}

	//HT_BAWinSize
	str = nvram_safe_get("HT_BAWinSize");
	if (str)
		fprintf(fp, "HT_BAWinSize=%d\n", atoi(str));
	else
	{
		warning = 37;
		fprintf(fp, "HT_BAWinSize=%d\n", 64);
	}

	//HT_GI
	str = nvram_safe_get("HT_GI");
	if (str)
		fprintf(fp, "HT_GI=%d\n", atoi(str));
	else
	{
		warning = 38;
		fprintf(fp, "HT_GI=%d\n", 1);
	}

	//HT_STBC
	str = nvram_safe_get("HT_STBC");
	if (str)
		fprintf(fp, "HT_STBC=%d\n", atoi(str));
	else
	{
		warning = 39;
		fprintf(fp, "HT_STBC=%d\n", 1);
	}

	//HT_MCS
	str = nvram_safe_get("HT_MCS");
	if (str)
		fprintf(fp, "HT_MCS=%d\n", atoi(str));
	else
	{
		warning = 40;
		fprintf(fp, "HT_MCS=%d\n", 33);
	}

	//HT_TxStream
	str = nvram_safe_get("HT_TxStream");
	if (str)
		fprintf(fp, "HT_TxStream=%d\n", atoi(str));
	else
	{
		warning = 41;
		fprintf(fp, "HT_TxStream=%d\n", 2);
	}

	//HT_RxStream
	str = nvram_safe_get("HT_RxStream");
	if (str)
		fprintf(fp, "HT_RxStream=%d\n", atoi(str));
	else
	{
		warning = 42;
		fprintf(fp, "HT_RxStream=%d\n", 3);
	}

	//HT_PROTECT
	str = nvram_safe_get("HT_PROTECT");
	if (str)
		fprintf(fp, "HT_PROTECT=%d\n", atoi(str));
	else
	{
		warning = 43;
		fprintf(fp, "HT_PROTECT=%d\n", 1);
	}

	//HT_DisallowTKIP
	fprintf(fp, "HT_DisallowTKIP=%d\n", 1);

	// TxBF
	str = nvram_safe_get("wl_txbf");
	if ((atoi(str) > 0) && nvram_match("wl_txbf_en", "1"))
	{
		fprintf(fp, "ITxBfEn=%d\n", 1);
		fprintf(fp, "ETxBfEnCond=%d\n", 1);
	}
	else
	{
		fprintf(fp, "ITxBfEn=%d\n", 0);
		fprintf(fp, "ETxBfEnCond=%d\n", 0);
	}

	//WscConfStatus
	str = nvram_safe_get("wsc_config_state");
	if (str)
		wsc_configure = atoi(str);
	else
	{
		warning = 44;
		wsc_configure = 0;
	}

	str2 = nvram_safe_get("x_Setting");
	if (str2)
		wsc_configure |= atoi(str2);

	if (wsc_configure == 0)
	{
		fprintf(fp, "WscConfMode=%d\n", 0);
		fprintf(fp, "WscConfStatus=%d\n", 1);
		g_wsc_configured = 0;						// AP is unconfigured
	}
	else
	{
		fprintf(fp, "WscConfMode=%d\n", 0);
		fprintf(fp, "WscConfStatus=%d\n", 2);
		g_wsc_configured = 1;						// AP is configured
	}

	
	fprintf(fp, "WscVendorPinCode=%s\n", nvram_safe_get("secret_code"));
//	fprintf(fp, "ApCliWscPinCode=%s\n", nvram_safe_get("secret_code"));	// removed from SDK 3.3.0.0

	//AccessPolicy0
	str = nvram_safe_get("wl_macmode");
	if (str)
	{
		if (!strcmp(str, "disabled"))
			fprintf(fp, "AccessPolicy0=%d\n", 0);
		else if (!strcmp(str, "allow"))
			fprintf(fp, "AccessPolicy0=%d\n", 1);
		else if (!strcmp(str, "deny"))
			fprintf(fp, "AccessPolicy0=%d\n", 2);
		else
			fprintf(fp, "AccessPolicy0=%d\n", 0);
	}
	else
	{
		warning = 46;
		fprintf(fp, "AccessPolicy0=%d\n", 0);
	}

	list[0]=0;
	list[1]=0;
	if (!nvram_match("wl_macmode", "disabled"))
	{
		num = atoi(nvram_safe_get("wl_macnum_x"));
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("wl_maclist_x", i, macbuf));
	}

	//AccessControlList0
	fprintf(fp, "AccessControlList0=%s\n", list+1);

	fprintf(fp, "AccessPolicy1=%d\n", 0);
	fprintf(fp, "AccessControlList1=\n");
	fprintf(fp, "AccessPolicy2=%d\n", 0);
	fprintf(fp, "AccessControlList2=\n");
	fprintf(fp, "AccessPolicy3=%d\n", 0);
	fprintf(fp, "AccessControlList3=\n");	
	fprintf(fp, "AccessPolicy4=%d\n", 0);
	fprintf(fp, "AccessControlList4=\n");
	fprintf(fp, "AccessPolicy5=%d\n", 0);
	fprintf(fp, "AccessControlList5=\n");
	fprintf(fp, "AccessPolicy6=%d\n", 0);
	fprintf(fp, "AccessControlList6=\n");
	fprintf(fp, "AccessPolicy7=%d\n", 0);
	fprintf(fp, "AccessControlList7=\n");

	if (!nvram_match("sw_mode_ex", "2") && !nvram_match("wl_mode_x", "0"))
	{
	//WdsEnable
	str = nvram_safe_get("wl_mode_x");
	if (str)
	{
		if (	(nvram_match("wl_auth_mode", "open") ||
			(nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes")))
		)
		{
			if (atoi(str) == 0)
				fprintf(fp, "WdsEnable=%d\n", 0);
			else if (atoi(str) == 1)
				fprintf(fp, "WdsEnable=%d\n", 2);
			else if (atoi(str) == 2)
			{
//				if (nvram_match("wl_lazywds", "1"))
				if (nvram_match("wl_wdsapply_x", "0"))
					fprintf(fp, "WdsEnable=%d\n", 4);
				else
					fprintf(fp, "WdsEnable=%d\n", 3);
			}
		}
		else
			fprintf(fp, "WdsEnable=%d\n", 0);
	}
	else
	{
		warning = 47;
		fprintf(fp, "WdsEnable=%d\n", 0);
	}

	//WdsPhyMode
	str = nvram_safe_get("wl_mode_x");
	if (str)
	{
		{
			if ((atoi(nvram_safe_get("wl_gmode")) == 2))
				fprintf(fp, "WdsPhyMode=%s\n", "HTMIX");
			else if ((atoi(nvram_safe_get("wl_gmode")) == 1))
				fprintf(fp, "WdsPhyMode=%s\n", "GREENFIELD");
			else
				fprintf(fp, "WdsPhyMode=\n");
		}
	}

	//WdsEncrypType
	if (nvram_match("wl_auth_mode", "open") && nvram_match("wl_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");
	else if (nvram_match("wl_auth_mode", "open") && !nvram_match("wl_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "WEP;WEP;WEP;WEP");
	else if (nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes"))
		fprintf(fp, "WdsEncrypType=%s\n", "AES;AES;AES;AES");
	else
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");

	list[0]=0;
	list[1]=0;
	if (	(nvram_match("wl_mode_x", "1") || (nvram_match("wl_mode_x", "2") && nvram_match("wl_wdsapply_x", "1"))) &&
		(nvram_match("wl_auth_mode", "open") ||
		(nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes")))
	)
	{
		num = atoi(nvram_safe_get("wl_wdsnum_x"));
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("wl_wdslist_x", i, macbuf));
	}

	//WdsList
	fprintf(fp, "WdsList=%s\n", list+1);

	//WdsKey
	if (nvram_match("wl_auth_mode", "open") && nvram_match("wl_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=\n");
		fprintf(fp, "Wds0Key=\n");
		fprintf(fp, "Wds1Key=\n");
		fprintf(fp, "Wds2Key=\n");
		fprintf(fp, "Wds3Key=\n");
	}
	else if (nvram_match("wl_auth_mode", "open") && !nvram_match("wl_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=%s;%s;%s;%s\n", nvram_safe_get("wl_key"), nvram_safe_get("wl_key"), nvram_safe_get("wl_key"), nvram_safe_get("wl_key"));
		sprintf(list, "wl_key%s", nvram_safe_get("wl_key"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get(list));
	}
	else if (nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes"))
	{
		fprintf(fp, "WdsKey=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get("wl_wpa_psk"));
	}
	} // sw_mode_ex

//	fprintf(fp, "WirelessEvent=%d\n", 0);

	//RADIUS_Server
	if (!strcmp(nvram_safe_get("wl_radius_ipaddr"), ""))
//		fprintf(fp, "RADIUS_Server=0;0;0;0;0;0;0;0\n");
		fprintf(fp, "RADIUS_Server=\n");
	else
//		fprintf(fp, "RADIUS_Server=%s;0;0;0;0;0;0;0\n", nvram_safe_get("wl_radius_ipaddr"));
		fprintf(fp, "RADIUS_Server=%s\n", nvram_safe_get("wl_radius_ipaddr"));

	//RADIUS_Port
	str = nvram_safe_get("wl_radius_port");
	if (str)
/*		
		fprintf(fp, "RADIUS_Port=%d;%d;%d;%d;%d;%d;%d;%d\n",	atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str));
*/
		fprintf(fp, "RADIUS_Port=%d\n",	atoi(str));
	else
	{
		warning = 50;
/*
		fprintf(fp, "RADIUS_Port=%d;%d;%d;%d;%d;%d;%d;%d\n", 1812, 1812, 1812, 1812, 1812, 1812, 1812, 1812);
*/
		fprintf(fp, "RADIUS_Port=%d\n", 1812);
	}

	//RADIUS_Key
	fprintf(fp, "RADIUS_Key1=%s\n", nvram_safe_get("wl_radius_key"));
	fprintf(fp, "RADIUS_Key2=\n");
	fprintf(fp, "RADIUS_Key3=\n");
	fprintf(fp, "RADIUS_Key4=\n");
	fprintf(fp, "RADIUS_Key5=\n");
	fprintf(fp, "RADIUS_Key6=\n");
	fprintf(fp, "RADIUS_Key7=\n");
	fprintf(fp, "RADIUS_Key8=\n");

	fprintf(fp, "RADIUS_Acct_Server=\n");
	fprintf(fp, "RADIUS_Acct_Port=%d\n", 1813);
	fprintf(fp, "RADIUS_Acct_Key=\n");

	//own_ip_addr
	if (flag_8021x == 1)
	{
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
		fprintf(fp, "Ethifname=\n");
		fprintf(fp, "EAPifname=%s\n", "br0");
	}
	else
	{
		fprintf(fp, "own_ip_addr=\n");
		fprintf(fp, "Ethifname=\n");
		fprintf(fp, "EAPifname=\n");
	}

	fprintf(fp, "PreAuthifname=\n");
	fprintf(fp, "session_timeout_interval=%d\n", 0);
	fprintf(fp, "idle_timeout_interval=%d\n", 0);

	//WiFiTest
	fprintf(fp, "WiFiTest=0\n");

	//TGnWifiTest
	fprintf(fp, "TGnWifiTest=0\n");

/*
	if (nvram_match("sw_mode_ex", "2") && !nvram_match("sta_ssid", ""))
	{
		int flag_wep;

		fprintf(fp, "ApCliEnable=1\n");
		fprintf(fp, "ApCliSsid=%s\n", nvram_safe_get("sta_ssid"));
		fprintf(fp, "ApCliBssid=\n");

		str = nvram_safe_get("sta_auth_mode");
		if (str)
		{
			if (!strcmp(str, "open") && nvram_match("sta_wep_x", "0"))
			{
				fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
				fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
			}
			else if (!strcmp(str, "open") || !strcmp(str, "shared"))
			{
				flag_wep = 1;
				fprintf(fp, "ApCliAuthMode=%s\n", "WEPAUTO");
				fprintf(fp, "ApCliEncrypType=%s\n", "WEP");
			}
			else if (!strcmp(str, "psk"))
			{
				if (nvram_match("sta_wpa_mode", "1"))
					fprintf(fp, "ApCliAuthMode=%s\n", "WPAPSK");
				else if (nvram_match("sta_wpa_mode", "2"))
					fprintf(fp, "ApCliAuthMode=%s\n", "WPA2PSK");

				//EncrypType
				if (nvram_match("sta_crypto", "tkip"))
					fprintf(fp, "ApCliEncrypType=%s\n", "TKIP");
				else if (nvram_match("sta_crypto", "aes"))
					fprintf(fp, "ApCliEncrypType=%s\n", "AES");

				//WPAPSK
				fprintf(fp, "ApCliWPAPSK=%s\n", nvram_safe_get("sta_wpa_psk"));
			}
			else
			{
				fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
				fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
			}
		}
		else
		{
			fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
			fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
		}

		//EncrypType
		if (flag_wep)
		{
			//DefaultKeyID
			fprintf(fp, "ApCliDefaultKeyID=%s\n", nvram_safe_get("sta_key"));

			//KeyType (0 -> Hex, 1->Ascii)
			fprintf(fp, "ApCliKey1Type=%s\n", nvram_safe_get("sta_key_type"));
			fprintf(fp, "ApCliKey2Type=%s\n", nvram_safe_get("sta_key_type"));
			fprintf(fp, "ApCliKey3Type=%s\n", nvram_safe_get("sta_key_type"));
			fprintf(fp, "ApCliKey4Type=%s\n", nvram_safe_get("sta_key_type"));

			//KeyStr
			fprintf(fp, "ApCliKey1Str=%s\n", nvram_safe_get("sta_key1"));
			fprintf(fp, "ApCliKey2Str=%s\n", nvram_safe_get("sta_key2"));
			fprintf(fp, "ApCliKey3Str=%s\n", nvram_safe_get("sta_key3"));
			fprintf(fp, "ApCliKey4Str=%s\n", nvram_safe_get("sta_key4"));
		}
		else
		{
			fprintf(fp, "ApCliDefaultKeyID=0\n");
			fprintf(fp, "ApCliKey1Type=0\n");
			fprintf(fp, "ApCliKey1Str=\n");
			fprintf(fp, "ApCliKey2Type=0\n");
			fprintf(fp, "ApCliKey2Str=\n");
			fprintf(fp, "ApCliKey3Type=0\n");
			fprintf(fp, "ApCliKey3Str=\n");
			fprintf(fp, "ApCliKey4Type=0\n");
			fprintf(fp, "ApCliKey4Str=\n");
		}
	}
	else
*/
	{
		fprintf(fp, "ApCliEnable=0\n");
		fprintf(fp, "ApCliSsid=\n");
		fprintf(fp, "ApCliBssid=\n");
		fprintf(fp, "ApCliAuthMode=\n");
		fprintf(fp, "ApCliEncrypType=\n");
		fprintf(fp, "ApCliWPAPSK=\n");
		fprintf(fp, "ApCliDefaultKeyID=0\n");
		fprintf(fp, "ApCliKey1Type=0\n");
		fprintf(fp, "ApCliKey1Str=\n");
		fprintf(fp, "ApCliKey2Type=0\n");
		fprintf(fp, "ApCliKey2Str=\n");
		fprintf(fp, "ApCliKey3Type=0\n");
		fprintf(fp, "ApCliKey3Str=\n");
		fprintf(fp, "ApCliKey4Type=0\n");
		fprintf(fp, "ApCliKey4Str=\n");
	}

	//RadioOn
	str = nvram_safe_get("wl_radio_x");
	if (str)
		fprintf(fp, "RadioOn=%d\n", atoi(str));
	else
	{
		warning = 52;
		fprintf(fp, "RadioOn=%d\n", 1);
	}

	fprintf(fp, "SSID=\n");
	fprintf(fp, "WPAPSK=\n");
	fprintf(fp, "Key1Str=\n");
	fprintf(fp, "Key2Str=\n");
	fprintf(fp, "Key3Str=\n");
	fprintf(fp, "Key4Str=\n");

	//IgmpSnEnable
/*
	str = nvram_safe_get("IgmpSnEnable");
	if (str)
		fprintf(fp, "IgmpSnEnable=%d\n", atoi(str));
	else
	{
		warning = 1;
		fprintf(fp, "IgmpSnEnable=%d\n", 1);
	}
*/
	/*	McastPhyMode, PHY mode for Multicast frames
	 *	McastMcs, MCS for Multicast frames, ranges from 0 to 7
	 *
	 *	MODE=1, MCS=0: Legacy CCK 1Mbps
	 *	MODE=1, MCS=1: Legacy CCK 2Mbps
	 *	MODE=1, MCS=2: Legacy CCK 5.5Mbps
	 *	MODE=1, MCS=3: Legacy CCK 11Mbps
	 *	MODE=2, MCS=0: Legacy OFDM 6Mbps
	 *	MODE=2, MCS=1: Legacy OFDM 9Mbps
	 *	MODE=2, MCS=2: Legacy OFDM 12Mbps
	 *	MODE=2, MCS=3: Legacy OFDM 18Mbps
	 *	MODE=2, MCS=4: Legacy OFDM 24Mbps
	 * 	MODE=2, MCS=5: Legacy OFDM 36Mbps
	 *	MODE=2, MCS=6: Legacy OFDM 48Mbps
	 *	MODE=2, MCS=7: Legacy OFDM 54Mbps
	 *	MODE=3, MCS=15: HTMIX 130/144Mbps
	 **/
	str = nvram_safe_get("wl_mrate");
	if (str)
	{
		if (atoi(str) == 0)	// Disable
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 0);
//			fprintf(fp, "McastPhyMode=%d\n", 0);
//			fprintf(fp, "McastMcs=%d\n", 0);
		}
/*
		else if (atoi(str) == 1)	// Legacy CCK 1Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 0);
		}
		else if (atoi(str) == 2)	// Legacy CCK 2Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 1);
		}
		else if (atoi(str) == 3)	// Legacy CCK 5.5Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 2);
		}
		else if (atoi(str) == 4)	// Legacy CCK 11Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 3);
		}
		else if (atoi(str) == 5)	// Legacy OFDM 6Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 0);
		}
		else if (atoi(str) == 6)	// Legacy OFDM 9Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 1);
		}
		else if (atoi(str) == 7)	// Legacy OFDM 12Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 2);
		}
		else if (atoi(str) == 8)	// Legacy OFDM 18Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 3);
		}
		else if (atoi(str) == 9)	// Legacy OFDM 24Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 4);
		}
		else if (atoi(str) == 10)	// Legacy OFDM 36Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 5);
		}
		else if (atoi(str) == 11)	// Legacy OFDM 48Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 6);
		}
		else if (atoi(str) == 12)	// Legacy OFDM 54Mbps
		{
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 7);
		}
*/
//		else if (atoi(str) == 13)	// HTMIX 130/144Mbps => Auto
		else
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 3);
			fprintf(fp, "McastMcs=%d\n", 15);
		}
	}
	else
	{
		fprintf(fp, "IgmpSnEnable=%d\n", 0);
		warning = 53;
	}

	if (warning)
	{
		printf("%d!!!!\n", warning);
		printf("Miss some configuration, please check!!!!\n");
	}

	fclose(fp);

	return 0;
}

int gen_ralink_config_rt()
{
	FILE *fp;
	char *str = NULL;
	char *str2 = NULL;
	int  i;
//	int ssid_num = 1;
	char wmm_enable[8];
	char wmm_noack[8];
	char macbuf[36];
	char list[2048];
	int flag_8021x = 0;
	int wsc_configure = 0;
	int warning = 0;
	int num;
	int ChannelNumMax = 11;

	printf("gen ralink iNIC config\n");
	system("mkdir -p /etc/Wireless/iNIC");
	if (!(fp=fopen("/etc/Wireless/iNIC/iNIC_ap.dat", "w+")))
		return 0;

	fprintf(fp, "#The word of \"Default\" must not be removed\n");
	fprintf(fp, "Default\n");

	//CountryRegion
	//CountryRegion
	str = nvram_safe_get("wl_country_code");
	if (str)
	{
		if (    (strcasecmp(str, "CA") == 0) || (strcasecmp(str, "CO") == 0) ||
			(strcasecmp(str, "DO") == 0) || (strcasecmp(str, "GT") == 0) ||
			(strcasecmp(str, "MX") == 0) || (strcasecmp(str, "NO") == 0) ||
			(strcasecmp(str, "PA") == 0) || (strcasecmp(str, "PR") == 0) ||
			(strcasecmp(str, "TW") == 0) || (strcasecmp(str, "US") == 0) ||
			(strcasecmp(str, "UZ") == 0))
		{
			fprintf(fp, "CountryRegion=%d\n", 0);   // channel 1-11
			ChannelNumMax = 11;
		}
		else if (strcasecmp(str, "DB") == 0  || strcasecmp(str, "") == 0)
		{
			fprintf(fp, "CountryRegion=%d\n", 5);   // channel 1-14
			ChannelNumMax = 14;
		}
		else
		{
			fprintf(fp, "CountryRegion=%d\n", 1);   // channel 1-13
			ChannelNumMax = 13;
		}
	}
	else
	{
		warning = 1;
		fprintf(fp, "CountryRegion=%d\n", 5);
	}

	//CountryRegion for A band
	str = nvram_safe_get("wl_country_code");
	if (str)
	{
		if (		(!strcasecmp(str, "AL")) ||
				(!strcasecmp(str, "DZ")) ||
				(!strcasecmp(str, "AU")) ||
				(!strcasecmp(str, "BH")) ||
				(!strcasecmp(str, "BY")) ||
				(!strcasecmp(str, "CA")) ||
				(!strcasecmp(str, "CL")) ||
				(!strcasecmp(str, "CO")) ||
				(!strcasecmp(str, "CR")) ||
				(!strcasecmp(str, "DO")) ||
				(!strcasecmp(str, "EC")) ||
				(!strcasecmp(str, "SV")) ||
				(!strcasecmp(str, "GT")) ||
				(!strcasecmp(str, "HN")) ||
				(!strcasecmp(str, "HK")) ||
				(!strcasecmp(str, "IN")) ||
				(!strcasecmp(str, "IL")) ||
				(!strcasecmp(str, "JO")) ||
				(!strcasecmp(str, "KZ")) ||
				(!strcasecmp(str, "KW")) ||
				(!strcasecmp(str, "LB")) ||
				(!strcasecmp(str, "MO")) ||
				(!strcasecmp(str, "MK")) ||
				(!strcasecmp(str, "MY")) ||
				(!strcasecmp(str, "MX")) ||
				(!strcasecmp(str, "MA")) ||
				(!strcasecmp(str, "NZ")) ||
				(!strcasecmp(str, "NO")) ||
				(!strcasecmp(str, "OM")) ||
				(!strcasecmp(str, "PK")) ||
				(!strcasecmp(str, "PA")) ||
				(!strcasecmp(str, "PR")) ||
				(!strcasecmp(str, "QA")) ||
				(!strcasecmp(str, "RO")) ||
				(!strcasecmp(str, "RU")) ||
				(!strcasecmp(str, "SA")) ||
				(!strcasecmp(str, "SG")) ||
				(!strcasecmp(str, "SY")) ||
				(!strcasecmp(str, "TH")) ||
				(!strcasecmp(str, "UA")) ||
				(!strcasecmp(str, "AE")) ||
				(!strcasecmp(str, "US")) ||
				(!strcasecmp(str, "VN")) ||
				(!strcasecmp(str, "YE")) ||
				(!strcasecmp(str, "ZW"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 0);
		}
		else if (	(!strcasecmp(str, "AT")) ||
				(!strcasecmp(str, "BE")) ||
				(!strcasecmp(str, "BR")) ||
				(!strcasecmp(str, "BG")) ||
				(!strcasecmp(str, "CY")) ||
				(!strcasecmp(str, "DK")) ||
				(!strcasecmp(str, "EE")) ||
				(!strcasecmp(str, "FI")) ||
				(!strcasecmp(str, "DE")) ||
				(!strcasecmp(str, "GR")) ||
				(!strcasecmp(str, "HU")) ||
				(!strcasecmp(str, "IS")) ||
				(!strcasecmp(str, "IE")) ||
				(!strcasecmp(str, "IT")) ||
				(!strcasecmp(str, "LV")) ||
				(!strcasecmp(str, "LI")) ||
				(!strcasecmp(str, "LT")) ||
				(!strcasecmp(str, "LU")) ||
				(!strcasecmp(str, "NL")) ||
				(!strcasecmp(str, "PL")) ||
				(!strcasecmp(str, "PT")) ||
				(!strcasecmp(str, "SK")) ||
				(!strcasecmp(str, "SI")) ||
				(!strcasecmp(str, "ZA")) ||
				(!strcasecmp(str, "ES")) ||
				(!strcasecmp(str, "SE")) ||
				(!strcasecmp(str, "CH")) ||
				(!strcasecmp(str, "GB")) ||
				(!strcasecmp(str, "UZ"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 1);
		}
		else if (	(!strcasecmp(str, "AM")) ||
				(!strcasecmp(str, "AZ")) ||
				(!strcasecmp(str, "HR")) ||
				(!strcasecmp(str, "CZ")) ||
				(!strcasecmp(str, "EG")) ||
				(!strcasecmp(str, "FR")) ||
				(!strcasecmp(str, "GE")) ||
				(!strcasecmp(str, "MC")) ||
				(!strcasecmp(str, "TT")) ||
				(!strcasecmp(str, "TN")) ||
				(!strcasecmp(str, "TR"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 2);
		}
		else if (	(!strcasecmp(str, "AR")) ||
				(!strcasecmp(str, "TW"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 3);
		}
		else if (	(!strcasecmp(str, "BZ")) ||
				(!strcasecmp(str, "BO")) ||
				(!strcasecmp(str, "BN")) ||
				(!strcasecmp(str, "CN")) ||
				(!strcasecmp(str, "ID")) ||
				(!strcasecmp(str, "IR")) ||
				(!strcasecmp(str, "PE")) ||
				(!strcasecmp(str, "PH"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 4);
		}
		else if (	(!strcasecmp(str, "KP")) ||
				(!strcasecmp(str, "KR")) ||
				(!strcasecmp(str, "UY")) ||
				(!strcasecmp(str, "VE"))
		)
		{
			fprintf(fp, "CountryRegionABand=%d\n", 5);
		}
		else if (!strcasecmp(str, "DB"))
		{
			fprintf(fp, "CountryRegionABand=%d\n", 7);
		}
		else if (!strcasecmp(str, "JP"))
		{
			fprintf(fp, "CountryRegionABand=%d\n", 9);
		}
		else
		{
			warning = 2;
			fprintf(fp, "CountryRegionABand=%d\n", 7);
		}
	}
	else
	{
		warning = 2;
		fprintf(fp, "CountryRegionABand=%d\n", 7);
	}

	//CountryCode
	str = nvram_safe_get("wl_country_code");
	if (str)
	{
		if (strcmp(str, "") == 0)
			fprintf(fp, "CountryCode=DB\n");
		else
			fprintf(fp, "CountryCode=%s\n", str);
	}
	else
	{
		warning = 3;
		fprintf(fp, "CountryCode=DB\n");
	}

	//SSID Num. [MSSID Only]
	fprintf(fp, "BssidNum=%d\n", 1);

	//SSID
	str = nvram_safe_get("rt_ssid");
	if (str)
		fprintf(fp, "SSID1=%s\n", str);
	else
	{
		warning = 4;
		fprintf(fp, "SSID1=%s\n", "default");
	}

	fprintf(fp, "SSID2=\n");
	fprintf(fp, "SSID3=\n");
	fprintf(fp, "SSID4=\n");
	fprintf(fp, "SSID5=\n");
	fprintf(fp, "SSID6=\n");
	fprintf(fp, "SSID7=\n");
	fprintf(fp, "SSID8=\n");

	//Network Mode
	str = nvram_safe_get("rt_gmode");
	if (str)
	{
		if (atoi(str) == 2)       // B,G,N
			fprintf(fp, "WirelessMode=%d\n", 9);
		else if (atoi(str) == 1)  // B,G
			fprintf(fp, "WirelessMode=%d\n", 0);
		else if (atoi(str) == 3)  // N
			fprintf(fp, "WirelessMode=%d\n", 6);
		else if (atoi(str) == 4)  // G
			fprintf(fp, "WirelessMode=%d\n", 4);
		else if (atoi(str) == 0)  // B
			fprintf(fp, "WirelessMode=%d\n", 1);
		else			// B,G,N
			fprintf(fp, "WirelessMode=%d\n", 9);
	}
	else
	{
		warning = 5;
		fprintf(fp, "WirelessMode=%d\n", 9);
	}

	fprintf(fp, "FixedTxMode=\n");

	fprintf(fp, "TxRate=%d\n", 0);

	//Channel
//	if (!nvram_match("sw_mode_ex", "2") && !nvram_match("rt_channel", "0"))
	{
		str = nvram_safe_get("rt_channel");

		if (str)
			fprintf(fp, "Channel=%d\n", atoi(str));
		else
		{
			warning = 6;
			fprintf(fp, "Channel=%d\n", 0);
		}
	}

	//BasicRate
	str = nvram_safe_get("rt_rateset");
	if (str)
	{
		if (!strcmp(str, "default"))	// 1, 2, 5.5, 11
			fprintf(fp, "BasicRate=%d\n", 15);
		else if (!strcmp(str, "all"))	// 1, 2, 5.5, 6, 11, 12, 24
			fprintf(fp, "BasicRate=%d\n", 351);
		else if (!strcmp(str, "12"))	// 1, 2
			fprintf(fp, "BasicRate=%d\n", 3);
		else
			fprintf(fp, "BasicRate=%d\n", 15);
	}
	else
	{
		warning = 7;
		fprintf(fp, "BasicRate=%d\n", 15);
	}

	//BeaconPeriod
	str = nvram_safe_get("rt_bcn");
	if (str)
	{
		if (atoi(str) > 1000 || atoi(str) < 20)
		{
			nvram_set("rt_bcn", "100");
			fprintf(fp, "BeaconPeriod=%d\n", 100);
		}
		else
			fprintf(fp, "BeaconPeriod=%d\n", atoi(str));
	}
	else
	{
		warning = 8;
		fprintf(fp, "BeaconPeriod=%d\n", 100);
	}

	//DTIM Period
	str = nvram_safe_get("rt_dtim");
	if (str)
		fprintf(fp, "DtimPeriod=%d\n", atoi(str));
	else
	{
		warning = 9;
		fprintf(fp, "DtimPeriod=%d\n", 1);
	}

	//TxPower
	str = nvram_safe_get("rt_TxPower");
	if (nvram_match("rt_radio_x", "0"))
		fprintf(fp, "TxPower=%d\n", 0);	
	else if (str)
		fprintf(fp, "TxPower=%d\n", atoi(str));
	else
	{
		warning = 10;
		fprintf(fp, "TxPower=%d\n", 100);
	}

	//DisableOLBC
	fprintf(fp, "DisableOLBC=%d\n", 0);

	//BGProtection
	str = nvram_safe_get("rt_gmode_protection");
	if (str)
	{
		if (!strcmp(str, "auto") && atoi(nvram_safe_get("rt_gmode")))
			fprintf(fp, "BGProtection=%d\n", 0);
		else if (!strcmp(str, "on"))
			fprintf(fp, "BGProtection=%d\n", 1);
		else
			fprintf(fp, "BGProtection=%d\n", 2);
	}
	else
	{
		warning = 11;
		fprintf(fp, "BGProtection=%d\n", 0);
	}

	//TxAntenna
	fprintf(fp, "TxAntenna=\n");

	//RxAntenna
	fprintf(fp, "RxAntenna=\n");

	//TxPreamble
	fprintf(fp, "TxPreamble=%d\n", 0);

	//RTSThreshold  Default=2347
	str = nvram_safe_get("rt_rts");
	if (str)
		fprintf(fp, "RTSThreshold=%d\n", atoi(str));
	else
	{
		warning = 12;
		fprintf(fp, "RTSThreshold=%d\n", 2347);
	}

	//FragThreshold  Default=2346
	str = nvram_safe_get("rt_frag");
	if (str)
		fprintf(fp, "FragThreshold=%d\n", atoi(str));
	else
	{
		warning = 13;
		fprintf(fp, "FragThreshold=%d\n", 2346);
	}

	//TxBurst
	str = nvram_safe_get("rt_TxBurst");
	if (str)
		fprintf(fp, "TxBurst=%d\n", atoi(str));
	else
	{
		warning = 14;
		fprintf(fp, "TxBurst=%d\n", 1);
	}

	//PktAggregate
	str = nvram_safe_get("rt_PktAggregate");
	if (str)
		fprintf(fp, "PktAggregate=%d\n", atoi(str));
	else
	{
		warning = 15;
		fprintf(fp, "PktAggregate=%d\n", 1);
	}

	fprintf(fp, "FreqDelta=%d\n", 0);
	fprintf(fp, "TurboRate=%d\n", 0);

	//WmmCapable
	bzero(wmm_enable, sizeof(char)*8);
//	for (i = 0; i < ssid_num; i++)
	{
		str = nvram_safe_get("rt_gmode");
		if (str && atoi(str) == 3)	// always enable WMM in N only mode
			sprintf(wmm_enable+strlen(wmm_enable), "%d", 1);
		else
			sprintf(wmm_enable+strlen(wmm_enable), "%d", atoi(nvram_safe_get("rt_wme")));
//		sprintf(wmm_enable+strlen(wmm_enable), "%c", ';');
	}
//	wmm_enable[strlen(wmm_enable) - 1] = '\0';
	wmm_enable[1] = '\0';
	fprintf(fp, "WmmCapable=%s\n", wmm_enable);

	fprintf(fp, "APAifsn=3;7;1;1\n");
	fprintf(fp, "APCwmin=4;4;3;2\n");
	fprintf(fp, "APCwmax=6;10;4;3\n");
	fprintf(fp, "APTxop=0;0;94;47\n");
	fprintf(fp, "APACM=0;0;0;0\n");
	fprintf(fp, "BSSAifsn=3;7;2;2\n");
	fprintf(fp, "BSSCwmin=4;4;3;2\n");
	fprintf(fp, "BSSCwmax=10;10;4;3\n");
	fprintf(fp, "BSSTxop=0;0;94;47\n");
	fprintf(fp, "BSSACM=0;0;0;0\n");

	//AckPolicy
	bzero(wmm_noack, sizeof(char)*8);
	for (i = 0; i < 4; i++)
	{
		sprintf(wmm_noack+strlen(wmm_noack), "%d", strcmp(nvram_safe_get("rt_wme_no_ack"), "on")? 0 : 1);
		sprintf(wmm_noack+strlen(wmm_noack), "%c", ';');
	}
	wmm_noack[strlen(wmm_noack) - 1] = '\0';
	fprintf(fp, "AckPolicy=%s\n", wmm_noack);

	//APSDCapable
	str = nvram_safe_get("rt_APSDCapable");
	if (str)
		fprintf(fp, "APSDCapable=%d\n", atoi(str));
	else
	{
		warning = 18;
		fprintf(fp, "APSDCapable=%d\n", 1);
	}

	//DLSDCapable
	str = nvram_safe_get("rt_DLSCapable");
	if (str)
		fprintf(fp, "DLSCapable=%d\n", atoi(str));
	else
	{
		warning = 19;
		fprintf(fp, "DLSCapable=%d\n", 0);
	}

	//NoForwarding pre SSID & NoForwardingBTNBSSID
	str = nvram_safe_get("rt_ap_isolate");
	if (str)
	{
		fprintf(fp, "NoForwarding=%d\n", atoi(str));
		fprintf(fp, "NoForwardingBTNBSSID=%d\n", atoi(str));
	}
	else
	{
		warning = 20;
		fprintf(fp, "NoForwarding=%d\n", 0);
		fprintf(fp, "NoForwardingBTNBSSID=%d\n", 0);
	}

	//HideSSID
	fprintf(fp, "HideSSID=%s\n", nvram_safe_get("rt_closed"));

	//ShortSlot
	fprintf(fp, "ShortSlot=%d\n", 1);


	//AutoChannelSelect
	{
		str = nvram_safe_get("rt_channel");
/*
		if (nvram_match("sw_mode_ex", "2"))
			fprintf(fp, "AutoChannelSelect=%d\n", 1);
		else */if (str)
		{
			if (atoi(str) == 0)
				fprintf(fp, "AutoChannelSelect=%d\n", 2);
			else
				fprintf(fp, "AutoChannelSelect=%d\n", 0);
		}
		else
		{
			warning = 21;
			fprintf(fp, "AutoChannelSelect=%d\n", 2);
		}
	}

	//IEEE8021X
	str = nvram_safe_get("rt_auth_mode");
	if (str && !strcmp(str, "radius"))
		fprintf(fp, "IEEE8021X=%d\n", 1);
	else
		fprintf(fp, "IEEE8021X=%d\n", 0);

	if (str)
	{
		if (	!strcmp(str, "radius") ||
			!strcmp(str, "wpa") ||
			!strcmp(str, "wpa2")	)
			flag_8021x = 1;
	}

	fprintf(fp, "IEEE80211H=0\n");
	fprintf(fp, "CarrierDetect=%d\n", 0);
//	fprintf(fp, "ITxBfEn=%d\n", 0);
	fprintf(fp, "PreAntSwitch=\n");
	fprintf(fp, "PhyRateLimit=%d\n", 0);
	fprintf(fp, "DebugFlags=%d\n", 0);
//	fprintf(fp, "ETxBfEnCond=%d\n", 0);
//	fprintf(fp, "ITxBfTimeout=%d\n", 0);
	fprintf(fp, "FineAGC=%d\n", 0);
	fprintf(fp, "StreamMode=%d\n", 0);
	fprintf(fp, "StreamModeMac0=\n");
	fprintf(fp, "StreamModeMac1=\n");
	fprintf(fp, "StreamModeMac2=\n");
	fprintf(fp, "StreamModeMac3=\n");
	fprintf(fp, "CSPeriod=10\n");
	fprintf(fp, "RDRegion=\n");
	fprintf(fp, "StationKeepAlive=%d\n", 0);
	fprintf(fp, "DfsLowerLimit=%d\n", 0);
	fprintf(fp, "DfsUpperLimit=%d\n", 0);
	fprintf(fp, "DfsIndoor=%d\n", 0);
	fprintf(fp, "DFSParamFromConfig=%d\n", 0);
	fprintf(fp, "FCCParamCh0=\n");
	fprintf(fp, "FCCParamCh1=\n");
	fprintf(fp, "FCCParamCh2=\n");
	fprintf(fp, "FCCParamCh3=\n");
	fprintf(fp, "CEParamCh0=\n");
	fprintf(fp, "CEParamCh1=\n");
	fprintf(fp, "CEParamCh2=\n");
	fprintf(fp, "CEParamCh3=\n");
	fprintf(fp, "JAPParamCh0=\n");
	fprintf(fp, "JAPParamCh1=\n");
	fprintf(fp, "JAPParamCh2=\n");
	fprintf(fp, "JAPParamCh3=\n");
	fprintf(fp, "JAPW53ParamCh0=\n");
	fprintf(fp, "JAPW53ParamCh1=\n");
	fprintf(fp, "JAPW53ParamCh2=\n");
	fprintf(fp, "JAPW53ParamCh3=\n");
	fprintf(fp, "FixDfsLimit=%d\n", 0);
	fprintf(fp, "LongPulseRadarTh=%d\n", 0);
	fprintf(fp, "AvgRssiReq=%d\n", 0);
	fprintf(fp, "DFS_R66=%d\n", 0);
	fprintf(fp, "BlockCh=\n");

	//GreenAP
	str = nvram_safe_get("rt_GreenAP");
/*
	if (nvram_match("sw_mode_ex", "2"))
		fprintf(fp, "GreenAP=%d\n", 0);
	else */if (str)
		fprintf(fp, "GreenAP=%d\n", atoi(str));
	else
	{
		warning = 23;
		fprintf(fp, "GreenAP=%d\n", 0);
	}

	//PreAuth
	fprintf(fp, "PreAuth=0\n");

	//AuthMode
	str = nvram_safe_get("rt_auth_mode");
	if (str)
	{
		if (!strcmp(str, "open"))
		{
			fprintf(fp, "AuthMode=%s\n", "OPEN");
		}
		else if (!strcmp(str, "shared"))
		{
			fprintf(fp, "AuthMode=%s\n", "SHARED");
		}
		else if (!strcmp(str, "psk"))
		{
			if (nvram_match("rt_wpa_mode", "0"))
			{
/*
				if (nvram_match("rt_crypto", "tkip"))
					fprintf(fp, "AuthMode=%s\n", "WPAPSK");
				else if (nvram_match("rt_crypto", "aes"))
					fprintf(fp, "AuthMode=%s\n", "WPA2PSK");
				else
*/
				fprintf(fp, "AuthMode=%s\n", "WPAPSKWPA2PSK");
			}
			else if (nvram_match("rt_wpa_mode", "1"))
				fprintf(fp, "AuthMode=%s\n", "WPAPSK");
			else if (nvram_match("rt_wpa_mode", "2"))
				fprintf(fp, "AuthMode=%s\n", "WPA2PSK");
		}
		else if (!strcmp(str, "wpa"))
		{
//			if (nvram_match("rt_wpa_mode", "0")) //2008.11 magic
			if (nvram_match("rt_wpa_mode", "4"))
			{
/*
				if (nvram_match("rt_crypto", "tkip"))
					fprintf(fp, "AuthMode=%s\n", "WPA");
				else if (nvram_match("rt_crypto", "aes"))
					fprintf(fp, "AuthMode=%s\n", "WPA2");
				else
*/
					fprintf(fp, "AuthMode=%s\n", "WPA1WPA2");
			}
		//2008.11 magic for new UI{
//			else if (nvram_match("rt_wpa_mode", "1"))
//				fprintf(fp, "AuthMode=%s\n", "WPA");
			else if (nvram_match("rt_wpa_mode", "3"))
				fprintf(fp, "AuthMode=%s\n", "WPA");
//			else if (nvram_match("rt_wpa_mode", "2"))
//				fprintf(fp, "AuthMode=%s\n", "WPA2");
			//2008.11 magic for new UI}
		}
		//2008.11 magic for new UI{
		else if (!strcmp(str, "wpa2"))
		{
			fprintf(fp, "AuthMode=%s\n", "WPA2");	
		}
		//2008.11 magic for new UI}
		else if (!strcmp(str, "radius"))
		{
			fprintf(fp, "AuthMode=%s\n", "OPEN");
		}
		else
		{
			fprintf(fp, "AuthMode=%s\n", "OPEN");
		}
	}
	else
	{
		warning = 24;
		fprintf(fp, "AuthMode=%s\n", "OPEN");
	}

	//EncrypType
	if (	(nvram_match("rt_auth_mode", "open") && nvram_match("rt_wep_x", "0")) /*||
		(nvram_match("rt_auth_mode", "radius") && nvram_match("rt_wep_x", "0"))*/
	)
		fprintf(fp, "EncrypType=%s\n", "NONE");
	else if (       (nvram_match("rt_auth_mode", "open") && !nvram_match("rt_wep_x", "0")) ||
			nvram_match("rt_auth_mode", "shared") ||
			nvram_match("rt_auth_mode", "radius")/* ||
			(nvram_match("rt_auth_mode", "radius") && !nvram_match("rt_wep_x", "0"))*/
	)
		fprintf(fp, "EncrypType=%s\n", "WEP");
	else if (nvram_match("rt_crypto", "tkip"))
		fprintf(fp, "EncrypType=%s\n", "TKIP");
	else if (nvram_match("rt_crypto", "aes"))
		fprintf(fp, "EncrypType=%s\n", "AES");
	else if (nvram_match("rt_crypto", "tkip+aes"))
		fprintf(fp, "EncrypType=%s\n", "TKIPAES");
	else
		fprintf(fp, "EncrypType=%s\n", "NONE");

	fprintf(fp, "WapiPsk1=\n");
	fprintf(fp, "WapiPsk2=\n");
	fprintf(fp, "WapiPsk3=\n");
	fprintf(fp, "WapiPsk4=\n");
	fprintf(fp, "WapiPsk5=\n");
	fprintf(fp, "WapiPsk6=\n");
	fprintf(fp, "WapiPsk7=\n");
	fprintf(fp, "WapiPsk8=\n");
	fprintf(fp, "WapiPskType=\n");
	fprintf(fp, "Wapiifname=\n");
	fprintf(fp, "WapiAsCertPath=\n");
	fprintf(fp, "WapiUserCertPath=\n");
	fprintf(fp, "WapiAsIpAddr=\n");
	fprintf(fp, "WapiAsPort=\n");

	//RekeyInterval
	str = nvram_safe_get("rt_wpa_gtk_rekey");
	if (str)
	{
		if (atol(str) == 0)
			fprintf(fp, "RekeyMethod=%s\n", "DISABLE");
		else
			fprintf(fp, "RekeyMethod=TIME\n");

		fprintf(fp, "RekeyInterval=%d\n", atol(str));
	}
	else
	{
		warning = 25;
		fprintf(fp, "RekeyMethod=%s\n", "DISABLE");
		fprintf(fp, "RekeyInterval=%d\n", 0);
	}

	//PMKCachePeriod
	fprintf(fp, "PMKCachePeriod=%d\n", 10);

	fprintf(fp, "MeshAutoLink=%d\n", 0);
	fprintf(fp, "MeshAuthMode=\n");
	fprintf(fp, "MeshEncrypType=\n");
	fprintf(fp, "MeshDefaultkey=%d\n", 0);
	fprintf(fp, "MeshWEPKEY=\n");
	fprintf(fp, "MeshWPAKEY=\n");
	fprintf(fp, "MeshId=\n");

	//WPAPSK
	fprintf(fp, "WPAPSK1=%s\n", nvram_safe_get("rt_wpa_psk"));

	fprintf(fp, "WPAPSK2=\n");
	fprintf(fp, "WPAPSK3=\n");
	fprintf(fp, "WPAPSK4=\n");
	fprintf(fp, "WPAPSK5=\n");
	fprintf(fp, "WPAPSK6=\n");
	fprintf(fp, "WPAPSK7=\n");
	fprintf(fp, "WPAPSK8=\n");

	//DefaultKeyID
	fprintf(fp, "DefaultKeyID=%s\n", nvram_safe_get("rt_key"));

	sprintf(list, "rt_key%s", nvram_safe_get("rt_key"));
	if ((strlen(nvram_safe_get(list)) == 5) || (strlen(nvram_safe_get(list)) == 13))
	{
		nvram_set("rt_key_type", "1");
		warning = 261;
	}
	else if ((strlen(nvram_safe_get(list)) == 10) || (strlen(nvram_safe_get(list)) == 26))
	{
		nvram_set("rt_key_type", "0");
		warning = 262;
	}
	else if ((strlen(nvram_safe_get(list)) != 0))
		warning = 263;

	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key1Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key1Str
	fprintf(fp, "Key1Str1=%s\n", nvram_safe_get("rt_key1"));
	fprintf(fp, "Key1Str2=\n");
	fprintf(fp, "Key1Str3=\n");
	fprintf(fp, "Key1Str4=\n");
	fprintf(fp, "Key1Str5=\n");
	fprintf(fp, "Key1Str6=\n");
	fprintf(fp, "Key1Str7=\n");
	fprintf(fp, "Key1Str8=\n");

	//Key2Type
	fprintf(fp, "Key2Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key2Str
	fprintf(fp, "Key2Str1=%s\n", nvram_safe_get("rt_key2"));
	fprintf(fp, "Key2Str2=\n");
	fprintf(fp, "Key2Str3=\n");
	fprintf(fp, "Key2Str4=\n");
	fprintf(fp, "Key2Str5=\n");
	fprintf(fp, "Key2Str6=\n");
	fprintf(fp, "Key2Str7=\n");
	fprintf(fp, "Key2Str8=\n");

	//Key3Type
	fprintf(fp, "Key3Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key3Str
	fprintf(fp, "Key3Str1=%s\n", nvram_safe_get("rt_key3"));
	fprintf(fp, "Key3Str2=\n");
	fprintf(fp, "Key3Str3=\n");
	fprintf(fp, "Key3Str4=\n");
	fprintf(fp, "Key3Str5=\n");
	fprintf(fp, "Key3Str6=\n");
	fprintf(fp, "Key3Str7=\n");
	fprintf(fp, "Key3Str8=\n");

	//Key4Type
	fprintf(fp, "Key4Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key4Str
	fprintf(fp, "Key4Str1=%s\n", nvram_safe_get("rt_key4"));
	fprintf(fp, "Key4Str2=\n");
	fprintf(fp, "Key4Str3=\n");
	fprintf(fp, "Key4Str4=\n");
	fprintf(fp, "Key4Str5=\n");
	fprintf(fp, "Key4Str6=\n");
	fprintf(fp, "Key4Str7=\n");
	fprintf(fp, "Key4Str8=\n");
/*
	fprintf(fp, "SecurityMode=%d\n", 0);
	fprintf(fp, "VLANEnable=%d\n", 0);
	fprintf(fp, "VLANName=\n");
	fprintf(fp, "VLANID=%d\n", 0);
	fprintf(fp, "VLANPriority=%d\n", 0);
*/
	fprintf(fp, "HSCounter=%d\n", 0);

	//HT_HTC
	str = nvram_safe_get("rt_HT_HTC");
	if (str)
		fprintf(fp, "HT_HTC=%d\n", atoi(str));
	else
	{
		warning = 27;
		fprintf(fp, "HT_HTC=%d\n", 1);
	}

	//HT_RDG
	str = nvram_safe_get("rt_HT_RDG");
	if (str)
		fprintf(fp, "HT_RDG=%d\n", atoi(str));
	else
	{
		warning = 28;
		fprintf(fp, "HT_RDG=%d\n", 0);
	}

	//HT_LinkAdapt
	str = nvram_safe_get("rt_HT_LinkAdapt");
	if (str)
		fprintf(fp, "HT_LinkAdapt=%d\n", atoi(str));
	else
	{
		warning = 29;
		fprintf(fp, "HT_LinkAdapt=%d\n", 1);
	}

	//HT_OpMode
	str = nvram_safe_get("rt_HT_OpMode");
	if (str)
		fprintf(fp, "HT_OpMode=%d\n", atoi(str));
	else
	{
		warning = 30;
		fprintf(fp, "HT_OpMode=%d\n", 0);
	}

	//HT_MpduDensity
	str = nvram_safe_get("rt_HT_MpduDensity");
	if (str)
		fprintf(fp, "HT_MpduDensity=%d\n", atoi(str));
	else
	{
		warning = 31;
		fprintf(fp, "HT_MpduDensity=%d\n", 5);
	}

	int Channel = atoi(nvram_safe_get("rt_channel"));
	int EXTCHA_MAX = 0;
	int HTBW_MAX = 1;

	if (Channel == 0)
		EXTCHA_MAX = 1;
	else if ((Channel >=1) && (Channel <= 4))
		EXTCHA_MAX = 0;
	else if ((Channel >= 5) && (Channel <= 7))
		EXTCHA_MAX = 1;
	else if ((Channel >= 8) && (Channel <= 14))
	{
		if ((ChannelNumMax - Channel) < 4)
			EXTCHA_MAX = 0;
		else
			EXTCHA_MAX = 1;
	}
	else
		HTBW_MAX = 0;

	//HT_EXTCHA
//	if (/*!nvram_match("sw_mode_ex", "2") && */!nvram_match("rt_channel", "0"))
	{
		str = nvram_safe_get("rt_HT_EXTCHA");
		if (str)
		{
			if (atoi(str) <= EXTCHA_MAX)
				fprintf(fp, "HT_EXTCHA=%d\n", atoi(str));
			else
				fprintf(fp, "HT_EXTCHA=%d\n", 0);
		}
		else
		{
			warning = 32;
/*
			fprintf(fp, "HT_EXTCHA=%d\n", 1);
*/
			fprintf(fp, "HT_EXTCHA=%d\n", 0);
		}
	}

	//HT_BW
	str = nvram_safe_get("rt_HT_BW");
/*
	if (nvram_match("sw_mode_ex", "2"))
		fprintf(fp, "HT_BW=%d\n", 1);
	else */if ((atoi(str) > 0) && (HTBW_MAX == 1))
		fprintf(fp, "HT_BW=%d\n", 1);
	else
	{
//		warning = 33;
		fprintf(fp, "HT_BW=%d\n", 0);
	}

	//HT_AutoBA
	str = nvram_safe_get("rt_HT_AutoBA");
	if (str)
		fprintf(fp, "HT_AutoBA=%d\n", atoi(str));
	else
	{
		warning = 34;
		fprintf(fp, "HT_AutoBA=%d\n", 1);
	}

	//HT_BADecline
	str = nvram_safe_get("rt_HT_BADecline");
	if (str)
		fprintf(fp, "HT_BADecline=%d\n", atoi(str));
	else
	{
		warning = 35;
		fprintf(fp, "HT_BADecline=%d\n", 0);
	}

	//HT_AMSDU
	str = nvram_safe_get("rt_HT_AMSDU");
	if (str)
		fprintf(fp, "HT_AMSDU=%d\n", atoi(str));
	else
	{
		warning = 36;
		fprintf(fp, "HT_AMSDU=%d\n", 0);
	}

	//HT_BAWinSize
	str = nvram_safe_get("rt_HT_BAWinSize");
	if (str)
		fprintf(fp, "HT_BAWinSize=%d\n", atoi(str));
	else
	{
		warning = 37;
		fprintf(fp, "HT_BAWinSize=%d\n", 64);
	}

	//HT_GI
	str = nvram_safe_get("rt_HT_GI");
	if (str)
		fprintf(fp, "HT_GI=%d\n", atoi(str));
	else
	{
		warning = 38;
		fprintf(fp, "HT_GI=%d\n", 1);
	}

	//HT_STBC
	str = nvram_safe_get("rt_HT_STBC");
	if (str)
		fprintf(fp, "HT_STBC=%d\n", atoi(str));
	else
	{
		warning = 39;
		fprintf(fp, "HT_STBC=%d\n", 1);
	}

	//HT_MCS
	str = nvram_safe_get("rt_HT_MCS");
	if (str)
		fprintf(fp, "HT_MCS=%d\n", atoi(str));
	else
	{
		warning = 40;
		fprintf(fp, "HT_MCS=%d\n", 33);
	}

	//HT_TxStream
	str = nvram_safe_get("rt_HT_TxStream");
	if (str)
		fprintf(fp, "HT_TxStream=%d\n", atoi(str));
	else
	{
		warning = 41;
		fprintf(fp, "HT_TxStream=%d\n", 2);
	}

	//HT_RxStream
	str = nvram_safe_get("rt_HT_RxStream");
	if (str)
		fprintf(fp, "HT_RxStream=%d\n", atoi(str));
	else
	{
		warning = 42;
		fprintf(fp, "HT_RxStream=%d\n", 2);
	}

	//HT_PROTECT
	str = nvram_safe_get("rt_HT_PROTECT");
	if (str)
		fprintf(fp, "HT_PROTECT=%d\n", atoi(str));
	else
	{
		warning = 43;
		fprintf(fp, "HT_PROTECT=%d\n", 1);
	}

	//HT_DisallowTKIP
	fprintf(fp, "HT_DisallowTKIP=%d\n", 1);

	//WscConfStatus
	str = nvram_safe_get("rt_wsc_config_state");
	if (str)
		wsc_configure = atoi(str);
	else
	{
		warning = 44;
		wsc_configure = 0;
	}

	str2 = nvram_safe_get("x_Setting");
	if (str2)
		wsc_configure |= atoi(str2);

	if (wsc_configure == 0)
	{
		fprintf(fp, "WscConfMode=%d\n", 0);
		fprintf(fp, "WscConfStatus=%d\n", 1);
		g_wsc_configured = 0;						// AP is unconfigured
	}
	else
	{
		fprintf(fp, "WscConfMode=%d\n", 0);
		fprintf(fp, "WscConfStatus=%d\n", 2);
		g_wsc_configured = 1;						// AP is configured
	}

	
	fprintf(fp, "WscVendorPinCode=%s\n", nvram_safe_get("secret_code"));
//	fprintf(fp, "ApCliWscPinCode=%s\n", nvram_safe_get("secret_code"));	// removed from SDK 3.3.0.0

	//AccessPolicy0
	str = nvram_safe_get("rt_macmode");
	if (str)
	{
		if (!strcmp(str, "disabled"))
			fprintf(fp, "AccessPolicy0=%d\n", 0);
		else if (!strcmp(str, "allow"))
			fprintf(fp, "AccessPolicy0=%d\n", 1);
		else if (!strcmp(str, "deny"))
			fprintf(fp, "AccessPolicy0=%d\n", 2);
		else
			fprintf(fp, "AccessPolicy0=%d\n", 0);
	}
	else
	{
		warning = 46;
		fprintf(fp, "AccessPolicy0=%d\n", 0);
	}

	list[0]=0;
	list[1]=0;
	if (!nvram_match("rt_macmode", "disabled"))
	{
		num = atoi(nvram_safe_get("rt_macnum_x"));
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("rt_maclist_x", i, macbuf));
	}

	//AccessControlList0
	fprintf(fp, "AccessControlList0=%s\n", list+1);

	fprintf(fp, "AccessPolicy1=%d\n", 0);
	fprintf(fp, "AccessControlList1=\n");
	fprintf(fp, "AccessPolicy2=%d\n", 0);
	fprintf(fp, "AccessControlList2=\n");
	fprintf(fp, "AccessPolicy3=%d\n", 0);
	fprintf(fp, "AccessControlList3=\n");	
	fprintf(fp, "AccessPolicy4=%d\n", 0);
	fprintf(fp, "AccessControlList4=\n");
	fprintf(fp, "AccessPolicy5=%d\n", 0);
	fprintf(fp, "AccessControlList5=\n");
	fprintf(fp, "AccessPolicy6=%d\n", 0);
	fprintf(fp, "AccessControlList6=\n");
	fprintf(fp, "AccessPolicy7=%d\n", 0);
	fprintf(fp, "AccessControlList7=\n");

	if (!nvram_match("sw_mode_ex", "2") && !nvram_match("rt_mode_x", "0"))
	{
	//WdsEnable
	str = nvram_safe_get("rt_mode_x");
	if (str)
	{
		if (	(nvram_match("rt_auth_mode", "open") ||
			(nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes")))
		)
		{
			if (atoi(str) == 0)
				fprintf(fp, "WdsEnable=%d\n", 0);
			else if (atoi(str) == 1)
				fprintf(fp, "WdsEnable=%d\n", 2);
			else if (atoi(str) == 2)
			{
//				if (nvram_match("rt_lazywds", "1"))
				if (nvram_match("rt_wdsapply_x", "0"))
					fprintf(fp, "WdsEnable=%d\n", 4);
				else
					fprintf(fp, "WdsEnable=%d\n", 3);
			}
		}
		else
			fprintf(fp, "WdsEnable=%d\n", 0);
	}
	else
	{
		warning = 47;
		fprintf(fp, "WdsEnable=%d\n", 0);
	}

	//WdsPhyMode
	str = nvram_safe_get("rt_mode_x");
	if (str)
	{
		{
			if (atoi(nvram_safe_get("rt_gmode")) == 2)
				fprintf(fp, "WdsPhyMode=%s\n", "HTMIX");
			else if (atoi(nvram_safe_get("rt_gmode")) == 3)
				fprintf(fp, "WdsPhyMode=%s\n", "GREENFIELD");
			else
				fprintf(fp, "WdsPhyMode=\n");
		}
	}

	//WdsEncrypType
	if (nvram_match("rt_auth_mode", "open") && nvram_match("rt_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");
	else if (nvram_match("rt_auth_mode", "open") && !nvram_match("rt_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "WEP;WEP;WEP;WEP");
	else if (nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes"))
		fprintf(fp, "WdsEncrypType=%s\n", "AES;AES;AES;AES");
	else
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");

	list[0]=0;
	list[1]=0;
	if (	(nvram_match("rt_mode_x", "1") || (nvram_match("rt_mode_x", "2") && nvram_match("rt_wdsapply_x", "1"))) &&
		(nvram_match("rt_auth_mode", "open") ||
		(nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes")))
	)
	{
		num = atoi(nvram_safe_get("rt_wdsnum_x"));
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("rt_wdslist_x", i, macbuf));
	}

	//WdsList
	fprintf(fp, "WdsList=%s\n", list+1);

	//WdsKey
	if (nvram_match("rt_auth_mode", "open") && nvram_match("rt_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=\n");
		fprintf(fp, "Wds0Key=\n");
		fprintf(fp, "Wds1Key=\n");
		fprintf(fp, "Wds2Key=\n");
		fprintf(fp, "Wds3Key=\n");
	}
	else if (nvram_match("rt_auth_mode", "open") && !nvram_match("rt_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=%s;%s;%s;%s\n", nvram_safe_get("rt_key"), nvram_safe_get("rt_key"), nvram_safe_get("rt_key"), nvram_safe_get("rt_key"));
		sprintf(list, "rt_key%s", nvram_safe_get("rt_key"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get(list));
	}
	else if (nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes"))
	{
		fprintf(fp, "WdsKey=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get("rt_wpa_psk"));
	}
	} // sw_mode_ex

//	fprintf(fp, "WirelessEvent=%d\n", 0);

	//RADIUS_Server
	if (!strcmp(nvram_safe_get("rt_radius_ipaddr"), ""))
//		fprintf(fp, "RADIUS_Server=0;0;0;0;0;0;0;0\n");
		fprintf(fp, "RADIUS_Server=\n");
	else
//		fprintf(fp, "RADIUS_Server=%s;0;0;0;0;0;0;0\n", nvram_safe_get("rt_radius_ipaddr"));
		fprintf(fp, "RADIUS_Server=%s\n", nvram_safe_get("rt_radius_ipaddr"));

	//RADIUS_Port
	str = nvram_safe_get("rt_radius_port");
	if (str)
/*		
		fprintf(fp, "RADIUS_Port=%d;%d;%d;%d;%d;%d;%d;%d\n",	atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str),
									atoi(str));
*/
		fprintf(fp, "RADIUS_Port=%d\n",	atoi(str));
	else
	{
		warning = 50;
/*
		fprintf(fp, "RADIUS_Port=%d;%d;%d;%d;%d;%d;%d;%d\n", 1812, 1812, 1812, 1812, 1812, 1812, 1812, 1812);
*/
		fprintf(fp, "RADIUS_Port=%d\n", 1812);
	}

	//RADIUS_Key
	fprintf(fp, "RADIUS_Key1=%s\n", nvram_safe_get("rt_radius_key"));
	fprintf(fp, "RADIUS_Key2=\n");
	fprintf(fp, "RADIUS_Key3=\n");
	fprintf(fp, "RADIUS_Key4=\n");
	fprintf(fp, "RADIUS_Key5=\n");
	fprintf(fp, "RADIUS_Key6=\n");
	fprintf(fp, "RADIUS_Key7=\n");
	fprintf(fp, "RADIUS_Key8=\n");

	fprintf(fp, "RADIUS_Acct_Server=\n");
	fprintf(fp, "RADIUS_Acct_Port=%d\n", 1813);
	fprintf(fp, "RADIUS_Acct_Key=\n");

	//own_ip_addr
	if (flag_8021x == 1)
	{
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
		fprintf(fp, "Ethifname=\n");
		fprintf(fp, "EAPifname=%s\n", "br0");
	}
	else
	{
		fprintf(fp, "own_ip_addr=\n");
		fprintf(fp, "Ethifname=\n");
		fprintf(fp, "EAPifname=\n");
	}

	fprintf(fp, "PreAuthifname=\n");
	fprintf(fp, "session_timeout_interval=%d\n", 0);
	fprintf(fp, "idle_timeout_interval=%d\n", 0);

	//WiFiTest
	fprintf(fp, "WiFiTest=0\n");

	//TGnWifiTest
	fprintf(fp, "TGnWifiTest=0\n");

/*
	if (nvram_match("sw_mode_ex", "2") && !nvram_match("rt_sta_ssid", ""))
	{
		int flag_wep;

		fprintf(fp, "ApCliEnable=1\n");
		fprintf(fp, "ApCliSsid=%s\n", nvram_safe_get("rt_sta_ssid"));
		fprintf(fp, "ApCliBssid=\n");

		str = nvram_safe_get("rt_sta_auth_mode");
		if (str)
		{
			if (!strcmp(str, "open") && nvram_match("rt_sta_wep_x", "0"))
			{
				fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
				fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
			}
			else if (!strcmp(str, "open") || !strcmp(str, "shared"))
			{
				flag_wep = 1;
				fprintf(fp, "ApCliAuthMode=%s\n", "WEPAUTO");
				fprintf(fp, "ApCliEncrypType=%s\n", "WEP");
			}
			else if (!strcmp(str, "psk"))
			{
				if (nvram_match("rt_sta_wpa_mode", "1"))
					fprintf(fp, "ApCliAuthMode=%s\n", "WPAPSK");
				else if (nvram_match("rt_sta_wpa_mode", "2"))
					fprintf(fp, "ApCliAuthMode=%s\n", "WPA2PSK");

				//EncrypType
				if (nvram_match("rt_sta_crypto", "tkip"))
					fprintf(fp, "ApCliEncrypType=%s\n", "TKIP");
				else if (nvram_match("rt_sta_crypto", "aes"))
					fprintf(fp, "ApCliEncrypType=%s\n", "AES");

				//WPAPSK
				fprintf(fp, "ApCliWPAPSK=%s\n", nvram_safe_get("rt_sta_wpa_psk"));
			}
			else
			{
				fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
				fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
			}
		}
		else
		{
			fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
			fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
		}

		//EncrypType
		if (flag_wep)
		{
			//DefaultKeyID
			fprintf(fp, "ApCliDefaultKeyID=%s\n", nvram_safe_get("rt_sta_key"));

			//KeyType (0 -> Hex, 1->Ascii)
			fprintf(fp, "ApCliKey1Type=%s\n", nvram_safe_get("rt_sta_key_type"));
			fprintf(fp, "ApCliKey2Type=%s\n", nvram_safe_get("rt_sta_key_type"));
			fprintf(fp, "ApCliKey3Type=%s\n", nvram_safe_get("rt_sta_key_type"));
			fprintf(fp, "ApCliKey4Type=%s\n", nvram_safe_get("rt_sta_key_type"));

			//KeyStr
			fprintf(fp, "ApCliKey1Str=%s\n", nvram_safe_get("rt_sta_key1"));
			fprintf(fp, "ApCliKey2Str=%s\n", nvram_safe_get("rt_sta_key2"));
			fprintf(fp, "ApCliKey3Str=%s\n", nvram_safe_get("rt_sta_key3"));
			fprintf(fp, "ApCliKey4Str=%s\n", nvram_safe_get("rt_sta_key4"));
		}
		else
		{
			fprintf(fp, "ApCliDefaultKeyID=0\n");
			fprintf(fp, "ApCliKey1Type=0\n");
			fprintf(fp, "ApCliKey1Str=\n");
			fprintf(fp, "ApCliKey2Type=0\n");
			fprintf(fp, "ApCliKey2Str=\n");
			fprintf(fp, "ApCliKey3Type=0\n");
			fprintf(fp, "ApCliKey3Str=\n");
			fprintf(fp, "ApCliKey4Type=0\n");
			fprintf(fp, "ApCliKey4Str=\n");
		}
	}
	else
*/
	{
		fprintf(fp, "ApCliEnable=0\n");
		fprintf(fp, "ApCliSsid=\n");
		fprintf(fp, "ApCliBssid=\n");
		fprintf(fp, "ApCliAuthMode=\n");
		fprintf(fp, "ApCliEncrypType=\n");
		fprintf(fp, "ApCliWPAPSK=\n");
		fprintf(fp, "ApCliDefaultKeyID=0\n");
		fprintf(fp, "ApCliKey1Type=0\n");
		fprintf(fp, "ApCliKey1Str=\n");
		fprintf(fp, "ApCliKey2Type=0\n");
		fprintf(fp, "ApCliKey2Str=\n");
		fprintf(fp, "ApCliKey3Type=0\n");
		fprintf(fp, "ApCliKey3Str=\n");
		fprintf(fp, "ApCliKey4Type=0\n");
		fprintf(fp, "ApCliKey4Str=\n");
	}

	//RadioOn
	str = nvram_safe_get("rt_radio_x");
	if (str)
		fprintf(fp, "RadioOn=%d\n", atoi(str));
	else
	{
		warning = 52;
		fprintf(fp, "RadioOn=%d\n", 1);
	}

	fprintf(fp, "SSID=\n");
	fprintf(fp, "WPAPSK=\n");
	fprintf(fp, "Key1Str=\n");
	fprintf(fp, "Key2Str=\n");
	fprintf(fp, "Key3Str=\n");
	fprintf(fp, "Key4Str=\n");

	//IgmpSnEnable
/*
	str = nvram_safe_get("rt_IgmpSnEnable");
	if (str)
		fprintf(fp, "IgmpSnEnable=%d\n", atoi(str));
	else
	{
		warning = 1;
		fprintf(fp, "IgmpSnEnable=%d\n", 1);
	}
*/
	/*	McastPhyMode, PHY mode for Multicast frames
	 *	McastMcs, MCS for Multicast frames, ranges from 0 to 7
	 *
	 *	MODE=1, MCS=0: Legacy CCK 1Mbps
	 *	MODE=1, MCS=1: Legacy CCK 2Mbps
	 *	MODE=1, MCS=2: Legacy CCK 5.5Mbps
	 *	MODE=1, MCS=3: Legacy CCK 11Mbps
	 *	MODE=2, MCS=0: Legacy OFDM 6Mbps
	 *	MODE=2, MCS=1: Legacy OFDM 9Mbps
	 *	MODE=2, MCS=2: Legacy OFDM 12Mbps
	 *	MODE=2, MCS=3: Legacy OFDM 18Mbps
	 *	MODE=2, MCS=4: Legacy OFDM 24Mbps
	 * 	MODE=2, MCS=5: Legacy OFDM 36Mbps
	 *	MODE=2, MCS=6: Legacy OFDM 48Mbps
	 *	MODE=2, MCS=7: Legacy OFDM 54Mbps
	 *	MODE=3, MCS=15: HTMIX 130/144Mbps
	 **/
	str = nvram_safe_get("rt_mrate");
	if (str)
	{
		if (atoi(str) == 0)	// Disable
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 0);
//			fprintf(fp, "McastPhyMode=%d\n", 0);
//			fprintf(fp, "McastMcs=%d\n", 0);
		}
#if 0
		else if (atoi(str) == 1)	// Legacy CCK 1Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 0);
		}
		else if (atoi(str) == 2)	// Legacy CCK 2Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 1);
		}
		else if (atoi(str) == 3)	// Legacy CCK 5.5Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 2);
		}
		else if (atoi(str) == 4)	// Legacy CCK 11Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 1);
			fprintf(fp, "McastMcs=%d\n", 3);
		}
		else if (atoi(str) == 5)	// Legacy OFDM 6Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 0);
		}
		else if (atoi(str) == 6)	// Legacy OFDM 9Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 1);
		}
		else if (atoi(str) == 7)	// Legacy OFDM 12Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 2);
		}
		else if (atoi(str) == 8)	// Legacy OFDM 18Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 3);
		}
		else if (atoi(str) == 9)	// Legacy OFDM 24Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 4);
		}
		else if (atoi(str) == 10)	// Legacy OFDM 36Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 5);
		}
		else if (atoi(str) == 11)	// Legacy OFDM 48Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 6);
		}
		else if (atoi(str) == 12)	// Legacy OFDM 54Mbps
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 2);
			fprintf(fp, "McastMcs=%d\n", 7);
		}
//		else if (atoi(str) == 13)	// HTMIX 130/144Mbps => Auto
#endif
		else
		{
			fprintf(fp, "IgmpSnEnable=%d\n", 1);
			fprintf(fp, "McastPhyMode=%d\n", 3);
			fprintf(fp, "McastMcs=%d\n", 15);
		}
	}
	else
	{
		fprintf(fp, "IgmpSnEnable=%d\n", 0);
		warning = 53;
	}

	if (warning)
	{
		printf("%d!!!!\n", warning);
		printf("Miss some configuration, please check!!!!\n");
	}

	fclose(fp);

	return 0;
}

int
wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq)
{
	int ret = 0;
 	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
	if ((ret = ioctl(s, cmd, pwrq)) < 0)
		perror(pwrq->ifr_name);

	/* cleanup */
	close(s);
	return ret;
}

PAIR_CHANNEL_FREQ_ENTRY ChannelFreqTable[] = {
	//channel Frequency
	{1,     2412000},
	{2,     2417000},
	{3,     2422000},
	{4,     2427000},
	{5,     2432000},
	{6,     2437000},
	{7,     2442000},
	{8,     2447000},
	{9,     2452000},
	{10,    2457000},
	{11,    2462000},
	{12,    2467000},
	{13,    2472000},
	{14,    2484000},
	{34,    5170000},
	{36,    5180000},
	{38,    5190000},
	{40,    5200000},
	{42,    5210000},
	{44,    5220000},
	{46,    5230000},
	{48,    5240000},
	{52,    5260000},
	{56,    5280000},
	{60,    5300000},
	{64,    5320000},
	{100,   5500000},
	{104,   5520000},
	{108,   5540000},
	{112,   5560000},
	{116,   5580000},
	{120,   5600000},
	{124,   5620000},
	{128,   5640000},
	{132,   5660000},
	{136,   5680000},
	{140,   5700000},
	{149,   5745000},
	{153,   5765000},
	{157,   5785000},
	{161,   5805000},
};

char G_bRadio = 1;
int G_nChanFreqCount = sizeof (ChannelFreqTable) / sizeof(PAIR_CHANNEL_FREQ_ENTRY);

/************************ CONSTANTS & MACROS ************************/

/*
 * Constants fof WE-9->15
 */
#define IW15_MAX_FREQUENCIES	16
#define IW15_MAX_BITRATES	8
#define IW15_MAX_TXPOWER	8
#define IW15_MAX_ENCODING_SIZES	8
#define IW15_MAX_SPY		8
#define IW15_MAX_AP		8

/****************************** TYPES ******************************/

/*
 *	Struct iw_range up to WE-15
 */
struct	iw15_range
{
	__u32		throughput;
	__u32		min_nwid;
	__u32		max_nwid;
	__u16		num_channels;
	__u8		num_frequency;
	struct iw_freq	freq[IW15_MAX_FREQUENCIES];
	__s32		sensitivity;
	struct iw_quality	max_qual;
	__u8		num_bitrates;
	__s32		bitrate[IW15_MAX_BITRATES];
	__s32		min_rts;
	__s32		max_rts;
	__s32		min_frag;
	__s32		max_frag;
	__s32		min_pmp;
	__s32		max_pmp;
	__s32		min_pmt;
	__s32		max_pmt;
	__u16		pmp_flags;
	__u16		pmt_flags;
	__u16		pm_capa;
	__u16		encoding_size[IW15_MAX_ENCODING_SIZES];
	__u8		num_encoding_sizes;
	__u8		max_encoding_tokens;
	__u16		txpower_capa;
	__u8		num_txpower;
	__s32		txpower[IW15_MAX_TXPOWER];
	__u8		we_version_compiled;
	__u8		we_version_source;
	__u16		retry_capa;
	__u16		retry_flags;
	__u16		r_time_flags;
	__s32		min_retry;
	__s32		max_retry;
	__s32		min_r_time;
	__s32		max_r_time;
	struct iw_quality	avg_qual;
};

/*
 * Union for all the versions of iwrange.
 * Fortunately, I mostly only add fields at the end, and big-bang
 * reorganisations are few.
 */
union	iw_range_raw
{
	struct iw15_range	range15;	/* WE 9->15 */
	struct iw_range		range;		/* WE 16->current */
};

/*
 * Offsets in iw_range struct
 */
#define iwr15_off(f)	( ((char *) &(((struct iw15_range *) NULL)->f)) - \
			  (char *) NULL)
#define iwr_off(f)	( ((char *) &(((struct iw_range *) NULL)->f)) - \
			  (char *) NULL)

/* Disable runtime version warning in ralink_get_range_info() */
int iw_ignore_version_sp = 0;

/*------------------------------------------------------------------*/
/*
 * Get the range information out of the driver
 */
int
ralink_get_range_info(iwrange *	range, char* buffer, int length)
{
  union iw_range_raw *	range_raw;

  /* Point to the buffer */
  range_raw = (union iw_range_raw *) buffer;

  /* For new versions, we can check the version directly, for old versions
   * we use magic. 300 bytes is a also magic number, don't touch... */
  if (length < 300)
    {
      /* That's v10 or earlier. Ouch ! Let's make a guess...*/
      range_raw->range.we_version_compiled = 9;
    }

  /* Check how it needs to be processed */
  if (range_raw->range.we_version_compiled > 15)
    {
      /* This is our native format, that's easy... */
      /* Copy stuff at the right place, ignore extra */
      memcpy((char *) range, buffer, sizeof(iwrange));
    }
  else
    {
      /* Zero unknown fields */
      bzero((char *) range, sizeof(struct iw_range));

      /* Initial part unmoved */
      memcpy((char *) range,
	     buffer,
	     iwr15_off(num_channels));
      /* Frequencies pushed futher down towards the end */
      memcpy((char *) range + iwr_off(num_channels),
	     buffer + iwr15_off(num_channels),
	     iwr15_off(sensitivity) - iwr15_off(num_channels));
      /* This one moved up */
      memcpy((char *) range + iwr_off(sensitivity),
	     buffer + iwr15_off(sensitivity),
	     iwr15_off(num_bitrates) - iwr15_off(sensitivity));
      /* This one goes after avg_qual */
      memcpy((char *) range + iwr_off(num_bitrates),
	     buffer + iwr15_off(num_bitrates),
	     iwr15_off(min_rts) - iwr15_off(num_bitrates));
      /* Number of bitrates has changed, put it after */
      memcpy((char *) range + iwr_off(min_rts),
	     buffer + iwr15_off(min_rts),
	     iwr15_off(txpower_capa) - iwr15_off(min_rts));
      /* Added encoding_login_index, put it after */
      memcpy((char *) range + iwr_off(txpower_capa),
	     buffer + iwr15_off(txpower_capa),
	     iwr15_off(txpower) - iwr15_off(txpower_capa));
      /* Hum... That's an unexpected glitch. Bummer. */
      memcpy((char *) range + iwr_off(txpower),
	     buffer + iwr15_off(txpower),
	     iwr15_off(avg_qual) - iwr15_off(txpower));
      /* Avg qual moved up next to max_qual */
      memcpy((char *) range + iwr_off(avg_qual),
	     buffer + iwr15_off(avg_qual),
	     sizeof(struct iw_quality));
    }

  /* We are now checking much less than we used to do, because we can
   * accomodate more WE version. But, there are still cases where things
   * will break... */
  if (!iw_ignore_version_sp)
    {
      /* We don't like very old version (unfortunately kernel 2.2.X) */
      if (range->we_version_compiled <= 10)
	{
	  dbg("Warning: Driver for device %s has been compiled with an ancient version\n", WIF);
	  dbg("of Wireless Extension, while this program support version 11 and later.\n");
	  dbg("Some things may be broken...\n\n");
	}

      /* We don't like future versions of WE, because we can't cope with
       * the unknown */
      if (range->we_version_compiled > WE_MAX_VERSION)
	{
	  dbg("Warning: Driver for device %s has been compiled with version %d\n", WIF, range->we_version_compiled);
	  dbg("of Wireless Extension, while this program supports up to version %d.\n", WE_VERSION);
	  dbg("Some things may be broken...\n\n");
	}

      /* Driver version verification */
      if ((range->we_version_compiled > 10) &&
	 (range->we_version_compiled < range->we_version_source))
	{
	  dbg("Warning: Driver for device %s recommend version %d of Wireless Extension,\n", WIF, range->we_version_source);
	  dbg("but has been compiled with version %d, therefore some driver features\n", range->we_version_compiled);
	  dbg("may not be available...\n\n");
	}
      /* Note : we are only trying to catch compile difference, not source.
       * If the driver source has not been updated to the latest, it doesn't
       * matter because the new fields are set to zero */
    }

  /* Don't complain twice.
   * In theory, the test apply to each individual driver, but usually
   * all drivers are compiled from the same kernel. */
  iw_ignore_version_sp = 1;

  return (0);
}

int
getSSID()
{
	struct iwreq wrq;
	wrq.u.data.flags = 0;
	char buffer[33];
	bzero(buffer, sizeof(buffer));
	wrq.u.essid.pointer = (caddr_t) buffer;
	wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
	wrq.u.essid.flags = 0;

	if (wl_ioctl(WIF, SIOCGIWESSID, &wrq) < 0)
	{
		dbg("!!!\n");
		return 0;
	}

	if (wrq.u.essid.length>0)
	{
		unsigned char SSID[33];
		memset(SSID, 0, sizeof(SSID));
		memcpy(SSID, wrq.u.essid.pointer, wrq.u.essid.length);
		puts(SSID);
	}

	return 0;
}

int
getChannel()
{
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq1;
	struct iwreq wrq2;
	char ch_str[3];

	if (wl_ioctl(WIF, SIOCGIWFREQ, &wrq1) < 0)
		return 0;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(WIF, SIOCGIWRANGE, &wrq2) < 0)
		return 0;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return 0;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return 0;
	}

	memset(ch_str, 0, sizeof(ch_str));
	sprintf(ch_str, "%d", channel);
	puts(ch_str);
	return 0;
}

int
getChannel_2G()
{
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq1;
	struct iwreq wrq2;
	char ch_str[3];

	if (wl_ioctl(WIF2G, SIOCGIWFREQ, &wrq1) < 0)
		return 0;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(WIF2G, SIOCGIWRANGE, &wrq2) < 0)
		return 0;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return 0;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return 0;
	}

	memset(ch_str, 0, sizeof(ch_str));
	sprintf(ch_str, "%d", channel);
	puts(ch_str);
	return 0;
}

int
getSiteSurvey()
{
	int retval = 0, i = 0, apCount = 0;
	char data[8192];
	char ssid_str[256];
	char header[128];
	struct iwreq wrq;
	SSA *ssap;

	memset(data, 0x00, 255);
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data; 
	wrq.u.data.flags = 0; 

	spinlock_lock(SPINLOCK_SiteSurvey);
	if (wl_ioctl(WIF, RTPRIV_IOCTL_SET, &wrq) < 0)
	{
		spinlock_unlock(0);

		dbg("Site Survey fails\n");
		return 0;
	}
	spinlock_unlock(SPINLOCK_SiteSurvey);

	dbg("Please wait");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".\n\n");

	memset(data, 0, 8192);
	strcpy(data, "");
	wrq.u.data.length = 8192;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(WIF, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return 0;
	}

	memset(header, 0, sizeof(header));
	//sprintf(header, "%-3s%-33s%-18s%-8s%-15s%-9s%-8s%-2s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT");
	sprintf(header, "%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode");
	dbg("\n%s", header);

	if (wrq.u.data.length > 0)
	{
		ssap=(SSA *)(wrq.u.data.pointer+strlen(header)+1);
		int len = strlen(wrq.u.data.pointer+strlen(header))-1;
		char *sp, *op;
 		op = sp = wrq.u.data.pointer+strlen(header)+1;

		while (*sp && ((len - (sp-op)) >= 0))
		{
			ssap->SiteSurvey[i].channel[3] = '\0';
			ssap->SiteSurvey[i].ssid[32] = '\0';
			ssap->SiteSurvey[i].bssid[17] = '\0';
			ssap->SiteSurvey[i].encryption[8] = '\0';
			ssap->SiteSurvey[i].authmode[15] = '\0';
			ssap->SiteSurvey[i].signal[8] = '\0';
			ssap->SiteSurvey[i].wmode[7] = '\0';
//			ssap->SiteSurvey[i].bsstype[2] = '\0';
//			ssap->SiteSurvey[i].centralchannel[2] = '\0';

			sp+=strlen(header);
			apCount=++i;
		}

		for (i = 0; i < apCount; i++)
		{
			dbg("%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n",
				ssap->SiteSurvey[i].channel,
				(char*)ssap->SiteSurvey[i].ssid,
				ssap->SiteSurvey[i].bssid,
				ssap->SiteSurvey[i].encryption,
				ssap->SiteSurvey[i].authmode,
				ssap->SiteSurvey[i].signal,
				ssap->SiteSurvey[i].wmode
//				ssap->SiteSurvey[i].bsstype,
//				ssap->SiteSurvey[i].centralchannel
			);
		}
		dbg("\n");
	}

	return 0;
}

int
getSiteSurvey_2G()
{
	int retval = 0, i = 0, apCount = 0;
	char data[8192];
	char ssid_str[256];
	char header[128];
	struct iwreq wrq;
	SSA *ssap;

	memset(data, 0x00, 255);
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data; 
	wrq.u.data.flags = 0; 

	spinlock_lock(SPINLOCK_SiteSurvey);
	if (wl_ioctl(WIF2G, RTPRIV_IOCTL_SET, &wrq) < 0)
	{
		spinlock_unlock(0);

		dbg("Site Survey fails\n");
		return 0;
	}
	spinlock_unlock(SPINLOCK_SiteSurvey);

	dbg("Please wait");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".\n\n");

	memset(data, 0, 8192);
	strcpy(data, "");
	wrq.u.data.length = 8192;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(WIF2G, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return 0;
	}

	memset(header, 0, sizeof(header));
	//sprintf(header, "%-3s%-33s%-18s%-8s%-15s%-9s%-8s%-2s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT");
	sprintf(header, "%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode");
	dbg("\n%s", header);

	if (wrq.u.data.length > 0)
	{
		ssap=(SSA *)(wrq.u.data.pointer+strlen(header)+1);
		int len = strlen(wrq.u.data.pointer+strlen(header))-1;
		char *sp, *op;
 		op = sp = wrq.u.data.pointer+strlen(header)+1;

		while (*sp && ((len - (sp-op)) >= 0))
		{
			ssap->SiteSurvey[i].channel[3] = '\0';
			ssap->SiteSurvey[i].ssid[32] = '\0';
			ssap->SiteSurvey[i].bssid[17] = '\0';
			ssap->SiteSurvey[i].encryption[8] = '\0';
			ssap->SiteSurvey[i].authmode[15] = '\0';
			ssap->SiteSurvey[i].signal[8] = '\0';
			ssap->SiteSurvey[i].wmode[7] = '\0';
//			ssap->SiteSurvey[i].bsstype[2] = '\0';
//			ssap->SiteSurvey[i].centralchannel[2] = '\0';

			sp+=strlen(header);
			apCount=++i;
		}

		for (i = 0; i < apCount; i++)
		{
			dbg("%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n",
				ssap->SiteSurvey[i].channel,
				(char*)ssap->SiteSurvey[i].ssid,
				ssap->SiteSurvey[i].bssid,
				ssap->SiteSurvey[i].encryption,
				ssap->SiteSurvey[i].authmode,
				ssap->SiteSurvey[i].signal,
				ssap->SiteSurvey[i].wmode
//				ssap->SiteSurvey[i].bsstype,
//				ssap->SiteSurvey[i].centralchannel
			);
		}
		dbg("\n");
	}

	return 0;
}

int getBSSID()	// get AP's BSSID
{
	unsigned char data[MACSIZE];
	char macaddr[18];
	struct iwreq wrq;

	memset(data, 0x00, MACSIZE);
	wrq.u.data.length = MACSIZE;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = OID_802_11_BSSID;

	if (wl_ioctl(WIF, RT_PRIV_IOCTL, &wrq) < 0)
	{
		dbg("errors in getting bssid!\n");
		return -1;
	}
	else
	{
		ether_etoa(data, macaddr);
		puts(macaddr);
		return 0;
	}
}

int
get_channel()
{
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq1;
	struct iwreq wrq2;

	if (wl_ioctl(WIF, SIOCGIWFREQ, &wrq1) < 0)
		return 0;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(WIF, SIOCGIWRANGE, &wrq2) < 0)
		return 0;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return 0;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return 0;
	}

	return channel;
}

int
get_channel_2G()
{
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq1;
	struct iwreq wrq2;

	if (wl_ioctl(WIF2G, SIOCGIWFREQ, &wrq1) < 0)
		return 0;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(WIF2G, SIOCGIWRANGE, &wrq2) < 0)
		return 0;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return 0;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return 0;
	}

	return channel;
}

int
asuscfe(const char *PwqV, const char *IF)
{
	if (strcmp(PwqV, "stat") == 0)
	{
		eval("iwpriv", IF, "stat");
	}
	else if (strstr(PwqV, "=") && strstr(PwqV, "=")!=PwqV)
	{
		eval("iwpriv", IF, "set", PwqV);
		puts("success");
	}
	return 0;
}

int
need_to_start_wps_5g()
{
	if (	nvram_match("wl_auth_mode", "shared") ||
		nvram_match("wl_auth_mode", "wpa") ||
		nvram_match("wl_auth_mode", "wpa2") ||
		nvram_match("wl_auth_mode", "radius") ||
		nvram_match("wl_radio_x", "0") ||
		nvram_match("sw_mode_ex", "3")	)
		return 0;

	return 1;
}

int
need_to_start_wps_2g()
{
	if (	nvram_match("rt_auth_mode", "shared") ||
		nvram_match("rt_auth_mode", "wpa") ||
		nvram_match("rt_auth_mode", "wpa2") ||
		nvram_match("rt_auth_mode", "radius") ||
		nvram_match("rt_radio_x", "0") ||
		nvram_match("sw_mode_ex", "3")	)
		return 0;

	return 1;
}

#if defined (W7_LOGO) || defined (WIFI_LOGO)
int
wps_pin(const char *pincode)
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return 0;

	if (!need_to_start_wps_5g()) return 0;

	if (!pincode || !(*pincode))
		return 0;

	if (strcmp(pincode, "0") && pinvalidate(pincode))
	{
		dbG("invalid pincode!\n");
		return 0;
	}

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}

	dbg("start wsc\n");

	doSystem("iwpriv %s set WscConfMode=%d", WIF, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);		// Enrollee + Proxy + Registrar

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);

	dbg("WPS: PIN\n");					// PIN method
	doSystem("iwpriv %s set WscMode=1", WIF);

	if (!strcmp(pincode, "0"))
	{
		g_isEnrollee = 1;
		doSystem("iwpriv %s set WscPinCode=0", WIF);
		doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);	// Trigger WPS AP to do simple config with WPS Client
	}
	else
	{
		doSystem("iwpriv %s set WscPinCode=%s", WIF, pincode);
		doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);	// Trigger WPS AP to do simple config with WPS Client
	}

	return 0;
}

int
wps_pin_2g(const char *pincode)
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return 0;

	if (!need_to_start_wps_2g()) return 0;

	if (!pincode || !(*pincode))
		return 0;

	if (strcmp(pincode, "0") && pinvalidate(pincode))
	{
		dbG("invalid pincode!\n");
		return 0;
	}

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	dbg("start wsc\n");

	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);		// Enrollee + Proxy + Registrar

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);

	dbg("WPS: PIN\n");					// PIN method
	doSystem("iwpriv %s set WscMode=1", WIF2G);

	if (!strcmp(pincode, "0"))
	{
		g_isEnrollee = 1;
		doSystem("iwpriv %s set WscPinCode=0", WIF2G);
		doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);	// Trigger WPS AP to do simple config with WPS Client
	}
	else
	{
		doSystem("iwpriv %s set WscPinCode=%s", WIF2G, pincode);
		doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);	// Trigger WPS AP to do simple config with WPS Client
	}

	return 0;
}

int
wps_pbc()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return 0;

	if (!need_to_start_wps_5g()) return 0;

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}

	dbg("start wsc\n");

	doSystem("iwpriv %s set WscConfMode=%d", WIF, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);		// Enrollee + Proxy + Registrar

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);

//	dbg("WPS: PBC\n");
	g_isEnrollee = 1;
	doSystem("iwpriv %s set WscMode=%d", WIF, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client

	return 0;
}

int
wps_pbc_2g()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return 0;

	if (!need_to_start_wps_2g()) return 0;

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	dbg("start wsc\n");

	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);		// Enrollee + Proxy + Registrar

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);

//	dbg("WPS: PBC\n");
	g_isEnrollee = 1;
	doSystem("iwpriv %s set WscMode=", WIF2G);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client

	return 0;
}

int
wps_pbc_both()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return 0;

	if (!need_to_start_wps_5g() || !need_to_start_wps_2g()) return 0;

	system("route delete 239.255.255.250 1>/dev/null 2>&1");

	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}

	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	dbg("start wsc\n");

	doSystem("iwpriv %s set WscConfMode=%d", WIF, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);		// Enrollee + Proxy + Registrar

	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);		// Enrollee + Proxy + Registrar

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);

//	dbg("WPS: PBC\n");
	g_isEnrollee = 1;
	doSystem("iwpriv %s set WscMode=%d", WIF, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client

	doSystem("iwpriv %s set WscMode=", WIF2G);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client

	return 0;
}
#else
int
wps_pin(const char *pincode)
{
	if (!need_to_start_wps_5g()) return 0;

	if (!pincode || !(*pincode))
		return 0;

	if (strcmp(pincode, "0") && pinvalidate(pincode))
	{
		dbG("invalid pincode!\n");
		return 0;
	}

//	dbg("WPS: PIN\n");
	doSystem("iwpriv %s set WscMode=1", WIF);

	if (!strcmp(pincode, "0"))
		doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);	// Trigger WPS AP to do simple config with WPS Client
	else
		doSystem("iwpriv %s set WscPinCode=%s", WIF, pincode);

	return 0;
}

int
wps_pin_2g(const char *pincode)
{
	if (!need_to_start_wps_2g()) return 0;

	if (!pincode || !(*pincode))
		return 0;

	if (strcmp(pincode, "0") && pinvalidate(pincode))
	{
		dbG("invalid pincode!\n");
		return 0;
	}

//	dbg("WPS: PIN\n");
	doSystem("iwpriv %s set WscMode=1", WIF2G);			// PIN method

	if (!strcmp(pincode, "0"))
		doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);	// Trigger WPS AP to do simple config with WPS Client
	else
		doSystem("iwpriv %s set WscPinCode=%s", WIF2G, pincode);

	return 0;
}

int
wps_pbc()
{
	if (!need_to_start_wps_5g()) return 0;

//	dbg("WPS: PBC\n");
	g_isEnrollee = 1;
	doSystem("iwpriv %s set WscMode=%d", WIF, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client

	return 0;
}

int
wps_pbc_2g()
{
	if (!need_to_start_wps_2g()) return 0;

//	dbg("WPS: PBC\n");
	g_isEnrollee = 1;
	doSystem("iwpriv %s set WscMode=2", WIF2G);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client

	return 0;
}

int
wps_pbc_both()
{
	if (!need_to_start_wps_5g() || !need_to_start_wps_2g()) return 0;

//	dbg("WPS: PBC\n");
	g_isEnrollee = 1;

	doSystem("iwpriv %s set WscMode=%d", WIF, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client

	doSystem("iwpriv %s set WscMode=%d", WIF2G, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client

	return 0;
}
#endif

void
wps_oob()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_5g()) return;

//	nvram_set("x_Setting", "0");
	nvram_set("wsc_config_state", "0");
#if defined (W7_LOGO) || defined (WIFI_LOGO)
	nvram_set("wps_mode", "2");
	nvram_set("wl_ssid", "ASUS_5G");
	nvram_set("wl_auth_mode", "open");
	nvram_set("wl_wep_x", "0");
	nvram_set("wl_wpa_mode", "0");
	nvram_set("wl_crypto", "tkip");
	nvram_set("wl_key", "1");
	nvram_set("wl_phrase_x", "");
	nvram_set("wl_key1", "");
	nvram_set("wl_key2", "");
	nvram_set("wl_key3", "");
	nvram_set("wl_key4", "");
	nvram_set("wl_wpa_psk", "");
#else
/*
	nvram_set("wl_ssid", "ASUSInitialAP");
	nvram_set("wl_auth_mode", "psk");
	nvram_set("wl_wep_x", "0");
	nvram_set("wl_wpa_mode", "1");
	nvram_set("wl_crypto", "tkip+aes");
	nvram_set("wl_key", "2");
	nvram_set("wl_wpa_psk", "12345678");
*/
	nvram_set("wl_ssid", "ASUS_5G");
	nvram_set("wl_auth_mode", "open");
	nvram_set("wl_wep_x", "0");
	nvram_set("wl_wpa_mode", "0");
	nvram_set("wl_crypto", "tkip");
	nvram_set("wl_key", "1");
	nvram_set("wl_phrase_x", "");
	nvram_set("wl_key1", "");
	nvram_set("wl_key2", "");
	nvram_set("wl_key3", "");
	nvram_set("wl_key4", "");
	nvram_set("wl_wpa_psk", "");
#endif
	nvram_commit_safe();

#if defined (W7_LOGO) || defined (WIFI_LOGO)
	doSystem("iwpriv %s set AuthMode=%s", WIF, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);
	if (strlen(nvram_safe_get("wl_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF, nvram_safe_get("wl_key1"));
	if (strlen(nvram_safe_get("wl_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF, nvram_safe_get("wl_key2"));
	if (strlen(nvram_safe_get("wl_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF, nvram_safe_get("wl_key3"));
	if (strlen(nvram_safe_get("wl_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF, nvram_safe_get("wl_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF, nvram_safe_get("wl_key"));
	doSystem("iwpriv %s set SSID=%s", WIF, nvram_safe_get("wl_ssid"));

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}		

	doSystem("iwpriv %s set WscConfMode=%d", WIF, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);		// Enrollee + Proxy + Registrar
	doSystem("iwpriv %s set WscConfStatus=%d", WIF, 1);		// AP is unconfigured
	g_wsc_configured = 0;

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);

	doSystem("iwpriv %s set WscMode=1", WIF);			// PIN method
//	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client
#else
/*
	doSystem("iwpriv %s set SSID=ASUSInitialAP", WIF);
	doSystem("iwpriv %s set AuthMode=WPAPSK", WIF);
	doSystem("iwpriv %s set EncrypType=TKIPAES", WIF);
	doSystem("iwpriv %s set WPAPSK=12345678", WIF);
	doSystem("iwpriv %s set DefaultKeyID=2", WIF);
*/
	doSystem("iwpriv %s set AuthMode=%s", WIF, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);
	if (strlen(nvram_safe_get("wl_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF, nvram_safe_get("wl_key1"));
	if (strlen(nvram_safe_get("wl_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF, nvram_safe_get("wl_key2"));
	if (strlen(nvram_safe_get("wl_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF, nvram_safe_get("wl_key3"));
	if (strlen(nvram_safe_get("wl_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF, nvram_safe_get("wl_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF, nvram_safe_get("wl_key"));
	doSystem("iwpriv %s set SSID=%s", WIF, nvram_safe_get("wl_ssid"));
	doSystem("iwpriv %s set WscConfStatus=%d", WIF, 1);		// AP is unconfigured
#endif
	g_isEnrollee = 0;
}

void
wps_oob_2g()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_2g()) return;

//	nvram_set("x_Setting", "0");
	nvram_set("rt_wsc_config_state", "0");
#if defined (W7_LOGO) || defined (WIFI_LOGO)
	nvram_set("wps_mode", "2");
	nvram_set("rt_ssid", "ASUS");
	nvram_set("rt_auth_mode", "open");
	nvram_set("rt_wep_x", "0");
	nvram_set("rt_wpa_mode", "0");
	nvram_set("rt_crypto", "tkip");
	nvram_set("rt_key", "1");
	nvram_set("rt_phrase_x", "");
	nvram_set("rt_key1", "");
	nvram_set("rt_key2", "");
	nvram_set("rt_key3", "");
	nvram_set("rt_key4", "");
	nvram_set("rt_wpa_psk", "");
#else
/*
	nvram_set("rt_ssid", "ASUSInitialAP");
	nvram_set("rt_auth_mode", "psk");
	nvram_set("rt_wep_x", "0");
	nvram_set("rt_wpa_mode", "1");
	nvram_set("rt_crypto", "tkip+aes");
	nvram_set("rt_key", "2");
	nvram_set("rt_wpa_psk", "12345678");
*/
	nvram_set("rt_ssid", "ASUS");
	nvram_set("rt_auth_mode", "open");
	nvram_set("rt_wep_x", "0");
	nvram_set("rt_wpa_mode", "0");
	nvram_set("rt_crypto", "tkip");
	nvram_set("rt_key", "1");
	nvram_set("rt_phrase_x", "");
	nvram_set("rt_key1", "");
	nvram_set("rt_key2", "");
	nvram_set("rt_key3", "");
	nvram_set("rt_key4", "");
	nvram_set("rt_wpa_psk", "");
#endif
	nvram_commit_safe();

#if defined (W7_LOGO) || defined (WIFI_LOGO)
	doSystem("iwpriv %s set AuthMode=%s", WIF2G, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF2G, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);
	if (strlen(nvram_safe_get("rt_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF2G, nvram_safe_get("rt_key1"));
	if (strlen(nvram_safe_get("rt_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF2G, nvram_safe_get("rt_key2"));
	if (strlen(nvram_safe_get("rt_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF2G, nvram_safe_get("rt_key3"));
	if (strlen(nvram_safe_get("rt_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF2G, nvram_safe_get("rt_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF2G, nvram_safe_get("rt_key"));
	doSystem("iwpriv %s set SSID=%s", WIF2G, nvram_safe_get("rt_ssid"));

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);		// Enrollee + Proxy + Registrar
	doSystem("iwpriv %s set WscConfStatus=%d", WIF2G, 1);		// AP is unconfigured
	g_wsc_configured = 0;

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);

	doSystem("iwpriv %s set WscMode=1", WIF2G);			// PIN method
//	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client
#else
/*
	doSystem("iwpriv %s set SSID=ASUSInitialAP", WIF2G);
	doSystem("iwpriv %s set AuthMode=WPAPSK", WIF2G);
	doSystem("iwpriv %s set EncrypType=TKIPAES", WIF2G);
	doSystem("iwpriv %s set WPAPSK=12345678", WIF2G);
	doSystem("iwpriv %s set DefaultKeyID=2", WIF2G);
*/
	doSystem("iwpriv %s set AuthMode=%s", WIF2G, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF2G, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);
	if (strlen(nvram_safe_get("rt_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF2G, nvram_safe_get("rt_key1"));
	if (strlen(nvram_safe_get("rt_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF2G, nvram_safe_get("rt_key2"));
	if (strlen(nvram_safe_get("rt_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF2G, nvram_safe_get("rt_key3"));
	if (strlen(nvram_safe_get("rt_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF2G, nvram_safe_get("rt_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF2G, nvram_safe_get("rt_key"));
	doSystem("iwpriv %s set SSID=%s", WIF2G, nvram_safe_get("rt_ssid"));
	doSystem("iwpriv %s set WscConfStatus=%d", WIF2G, 1);		// AP is unconfigured
#endif
	g_isEnrollee = 0;
}

void
wps_oob_both()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_5g() || !need_to_start_wps_2g()) return;

//	nvram_set("x_Setting", "0");
	nvram_set("wsc_config_state", "0");
	nvram_set("rt_wsc_config_state", "0");
#if defined (W7_LOGO) || defined (WIFI_LOGO)
	nvram_set("wps_mode", "2");

	nvram_set("wl_ssid", "ASUS_5G");
	nvram_set("wl_auth_mode", "open");
	nvram_set("wl_wep_x", "0");
	nvram_set("wl_wpa_mode", "0");
	nvram_set("wl_crypto", "tkip");
	nvram_set("wl_key", "1");
	nvram_set("wl_phrase_x", "");
	nvram_set("wl_key1", "");
	nvram_set("wl_key2", "");
	nvram_set("wl_key3", "");
	nvram_set("wl_key4", "");
	nvram_set("wl_wpa_psk", "");

	nvram_set("rt_ssid", "ASUS");
	nvram_set("rt_auth_mode", "open");
	nvram_set("rt_wep_x", "0");
	nvram_set("rt_wpa_mode", "0");
	nvram_set("rt_crypto", "tkip");
	nvram_set("rt_key", "1");
	nvram_set("rt_phrase_x", "");
	nvram_set("rt_key1", "");
	nvram_set("rt_key2", "");
	nvram_set("rt_key3", "");
	nvram_set("rt_key4", "");
	nvram_set("rt_wpa_psk", "");
#else
/*
	nvram_set("wl_ssid", "ASUSInitialAP");
	nvram_set("wl_auth_mode", "psk");
	nvram_set("wl_wep_x", "0");
	nvram_set("wl_wpa_mode", "1");
	nvram_set("wl_crypto", "tkip+aes");
	nvram_set("wl_key", "2");
	nvram_set("wl_wpa_psk", "12345678");
*/
	nvram_set("wl_ssid", "ASUS_5G");
	nvram_set("wl_auth_mode", "open");
	nvram_set("wl_wep_x", "0");
	nvram_set("wl_wpa_mode", "0");
	nvram_set("wl_crypto", "tkip");
	nvram_set("wl_key", "1");
	nvram_set("wl_phrase_x", "");
	nvram_set("wl_key1", "");
	nvram_set("wl_key2", "");
	nvram_set("wl_key3", "");
	nvram_set("wl_key4", "");
	nvram_set("wl_wpa_psk", "");

	nvram_set("rt_ssid", "ASUS");
	nvram_set("rt_auth_mode", "open");
	nvram_set("rt_wep_x", "0");
	nvram_set("rt_wpa_mode", "0");
	nvram_set("rt_crypto", "tkip");
	nvram_set("rt_key", "1");
	nvram_set("rt_phrase_x", "");
	nvram_set("rt_key1", "");
	nvram_set("rt_key2", "");
	nvram_set("rt_key3", "");
	nvram_set("rt_key4", "");
	nvram_set("rt_wpa_psk", "");
#endif
	nvram_commit_safe();

#if defined (W7_LOGO) || defined (WIFI_LOGO)
	doSystem("iwpriv %s set AuthMode=%s", WIF, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);
	if (strlen(nvram_safe_get("wl_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF, nvram_safe_get("wl_key1"));
	if (strlen(nvram_safe_get("wl_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF, nvram_safe_get("wl_key2"));
	if (strlen(nvram_safe_get("wl_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF, nvram_safe_get("wl_key3"));
	if (strlen(nvram_safe_get("wl_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF, nvram_safe_get("wl_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF, nvram_safe_get("wl_key"));
	doSystem("iwpriv %s set SSID=%s", WIF, nvram_safe_get("wl_ssid"));

	doSystem("iwpriv %s set AuthMode=%s", WIF2G, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF2G, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);
	if (strlen(nvram_safe_get("rt_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF2G, nvram_safe_get("rt_key1"));
	if (strlen(nvram_safe_get("rt_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF2G, nvram_safe_get("rt_key2"));
	if (strlen(nvram_safe_get("rt_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF2G, nvram_safe_get("rt_key3"));
	if (strlen(nvram_safe_get("rt_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF2G, nvram_safe_get("rt_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF2G, nvram_safe_get("rt_key"));
	doSystem("iwpriv %s set SSID=%s", WIF2G, nvram_safe_get("rt_ssid"));

	system("route delete 239.255.255.250 1>/dev/null 2>&1");

	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}

	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	doSystem("iwpriv %s set WscConfMode=%d", WIF, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);		// Enrollee + Proxy + Registrar
	doSystem("iwpriv %s set WscConfStatus=%d", WIF, 1);		// AP is unconfigured

	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);		// Enrollee + Proxy + Registrar
	doSystem("iwpriv %s set WscConfStatus=%d", WIF2G, 1);		// AP is unconfigured

	g_wsc_configured = 0;

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));

	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);

	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);

	doSystem("iwpriv %s set WscMode=1", WIF);			// PIN method

	doSystem("iwpriv %s set WscMode=1", WIF2G);			// PIN method

//	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client
#else
/*
	doSystem("iwpriv %s set SSID=ASUSInitialAP", WIF);
	doSystem("iwpriv %s set AuthMode=WPAPSK", WIF);
	doSystem("iwpriv %s set EncrypType=TKIPAES", WIF);
	doSystem("iwpriv %s set WPAPSK=12345678", WIF);
	doSystem("iwpriv %s set DefaultKeyID=2", WIF);
*/
	doSystem("iwpriv %s set AuthMode=%s", WIF, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);
	if (strlen(nvram_safe_get("wl_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF, nvram_safe_get("wl_key1"));
	if (strlen(nvram_safe_get("wl_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF, nvram_safe_get("wl_key2"));
	if (strlen(nvram_safe_get("wl_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF, nvram_safe_get("wl_key3"));
	if (strlen(nvram_safe_get("wl_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF, nvram_safe_get("wl_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF, nvram_safe_get("wl_key"));
	doSystem("iwpriv %s set SSID=%s", WIF, nvram_safe_get("wl_ssid"));
	doSystem("iwpriv %s set WscConfStatus=%d", WIF, 1);		// AP is unconfigured

	doSystem("iwpriv %s set AuthMode=%s", WIF2G, "OPEN");
	doSystem("iwpriv %s set EncrypType=%s", WIF2G, "NONE");
	doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);
	if (strlen(nvram_safe_get("rt_key1")))
	doSystem("iwpriv %s set Key1=%s", WIF2G, nvram_safe_get("rt_key1"));
	if (strlen(nvram_safe_get("rt_key2")))
	doSystem("iwpriv %s set Key2=%s", WIF2G, nvram_safe_get("rt_key2"));
	if (strlen(nvram_safe_get("rt_key3")))
	doSystem("iwpriv %s set Key3=%s", WIF2G, nvram_safe_get("rt_key3"));
	if (strlen(nvram_safe_get("rt_key4")))
	doSystem("iwpriv %s set Key4=%s", WIF2G, nvram_safe_get("rt_key4"));
	doSystem("iwpriv %s set DefaultKeyID=%s", WIF2G, nvram_safe_get("rt_key"));
	doSystem("iwpriv %s set SSID=%s", WIF2G, nvram_safe_get("rt_ssid"));
	doSystem("iwpriv %s set WscConfStatus=%d", WIF2G, 1);		// AP is unconfigured
#endif
	g_isEnrollee = 0;
}

void
start_wsc()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_5g()) return;

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}

	dbg("start wsc\n");

	doSystem("iwpriv %s set WscConfMode=%d", WIF, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);		// Enrollee + Proxy + Registrar

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);

#if defined (W7_LOGO) || defined (WIFI_LOGO)
/*
	if (nvram_match("wsc_config_state", "1"))
		doSystem("iwpriv %s set WscConfStatus=%d", WIF, 2);	// AP is configured
	else
		doSystem("iwpriv %s set WscConfStatus=%d", WIF, 1);
*/
	if (nvram_match("wps_mode", "1"))
	{
		dbg("WPS: PIN\n");				// PIN method
		doSystem("iwpriv %s set WscMode=1", WIF);
	}
	else
	{
		dbg("WPS: PBC\n");				// PBC method
		doSystem("iwpriv %s set WscMode=2", WIF);
	}

	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client
#endif
	g_isEnrollee = 0;
}

void
start_wsc_2g()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_2g()) return;

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	dbg("start wsc\n");

	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 0);		// WPS disabled
	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);		// Enrollee + Proxy + Registrar

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);

#if defined (W7_LOGO) || defined (WIFI_LOGO)
/*
	if (nvram_match("rt_wsc_config_state", "1"))
		doSystem("iwpriv %s set WscConfStatus=%d", WIF2G, 2);	// AP is configured
	else
		doSystem("iwpriv %s set WscConfStatus=%d", WIF2G, 1);
*/
	if (nvram_match("wps_mode", "1"))
	{
		dbg("WPS: PIN\n");				// PIN method
		doSystem("iwpriv %s set WscMode=1", WIF2G);
	}
	else
	{
		dbg("WPS: PBC\n");				// PBC method
		doSystem("iwpriv %s set WscMode=2", WIF2G);
	}

	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client
#endif
	g_isEnrollee = 0;
}

void
start_wsc_pbc()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_5g()) return;

	dbg("start wsc\n");
	if (nvram_match("wps_enable", "0"))
	{
		system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
		if (check_if_file_exist(WSCD_PIDFILE_5G))
		{
			doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
			unlink(WSCD_PIDFILE_5G);
		}
		
		char str_lan_ipaddr[16];
		if (nvram_match("lan_ipaddr_t", ""))
			strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
		else
			strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
		doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);
		doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);	// Enrollee + Proxy + Registrar
	}

	dbg("WPS: PBC\n");
	doSystem("iwpriv %s set WscMode=%d", WIF, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client

	nvram_set("wps_enable", "1");
	nvram_set("wps_mode", "2");
}

void
start_wsc_pbc_2g()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_2g()) return;

	dbg("start wsc\n");
	if (nvram_match("wps_enable", "0"))
	{
		system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
		if (check_if_file_exist(WSCD_PIDFILE_2G))
		{
			doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
			doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
		}
		
		char str_lan_ipaddr[16];
		if (nvram_match("lan_ipaddr_t", ""))
			strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
		else
			strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
		doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);
		doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);	// Enrollee + Proxy + Registrar
	}

	dbg("WPS: PBC\n");
	doSystem("iwpriv %s set WscMode=2", WIF2G);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client

	nvram_set("wps_enable", "1");
	nvram_set("wps_mode", "2");
}

void
start_wsc_pbc_both()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
		return;

	if (!need_to_start_wps_5g() || !need_to_start_wps_2g()) return;

	dbg("start wsc\n");
//	if (nvram_match("wps_enable", "0"))
	{
		system("route delete 239.255.255.250 1>/dev/null 2>&1");

		if (check_if_file_exist(WSCD_PIDFILE_5G))
		{
			doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
			unlink(WSCD_PIDFILE_5G);
		}

		if (check_if_file_exist(WSCD_PIDFILE_2G))
		{
			doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
			doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
		}

		system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
		char str_lan_ipaddr[16];
		if (nvram_match("lan_ipaddr_t", ""))
			strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
		else
			strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));

		doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);
		doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);	// Enrollee + Proxy + Registrar

		doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);
		doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);	// Enrollee + Proxy + Registrar
	}

	dbg("WPS: PBC\n");

	doSystem("iwpriv %s set WscMode=%d", WIF, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF, 1);		// Trigger WPS AP to do simple config with WPS Client

	doSystem("iwpriv %s set WscMode=%d", WIF2G, 2);			// PBC method
	doSystem("iwpriv %s set WscGetConf=%d", WIF2G, 1);		// Trigger WPS AP to do simple config with WPS Client

	nvram_set("wps_enable", "1");
	nvram_set("wps_mode", "2");
}

void
start_wsc_pin_enrollee()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
	{
		nvram_set("wps_enable", "0");
		nvram_set("wps_start_flag", "0");
		return;
	}

	if (!need_to_start_wps_5g()) return;

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}

	dbg("start wsc\n");

	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF);
	doSystem("iwpriv %s set WscConfMode=%d", WIF, 7);		// Enrollee + Proxy + Registrar	

	dbg("WPS: PIN\n");
	doSystem("iwpriv %s set WscMode=1", WIF);

	nvram_set("wps_start_flag", "1");
}

void
start_wsc_pin_enrollee_2g()
{
	if (nvram_match("lan_ipaddr_t", "") && nvram_match("lan_ipaddr", ""))
	{
		nvram_set("wps_enable", "0");
		nvram_set("wps_start_flag", "0");
		return;
	}

	if (!need_to_start_wps_2g()) return;

	system("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	dbg("start wsc\n");

	char str_lan_ipaddr[16];
	if (nvram_match("lan_ipaddr_t", ""))
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(str_lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	doSystem("wscd -m 1 -a %s -i %s &", str_lan_ipaddr, WIF2G);
	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 7);		// Enrollee + Proxy + Registrar

	dbg("WPS: PIN\n");
	doSystem("iwpriv %s set WscMode=1", WIF2G);			// PIN method

	nvram_set("wps_start_flag", "1");
}

void
stop_wsc()
{
	if (!need_to_start_wps_5g()) return;

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_5G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_5G);
		unlink(WSCD_PIDFILE_5G);
	}

	doSystem("iwpriv %s set WscConfMode=%d", WIF, 0);		// WPS disabled
	doSystem("iwpriv %s set WscStatus=%d", WIF, 0);			// Not Used
}

void
stop_wsc_2g()
{
	if (!need_to_start_wps_2g()) return;

	system("route delete 239.255.255.250 1>/dev/null 2>&1");
	if (check_if_file_exist(WSCD_PIDFILE_2G))
	{
		doSystem("kill -9 `cat %s`", WSCD_PIDFILE_2G);
		doSystem("rm -f /var/run/wscd.pid.%s", WIF2G);
	}

	doSystem("iwpriv %s set WscConfMode=%d", WIF2G, 0);		// WPS disabled
	doSystem("iwpriv %s set WscStatus=%d", WIF2G, 0);		// Not Used
}

int getWscStatus()
{
	int socket_id;
	int data = 0;
	struct iwreq wrq;
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;

	if (wl_ioctl(WIF, RT_PRIV_IOCTL, &wrq) < 0)
		dbg("errors in getting WSC status\n");

	return data;
}

int getWscStatus_2g()
{
	int socket_id;
	int data = 0;
	struct iwreq wrq;
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;

	if (wl_ioctl(WIF2G, RT_PRIV_IOCTL, &wrq) < 0)
		dbg("errors in getting WSC status\n");

	return data;
}

int getWscProfile(char *interface, WSC_CONFIGURED_VALUE *data, int len)
{
	int socket_id;
	struct iwreq wrq;

	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy((char *)data, "get_wsc_profile");
	strcpy(wrq.ifr_name, interface);
	wrq.u.data.length = len;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;
	ioctl(socket_id, RTPRIV_IOCTL_WSC_PROFILE, &wrq);
	close(socket_id);
	return 0;
}

int
stainfo()
{
	char data[2048];
	struct iwreq wrq;

	memset(data, 0x00, 2048);
	wrq.u.data.length = 2048;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;

	if(wl_ioctl(WIF, RTPRIV_IOCTL_GSTAINFO, &wrq) < 0)
	{
		dbg("errors in getting STAINFO result\n");
		return 0;
	}

	if(wrq.u.data.length > 0)
	{
		puts(wrq.u.data.pointer);
	}

	return 0;
}

int
stainfo_2g()
{
	char data[2048];
	struct iwreq wrq;

	memset(data, 0x00, 2048);
	wrq.u.data.length = 2048;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;

	if(wl_ioctl(WIF2G, RTPRIV_IOCTL_GSTAINFO, &wrq) < 0)
	{
		dbg("errors in getting STAINFO result\n");
		return 0;
	}

	if(wrq.u.data.length > 0)
	{
		puts(wrq.u.data.pointer);
	}

	return 0;
}

int
getstat()
{
	char data[4096];
	struct iwreq wrq;

	memset(data, 0x00, 4096);
	wrq.u.data.length = 4096;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;

	if(wl_ioctl(WIF, RTPRIV_IOCTL_GSTAT, &wrq) < 0)
	{
		dbg("errors in getting STAT result\n");
		return 0;
	}

	if(wrq.u.data.length > 0)
	{
		puts(wrq.u.data.pointer);
	}

	return 0;
}

int
getstat_2g()
{
	char data[4096];
	struct iwreq wrq;

	memset(data, 0x00, 4096);
	wrq.u.data.length = 4096;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;

	if(wl_ioctl(WIF2G, RTPRIV_IOCTL_GSTAT, &wrq) < 0)
	{
		dbg("errors in getting STAT result\n");
		return 0;
	}

	if(wrq.u.data.length > 0)
	{
		puts(wrq.u.data.pointer);
	}

	return 0;
}

int
getrssi()
{
	char data[32];
	struct iwreq wrq;

	memset(data, 0x00, 32);
	wrq.u.data.length = 32;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;

	if(wl_ioctl(WIF, RTPRIV_IOCTL_GRSSI, &wrq) < 0)
	{
		dbg("errors in getting RSSI result\n");
		return 0;
	}

	if(wrq.u.data.length > 0)
	{
		puts(wrq.u.data.pointer);
	}

	return 0;
}

int
getrssi_2g()
{
	char data[32];
	struct iwreq wrq;

	memset(data, 0x00, 32);
	wrq.u.data.length = 32;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;

	if(wl_ioctl(WIF2G, RTPRIV_IOCTL_GRSSI, &wrq) < 0)
	{
		dbg("errors in getting RSSI result\n");
		return 0;
	}

	if(wrq.u.data.length > 0)
	{
		puts(wrq.u.data.pointer);
	}

	return 0;
}

int
gettxbfcal()
{
	char data[32];
	struct iwreq wrq;

	memset(data, 0x00, 32);
	wrq.u.data.length = 32;
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = 0;

	if(wl_ioctl(WIF, RTPRIV_IOCTL_GTXBFCALP, &wrq) < 0)
	{
		dbg("errors in getting TxBfCal result\n");
		return 0;
	}

	if(wrq.u.data.length > 0)
	{
		puts(wrq.u.data.pointer);
	}

	return 0;
}

void
wsc_user_commit()
{
	int flag_wep;

	if (nvram_match("rt_wsc_config_state", "2"))
	{
		flag_wep = 0;

		if (nvram_match("rt_auth_mode", "open") && nvram_match("rt_wep_x", "0"))
		{
			doSystem("iwpriv %s set AuthMode=%s", WIF2G, "OPEN");
			doSystem("iwpriv %s set EncrypType=%s", WIF2G, "NONE");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);
		}
		else if (nvram_match("rt_auth_mode", "open"))
		{
			flag_wep = 1;
			doSystem("iwpriv %s set AuthMode=%s", WIF2G, "OPEN");
			doSystem("iwpriv %s set EncrypType=%s", WIF2G, "WEP");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);
		}
		else if (nvram_match("rt_auth_mode", "shared"))
		{
			flag_wep = 1;
			doSystem("iwpriv %s set AuthMode=%s", WIF2G, "SHARED");
			doSystem("iwpriv %s set EncrypType=%s", WIF2G, "WEP");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);
		}
		else if (nvram_match("rt_auth_mode", "psk"))
		{
			if (nvram_match("rt_wpa_mode", "0"))
				doSystem("iwpriv %s set AuthMode=%s", WIF2G, "WPAPSKWPA2PSK");
			else if (nvram_match("rt_wpa_mode", "1"))
				doSystem("iwpriv %s set AuthMode=%s", WIF2G, "WPAPSK");
			else if (nvram_match("rt_wpa_mode", "2"))
				doSystem("iwpriv %s set AuthMode=%s", WIF2G, "WPA2PSK");

			//EncrypType
			if (nvram_match("rt_crypto", "tkip"))
				doSystem("iwpriv %s set EncrypType=%s", WIF2G, "TKIP");
			else if (nvram_match("rt_crypto", "aes"))
				doSystem("iwpriv %s set EncrypType=%s", WIF2G, "AES");
			else if (nvram_match("rt_crypto", "tkip+aes"))
				doSystem("iwpriv %s set EncrypType=%s", WIF2G, "TKIPAES");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF2G, 0);

			doSystem("iwpriv %s set SSID=%s", WIF2G, nvram_safe_get("rt_ssid"));

			//WPAPSK
			doSystem("iwpriv %s set WPAPSK=%s", WIF2G, nvram_safe_get("rt_wpa_psk"));

			doSystem("iwpriv %s set DefaultKeyID=%s", WIF2G, "2");
		}
		else
		{
			doSystem("iwpriv %s set AuthMode=%s", WIF2G, "OPEN");
			doSystem("iwpriv %s set EncrypType=%s", WIF2G, "NONE");
		}

//		if (!nvram_match("rt_wep_x", "0"))
		if (flag_wep)
		{
			//KeyStr
			if (strlen(nvram_safe_get("rt_key1")))
			doSystem("iwpriv %s set Key1=%s", WIF2G, nvram_safe_get("rt_key1"));
			if (strlen(nvram_safe_get("rt_key2")))
			doSystem("iwpriv %s set Key2=%s", WIF2G, nvram_safe_get("rt_key2"));
			if (strlen(nvram_safe_get("rt_key3")))
			doSystem("iwpriv %s set Key3=%s", WIF2G, nvram_safe_get("rt_key3"));
			if (strlen(nvram_safe_get("rt_key4")))
			doSystem("iwpriv %s set Key4=%s", WIF2G, nvram_safe_get("rt_key4"));

			//DefaultKeyID
			doSystem("iwpriv %s set DefaultKeyID=%s", WIF2G, nvram_safe_get("rt_key"));
		}

		doSystem("iwpriv %s set SSID=%s", WIF2G, nvram_safe_get("rt_ssid"));

		nvram_set("rt_wsc_config_state", "1");
		doSystem("iwpriv %s set WscConfStatus=%d", WIF2G, 2);   // AP is configured
	}

	if (nvram_match("wsc_config_state", "2"))
	{
		flag_wep = 0;

		if (nvram_match("wl_auth_mode", "open") && nvram_match("wl_wep_x", "0"))
		{
			doSystem("iwpriv %s set AuthMode=%s", WIF, "OPEN");
			doSystem("iwpriv %s set EncrypType=%s", WIF, "NONE");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);
		}
		else if (nvram_match("wl_auth_mode", "open"))
		{
			flag_wep = 1;
			doSystem("iwpriv %s set AuthMode=%s", WIF, "OPEN");
			doSystem("iwpriv %s set EncrypType=%s", WIF, "WEP");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);
		}
		else if (nvram_match("wl_auth_mode", "shared"))
		{
			flag_wep = 1;
			doSystem("iwpriv %s set AuthMode=%s", WIF, "SHARED");
			doSystem("iwpriv %s set EncrypType=%s", WIF, "WEP");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);
		}
		else if (nvram_match("wl_auth_mode", "psk"))
		{
			if (nvram_match("wl_wpa_mode", "0"))
				doSystem("iwpriv %s set AuthMode=%s", WIF, "WPAPSKWPA2PSK");
			else if (nvram_match("wl_wpa_mode", "1"))
				doSystem("iwpriv %s set AuthMode=%s", WIF, "WPAPSK");
			else if (nvram_match("wl_wpa_mode", "2"))
				doSystem("iwpriv %s set AuthMode=%s", WIF, "WPA2PSK");

			//EncrypType
			if (nvram_match("wl_crypto", "tkip"))
				doSystem("iwpriv %s set EncrypType=%s", WIF, "TKIP");
			else if (nvram_match("wl_crypto", "aes"))
				doSystem("iwpriv %s set EncrypType=%s", WIF, "AES");
			else if (nvram_match("wl_crypto", "tkip+aes"))
				doSystem("iwpriv %s set EncrypType=%s", WIF, "TKIPAES");

			doSystem("iwpriv %s set IEEE8021X=%d", WIF, 0);

			doSystem("iwpriv %s set SSID=%s", WIF, nvram_safe_get("wl_ssid"));

			//WPAPSK
			doSystem("iwpriv %s set WPAPSK=%s", WIF, nvram_safe_get("wl_wpa_psk"));

			doSystem("iwpriv %s set DefaultKeyID=%s", WIF, "2");
		}
		else
		{
			doSystem("iwpriv %s set AuthMode=%s", WIF, "OPEN");
			doSystem("iwpriv %s set EncrypType=%s", WIF, "NONE");
		}

//		if (!nvram_match("wl_wep_x", "0"))
		if (flag_wep)
		{
			//KeyStr
			if (strlen(nvram_safe_get("wl_key1")))
			doSystem("iwpriv %s set Key1=%s", WIF, nvram_safe_get("wl_key1"));
			if (strlen(nvram_safe_get("wl_key2")))
			doSystem("iwpriv %s set Key2=%s", WIF, nvram_safe_get("wl_key2"));
			if (strlen(nvram_safe_get("wl_key3")))
			doSystem("iwpriv %s set Key3=%s", WIF, nvram_safe_get("wl_key3"));
			if (strlen(nvram_safe_get("wl_key4")))
			doSystem("iwpriv %s set Key4=%s", WIF, nvram_safe_get("wl_key4"));

			//DefaultKeyID
			doSystem("iwpriv %s set DefaultKeyID=%s", WIF, nvram_safe_get("wl_key"));
		}

		doSystem("iwpriv %s set SSID=%s", WIF, nvram_safe_get("wl_ssid"));

		nvram_set("wsc_config_state", "1");
		doSystem("iwpriv %s set WscConfStatus=%d", WIF, 2);     // AP is configured
	}
}
