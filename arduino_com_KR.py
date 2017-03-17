import RPi.GPIO as GPIO
import serial 
import time
import thread
import threading


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
red_btn=18
green_btn=16

killFlag=False

GPIO.setmode(GPIO.BOARD) ## Use board pin numbering


GPIO.setup(green_led, GPIO.OUT) ## Setup GPIO Pin as OUTPUT
GPIO.setup(red_led, GPIO.OUT) ## Setup GPIO Pin as OUTPUT

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
		read_sonar()
		print "Exiting " + self.name


def read_sonar():
	global killFlag
	while True:
		if(killFlag):
			break
		else:	
			sonar=ser.readline()
			sf.seek(0)
			sf.write(sonar)
			sf.truncate()
			time.sleep(0.02)
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
	global red,green,mf,ser,serin
	print("Follow me started!")
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
			print("followme stop")
			break
		
		else:
			reading=serin.readline()	#read arduino
			sensors=(reading.split("#")[1])	#get only the value ex: 1101
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
				for i in range (0,10):		
					ser.write(send_command2+"\n")
					time.sleep(0.01)
				time.sleep(0.2)
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

#start thread
sonarthread.start()

GPIO.output(red_led,True) ##set RED on	----------15
GPIO.output(green_led,False) ##set GREEN off---------13

red=True;
green=False;


while True:
	
	
	if ((GPIO.input(green_btn)) and (GPIO.input(red_btn))):
		ser.write("ms\n")
		time.sleep(0.1)
		ser.write("ss\n")
		killFlag=True
		GPIO.output(red_led,False)	#Light off RED LED
		GPIO.output(green_led,False)	#Light off Green LED
		#clean up
		GPIO.cleanup()
		print("Terminated!")
		#terminate
		break	
	
	if (GPIO.input(green_btn)):	#start button (near green LED) pressed
		GPIO.output(green_led,True)	#Light on Green LED
		green=True
		GPIO.output(red_led,False)	#Light off RED LED
		red=False
		
		'''
		#led testing
		while True:
			if (GPIO.input(red_btn)):
				GPIO.output(green_led,False)
				green=False
				GPIO.output(red_led,True)
				red=True
				break
			time.sleep(0.05)
		'''

		#Follow me started
		followmefun()
		
	fwdcom()	

	time.sleep(0.01)	







