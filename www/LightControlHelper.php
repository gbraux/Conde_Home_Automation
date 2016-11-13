<?php
echo $_GET['dataLine'];
echo $_GET['onOff'];
echo "<br>";

if ($_GET['dataLine'] == "all")
{
	exec("sudo /home/osmc/Conde_Home_Automation/LightControl/LightControl 1 ".$_GET['onOff']);
	exec("sudo /home/osmc/Conde_Home_Automation/LightControl/LightControl 2 ".$_GET['onOff']);
	exec("sudo /home/osmc/Conde_Home_Automation/LightControl/LightControl 3 ".$_GET['onOff']);
	exec("sudo /home/osmc/Conde_Home_Automation/LightControl/LightControl 4 ".$_GET['onOff']);
}

echo "sudo /home/osmc/Conde_Home_Automation/LightControl/LightControl ".$_GET['dataLine']." ".$_GET['onOff'];

exec("sudo /home/osmc/Conde_Home_Automation/LightControl/LightControl ".$_GET['dataLine']." ".$_GET['onOff']);
?>
