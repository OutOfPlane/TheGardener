#ifndef WEBPAGE_H
#define WEBPAGE_H

const char *webpageHeader1 = R"EOF(
<!DOCTYPE html><html lang="en-US">
<head><title>
)EOF";

const char *webpageHeader2 = R"EOF(
</title>
<meta name=viewport content="width=device-width,initial-scale=1">
<style>
article { background: #f2f2f2; padding: 1.3em; }
body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }
div { padding: 0.5em; }
h1 { margin: 0.5em 0 0 0; padding: 0.5em; }
input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; border-radius: 10px; }
label { color: #333; display: block; font-style: italic; font-weight: bold; }
nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }
nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } 
textarea { width: 100%; }
td {text-align: right;}
</style>
<script>
var getJSON = function(url, callback) {
var xhr = new XMLHttpRequest();
xhr.open('GET', url, true);
xhr.responseType = 'json';
xhr.onload = function() {
var status = xhr.status;
if (status === 200) {
callback(null, xhr.response);
} else {
callback(status, xhr.response);
}
};
xhr.send();
};
</script>
<meta charset="UTF-8">
</head>
<body>
)EOF";

const char *webpageDataTable = R"EOF(
<table>
<tr>
<th>ITEM</th>
<th>SETPOINT</th>
<th>FEEDBACK</th>
</tr>
<tr>
<td>Firmware</td>
<td id="fwvers">-</td>
<td id="fwfeat">-</td>
</tr>
<tr>
<td>WiFi</td>
<td id="wifissid">-</td>
<td id="wifistat">-</td>
</tr>
<tr>
<td>Temperature</td>
<td></td>
<td id="temp_d">-</td>
</tr>
<tr>
<td>Humidity</td>
<td></td>
<td id="hum_r">-</td>
</tr>
<tr>
<td>VIN</td>
<td></td>
<td id="vin_mV">-</td>
</tr>
<tr>
<td>3V3 AN</td>
<td></td>
<td id="v3v3_mV">-</td>
</tr>
<tr>
<tr>
<td>12V AN</td>
<td></td>
<td id="v12va_mV">-</td>
</tr>
<tr>
<td>IO1</td>
<td id="OUT1_v">-</td>
<td id="IN1_f">-</td>
</tr>
<tr>
<td>IO2</td>
<td id="OUT2_v">-</td>
<td id="IN2_v">-</td>
</tr>
<tr>
<td>AIN0</td>
<td></td>
<td id="AIN0_mV">-</td>
</tr>
<tr>
<td>AIN1</td>
<td></td>
<td id="AIN1_mV">-</td>
</tr>
<tr>
<td>AIN2</td>
<td></td>
<td id="AIN2_mV">-</td>
</tr>
<tr>
<td>AIN3</td>
<td></td>
<td id="AIN3_mV">-</td>
</tr>
<tr>
<td>AOUT</td>
<td id="AOUT_mV">-</td>
<td></td>
</tr>
<tr>
<td>AUX1</td>
<td id="AUX1_s">-</td>
<td id="iaux1_mA">-</td>
</tr>
<tr>
<td>AUX2</td>
<td id="AUX2_s">-</td>
<td id="iaux2_mA">-</td>
</tr>
<tr>
<td>MOTOR</td>
<td id="mot_p">-</td>
<td id="imot_mA">-</td>
</tr>
<tr>
<td>LED RD</td>
<td id="rd_p">-</td>
<td id="ird_mA">-</td>
</tr>
<tr>
<td>LED GN</td>
<td id="gn_p">-</td>
<td id="ign_mA">-</td>
</tr>
<tr>
<td>LED BL</td>
<td id="bl_p">-</td>
<td id="ibl_mA">-</td>
</tr>
<tr>
<td>LED WH</td>
<td id="wh_p">-</td>
<td id="iwh_mA">-</td>
</tr>
<tr>
<td>S-UNIT</td>
<td id="sunit_id">-</td>
<td id="sAge_t">-</td>
</tr>
<tr>
<td>CH0</td>
<td></td>
<td id="sunit_ch0">-</td>
</tr>
<tr>
<td>CH1</td>
<td></td>
<td id="sunit_ch1">-</td>
</tr>
<tr>
<td>CH2</td>
<td></td>
<td id="sunit_ch2">-</td>
</tr>
<tr>
<td>CH3</td>
<td></td>
<td id="sunit_ch3">-</td>
</tr>
<tr>
<td>Temp</td>
<td></td>
<td id="sunit_temp">-</td>
</tr>
</table>
)EOF";

const char *webpageTableUpdateScript = R"EOF(
<script>
function retrieveData() {
getJSON("/data", function(err, data) {
if(err != null){
console.log(err)
}else{
for (var key in data) {
if (data.hasOwnProperty(key)) {
if(document.getElementById(key) != null){
document.getElementById(key).innerHTML = data[key];
if(key.includes("_mA"))
document.getElementById(key).innerHTML = (data[key]/1000.0).toFixed(3) + " A";
if(key.includes("_mV"))
document.getElementById(key).innerHTML = (data[key]/1000.0).toFixed(3) + " V";
if(key.includes("_f"))
document.getElementById(key).innerHTML = data[key] + " Hz";
if(key.includes("_p"))
document.getElementById(key).innerHTML = data[key] + " %";
if(key.includes("_t"))
document.getElementById(key).innerHTML = data[key] + " s";
if(key.includes("_s"))
document.getElementById(key).innerHTML = ["OFF", "ON"][data[key]];
if(key.includes("_v"))
document.getElementById(key).innerHTML = ["LOW", "HIGH"][data[key]];
if(key.includes("_d"))
document.getElementById(key).innerHTML = (data[key]/10.0).toFixed(1) + " °C";
if(key.includes("_r"))
document.getElementById(key).innerHTML = (data[key]/10.0).toFixed(1) + " %rH";
}
if(key.includes("sunit")){
document.getElementById("sunit_id").innerHTML = data["sunit"]["SUID"];
document.getElementById("sunit_ch0").innerHTML = (data["sunit"]["CH0"]).toFixed(2) + " V";
document.getElementById("sunit_ch1").innerHTML = (data["sunit"]["CH1"]).toFixed(2) + " V";
document.getElementById("sunit_ch2").innerHTML = (data["sunit"]["CH2"]).toFixed(2) + " V";
document.getElementById("sunit_ch3").innerHTML = (data["sunit"]["CH3"]).toFixed(2) + " V";
document.getElementById("sunit_temp").innerHTML = (data["sunit"]["TEMP"]).toFixed(1) + " °C";
}
}
}
}
});
}
var interval = setInterval(retrieveData, 1000);
</script>
)EOF";

const char *webpageFooter = R"EOF(
</body>
</html>
)EOF";

#endif