import time
import serial
import os
import sys

if os.name == 'nt':
	ser = serial.Serial(
		port='COM10',
		baudrate=9600
		
	)
	
	ser.rts = False
	ser.dtr = False
else:
	ser = serial.Serial(
		port='/dev/ttyUSB0',
		baudrate=9600
	)
	
	ser.rts = False
	ser.dtr = False
	#ser.open()


if ser.isOpen() == False:
    ser.open()


if len(sys.argv) != 2 :
	print("This tool need a command to send")
else:
	cmd=sys.argv[1]
	
	out = ''
	# wait for RFbee init
	time.sleep(3)
	while ser.inWaiting() > 0:
		out += str(ser.read(1).decode("ascii"))
		
	if out != '':
		print(">>" + out)
	
	time.sleep(3)
	ser.write(bytearray(cmd+"\n",'ascii'))
	print(">>%s"%(cmd))
	
	out = ''
	# let's wait one second before reading output (let's give device time to answer)
	time.sleep(1)
	while ser.inWaiting() > 0:
		out += str(ser.read(1).decode("ascii"))
		
	if out != '':
		print(">>" + out)
