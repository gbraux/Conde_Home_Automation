# System modules
from queue import Queue
from threading import Thread
import time
from pysnmp.hlapi import *
from pysnmp.carrier.asynsock.dispatch import AsynsockDispatcher
from pysnmp.carrier.asynsock.dgram import udp, udp6
from pyasn1.codec.ber import decoder
from pysnmp.proto import api
import time
import subprocess
import sunrise
import datetime  
import mysql.connector
import socketserver
import http.server
import http.client
import threading
import logging
import sys
import re
import urllib
import math
import apscheduler
from apscheduler.schedulers.background import BackgroundScheduler
import json

import heater_control

#import presence_rest_server

class GccUserClass():
	# Rocket simulates a rocket ship for a game,
	#  or a physics simulation.
	
	def __init__(self, Name, TelMac, DeviceID, Presence, HasChanged, ChangeSource, ChangeDate):
		# Each rocket has an (x,y) position.
		self.Name = Name
		self.TelMac = TelMac
		self.DeviceID = DeviceID
		self.Presence = Presence
		self.HasChanged = HasChanged
		self.ChangeSource = ChangeSource
		self.ChangeDate = ChangeDate

		#ChangeSource
		# --> wlc
		# --> trap
		# --> geofence

WLC_SNMP_COMMUNITY = "public"
WLC_SNMP_IP = "192.168.11.251"
WLC_SNMP_PORT = 161
WLC_SNMP_MAC_CLIENTS_OID = "1.3.6.1.4.1.14179.2.1.4.1.1"
WLC_SNMP_TRAP_EVENT_OID = "1.3.6.1.6.3.1.1.4.1.0"
WLC_SNMP_TRAP_ASSOCIATION_OID = "1.3.6.1.4.1.14179.2.6.3.53"
WLC_SNMP_TRAP_DISASSOCIATION_OID = "1.3.6.1.4.1.14179.2.6.3.1"
WLC_SNMP_TRAP_MACBS_OID = "1.3.6.1.4.1.14179.2.6.2.34.0"

WLAN_HOME_MACS = ["18af61cf2828","98fe9438fced"]

GCC_GUILLAUME = GccUserClass('guillaume','18af61cf2828',"15F434EF-D85E-408B-A261-2C931AA38BAA",False,False,"", datetime.datetime.now())
GCC_CANDOU = GccUserClass('candou','98fe9438fced',"065AAC44-79FE-41BE-8EA6-5461EE901C1C",False,False,"",datetime.datetime.now())

#  {'Name' : 'guillaume','TelMac' : '18af61cf2828', Presence : False, HasChanged : False}7
# GCC_CANDOU = {'Name' : 'candou','TelMac' : '98fe9438fced',Presence : False, HasChanged : False}
GCC_USERS = [GCC_GUILLAUME, GCC_CANDOU]
logging.basicConfig(format='%(asctime)s		%(levelname)s	%(funcName)30s()	%(message)s', level=logging.INFO)



# Local modules
# import feedparser

# Set up some global variables
num_fetch_threads = 1
PresenceEventQueue = Queue()


sched = BackgroundScheduler()

@sched.scheduled_job('cron', id='my_job_id', minute='0,30')
def HalfHourScheduledEvent():
	logging.info("Half Hour Scheduled Event Triggered !")
	heater_control.UpdateHeatersStates()
	
def IsThereAnybody():
	logging.info("Checking MYSQL to find if sbd is there ! ...")
	# Get previous status from MYSQL
	conn = mysql.connector.connect(host="localhost",user="root",password="raspberry", database="domotique")
	cursor = conn.cursor()
	cursor.execute("SELECT timestamp, guillaume_ishere, candou_ishere  from presence ORDER BY timestamp DESC LIMIT 1")
	row = cursor.fetchone()
	wasGuillaumeHere = bool(row[1])
	wasCandouHere = bool(row[2])
	conn.close()

	logging.info("Last presence status -> Guillaume ("+str(wasGuillaumeHere)+") Candou ("+str(wasCandouHere)+")")

	if wasGuillaumeHere | wasCandouHere:
		return True
	else:
		return False

def GeofenceEvent(data, trigger, provider):

	logging.info("New Geofence Event Received ("+trigger+" "+provider+")")
	logging.info(data)

	receivedDate = datetime.datetime.fromtimestamp(math.floor(float(data["timestamp"][0])))

	who = ""

	for gcc_user in GCC_USERS:
			if gcc_user.DeviceID in data["b'device"]:
				who = gcc_user
				logging.info("Geofence event related to G or C. Continue.")

	if who == "":
		logging.info("Geofence event not related to G or C. Abort !")
		return

	if provider == "locative":
		if trigger == "departure":
			who.Presence = False
			who.ChangeSource = "geofence"
			who.ChangeDate = receivedDate
			logging.info(trigger+" trigger found for user "+who.Name+" generated at "+receivedDate.strftime("%Y-%m-%d %H:%M"))
			PresenceEventQueue.put(who)
		elif trigger == "arrival":
			who.Presence = True
			who.ChangeSource = "geofence"
			who.ChangeDate = receivedDate
			logging.info(trigger+" trigger found for user "+who.Name+" generated at "+receivedDate.strftime("%Y-%m-%d %H:%M"))
			PresenceEventQueue.put(who)
	

def NewHouseStateAction(isSbd):
	
	logging.info("Changement de status de la maison (vide = 0, occupee = 1) --> "+str(int(isSbd)))
	
	## Eclairage
	if isSbd:
		#Eclairage
		s = sunrise.sun(lat=48.741906,long=1.660730) 
		sunrise_time = s.sunrise(when=datetime.datetime.utcnow())
		sunset_time = s.sunset(when=datetime.datetime.utcnow())
		logging.info("Lever du soleil du jour (UTC) : "+sunrise_time.strftime("%Y-%m-%d %H:%M"))
		logging.info("Coucher du soleil du jour (UTC) : "+sunset_time.strftime("%Y-%m-%d %H:%M"))
		currentDate = datetime.datetime.utcnow()

		if (currentDate.time() <= sunrise_time):
			logging.info("Il est entre 0h00 et sunrise (nuit)")
			logging.info("On allume")
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '1', '1'])
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '2', '1'])
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '3', '1'])
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '4', '1'])

		if (sunrise_time <= currentDate.time()) & (currentDate.time() <= sunset_time):
			logging.info("Il est entre Sunrise et Sunset (jour)")
			logging.info("Pas besoin de lumiere")

		if (currentDate.time() >= sunset_time):
			logging.info("Il est entre Sunset et 0h00 (nuit)")
			logging.info("On allume")
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '1', '1'])
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '2', '1'])
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '3', '1'])
			subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '4', '1'])
	else:
		#Eclairage
		logging.info("On Eteint")
		subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '1', '0'])
		subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '2', '0'])
		subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '3', '0'])
		subprocess.call(['/home/osmc/Conde_Home_Automation/LightControl/LightControl', '4', '0'])


	## Chauffage
	heater_control.UpdateHeatersStates()

def PresenceEvent(i, q):
	while True:
		logging.info("Looking for the next data in queue")
		presenceData = q.get()
		logging.info("Presence Event for user "+presenceData.Name+" (Trigger : "+presenceData.ChangeSource+")")

		# Droper les events qui sont considérés comme non pertinents
		if ((presenceData.Presence == False) & ((presenceData.ChangeSource == "wlc") | (presenceData.ChangeSource == "trap"))):
			logging.info("Got WLC Exit event - Ignoring")
			q.task_done()
			continue

		currentLocalDate = datetime.datetime.now()
		timestamp = int(time.time())
		rec_date = currentLocalDate.strftime("%Y-%m-%d")
		rec_time = currentLocalDate.strftime("%H:%M:%S")
		sqlrec=""

		whoIsHere = []
		isCandouHere = False
		isGuillaumeHere = False;
		isTrapGC = False

		# Get previous status from MYSQL
		conn = mysql.connector.connect(host="localhost",user="root",password="raspberry", database="domotique")
		cursor = conn.cursor()
		cursor.execute("SELECT timestamp, guillaume_ishere, candou_ishere  from presence ORDER BY timestamp DESC LIMIT 1")
		row = cursor.fetchone()
		wasGuillaumeHere = bool(row[1])
		wasCandouHere = bool(row[2])
		conn.close()

		if presenceData.Name == "guillaume":
			if (wasGuillaumeHere == presenceData.Presence):
				logging.info("NOTHING HAS CHANGED - NO ACTION")
				q.task_done()
				continue
			
			else:
				
				#getting last update ts & type
				conn = mysql.connector.connect(host="localhost",user="root",password="raspberry", database="domotique")
				cursor = conn.cursor()
				cursor.execute("SELECT timestamp, guillaume_ishere, candou_ishere, change_trigger  from presence WHERE guillaume_ishere = "+str(int(not presenceData.Presence))+"  ORDER BY timestamp DESC LIMIT 1")
				row = cursor.fetchone()
				ts = int(row[0])
				lastType = str(row[3])
				conn.close()
				logging.info("Last known update for user "+presenceData.Name+" : "+str(ts)+" "+lastType)
				
				# Wait 30 minutes if getting a WLC presence notification, but geofence occured only 30 min before
				if (((presenceData.ChangeSource == "wlc") | (presenceData.ChangeSource == "trap")) & (lastType == "geofence")):
					logging.info("WLC/Trap : Checking if there was no recent Geofence notification")

					if timestamp - 1800 > ts:
						logging.info("WLC/Trap : Valid event. Last geofence ocured more that 30 min earlier")
					else:
						logging.info("WLC/Trap : INVALID EVENT. GEOFENCE OCCURED TOO RECENTLY")
						q.task_done()
						continue

				# Check if geofence request is still valid (= not superseded by other event)
				# We find the timestamp of the oposite event in Mysql and compares with the Geofenced event
				if (presenceData.ChangeSource == "geofence"):
					logging.info("Geofence : Checking validity of timestamp")

					if presenceData.ChangeDate > datetime.datetime.fromtimestamp(ts):
						logging.info("Geofence : Valid timestamp (earlier than last update). Continue.")
					else:
						logging.info("INVALID GEOFENCE TIMESTAMP - NO ACTION")
						q.task_done()
						continue

				rec_dateg = presenceData.ChangeDate.strftime("%Y-%m-%d")
				rec_timeg = presenceData.ChangeDate.strftime("%H:%M:%S")

				str(int(round(time.mktime(presenceData.ChangeDate.timetuple()))))

				sqlrec = "REPLACE INTO presence (timestamp, rec_date, rec_time, change_trigger, guillaume_ishere, candou_ishere) VALUES ("+str(int(round(time.mktime(presenceData.ChangeDate.timetuple()))))+",'"+rec_dateg+"','"+rec_timeg+"','"+presenceData.ChangeSource+"',"+str(int(presenceData.Presence))+","+str(int(wasCandouHere))+")"

				#print(sqlrec)
				isGuillaumeHere = presenceData.Presence
				isCandouHere = wasCandouHere

		elif presenceData.Name == "candou":
			if (wasCandouHere == presenceData.Presence):
				logging.info("NOTHING HAS CHANGED - NO ACTION")
				q.task_done()
				continue
			else:

				#getting last update ts & type
				conn = mysql.connector.connect(host="localhost",user="root",password="raspberry", database="domotique")
				cursor = conn.cursor()
				cursor.execute("SELECT timestamp, guillaume_ishere, candou_ishere, change_trigger  from presence WHERE candou_ishere = "+str(int(not presenceData.Presence))+"  ORDER BY timestamp DESC LIMIT 1")
				row = cursor.fetchone()
				ts = int(row[0])
				lastType = str(row[3])
				conn.close()
				logging.info("Last known update for user "+presenceData.Name+" : "+str(ts)+" "+lastType)
				
				# Wait 30 minutes if getting a WLC presence notification, but geofence occured only 30 min before
				if (((presenceData.ChangeSource == "wlc") | (presenceData.ChangeSource == "trap")) & (lastType == "geofence")):
					logging.info("WLC/Trap : Checking if there was no recent Geofence notification")

					if timestamp - 1800 > ts:
						logging.info("WLC/Trap : Valid event. Last geofence ocured more that 30 min earlier")
					else:
						logging.info("WLC/Trap : INVALID EVENT. GEOFENCE OCCURED TOO RECENTLY")
						q.task_done()
						continue


				# Check if geofence request is still valid (= not superseded by other event)
				# We find the timestamp of the oposite event in Mysql and compares with the Geofenced event
				if (presenceData.ChangeSource == "geofence"):
					logging.info("Geofence : Checking validity of timestamp")
					conn = mysql.connector.connect(host="localhost",user="root",password="raspberry", database="domotique")
					cursor = conn.cursor()
					cursor.execute("SELECT timestamp, guillaume_ishere, candou_ishere  from presence WHERE candou_ishere = "+str(int(not presenceData.Presence))+"  ORDER BY timestamp DESC LIMIT 1")
					row = cursor.fetchone()
					ts = int(row[0])
					conn.close()

					if presenceData.ChangeDate > datetime.datetime.fromtimestamp(ts):
						logging.info("Geofence : Valid timestamp (earlier than last update). Continue.")
					else:
						logging.info("INVALID GEOFENCE TIMESTAMP - NO ACTION")
						q.task_done()
						continue

				rec_datec = presenceData.ChangeDate.strftime("%Y-%m-%d")
				rec_timec = presenceData.ChangeDate.strftime("%H:%M:%S")

				sqlrec = "REPLACE INTO presence (timestamp, rec_date, rec_time, change_trigger, guillaume_ishere, candou_ishere) VALUES ("+str(int(round(time.mktime(presenceData.ChangeDate.timetuple()))))+",'"+rec_datec+"','"+rec_timec+"','"+presenceData.ChangeSource+"',"+str(int(wasGuillaumeHere))+","+str(int(presenceData.Presence))+")"

				#print(sqlrec)
				isGuillaumeHere = wasGuillaumeHere
				isCandouHere = presenceData.Presence

		else:
			q.task_done()
			continue

		#if needed, write new status in mysql
		logging.info("Writing new status to MYSQL")
		logging.info("SQL Request : "+sqlrec)
		conn = mysql.connector.connect(host="localhost",user="root",password="raspberry", database="domotique")
		cursor = conn.cursor()
		cursor.execute(sqlrec)
		conn.commit()
		conn.close()

		logging.info("wasCandouHere "+str(int(wasCandouHere)))
		logging.info("wasGuillaumeHere "+str(int(wasGuillaumeHere)))
		logging.info("isCandouHere "+str(int(isCandouHere)))
		logging.info("isGuillaumeHere "+str(int(isGuillaumeHere)))

		# Si plus personne à la maison (00)
		if (((wasCandouHere == True) | (wasGuillaumeHere == True)) & ((isGuillaumeHere == False) & (isCandouHere == False))):
			NewHouseStateAction(False)

		# Si qqcn viens d'arriver (00 > 10/01)
		if (((wasCandouHere == False) & (wasGuillaumeHere == False)) & ((isGuillaumeHere == True) | (isCandouHere == True))):
			NewHouseStateAction(True)
		
		
		q.task_done()

def UpdateWlanUsers():
	macs = GetWlanUsersMac()

	## A adapter pour la sortie des gens ... Pas fiable (utilise la géoloc plutot)
	for gcc_user in GCC_USERS:
		if gcc_user.TelMac in macs:
			gcc_user.Presence = True
			gcc_user.ChangeDate = datetime.datetime.fromtimestamp(int(time.time()))
			gcc_user.ChangeSource = "wlc"
			logging.info("Filling Queue")
			PresenceEventQueue.put(gcc_user)
		else:
			gcc_user.Presence = False
			gcc_user.ChangeDate = datetime.datetime.fromtimestamp(int(time.time()))
			gcc_user.ChangeSource = "wlc"
			logging.info("Filling Queue")
			PresenceEventQueue.put(gcc_user)

def GetWlanUsersMacUpdaterThread():
	while True:
		logging.info("Wlan Assoc Update - Toute les minutes")
		UpdateWlanUsers()
		time.sleep(600)

def GetWlanUsersMac():
	
	wlanMacs = []

	logging.info("-> Requete SNMP de recherche des clients WLAN")
	
	try:
		for errorIndication, errorStatus, \
			errorIndex, varBinds in bulkCmd(
				SnmpEngine(),
				CommunityData(WLC_SNMP_COMMUNITY),
				UdpTransportTarget((WLC_SNMP_IP, WLC_SNMP_PORT)),
				ContextData(),
				0, 40,  # GETBULK specific: request up to 50 OIDs in a single response
				ObjectType(ObjectIdentity(WLC_SNMP_MAC_CLIENTS_OID)),
				lookupMib=False, lexicographicMode=False):

			if errorIndication:
				logging.info(errorIndication)
				break
			elif errorStatus:
				logging.info('%s at %s' % (errorStatus.prettyPrint(),
									errorIndex and varBinds[int(errorIndex)-1][0] or '?'))
				break
			else:
				for varBind in varBinds:
					logging.info(' = '.join([x.prettyPrint() for x in varBind]))
					wlanMacs.append(varBind[1].prettyPrint().lstrip("0x"))
					
		return wlanMacs
	except:
		logging.info("Erreur SNMP (Network unreachable ??)")
		return []
		
	
	


def SnmpWifiTrapReceiver(transportDispatcher, transportDomain, transportAddress, wholeMsg):
		
	assNotifFound = False
	deAssNotifFound = False
	goodNotif = False
	clientMac = ""

	while wholeMsg:
		msgVer = int(api.decodeMessageVersion(wholeMsg))
		if msgVer in api.protoModules:
			pMod = api.protoModules[msgVer]
		else:
			logging.info('Unsupported SNMP version %s' % msgVer)
			return
		reqMsg, wholeMsg = decoder.decode(
			wholeMsg, asn1Spec=pMod.Message(),
			)
		logging.info('Notification message from %s:%s: ' % (
			transportDomain, transportAddress
			)
		)
		reqPDU = pMod.apiMessage.getPDU(reqMsg)
		if reqPDU.isSameTypeWith(pMod.TrapPDU()):
			varBinds = pMod.apiPDU.getVarBindList(reqPDU)
			logging.info('Var-binds:')
			for oid, val in varBinds:
				
				if oid.prettyPrint() == WLC_SNMP_TRAP_EVENT_OID:
					logging.info("TRAP OID DETECTED")
					trapOid = val.getComponent().getComponent().getComponent().prettyPrint()

					if trapOid == WLC_SNMP_TRAP_ASSOCIATION_OID:
						assNotifFound = True
						goodNotif = True
						logging.info("ASSOCIATION DETECTED")

					if trapOid == WLC_SNMP_TRAP_DISASSOCIATION_OID:
						deAssNotifFound = True
						goodNotif = True
						logging.info("DISASSOCIATION DETECTED")

				if oid.prettyPrint() == WLC_SNMP_TRAP_MACBS_OID:
					clientMac = val.getComponent().getComponent().getComponent().prettyPrint().lstrip("0x")
					logging.info("BS MAC ADDRESS : "+clientMac)

			if (goodNotif == False):
				logging.info("TRAP NOT USEFULL - DROPPING")
				return

	isTrapGC = False
	#foundUser = GccUserClass()
	

	# In Fact ... We don't care about the trap content ... Not relevant. We just use the trap as a trigger to check Wlan Assocs usint the SNMP API
	

	for gccUser in GCC_USERS:
		if gccUser.TelMac == clientMac:
			foundUser = gccUser
			isTrapGC = True
	
	if isTrapGC:
		if assNotifFound:
			foundUser.Presence = True
			foundUser.ChangeSource = "trap"
		if deAssNotifFound:	
			foundUser.Presence = False
			foundUser.ChangeSource = "trap"

		logging.info("TRAP RELATED TO GUILLAUME OR CANDOU - LAUNCHING UpdateWlanUsers()")
		#PresenceEventQueue.put(foundUser)
		UpdateWlanUsers()
		
	else:
		logging.info("TRAP NOT RELATED TO GUILLAUME OR CANDOU")

	

def SNMPTrapListenerThread():
		
	transportDispatcher = AsynsockDispatcher()
	transportDispatcher.registerRecvCbFun(SnmpWifiTrapReceiver)
	transportDispatcher.registerTransport(udp.domainName, udp.UdpSocketTransport().openServerMode(('0.0.0.0', 162)))
	transportDispatcher.jobStarted(1)
	transportDispatcher.runDispatcher()


def StartRestServerThread():
		
	server = ThreadedHTTPServer(('0.0.0.0', 3001), MyRequestHandler)
	logging.info("Starting REST server, use <Ctrl-C> to stop")
	#server.serve_forever()
	
	# --- CAN BE DELETED AFTER SUCESSFULL THREADING TEST ---
	# Handler = MyRequestHandler
	# SocketServer.TCPServer.allow_reuse_address = True
	# server = SocketServer.TCPServer(('0.0.0.0', 8080), Handler)
	
	#server.serve_forever() --> VRAIEMENT COMMENTE ...
	
	thread = threading.Thread(target = server.serve_forever)
	thread.setDaemon(False)
	thread.start()
	# -------------------------------------------------------
	
	logging.info("--- REST Server started (3001) ---")
	return server

class ThreadedHTTPServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
	"""Handle requests in a separate thread."""	
class MyRequestHandler(http.server.SimpleHTTPRequestHandler):
	def do_POST(self):
			
		if None != re.search('/locativeArrivalTrigger', self.path):
			self.send_response(200)
			self.send_header('Access-Control-Allow-Origin', '*')
			self.end_headers()
			
			content_len = int(self.headers.get('content-length', 0))
			post_body = self.rfile.read(content_len)
			
			datas = urllib.parse.parse_qs(str(post_body))

			#logging.info(datas)
			GeofenceEvent(datas, "arrival","locative")
			

		if None != re.search('/locativeDepartureTrigger', self.path):
			self.send_response(200)
			self.send_header('Access-Control-Allow-Origin', '*')
			self.end_headers()
			
			content_len = int(self.headers.get('content-length', 0))
			post_body = self.rfile.read(content_len)
			
			datas = urllib.parse.parse_qs(str(post_body))
			#logging.info(datas)

			GeofenceEvent(datas, "departure","locative")



		if None != re.search('/setHeaters', self.path):
			self.send_response(200)
			self.send_header('Access-Control-Allow-Origin', '*')
			self.end_headers()
			
			content_len = int(self.headers.get('content-length', 0))
			post_body = self.rfile.read(content_len)
			
			data = post_body.decode('ascii')

			# Possible data
			# - Auto
			# - Sun1/Moon1/Hg1/Off1

			#logging.info(datas)
			heater_control.SetHeaterCommand(data)


	def do_GET(self):
			
		if None != re.search('/getHeaters', self.path):
			self.send_response(200)
			self.send_header('Content-Type', 'application/json')
			self.send_header('Access-Control-Allow-Origin', '*')
			self.end_headers()
			
			#Build
			dataDict = {"globalMode" : heater_control.globalMode,
			"modeZ1" : heater_control.modeZ1,
			"modeZ2" : heater_control.modeZ2,
			"modeZ3" : heater_control.modeZ3,
			"tempRemaining" : heater_control.tempRemaining}
			
			#logging.info(json.dumps(dataDict))

			self.wfile.write(json.dumps(dataDict).encode())


	def log_message(self, format, *args):
		#sys.stdout.write("%s --> [%s] %s\n" % (self.address_string(), self.log_date_time_string(), format%args))
		logging.info("REST request received : "+format%args)



if __name__ == '__main__':

	# Start scheduler (Cron-Like) to update Heater
	sched.start()

	# Set up some threads to manage presence event queue
	for i in range(num_fetch_threads):
		worker = Thread(target=PresenceEvent, args=(i, PresenceEventQueue,))
		worker.setDaemon(False)
		worker.start()

	# Start SNMP Trap Catcher Thread
	thread = threading.Thread(target = SNMPTrapListenerThread)
	thread.setDaemon(False)
	thread.start()

	# Start Wlan Assoc Refresh Thread
	thread = threading.Thread(target = GetWlanUsersMacUpdaterThread)
	thread.setDaemon(False)
	thread.start()

	# Start REST Server (Geofence) - Not very stable - to be checked
	StartRestServerThread()

	#Start X2D Queue
	heater_control.SendX2DCommandThread()

	#Inital update of the heaters (before regular 30 minutes scheduled update)
	time.sleep(10)
	heater_control.UpdateHeatersStates()

	# thread = threading.Thread(target = RestSrv)
	# thread.setDaemon(False)
	# thread.start()

	#time.sleep(10)
	while True:
		#print("start heaters upsate")
		#heater_control.UpdateHeatersStates()
		time.sleep(600)

	# Download the feed(s) and put the enclosure URLs into
	# # the queue.
	# for url in feed_urls:
	# 	logging.info('Queuing:', url)
	# 	PresenceEventQueue.put(url)
			
	# Now wait for the queue to be empty, indicating that we have
	# processed all of the downloads.
	# logging.info('*** Main thread waiting')
	# enclosure_queue.join()
	# logging.info('*** Done')
