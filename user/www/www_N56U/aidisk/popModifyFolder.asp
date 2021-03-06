<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Rename Folder</title>
<link rel="stylesheet" href="../form_style.css"  type="text/css">

<script type="text/javascript" src="../state.js"></script>
<script type="text/javascript">
var selectedPool = parent.getSelectedPool();
var selectedFolder = parent.getSelectedFolder();
var folderlist = parent.get_sharedfolder_in_pool(selectedPool);

function initial(){
	showtext($("selected_Pool"), selectedPool);
	showtext($("selected_Folder"), showhtmlspace(showhtmland(selectedFolder)));
	
	clickevent();
}

function clickevent(){
	$("Submit").onclick = function(){
			if(validForm()){
				$("pool").value = selectedPool;
				$("folder").value = selectedFolder;
				
				parent.showLoading();
				document.modifyFolderForm.submit();
				parent.hidePop("apply");
			}
		};
}

function validForm(){
	$("new_folder").value = trim($("new_folder").value);
	
	// share name
	if($("new_folder").value.length == 0){
		alert("<#File_content_alert_desc6#>");
		$("new_folder").focus();
		return false;
	}
	
	var re = new RegExp("[^a-zA-Z0-9 _-]+","gi");
	if(re.test($("new_folder").value)){
		alert("<#File_content_alert_desc7#>");
		$("new_folder").focus();
		return false;
	}
	
	if(parent.checkDuplicateName($("new_folder").value, folderlist)){
		alert("<#File_content_alert_desc8#>");
		$("new_folder").focus();
		return false;
	}
	
	if(trim($("new_folder").value).length > 12)
		if (!(confirm("<#File_content_alert_desc10#>")))
			return false;
	
	return true;
}
</script>
</head>

<body onLoad="initial();">
<form method="post" name="modifyFolderForm" action="modify_sharedfolder.asp" target="hidden_frame">
<input type="hidden" name="pool" id="pool" value="">
<input type="hidden" name="folder" id="folder" value="">
	<table width="100%" class="popTable" border="0" align="center" cellpadding="0" cellspacing="0">
	<thead>
      <tr>
        <td colspan="2"><span><#ModFolderTitle#></span><img src="../images/button-close.gif" onClick="parent.hidePop('OverlayMask');"></td>
      </tr>
	</thead>	  
	<tbody>
      <tr>
        <td  colspan="2" height="30"><#ModFolderAlert#></td>
      </tr>
      <tr>
        <th><#PoolName#>: </th>
        <td colspan="3"><span id="selected_Pool"></span></td>
	  </tr>
      <tr>
        <th><#FolderName#>: </th>
        <td colspan="3"><span id="selected_Folder"></span></td>
      </tr>
      <tr>
        <th><#NewFolderName#>: </th>
        <td><input class="input" type="text" name="new_folder" id="new_folder"></td>
      </tr>
      <tr bgcolor="#E6E6E6">
        <th colspan="2" align="right"><input id="Submit" type="button" class="button" value="<#CTL_modify#>"></th>
      </tr>
	</tbody>	  
    </table>
</form>
</body>
</html>
