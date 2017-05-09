#!/bin/bash/python
import serial.tools.list_ports
import sys
import atexit
import platform

ARUDINO_SERIAL_NUMBER_LENGTH = 20
ARDUINO_SERIAL_NUMBER_IDENTIFIER = "SER="
ARDUINO_UNO_PID_CC = "VID:PID=2341:0043"
ARDUINO_UNO_PID_ORG = "VID:PID=2A03:0043"
SERIAL_PORT_PATH = "/dev"

# class Arduino:
#     def __init__(serialNumber, serialPort):
#         self.serialNumber = serialNumber
#         self.serialPort = serialPort

def findArduinoSerialNumber(info):
    serialBegin = info.index(ARDUINO_SERIAL_NUMBER_IDENTIFIER) + len(ARDUINO_SERIAL_NUMBER_IDENTIFIER)
    serialEnd = serialBegin + ARUDINO_SERIAL_NUMBER_LENGTH

    return info[serialBegin:serialEnd]

def findArduinoUnoPort():
    portList = list(serial.tools.list_ports.comports())

    for port in portList:
        isArduinoUno = True
        pidIndex = 0
        portIndex = 0

        # detect where the serial number is located (for Arduino.cc)
        if ARDUINO_UNO_PID_CC in port[0]:
            print("SerialNumberPortIndex0")
            pidIndex = 0
        elif ARDUINO_UNO_PID_CC in port[1]:
            print("SerialNumberPortIndex1")
            pidIndex = 1
        elif ARDUINO_UNO_PID_CC in port[2]:
            print("SerialNumberPortIndex3")
            pidIndex = 2
        elif ARDUINO_UNO_PID_ORG in port[0]:
            print("SerialNumberPortIndex0")
            pidIndex = 0
        elif ARDUINO_UNO_PID_ORG in port[1]:
            print("SerialNumberPortIndex1")
            pidIndex = 1
        elif ARDUINO_UNO_PID_ORG in port[2]:
            print("SerialNumberPortIndex3")
            pidIndex = 2
        else:
            print("Noduino :(")
            isArduinoUno = False

        # detect where the serial port path is located
        if SERIAL_PORT_PATH in port[0]:
            print("SerialPortPath0")
            portIndex = 0
        elif SERIAL_PORT_PATH in port[1]:
            print("SerialPortPath1")
            portIndex = 1
        elif SERIAL_PORT_PATH in port[2]:
            print("SerialPortPath2")
            portIndex = 2

        if isArduinoUno:
            serialNumber = findArduinoSerialNumber(port[pidIndex])

            print("Arduino[" + serialNumber + "]: " + port[portIndex])

            return port[portIndex]
        else:
            print("Device was not an Arduino")


unoPort = findArduinoUnoPort()
serialUno = serial.Serial(unoPort, 9600)

while True:
    #try:
        valueRead = serialUno.readline(500)
        print(valueRead)
    #except:
     #   pass

'''
while True:
    try:
        if serialUno.inWaiting() != 0:
            valueRead = serialUno.readline(500)
            #TODO: Do SQL stuff here later
            print(valueRead)
    except:
        try:
            unoPort = findArduinoUnoPort()

            if unoPort:
                serialUno = serial.Serial(unoPort, 9600)
        except:
            pass
'''
