<?php
// appel du script de connexion
require("mysql_connect.php");
// On r�cup�re le timestamp du dernier enregistrement
$sql="select max(timestamp) from teleinfo";
$query=mysql_query($sql);                  
$list=mysql_fetch_array($query);
// On d�termine le stop et le start de fa�on � r�cup�rer dans la prochaine requ�te que les  donn�es des derni�res xx heures
$stop=$list[0];
$start=$stop-(86400*30);//86400=24 heures donc 86400*2=48 heures
// R�cup�ration des donn�es sur les derni�res 48 heures avec un tri ascendant sur le timestamp
//$sql = "SELECT timestamp, temp FROM teleinfo where timestamp >= '$start' and  timestamp <= '$stop' ORDER BY 1";  

//$sql = "SELECT DATE(timestamp) AS theday, AVG(temp) AS avgtmp, timestamp FROM teleinfo where timestamp >= '$start' and  timestamp <= '$stop' GROUP BY theday";  




$sql = "SELECT DATE(from_unixtime(timestamp)) AS theday, AVG(temp) AS avgtmp, AVG(timestamp) AS timestamp FROM teleinfo where timestamp >= '$start' and  timestamp <= '$stop' GROUP BY theday";
//SELECT DATE(from_unixtime(timestamp)) AS theday, AVG(temp) AS avgtmp, AVG(timestamp) AS timestamp FROM teleinfo where timestamp >= '$start' and  timestamp <= '$stop' GROUP BY theday

$query=mysql_query($sql);                   
$i=0;
while ($list = mysql_fetch_assoc($query)) {      
if (date("I",time())==0) { 
	$time[$i]=($list['timestamp']+3600)*1000;
	} 
else {
	$time[$i]=($list['timestamp']+7200)*1000;
  } 

$outdoortemperature[$i]=$list['avgtmp']*1;
$i++;
}
?>


<script type="text/javascript"> 
eval(<?php echo  "'var time =  ".json_encode($time)."'" ?>);
eval(<?php echo  "'var outdoortemperature =  ".json_encode($outdoortemperature)."'" ?>);
</script>






<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" 
"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<HEAD>
<META NAME="SUBJECT" CONTENT="Graphique Vantage Pro Davis">
<META NAME="DESCRIPTION" CONTENT="graphique dynamique">
<META NAME="KEYWORDS" CONTENT=" m�t�o, pluie, vent, temp�rature, temperature, 
station, Vantage, Davis, Pro, Davis Vantage Pro,pression, UV, soleil,">
<META NAME="REVISIT-AFTER" CONTENT="5 DAYS">
<meta http-equiv="X-UA-Compatible" content="IE=9; IE=9" >
<META NAME="LANGUAGE" CONTENT="FR">
<meta http-equiv="content-type" content="text/plain; charset=ISO-8859-1"> 
<title>M�t�o - 1555 Villarzel / Graphique dynamique temp�rature et point de ros�e sur 48 heures</title>
               
<!-- 1. Add these JavaScript inclusions in the head of your page -->

 <script type="text/javascript" src="jquery.min.js"></script> 
 <script type="text/javascript" src="highcharts/js/highcharts.js"></script> 
 <script type="text/javascript" src="highcharts/js/themes/grid.js"></script>
<script type="text/javascript" src="highcharts/js/modules/exporting.js"></script>

<!-- 2. Add the JavaScript to initialize the chart on document ready -->

<script type="text/javascript"> 

function comArr(unitsArray) { var outarr = [];
for (var i = 0; i < time.length; i++) { outarr[i] = [time[i], unitsArray[i]];
}
return outarr;
} 

$(function () {
var chart; 
$(document).ready(function() { 
var highchartsOptions = Highcharts.setOptions(Highcharts.theme);	
Highcharts.setOptions({
lang: {

months: ["Janvier "," F�vrier ","Mars "," Avril "," Mai "," Juin "," Juillet "," Ao�t "," Septembre ",
" Octobre "," Novembre "," D�cembre"],
weekdays: ["Dim "," Lun "," Mar "," Mer "," Jeu "," Ven "," Sam"],
shortMonths: ['Jan', 'Fev', 'Mar', 'Avr', 'Mai', 'Juin', 'Juil','Ao�t', 'Sept', 'Oct', 'Nov', 'D�c'],
decimalPoint: ',',
resetZoom: 'Reset zoom',
resetZoomTitle: 'Reset zoom � 1:1',
downloadPNG: "T�l�charger au format PNG image",
downloadJPEG: "T�l�charger au format JPEG image",
downloadPDF: "T�l�charger au format PDF document",
downloadSVG: "T�l�charger au format SVG vector image",
exportButtonTitle: "Exporter image ou document",
printChart: "Imprimer le graphique",
loading: "Laden..."
 }

});

chart = new Highcharts.Chart({
chart: {
renderTo: 'container',
zoomType: 'x',
type: 'spline',
marginRight: 10,
marginBottom: 60,
plotBorderColor: '#346691',
plotBorderWidth: 1,
},
               
title: {
text: 'Temp�ratures des derni�res 48 heures',
x: -20 //center
},

subtitle: {
text: 'Source: M�t�o Villarzel',
x: -20
},
credits: {
text: '� M�t�o Villarzel',
href: 'http://www.boock.ch/meteo-villarzel.php'
 },

xAxis: {
type: 'datetime', 
startOnTick: false,
},

yAxis: {
title: {
text: 'Temperature (�C)' 
},

plotLines: [{
value: 0,
width: 1,
color: '#FF0000'
}
]
 },

tooltip: {
crosshairs:[true],
borderColor: '#4b85b7',
shared: true,
backgroundColor: '#edf1c8',
formatter: function() {
var s = '<b>'+ Highcharts.dateFormat('%e %B � %H:%M', 
this.x) +'</b>';
$.each(this.points, function(i, point) {
var unit = {
'Point de ros�e': ' �C',
'Temp�rature': ' �C',
'Facteur vent' : '',
'Humidex' : ''
 }[this.point.series.name];
s = s + '<br>' + '<span style="color:'+ point.series.color +'">' + point.series.name + '</span> : ' 
+Highcharts.numberFormat(point.y,1,","," ")+ unit;
});
return s;
 },
 },

plotOptions: {
series: {
marker: {
enabled: false
 }
 }
},

series: [ 
{
name: 'Temp�rature',
zIndex: 1,
color: '#ff0000',
data: comArr(outdoortemperature) 
 }]
});
});
});
                

</script>
</head>
<body>
<div id="container" style="margin: 0 auto"></div>
</body>
</html>