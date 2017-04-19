import serial
import time

ser = serial.Serial('/dev/ttyACM0',9600)
ser.open()
time.sleep(5)
while True:
	incdata = ser.readline()
	file = open('cage1.txt', 'a')
	file.write(incdata)
	file.close()