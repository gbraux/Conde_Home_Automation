#!/usr/bin/env python
# -*- coding: utf-8 -*-
import http.server
import http.client
import socketserver
import base64
import re
import sqlite3
import datetime
import urllib.parse
import json
import threading
import time
import sys
import signal
import logging
import cgi
import http.server
import urllib.request
import xml.etree.ElementTree as ET

import presence_automation

def startRestServerThread():
	
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
			presence_automation.GeofenceEvent(datas, "arrival","locative")
			

		if None != re.search('/locativeDepartureTrigger', self.path):
			self.send_response(200)
			self.send_header('Access-Control-Allow-Origin', '*')
			self.end_headers()
			
			content_len = int(self.headers.get('content-length', 0))
			post_body = self.rfile.read(content_len)
			
			datas = urllib.parse.parse_qs(str(post_body))
			#logging.info(datas)

			presence_automation.GeofenceEvent(datas, "departure","locative")


	def log_message(self, format, *args):
		#sys.stdout.write("%s --> [%s] %s\n" % (self.address_string(), self.log_date_time_string(), format%args))
		logging.info("Post REST request received : "+format%args)



#startThread()