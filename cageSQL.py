#!/usr/bin/env python
import serial.tools.list_ports
import sys
import atexit
import platform
import time
import MySQLdb

# Arduino detection information
ARUDINO_SERIAL_NUMBER_LENGTH = 20
ARDUINO_SERIAL_NUMBER_IDENTIFIER = "SER="
ARDUINO_UNO_PID = "VID:PID=2341:0043"
SERIAL_PORT_PATH = "/dev"

arduinoStatus = {}
connectedArduinos = {}

# DB Data Indeces
CAGE = 0
PROGRAM = 1
MOUSE = 2
TRIAL = 3
DURATION = 4
RESULT = 5
WAIT_FOR_AP = 6
WAITED = 7
ITERATION = 8

DATA_LENGTH = 9

def parseInput(data):
    parsedData = []

    for text in data:
        parsedData.append(int(text))
    
    return parsedData

def getCurrentMillis():
    return int(time.time() * 1000)

def saveArduinoEvents(serialNumber, serialPort, event):
    timestamp = getCurrentMillis()
    
    db = MySQLdb.connect(
        host="localhost",
        user="root",
        passwd="root",
        db="cage")

    cur = db.cursor()

    try:
        cur.execute("""
            INSERT INTO arduino_events
            (`serialNumber`, `serialPort`, `timestamp`, `event`)
            VALUES(%s,%s,%s,%s)""", (
            serialNumber, serialPort, timestamp, event
        ))
        db.commit()
    except MySQLdb.Error, e:
        try:
            print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            return None
        except IndexError:
            print "MySQL Error: %s" % str(e)
            return None
    except TypeError, e:
        print("MySQL Error: TypeError: %s" % str(e))
        return None
    except ValueError, e:
        print("MySQL Error: ValueError: %s" % str(e))
        return None  

    db.close()

def saveToDatabase(data):
    print("Parsing data...")
    data = parseInput(data)
    
    print("Create a connection to cage database")
    db = MySQLdb.connect(
        host="localhost",
        user="root",
        passwd="root",
        db="cage")

    print("Create cursor to execute queries")
    cur = db.cursor()

    print("Calculating timestamps")
    endTimestamp = getCurrentMillis()
    startTimestamp = endTimestamp - data[DURATION]

    print("Store information in database")
    try:
        cur.execute("""
            INSERT INTO trial_data
            (`cage`, `program`, `mouse`, `trial`, `trialStart`, `trialEnd`, `duration`, `result`, `wait4AP`, `waited`, `iteration`)
            VALUES(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)""", (
            data[CAGE], data[PROGRAM], data[MOUSE], data[TRIAL], startTimestamp, endTimestamp,
            data[DURATION], data[RESULT], data[WAIT_FOR_AP], data[WAITED], data[ITERATION]
        ))
        db.commit()
        print("Commit")
    except MySQLdb.Error, e:
        try:
            print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            return None
        except IndexError:
            print "MySQL Error: %s" % str(e)
            return None
    except TypeError, e:
        print("MySQL Error: TypeError: %s" % str(e))
        return None
    except ValueError, e:
        print("MySQL Error: ValueError: %s" % str(e))
        return None  

    print("Close database connection")
    db.close()

def findArduinoSerialNumber(info):
    serialBegin = info.index(ARDUINO_SERIAL_NUMBER_IDENTIFIER) + len(ARDUINO_SERIAL_NUMBER_IDENTIFIER)
    serialEnd = serialBegin + ARUDINO_SERIAL_NUMBER_LENGTH

    return info[serialBegin:serialEnd]

def findArduinos():
    portList = list(serial.tools.list_ports.comports())

    for port in portList:
        isArduino = True
        pidIndex = 0
        portIndex = 0

        # detect where the serial number is located
        if ARDUINO_UNO_PID in port[0]:
            pidIndex = 0
        elif ARDUINO_UNO_PID in port[1]:
            pidIndex = 1
        elif ARDUINO_UNO_PID in port[2]:
            pidIndex = 2
        else:
            isArduino = False

        if isArduino:
            # detect where the serial port path is located
            if SERIAL_PORT_PATH in port[0]:
                portIndex = 0
            elif SERIAL_PORT_PATH in port[1]:
                portIndex = 1
            elif SERIAL_PORT_PATH in port[2]:
                portIndex = 2

            serialPort = port[portIndex]
            serialNumber = findArduinoSerialNumber(port[pidIndex])

            # Check if Arduino is already in list
            if not serialNumber in connectedArduinos:
                print("New Arduino " + serialNumber + " on port " + serialPort)
                device = serial.Serial(serialPort, 9600)
                connectedArduinos[serialNumber] = device
                arduinoStatus[serialNumber] = True
                saveArduinoEvents(serialNumber, serialPort, 'CONNECT')
            else:
                device = connectedArduinos[serialNumber]

                if device.port != serialPort:
                    print("Reconnecting to Arduino " + serialNumber + " on port " + serialPort)
                    device.close()
                    device.setPort(serialPort)
                    device.open()
                    arduinoStatus[serialNumber] = True
                    saveArduinoEvents(serialNumber, serialPort, 'RECONNECT')
                    
while True:
    findArduinos()

    #print("Connected Arudinos: " + str(len(connectedArduinos)))

    for deviceID in connectedArduinos:
        device = connectedArduinos[deviceID]
        try:
            #print("Checking input on " + deviceID);
            if device.inWaiting() != 0:
                # Read and print raw data
                rawData = device.readline(500)
                print(rawData)

                # Check for Arduino reset status message
                if "RESET ARDUINO" in rawData:
                    saveArduinoEvents(deviceID, device.port, 'DAILY_RESET')

                # If experiment data, store them
                data = rawData.split()
                if len(data) == DATA_LENGTH:
                    print("Saving data...")
                    saveToDatabase(data)
        except:
            if arduinoStatus[deviceID] == True:
                arduinoStatus[deviceID] = False
                print("Arduino " + deviceID + " on port " + device.port + " disconnected!")
                saveArduinoEvents(deviceID, device.port, 'DISCONNECT')
            pass

