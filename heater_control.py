import time
import serial
import os
import sys
import csv
import HandlePlanning
from datetime import  datetime
import logging
import HandlePlanning


def UpdateHeatersStates():
    path=os.path.dirname(__file__)+'/'
    if path=='/' :
            path='./'

    data=list()
    dayNames=['lundi', 'mardi', 'mercredi', 'jeudi', 'vendredi', 'samedi', 'dimanche']
    stateNames=['Moon','Sun']

    with open('chauffage.csv', 'rt') as f:
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

    stat1=stateNames[zone1.getStatus(dayName, timeStr)]
    logging.info("Heaters Status for Zone 1 : "+stat1)
    stat2=stateNames[zone2.getStatus(dayName, timeStr)]
    logging.info("Heaters Status for Zone 2 : "+stat2)


    
    #os.system("python3 %spyX2DCmd.py %s1"%(path,stat1))
    #time.sleep(15)
    #os.system("python3 %spyX2DCmd.py %s2"%(path,stat2))

    SendX2DCommand(stat1+"1")
    SendX2DCommand(stat2+"2")


    '''  
    # Some Tests
    zone1.getStatus("mercredi", "10:30:05")
    zone1.getStatus("mercredi", "11:00:05")
    zone1.getStatus("mercredi", "11:30:05")
    zone1.getStatus("mercredi", "12:00:05")
    '''

def SendX2DCommand(command):
    
    logging.info("Opening Serial Port")
    
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
        logging.info("Serial Port Opened")

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