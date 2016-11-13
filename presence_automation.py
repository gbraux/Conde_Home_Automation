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

class GccUserClass():
	# Rocket simulates a rocket ship for a game,
	#  or a physics simulation.
	
	def __init__(self, Name, TelMac, Presence, HasChanged, ChangeSource):
		# Each rocket has an (x,y) position.
		self.Name = Name
		self.TelMac = TelMac
		self.Presence = Presence
		self.HasChanged = HasChanged
		self.ChangeSource = ChangeSource

WLC_SNMP_COMMUNITY = "public"
WLC_SNMP_IP = "192.168.11.251"
WLC_SNMP_PORT = 161
WLC_SNMP_MAC_CLIENTS_OID = "1.3.6.1.4.1.14179.2.1.4.1.1"
WLC_SNMP_TRAP_EVENT_OID = "1.3.6.1.6.3.1.1.4.1.0"
WLC_SNMP_TRAP_ASSOCIATION_OID = "1.3.6.1.4.1.14179.2.6.3.53"
WLC_SNMP_TRAP_DISASSOCIATION_OID = "1.3.6.1.4.1.14179.2.6.3.1"
WLC_SNMP_TRAP_MACBS_OID = "1.3.6.1.4.1.14179.2.6.2.34.0"

WLAN_HOME_MACS = ["18af61cf2828","98fe9438fced"]

GCC_GUILLAUME = GccUserClass('guillaume','18af61cf2828',False,False,"")
GCC_CANDOU = GccUserClass('candou','98fe9438fced',False,False,"")

#  {'Name' : 'guillaume','TelMac' : '18af61cf2828', Presence : False, HasChanged : False}7
# GCC_CANDOU = {'Name' : 'candou','TelMac' : '98fe9438fced',Presence : False, HasChanged : False}
GCC_USERS = [GCC_GUILLAUME, GCC_CANDOU]
logging.basicConfig(format='%(asctime)s		%(levelname)s	%(funcName)30s()		%(message)s', level=logging.DEBUG)



# Local modules
# import feedparser

# Set up some global variables
num_fetch_threads = 1
PresenceEventQueue = Queue()

# A real app wouldn't use hard-coded data...
feed_urls = [ 'http://www.castsampler.com/cast/feed/rss/guest', 'caca', 'boudin','titi'
			 ]

def NewHouseStateAction(isSbd):
	
	logging.info("Changement de status de la maison (vide = 0, occupee = 1) --> "+str(int(isSbd)))
	
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
			subprocess.call(['/home/osmc/LightControl', '1', '1'])
			subprocess.call(['/home/osmc/LightControl', '2', '1'])
			subprocess.call(['/home/osmc/LightControl', '3', '1'])
			subprocess.call(['/home/osmc/LightControl', '4', '1'])

		if (sunrise_time <= currentDate.time()) & (currentDate.time() <= sunset_time):
			logging.info("Il est entre Sunrise et Sunset (jour)")
			logging.info("Pas besoin de lumiere")

		if (currentDate.time() >= sunset_time):
			logging.info("Il est entre Sunset et 0h00 (nuit)")
			logging.info("On allume")
			subprocess.call(['/home/osmc/LightControl', '1', '1'])
			subprocess.call(['/home/osmc/LightControl', '2', '1'])
			subprocess.call(['/home/osmc/LightControl', '3', '1'])
			subprocess.call(['/home/osmc/LightControl', '4', '1'])
	else:
		#Eclairage
		logging.info("On Eteint")
		subprocess.call(['/home/osmc/LightControl', '1', '0'])
		subprocess.call(['/home/osmc/LightControl', '2', '0'])
		subprocess.call(['/home/osmc/LightControl', '3', '0'])
		subprocess.call(['/home/osmc/LightControl', '4', '0'])

def PresenceEvent(i, q):
	while True:
		logging.info("Looking for the next data in queue")
		presenceData = q.get()
		logging.info("Presence Event for user "+presenceData.Name)

		currentLocalDate = datetime.datetime.now()
		timestamp = int(time.time())
		rec_date = currentLocalDate.strftime("%Y-%m-%d")
		rec_time = currentLocalDate.strftime("%H:%M:%S")
		sqlrec=""

		whoIsHere = []
		isCandouHere = False
		isGuillaumeHere = False;
		isTrapGC = False

		# Get old status from MYSQL
		conn = mysql.connector.connect(host="localhost",user="root",password="raspberry", database="domotique")
		cursor = conn.cursor()
		cursor.execute("SELECT guillaume_ishere, candou_ishere  from presence ORDER BY timestamp DESC LIMIT 1")
		row = cursor.fetchone()
		wasGuillaumeHere = bool(row[0])
		wasCandouHere = bool(row[1])
		conn.close()


		if presenceData.Name == "guillaume":
			if (wasGuillaumeHere == presenceData.Presence):
				logging.info("NOTHING HAS CHANGED - NO ACTION")
				q.task_done()
				continue
			else:
				sqlrec = "INSERT INTO presence (timestamp, rec_date, rec_time, guillaume_ishere, candou_ishere) VALUES ("+str(timestamp)+",'"+rec_date+"','"+rec_time+"',"+str(int(presenceData.Presence))+","+str(int(wasCandouHere))+")"
				isGuillaumeHere = presenceData.Presence
				isCandouHere = wasCandouHere

		elif presenceData.Name == "candou":
			if (wasCandouHere == presenceData.Presence):
				logging.info("NOTHING HAS CHANGED - NO ACTION")
				q.task_done()
				continue
			else:
				sqlrec = "INSERT INTO presence (timestamp, rec_date, rec_time, guillaume_ishere, candou_ishere) VALUES ("+str(timestamp)+",'"+rec_date+"','"+rec_time+"',"+str(int(wasGuillaumeHere))+","+str(int(presenceData.Presence))+")"
				isGuillaumeHere = wasGuillaumeHere
				isCandouHere = presenceData.Presence

		else:
			q.task_done()
			continue

		#if needed, write new status in mysql
		logging.info("Writing new status to MYSQL")
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
			gcc_user.ChangeSource = "wlc"
			logging.info("Filling Queue")
			PresenceEventQueue.put(gcc_user)
		else:
			gcc_user.Presence = False
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

if __name__ == '__main__':



	# Set up some threads to fetch the enclosures
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

	while True:
		time.sleep(5)

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