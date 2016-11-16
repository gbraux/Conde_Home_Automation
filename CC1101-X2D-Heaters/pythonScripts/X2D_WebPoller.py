import urllib.request
import requests
import json
import time
import os

scriptPath=os.path.dirname(__file__)+'/'
if scriptPath=='/' :
        scriptPath='./'

urlPath="http://192.168.11.25:3000/home/osmc/Conde_Home_Automation/www/HeaterControl/"
if os.name == 'nt':
	pythonPath='c:\Python27\python.exe'
else :
	pythonPath='python3'

url = urlPath+"modeStatus.php"
response = requests.get(url)
mode = response.json()
print(mode)


if mode['mode'] == "Manual" :
	url = urlPath+"readZones.php"

else :
	url = urlPath+"zonesStatusFromPlanning.php"

response = requests.get(url)
zones = response.json()
print(zones)

for zone,value in zones.items():
	if zone == "zone1" :
		param = value+"1"
	
	if zone == "zone2" :
		param = value+"2"
	
	if zone == "zone3" :
		param = value+"3"

	os.system(pythonPath+" "+scriptPath+"pyX2DCmd.py "+param)
	
	time.sleep(15)
