/*
 * HTML Definations used by the Program
 * Ramesh Kr Sah
 * 12/2016
 * */
 
const char *html_header =
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
"<title>%s</title>"	//page title
"<style>"
"body{margin: 0;font-family: Tahoma;font-size: 13px;background: #eee;}"
"#banner{background: #93B74C;padding: 5px;text-align: center;}"
"#banner * {font-size: 24px;vertical-align: middle;color: white;font-weight: bold;font-family: Verdana;}"
"form{width: 960px;background: white;margin: 0px auto;padding: 50px 5px;border: 1px solid #999;border-top: 0px;}"
"form div{padding: 5px 100px;border-bottom: 1px solid #eee;}"
"label{display: inline-block;text-align:left;width: 206px;padding-top: 2px;padding-right: 10px;"
"font-family: Tahoma;font-size: 13px;}";

const char *html_header1 =
"fieldset{margin-bottom: 15px;border: 1px solid #EEEEEE;}"
".input{width:20px;}"
"#footer{font-size: 11px;margin-top: 5px;}"
"a{color: #93B74C;text-decoration: none;}"
"a:hover{text-decoration: underline;}"
"#menu{width:960px;}"
"#menu1{display:inline;width:960px;float:left;position:center;margin:0px auto;list-style:none;height:30px;"
"border-bottom:1px solid #eee;}"
".menu{float:left;padding-left:20px;padding-right:20px;line-height:30px;border-right:1px solid #eee;}"
".label{display:inline-block;width:45%%;}.border{border: 1px solid #93B74C;}"
"</style>";

const char *html_header2 =
"</head><body><center><div id=\"banner\">"
"<span style=\" margin-left:80px; margin-right:80px;\">%s</span>"
"</div><div id=\"menu\"><ul id=\"menu1\">"
/*	Krishna	*/
"<li class=\"menu\" style=\"margin-left:270px; border-left:1px solid #EEEEEE;\"><a href=\"home\""
" > Configure Node </a></li>"
"</ul></div><form method=\"post\">";

const char *config_boxes = 
"SSID: <input type=\"text\" name=\"ssid\" value=\"%s\"><br>"
"Password: <input type=\"text\" name=\"passwd\" value=\"%s\"><br>"
"Controller Name: <input type=\"text\" name=\"conname\" value=\"%s\"><br>"
"Node Name: <input type=\"text\" name=\"nodname\" value=\"%s\"><br>";

const char *check_box = 
"<input type=\"checkbox\" name=\"unreg\" value=\"1\"> Remove register status? <br>";

const char *htmlFooter =	
"<div style=\"text-align: center; padding-top: 20px\">\
<button name=\"Submit\" value=\"Submit\">Save & Reset</button>\
</div>\
</form>\
<div id=\"footer\">&copy; Copyright 2018 Real Time Solutions <a href=\"http://www.easy-q.com\" target=\"_new\">\
<b>EASY-Q</b>\
</a>\
</div>\
</center>\
<script>\
window.onload = function(){\
document.getElementById(\"inputString\").value = decodeURIComponent(document.getElementById(\"inputString\").value);\
document.getElementById(\"inputString1\").value = decodeURIComponent(document.getElementById(\"inputString1\").value);}\
</script>\
</body>\
</html>";
