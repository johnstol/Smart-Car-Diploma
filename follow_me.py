import RPi.GPIO as GPIO ## Import GPIO library
import serial 
import time

#serout = serial.Serial('/dev/ttyACM0', 9600)	#arduino1 connected via USB
#ser = serial.Serial('/dev/ttyUSB0', 9600)	#arduino2 connected via USB

'''
#Raspberry Pi 1 model A,B rev2
red_led=21
green_led=19
red_btn=11
green_btn=7
'''


#Raspberry Pi 3 model B
red_led=15
green_led=13
red_btn=18
green_btn=16


GPIO.setmode(GPIO.BOARD) ## Use board pin numbering


GPIO.setup(green_led, GPIO.OUT) ## Setup GPIO Pin as OUTPUT
GPIO.setup(red_led, GPIO.OUT) ## Setup GPIO Pin as OUTPUT

GPIO.setup(green_btn, GPIO.IN) ## Setup GPIO Pin as INPUT	------16
GPIO.setup(red_btn, GPIO.IN) ## Setup GPIO Pin as INPUT ----18

GPIO.output(red_led,True) ##set RED on	----------15
GPIO.output(green_led,False) ##set GREEN off---------13

red=True;
green=False;



def followmefun():
	global red,green
	while True:
		#Stop Follow me 
		if (GPIO.input(red_btn)):	#stop button (near red LED) pressed
			serout.write("ms")
			time.sleep(0.01)
			serout.write("ss")
			GPIO.output(green_led,False)	#Light off Green LED
			green=False
			GPIO.output(red_led,True)	#Light on RED LED
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
					GPIO.output(red_led,False)	#Light off RED LED
					red=False
					
			#move right		
			elif(sensors=="100" or 	sensors=="101" or sensors=="110" or sensors=="111"):
				serout.write("mf")
				time.sleep(0.01)
				serout.write("mr")
				if(red):
					GPIO.output(red_led,False)	#Light off RED LED
					red=False
					
			#move left		
			elif(sensors=="1010" or 	sensors=="1001" or sensors=="1000" or sensors=="1011"):
				serout.write("mf")
				time.sleep(0.01)
				serout.write("ml")
				if(red):
					GPIO.output(red_led,False)	#Light off RED LED
					red=False
			
			#stop moving
			elif(sensors=="0000" or sensors=="1111"):
				serout.write("ms")
				time.sleep(0.01)
				serout.write("ss")
				if(red):
					GPIO.output(red_led,False)	#Light off RED LED
					red=False
			
			#something went wrong
			else:
				serout.write("ms")
				time.sleep(0.01)
				serout.write("ss")
				#print(sensors)
				if not(red):
					GPIO.output(red_led,True)	#Light on RED LED
					red=True
					
			time.sleep(0.05)	
	return





while True:
	#debug only
	if ((GPIO.input(green_btn)) and (GPIO.input(red_btn))):
		GPIO.output(red_led,False)	#Light off RED LED
		GPIO.output(green_led,False)	#Light off Green LED
		#clean up
		GPIO.cleanup()
		#terminate
		break

	if (GPIO.input(green_btn)):	#start button (near green LED) pressed
		GPIO.output(green_led,True)	#Light on Green LED
		green=True
		GPIO.output(red_led,False)	#Light off RED LED
		red=False
		
		while True:
			if (GPIO.input(red_btn)):
				GPIO.output(green_led,False)
				green=False
				GPIO.output(red_led,True)
				red=True
				break
			time.sleep(0.05)


		#Follow me started
		#followmefun()

	time.sleep(0.2)		
