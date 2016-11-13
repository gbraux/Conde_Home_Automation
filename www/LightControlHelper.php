<?php
echo $_GET['dataLine'];
echo $_GET['onOff'];
echo "<br>";

if ($_GET['dataLine'] == "all")
{
	exec("sudo /home/osmc/LightControl 1 ".$_GET['onOff']);
	exec("sudo /home/osmc/LightControl 2 ".$_GET['onOff']);
	exec("sudo /home/osmc/LightControl 3 ".$_GET['onOff']);
	exec("sudo /home/osmc/LightControl 4 ".$_GET['onOff']);
}

echo "sudo /home/osmc/LightControl ".$_GET['dataLine']." ".$_GET['onOff'];

exec("sudo /home/osmc/LightControl ".$_GET['dataLine']." ".$_GET['onOff']);
?>
