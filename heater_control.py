import time
import serial
import os
import sys
import csv
import HandlePlanning
from datetime import  datetime
import logging
import HandlePlanning
from queue import Queue
from threading import Thread
import presence_automation
from configparser import ConfigParser

# ---- Config File Read/Init ----
config = ConfigParser()
config.read("/home/osmc/Conde_Home_Automation/config.ini")
# print(config)
# print(config.get('heaterstates', 'lastmode'))

globalMode = config.get('heaterstates', 'lastmode')
modeZ1 = config.get('heaterstates', 'laststate1')
modeZ2 = config.get('heaterstates', 'laststate2')
modeZ3 = config.get('heaterstates', 'laststate3')
season = config.get('heaterstates', 'season')


# globalMode = "Auto"
# modeZ1 = "Moon"
# modeZ2 = "Moon"
# modeZ3 = "Moon"
tempRemaining = 0 # minutes

X2DEventQueue = Queue()

def SaveHeaterStatesToFile():

	global globalMode
	global modeZ1
	global modeZ2
	global modeZ3

	if (globalMode=="Temp"):
		config.set('heaterstates', 'lastmode', "Auto")
	else:
		config.set('heaterstates', 'lastmode', globalMode)

	config.set('heaterstates', 'laststate1', modeZ1)
	config.set('heaterstates', 'laststate2', modeZ2)
	config.set('heaterstates', 'laststate3', modeZ3)
	
	with open('/home/osmc/Conde_Home_Automation/config.ini', 'w') as configfile:
		config.write(configfile)

	logging.info("Heater State saved to INI File")

def StartTempModeThread():
	worker = Thread(target=StartTempMode)
	worker.setDaemon(False)
	worker.start()

def StartTempMode():
	
	global globalMode
	global modeZ1
	global modeZ2
	global modeZ3
	global tempRemaining
	
	while ((globalMode == "Temp") & (tempRemaining > 0)):
		logging.info("Heater Temporary Mode is ON ! Refreshing states")
		logging.info("Temporary Mode time remaining : "+str(tempRemaining)+" minutes")
		UpdateHeatersStates()

		tempRemaining = tempRemaining - 1;

		if (tempRemaining < 0):
			tempRemaining = 0

		time.sleep(60)

	globalMode = "Auto"
	UpdateHeatersStates()
	

def SetHeaterCommand(command):

	global globalMode
	global modeZ1
	global modeZ2
	global modeZ3
	global tempRemaining

	logging.info("Heater Command received : "+command)

	if command.startswith("Temp"):
		# format
		# Temp Hg1 60

		tempSplit = command.split()
		globalMode = "Temp"
		tempRemaining = float(tempSplit[2])
		modeZ1 = str(tempSplit[1])[:-1]
		modeZ2 = str(tempSplit[1])[:-1]
		modeZ3 = str(tempSplit[1])[:-1]
		StartTempModeThread()
	
	elif command.endswith("1"):
		modeZ1 = str(command)[:-1]
		globalMode = "Manual"
		SendState(command)

	elif command.endswith("2"):
		modeZ2 = str(command)[:-1]
		globalMode = "Manual"
		SendState(command)

	elif command.endswith("3"):
		modeZ3 = str(command)[:-1]
		globalMode = "Manual"
		#SendState(command)

	elif (command == "Auto"):
		globalMode = "Auto"
		UpdateHeatersStates()
	elif (command == "Manual"):
		globalMode = "Manual"
		UpdateHeatersStates()

def SendState(state):

	modeNum = state[-1:] 

	if state.startswith("Sun"):
		SendX2DCommand(state)
		SendX2DCommand("On"+modeNum)

	if state.startswith("Moon"):
		SendX2DCommand(state)
		SendX2DCommand("On"+modeNum)

	if state.startswith("On"):
		SendX2DCommand(state)

	if state.startswith("Hg"):
		SendX2DCommand(state)

	if state.startswith("Off"):
		SendX2DCommand(state)

def UpdateHeatersStates():
	
	global globalMode
	global modeZ1
	global modeZ2
	global modeZ3
	global season

	if (globalMode == "Auto"):
	
		logging.info("Heaters are in Auto Mode")

		if presence_automation.IsThereAnybody() == False:
    			
			logging.info("Nobody is at home - Putting the heaters in Moon mode")
			modeZ1 = "Moon"
			SendX2DCommand("Moon"+"1")
			SendX2DCommand("On1")

			SendX2DCommand("Moon"+"2")
			SendX2DCommand("On2")
				
		else:
		
			path=os.path.dirname(__file__)+'/'
			if path=='/' :
					path='./'

			data=list()
			dayNames=['lundi', 'mardi', 'mercredi', 'jeudi', 'vendredi', 'samedi', 'dimanche']
			stateNames=['Moon','Sun']

			if (season == "Summer"):
				csvFile = "/home/osmc/Conde_Home_Automation/chauffage-summer.csv"
			else:
				csvFile = "/home/osmc/Conde_Home_Automation/chauffage.csv"

			with open(csvFile, 'rt') as f:
				reader = csv.reader(f, delimiter=';', quotechar='"', quoting=csv.QUOTE_ALL)
				for row in reader:
					# print row
					data.append(row)
				
			zone1=HandlePlanning.HandlePlanning()
			zone2=HandlePlanning.HandlePlanning()
				
			zone1.setData(data)
			zone2.setData(data)

			horaires=zone1.getHoraires()
			zone2.getHoraires()
				
			zone1.getZone("Zone 1")
			zone2.getZone("Zone 2")

			d=datetime.now()
			dayName=dayNames[d.weekday()]
			#print(dayName)	

			timeStr=d.strftime("%H:%M:%S")
			#print(timeStr)

			'''
			# Some tests
			# timeStr="07:35:00"
			# timeStr="07:00:00"
			# timeStr="06:35:00"
			'''

			#timeStr="23:31:00"

			stat1=stateNames[zone1.getStatus(dayName, timeStr)]
			logging.info("Heaters Status for Zone 1 : "+stat1)
			stat2=stateNames[zone2.getStatus(dayName, timeStr)]
			logging.info("Heaters Status for Zone 2 : "+stat2)


			
			#os.system("python3 %spyX2DCmd.py %s1"%(path,stat1))
			#time.sleep(15)
			#os.system("python3 %spyX2DCmd.py %s2"%(path,stat2))

			modeZ1 = stat1
			SendX2DCommand(stat1+"1")
			SendX2DCommand("On1")

			modeZ2 = stat2
			SendX2DCommand(stat2+"2")
			SendX2DCommand("On2")

	else:
		logging.info("Heaters are NOT in auto mode (manual or Temporary Mode). Refreshing manual States")
		SendState(modeZ1+"1")
		SendState(modeZ2+"2")
		# SendState(modeZ3+"3")

def SendX2DCommandThread():
	worker = Thread(target=SendX2DCommandQueue, args=(X2DEventQueue,))
	worker.setDaemon(False)
	worker.start()

def SendX2DCommand(command):
	X2DEventQueue.put(command)
	
def SendX2DCommandQueue(q):
	while True:
		logging.info("X2D Command Queue waiting ...")
		command = q.get()
		logging.info("X2D Command in queue : "+command)
		SaveHeaterStatesToFile()
		logging.info("Opening Serial Port")
		
		try:
			if os.name == 'nt':
				ser = serial.Serial(
					port='COM10',
					baudrate=9600
					
				)
			
				ser.rts = False
				ser.dtr = False
			else:
				ser = serial.Serial(
					port='/dev/USB868',
					baudrate=9600
				)
				
				ser.rts = False
				ser.dtr = False
				#ser.open()


			if ser.isOpen() == False:
				ser.open()
				logging.info("Serial Port Opened")

		except:
			logging.info("ERROR WHILE OPENING SERIAL PORT !")
			q.task_done()
			continue

		out = ''
		# wait for RFbee init
		time.sleep(3)
		while ser.inWaiting() > 0:
			out += str(ser.read(1).decode("ascii"))
			
		if out != '':
			#print(">>" + out)
			logging.info("Arduino/CC1101 initialization Complete")
			logging.debug(out)

		time.sleep(3)
		ser.write(bytearray(command+"\n",'ascii'))
		#print(">>%s"%(command))
		logging.info(command+" sent to Arduino")

		out = ''
		# let's wait one second before reading output (let's give device time to answer)
		time.sleep(1)
		while ser.inWaiting() > 0:
			out += str(ser.read(1).decode("ascii"))
			
		if "ok" in out :
			logging.info("Arduino message sent successfuly")
			logging.debug(out)
		else:
			logging.error("Error sending command")

		q.task_done()
