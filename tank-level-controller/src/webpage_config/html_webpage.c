/*
 * HTML Definations used by the Program
 * Ramesh Kr Sah
 * 12/2016
*/
 
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
" > Tank Mode </a></li>"
"<li class=\"menu\"><a href=\"link_tank\" > Link Tank </a></li>"
"<li class=\"menu\"><a href=\"status\" > Status </a></li>"
"<li class=\"menu\"><a href=\"mode_config\" > Mode Config </a></li>"
"<li class=\"menu\"><a href=\"rem\" > Remove Nodes </a></li>"
"</ul></div><form method=\"post\">";

const char *drop_list =
"<div><label>%s</label>"
"<select name=\"%s\">"
"<option value=\"\" disabled selected>%s</option>"
"<option value=\"1\">Source</option>"
"<option value=\"2\">Destination</option>"
"<option value=\"3\">Source + Destination</option>"
"</select>"
"<br></div>";

const char *ind_list = 
"<input type=\"checkbox\" name=\"rmdev\" value=\"%s\"> %s <br>";

const char *mon_mode = 
"Is this Controller also an Monitor? <input type=\"checkbox\" name=\"monmode\" value=\"1\" %s><br>";

const char *esp_mode_drop_list =
"<div><label>Operation Mode</label>"
"<select name=\"opmode\">"
"<option value=\"\" disabled selected>%s</option>"
"<option value=\"1\">Standalone Mode</option>"
"<option value=\"2\">Router Mode</option>"
"<option value=\"3\">MQTT Mode</option>"
"</select>"
"<br></div>";

const char *text_box = 
"%s <input type=\"text\" name=\"%s\" value=\"%s\"><br>";

const char *tank_src_list_dyn_1 = 
"<div><label>%s</label>"
"<select name=\"%s\">"
"<option value=\"\" disabled selected>%s</option>";

const char *tank_src_list_dyn_2 = 
"<option value=\"%s\">%s</option>";

const char *tank_src_list_dyn_3 =
"</select>";

const char *pin_assignment = 
"<select name=\"%s\">"
"<option value=\"\" disabled selected>%s</option>"
"<option value=\"13\">Pin 13</option>"
"<option value=\"14\">Pin 14</option>"
"<option value=\"4\">Pin 4</option>"
"<option value=\"0\">Pin 0</option>"
"<option value=\"16\">Pin 16</option>"
"</select>"
"<br></div>";

const char *tank_stat =
"<div>"
"<label>%s</label>"
"<label>%s</label>"
"%s"
"</div>";

const char *htmlFooter =
"<div style=\"text-align: center; padding-top: 20px\">\
<button name=\"Submit\" value=\"Submit\">Save</button>\
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
