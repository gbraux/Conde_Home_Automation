<?php
// appel du script de connexion
require("mysql_connect.php");
// On récupère le timestamp du dernier enregistrement
$sql="select max(timestamp) from teleinfo";
$query=mysql_query($sql);                  
$list=mysql_fetch_array($query);
// On détermine le stop et le start de façon à récupérer dans la prochaine requête que les  données des dernières xx heures
$stop=$list[0];
$start=$stop-(86400*30);//86400=24 heures donc 86400*2=48 heures
// Récupération des données sur les dernières 48 heures avec un tri ascendant sur le timestamp
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
<META NAME="KEYWORDS" CONTENT=" météo, pluie, vent, température, temperature, 
station, Vantage, Davis, Pro, Davis Vantage Pro,pression, UV, soleil,">
<META NAME="REVISIT-AFTER" CONTENT="5 DAYS">
<meta http-equiv="X-UA-Compatible" content="IE=9; IE=9" >
<META NAME="LANGUAGE" CONTENT="FR">
<meta http-equiv="content-type" content="text/plain; charset=ISO-8859-1"> 
<title>Météo - 1555 Villarzel / Graphique dynamique température et point de rosée sur 48 heures</title>
               
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

months: ["Janvier "," Février ","Mars "," Avril "," Mai "," Juin "," Juillet "," Août "," Septembre ",
" Octobre "," Novembre "," Décembre"],
weekdays: ["Dim "," Lun "," Mar "," Mer "," Jeu "," Ven "," Sam"],
shortMonths: ['Jan', 'Fev', 'Mar', 'Avr', 'Mai', 'Juin', 'Juil','Août', 'Sept', 'Oct', 'Nov', 'Déc'],
decimalPoint: ',',
resetZoom: 'Reset zoom',
resetZoomTitle: 'Reset zoom à 1:1',
downloadPNG: "Télécharger au format PNG image",
downloadJPEG: "Télécharger au format JPEG image",
downloadPDF: "Télécharger au format PDF document",
downloadSVG: "Télécharger au format SVG vector image",
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
text: 'Températures des dernières 48 heures',
x: -20 //center
},

subtitle: {
text: 'Source: Météo Villarzel',
x: -20
},
credits: {
text: '© Météo Villarzel',
href: 'http://www.boock.ch/meteo-villarzel.php'
 },

xAxis: {
type: 'datetime', 
startOnTick: false,
},

yAxis: {
title: {
text: 'Temperature (°C)' 
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
var s = '<b>'+ Highcharts.dateFormat('%e %B à %H:%M', 
this.x) +'</b>';
$.each(this.points, function(i, point) {
var unit = {
'Point de rosée': ' °C',
'Température': ' °C',
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
name: 'Température',
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