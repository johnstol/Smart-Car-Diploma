import RPi.GPIO as GPIO
import serial 
import time
import thread
import threading
import os
from bt_proximity import BluetoothRSSI
import sys

#open serial
ser = serial.Serial('/dev/ttyACM0', 9600)
serin = serial.Serial('/dev/ttyUSB0', 9600)	#arduino2 connected via USB
#open files
sf=open("/home/onram/sonar.txt","w");	#sf ---> sonar file
cf=open("/home/onram/camera.txt","r+"); #cf ---> camera file
mf=open("/home/onram/movement.txt","r+"); #mf ---> movement file


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
blue_led=7
red_btn=18
green_btn=16
rssi=0

killFlag=False

GPIO.setmode(GPIO.BOARD) ## Use board pin numbering


GPIO.setup(green_led, GPIO.OUT) ## Setup GPIO Pin as OUTPUT
GPIO.setup(red_led, GPIO.OUT) ## Setup GPIO Pin as OUTPUT
GPIO.setup(blue_led, GPIO.OUT) ## Setup GPIO Pin as OUTPUT

GPIO.setup(green_btn, GPIO.IN) ## Setup GPIO Pin as INPUT	------16
GPIO.setup(red_btn, GPIO.IN) ## Setup GPIO Pin as INPUT ----18

#define read_sonar function

class mythread (threading.Thread):
	def __init__(self, threadID, name):
		threading.Thread.__init__(self)
		self.threadID = threadID
		self.name = name
	def run(self):
		print "Starting " + self.name
		if (self.threadID==1):
			read_sonar()
		elif (self.threadID==2):
			read_blutooth()
		print "Exiting " + self.name
		

def read_sonar():
	global killFlag
	while not (killFlag):
		sonar=ser.readline()
		sf.seek(0)
		sf.write(sonar)
		sf.truncate()
		time.sleep(0.02)
	return		

	
def read_blutooth():
	global rssi, killFlag
	debug=False
	
	#create bluetooth object with the following address
	btrssi = BluetoothRSSI('98:D3:31:80:15:9C')
	zerocounter=0
	sameval_counter=0
	prev_val=0
	disconnected =False
	while not(killFlag):
		#get rssi
		temp_rssi=btrssi.get_rssi()
		if (debug=True):
			print("temp_rssi: %s",temp_rssi)
		
		#disconnected
		if temp_rssi is None or (temp_rssi == 0):
			rssi=0
			zerocounter+=1
			
		else:
			disconnected=False
			zerocounter=0
			rssi=int(temp_rssi)
			if(prev_val==rssi):
				sameval_counter+=1
			else:
				prev_val=rssi
				sameval_counter=0
		
		#if we get the same value up to 10 times continuesly it means that bluetooth disconnected
		if(sameval_counter>9):
			rssi=0
			sameval_counter=0
			disconnected=True
		#re-connect
		if (zerocounter > 4)or disconnected:
			btrssi = BluetoothRSSI('98:D3:31:80:15:9C')
			zerocounter=0	
		
		time.sleep(0.5)
	return	

	
def fwdcom():
	global cf,mf,ser
	#forward camera command
	cf.seek(0)
	camera= cf.readline()
	cam_com= camera.split()
	if not cam_com:
		cf.seek(0)
		cf.write("0")
	elif cam_com[0] != "0":
		ser.write(cam_com[0]+"\n")
		cf.seek(0)
		cf.write("0")
		cf.truncate()
				
	#forward movement command				
	mf.seek(0)
	move=mf.readline()
	mov_com=move.split()
	if not mov_com:
		mf.seek(0)
		mf.write("0")
	elif mov_com[0] != "0":
		ser.write(mov_com[0]+"\n")
		mf.seek(0)
		mf.write("0")
		mf.truncate()
	
	return	
		


def followmefun():
	global red,green,mf,ser,serin,rssi,blue
	debug=False
	print("Follow me started!")
	
	#clear input buffer
	serin.reset_input_buffer()
	prev_cmd="NULL"
	ZeroCount=0
	keep_Rolling=False
	while True:
		send_command1="NULL"	#mf,ms,mb
		send_command2="NULL"	#mr,ml,ss
		
		#Stop Follow me 
		if (GPIO.input(red_btn)):	#stop button (near red LED) pressed
			ser.write("ms\n")
			time.sleep(0.1)
			ser.write("ss\n")
			GPIO.output(green_led,False)	#Light off Green LED
			green=False
			GPIO.output(red_led,True)	#Light on RED LED
			red=True
			print("followme stopped")
			break
		
		else:
			reading=serin.readline()	#read arduino anyway - keep serial-buffer empty
			sensors=(reading.split("#")[1])	#get only the value ex: 1101
			if(debug):
				print"rssi: %d" %rssi
			if(rssi>-5 and rssi <0):
				if not(blue):
					GPIO.output(blue_led,True)	#Light on Blue LED
					blue=True
				ser.write("ms\n")
				time.sleep(0.1)
				ser.write("ss\n")
			else:
				if (blue):
					GPIO.output(blue_led,False)	#Light on Blue LED
					blue=False

				#uncommend for debug
				if (debug):
					print(sensors)
				#if we get 5 Zeros continuesly stop
				if(sensors == "0"):
					if(red):
							GPIO.output(red_led,False)	#Light off RED LED
							red=False
					if(ZeroCount<4):
						if(keep_Rolling):
							send_command1="mf"
						ZeroCount+=1
					else:
						ZeroCount=0;
						keep_Rolling=False
						send_command1="ms"
						send_command2="ss"
				else:
					keep_Rolling=True
					if(ZeroCount>0):
						ZeroCount=0
						
					#move forward			
					if(sensors=="11" or sensors=="1" or sensors=="10"):
						send_command1="mf"
						#return steering wheel to middle posiotion
						if (prev_cmd=="R"):
							prev_cmd="NULL"
							send_command2="ml"
						elif (prev_cmd=="L"):
							prev_cmd="NULL"
							send_command2="mr"
							
						#turn off red LED
						if(red):
							GPIO.output(red_led,False)	#Light off RED LED
							red=False					
					
					#move right		
					elif(sensors=="100" or 	sensors=="101" or sensors=="110" or sensors=="111"):
						prev_cmd="R"
						send_command1="mf"
						send_command2="mr"
						if(red):
							GPIO.output(red_led,False)	#Light off RED LED
							red=False
							
					#move left		
					elif(sensors=="1010" or sensors=="1001" or sensors=="1000" or sensors=="1011"):
						prev_cmd="L"
						send_command1="mf"
						send_command2="ml"
						if(red):
							GPIO.output(red_led,False)	#Light off RED LED
							red=False	
											
					#something went wrong
					else:
						send_command1="ms"
						send_command2="ss"
						#print(sensors)
						if not(red):
							GPIO.output(red_led,True)	#Light on RED LED
							red=True
				
				#send command2	
				if(send_command2=="mr" or send_command2=="ml"):
					#turn right/left
					for i in range (0,10):		
						ser.write(send_command2+"\n")
						time.sleep(0.01)
					time.sleep(0.2)
					#stop turning
					for i in range (0,10):		
						ser.write("ss\n")
						time.sleep(0.01)

				elif(send_command2=="ss"):
					for i in range (0,10):		
						ser.write("ss\n")
						time.sleep(0.01)
				
				else:
					for i in range (0,10):		
						ser.write(send_command1+"\n")
						time.sleep(0.01)
				
				time.sleep(0.1)		
						
				#send command1
				for i in range (0,10):		
					ser.write(send_command1+"\n")
					time.sleep(0.01)
						
				
				#time.sleep(0.5)	
	return
		
#create thread
sonarthread= mythread(1, "Sonar_thread")
Bluetoothread= mythread(2, "Bluetooth_thread")

#start thread
sonarthread.start()
Bluetoothread.start()

GPIO.output(red_led,True) ##set RED on	----------15
GPIO.output(green_led,False) ##set GREEN off---------13
GPIO.output(blue_led,False) ##set GREEN off---------7

red=True;
green=False;
blue=False;


while True:
	
	

	if (GPIO.input(red_btn)):
		ser.write("ms\n")
		time.sleep(0.1)
		ser.write("ss\n")
		killFlag=True
		time.sleep(3)
		if(GPIO.input(green_btn)):
			GPIO.output(green_led,True)	#Light off Green LED
			time.sleep(1)
			GPIO.output(green_led,False)	#Light off Green LED
			print("Shuting down...")
			os.system('sudo shutdown -h now')
		else:
			GPIO.output(red_led,False)	#Light off RED LED
			GPIO.output(green_led,False)	#Light off Green LED
			print("Terminated!")
		#clean up GPIOs
		GPIO.cleanup()	
		break
		
	
	elif (GPIO.input(green_btn)):	#start button (near green LED) pressed
		GPIO.output(green_led,True)	#Light on Green LED
		green=True
		GPIO.output(red_led,False)	#Light off RED LED
		red=False
		#start Follow me 
		followmefun()
		
	fwdcom()	

	time.sleep(0.01)	







