import RPi.GPIO as GPIO ## Import GPIO library
import serial 
import time

serout = serial.Serial('/dev/ttyACM0', 19200)	#arduino1 connected via USB
ser = serial.Serial('/dev/ttyUSB0', 9600)	#arduino2 connected via USB

GPIO.setmode(GPIO.BOARD) ## Use board pin numbering


GPIO.setup(19, GPIO.OUT) ## Setup GPIO Pin 19 to OUTPUT
GPIO.setup(21, GPIO.OUT) ## Setup GPIO Pin 21 to OUTPUT

GPIO.setup(7, GPIO.IN) ## Setup GPIO Pin 11 to INPUT
GPIO.setup(11, GPIO.IN) ## Setup GPIO Pin 7 to INPUT

GPIO.output(21,True) ##set RED on
GPIO.output(19,False) ##set GREEN off

red=True;
green=False;



def followmefun():
	global red,green
	while True:
		#Stop Follow me 
		if (GPIO.input(11)):	#stop button (near red LED) pressed
			serout.write("ms")
			time.sleep(0.01)
			serout.write("ss")
			GPIO.output(19,False)	#Light off Green LED
			green=False
			GPIO.output(21,True)	#Light on RED LED
			red=True
			break
		
		else:
			print("Going to read serial")
			reading=ser.readline()	#read arduino
			print("Serial read successfully!")
			sensors=(reading.split("#")[1])	#get only the value ex: 1101
			print (sensors)
			#move forward
			if(sensors=="1" or sensors=="10" or sensors=="11"):
				serout.write("mf")
				time.sleep(0.01)
				serout.write("ss")
				if(red):
					GPIO.output(21,False)	#Light off RED LED
					red=False
					
			#move right		
			elif(sensors=="100" or 	sensors=="101" or sensors=="110" or sensors=="111"):
				serout.write("mf")
				time.sleep(0.01)
				serout.write("mr")
				if(red):
					GPIO.output(21,False)	#Light off RED LED
					red=False
					
			#move left		
			elif(sensors=="1010" or 	sensors=="1001" or sensors=="1000" or sensors=="1011"):
				serout.write("mf")
				time.sleep(0.01)
				serout.write("ml")
				if(red):
					GPIO.output(21,False)	#Light off RED LED
					red=False
			
			#stop moving
			elif(sensors=="0000" or sensors=="1111"):
				serout.write("ms")
				time.sleep(0.01)
				serout.write("ss")
				if(red):
					GPIO.output(21,False)	#Light off RED LED
					red=False
			
			#something went wrong
			else:
				serout.write("ms")
				time.sleep(0.01)
				serout.write("ss")
				#print(sensors)
				if not(red):
					GPIO.output(21,True)	#Light on RED LED
					red=True
					
			time.sleep(0.05)	
	return





while True:
	#debug only
	if ((GPIO.input(7)) and (GPIO.input(11))):
		GPIO.output(21,False)	#Light off RED LED
		GPIO.output(19,False)	#Light off Green LED
		#clean up
		GPIO.cleanup()
		#terminate
		break

	if (GPIO.input(7)):	#start button (near green LED) pressed
		GPIO.output(19,True)	#Light on Green LED
		green=True
		GPIO.output(21,False)	#Light off RED LED
		red=False
		
		#Follow me started
		followmefun()

	time.sleep(0.2)		