import RPi.GPIO as GPIO
import serial 
import time
import thread
import threading
import os
from bt_proximity import BluetoothRSSI
import sys

#open serial
ser = serial.Serial('/dev/ttyACM0', 9600)	#arduino1 connected via USB
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


#########################################################
#############sonar_serns=[FR,FL,BR,BL,CM]################
#########################################################
sonar_serns=[None]*5	
killFlag=False
red=True;
green=False;
blue=False;
rssi=0
avoid_dist=30


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
		if (self.threadID == 1):
			read_sonar()
		elif (self.threadID == 2):
			read_blutooth()
		print "Exiting " + self.name
		

def read_sonar():
	global killFlag,ser,sonar_serns
	debug=False
	while not (killFlag):
		sonar=ser.readline()
		sf.seek(0)
		sf.write(sonar)
		sf.truncate()
		sonar_list=sonar.split("-")
		
		if(debug):
			print(sonar_list)
			print(len(sonar_list))
			
		if len(sonar_list) == 6:
			del sonar_list[5]	#delete the last item, its \r\n
			
			for i in range (0,5):
				if not sonar_list[i] :	#if an item is empty turn it to zero
					sonar_list[i]=0
				elif len(sonar_list[i])>3:
					sonar_list[i]=0

			sonar_serns=map(int,sonar_list) #cast string list to int
			
			if (debug):
				print(sonar_serns)

		time.sleep(0.05)
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
		if (debug):
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

def send_my_command(command,mytime):
	global ser
	if not mytime:
		ser.write(command+"\n")
	elif (mytime == 0):	
		for i in range (0,10):		
			ser.write(command+"\n")
			time.sleep(0.01)
	else:
		for i in range (0,10):		
			ser.write(command+"\n")
			time.sleep(mytime)

	return

def avoid_fun():
	global sonar_serns
	safe_to_avoid=70
	#reset camera
	send_my_command("cc",0)
	time.sleep(0.5)
	#move backwards
	while(sonar_serns[0]<safe_to_avoid and sonar_serns[1]<safe_to_avoid):
		send_my_command("mb",0)
		time.sleep(0.1)	
		send_my_command("ms",0)
	time.sleep(0.5)
	#turn camera Right and get Right distance
	send_my_command("cR",0)
	time.sleep(2)
	right_dist=sonar_serns[4]
	#turn camera Left and get Left distance
	send_my_command("cL",0)
	time.sleep(2)
	left_dist=sonar_serns[4]
	#reset camera
	send_my_command("cc",0)
	time.sleep(0.5)
	
	print('right:', right_dist, ' and left: ',left_dist)
	if (right_dist<70 and left_dist<70):
		print "Cannot be avoided"
		avoidance=False
	else:
		avoidance=True
		if (right_dist < left_dist):
			send_my_command("ml",0)
			time.sleep(0.1)
			send_my_command("mf",0)
			time.sleep(1.5)
			send_my_command("ms",0)
			time.sleep(0.1)
			send_my_command("mr",0.02)
			time.sleep(0.1)
			send_my_command("mf",0)
		else:
			send_my_command("mr",0)
			time.sleep(0.1)
			send_my_command("mf",0)
			time.sleep(1.5)
			send_my_command("ms",0)
			time.sleep(0.1)
			send_my_command("ml",0.02)
			time.sleep(0.1)
			send_my_command("mf",0)
			
	return avoidance
	
def check_obstacle():
	global sonar_serns
	#check FR and FL sensors
	if ((sonar_serns[0] <= avoid_dist and sonar_serns[0] != 0) or (sonar_serns[1] <= avoid_dist and sonar_serns[1] != 0)):
		return True
	else:
		return False
	
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
	global red,green,mf,ser,rssi,blue
	debug=False
	print("Follow me started!")
	
	#clear input buffer
	serin.reset_input_buffer()
	reading=serin.readline()	#read arduino anyway - keep serial_buffer empty
	sensors=(reading.split("#")[1])	#get only the value ex: 1101	
	avoid_counter=0 #counts the number of continuesly calls of avoid_fun
	avoid_threshold=3
	prev_cmd="NULL"
	ZeroCount=0
	keep_Rolling=False
	while True:
	
		#clear input buffer
		serin.reset_input_buffer()
		reading=serin.readline()	#read arduino anyway - keep serial_buffer empty
		sensors=(reading.split("#")[1])	#get only the value ex: 1101		
		
		#clear commands
		send_command1="NULL"	#mf,ms,mb
		send_command2="NULL"	#mr,ml,ss

		#Stop Follow me 
		if (GPIO.input(red_btn)):	#stop button (near red LED) pressed
			send_my_command("ms",0)
			time.sleep(0.1)			
			send_my_command("ss",0)
			GPIO.output(green_led,False)	#Light off Green LED
			green=False
			GPIO.output(red_led,True)	#Light on RED LED
			red=True
			print("followme stopped")
			break
		
		else:
			if(debug):
				print"rssi: %d" %rssi
			#client is close enough to stop
			if(rssi>-5 and rssi <0):
				if not(blue):
					GPIO.output(blue_led,True)	#Light on Blue LED
					blue=True
				ser.write("ms\n")
				time.sleep(0.1)
				ser.write("ss\n")
			else:
				##Light off Blue LED
				if (blue):
					GPIO.output(blue_led,False)	
					blue=False

				if (debug):
					print(sensors)
					
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
				#
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
				
				#check for obstacles
				if(check_obstacle()):
					#if we tried less times than avoid_threshold there's still hope to avoid the obstacle
					#else the car cannot avoid this obstacle avoid it manually ;)
					if (avoid_counter<avoid_threshold):
						print "Avoid algorithm engaged"	
						#time.sleep(2)
						if not(avoid_fun()):
							avoid_counter=3
						else:
							avoid_counter+=1
						
				else:
					avoid_counter=0
					#send command2
					if send_command2 is not None:
						send_my_command(send_command2,0)
							
						time.sleep(0.2)
						
						#stop turning
						if(send_command2=="mr" or send_command2=="ml"):
							send_my_command("ss",0)											
					time.sleep(0.1)				
					#send command1
					if send_command1 is not None:
						send_my_command(send_command1,0)
	return
		
#create thread
sonarthread= mythread(1, "Sonar_thread")
Bluetoothread= mythread(2, "Bluetooth_thread")
#irthread=mythread(3,"IR_thread")
#start thread
sonarthread.start()
Bluetoothread.start()
#irthread.start()

GPIO.output(red_led,True) ##set RED on	----------15
GPIO.output(green_led,False) ##set GREEN off---------13
GPIO.output(blue_led,False) ##set GREEN off---------7



while True:

	if (GPIO.input(red_btn)):
		ser.write("ms\n")
		time.sleep(0.1)
		ser.write("ss\n")
		time.sleep(2)
		if(GPIO.input(green_btn)):
			killFlag=True
			GPIO.output(green_led,True)	#Light off Green LED
			time.sleep(1)
			GPIO.output(green_led,False)	#Light off Green LED
			print("Shuting down...")
			os.system('sudo shutdown -h now')
		elif(GPIO.input(red_btn)):
			killFlag=True
			time.sleep(2)
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







