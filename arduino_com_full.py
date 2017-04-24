import RPi.GPIO as GPIO
import serial 
import time
import thread
import threading
import os
from bt_proximity import BluetoothRSSI
import sys

#open serial
ser = serial.Serial('/dev/ttyACM0', 115200)	#arduino1 connected via USB 19200
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
pause_sonar=False
red=True;
green=False;
blue=False;
rssi=0
avoid_dist=30
ser_read_lock=threading.Lock()
turn_side=0 #1:right 2:left 0:none (its first time?)


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
		#serial read with lock
		with ser_read_lock:
			sonar="$ok$"
			while (sonar.find("-")<0):
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
				elif len(sonar_list[i])>3:	#if the item's size is more than 3chars its wrong turn it to zero
					sonar_list[i]=0

			sonar_serns=map(int,sonar_list) #cast string list to int
			
			if (debug):
				print(sonar_serns)

		time.sleep(0.05)
	return		
	
def read_blutooth():
	global rssi, killFlag
	debug=False
	same_val_threshold=9
	zerocounter_threshold=4
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
		if(sameval_counter>same_val_threshold):
			rssi=0
			sameval_counter=0
			disconnected=True
		#re-connect
		if (zerocounter > zerocounter_threshold)or disconnected:
			btrssi = BluetoothRSSI('98:D3:31:80:15:9C')
			zerocounter=0	
		
		time.sleep(0.5)
	return	

def send_my_command(command,mytime):
	global ser
	#send command one time with no wait for ack
	if not mytime:
		ser.write(command+"\n")
	#send command 10 times in 0.1s (need for movement on abrasive surface or if the car carries large weight)
	elif (mytime == 0):	
		for i in range (0,10):		
			ser.write(command+"\n")
			time.sleep(0.01)
	#send command untill you receive an ack		
	elif (mytime=="ack"):
		ack=''
		with ser_read_lock:
			while (ack != "ok"):
				ser.write(command+"\n")
				for i in range (0,10):	
					ack=ser.readline()
					if ack.find('ok') > 0:
						ack="ok"
						break

	#send command 10 times with given time		
	else:
		mytime_type=type(mytime)
		if(mytime_type.__name__=="float" or mytime_type.__name__=="int"):
			for i in range (0,10):		
				ser.write(command+"\n")
				time.sleep(mytime)
		else:
			print "mytime type is:",mytime_type,".Expected float or int"

	return

def avoid_fun(count_call_times):
	#count_call_times=the number of continuesly calls of avoid_fun
	global sonar_serns,ser,turn_side
	safe_to_avoid=70 #70cm
	safe_to_stop=40	#40cm
	safe_obstacle_dist=30 #30cm
	mv_threshold=2.4 #2.4s
	duration=0
	debug=False
	
	#reset camera
	send_my_command("cc","ack")
	
	if(debug):
		print "count_call_times:", count_call_times
	
	#if its first time (posible new circle of continious calls) clear the previous turn_side you chose (you dont need it anymore)
	if(count_call_times==0):
		turn_side=0
	time.sleep(0.5)
	#move backwards
	if(sonar_serns[0]==1 and sonar_serns[1]==1):
		print"small obstacle"
		send_my_command("mb","ack")
		time.sleep(mv_threshold)
	else:	
		while( not check_obstacle("front_and", safe_to_avoid)):
			send_my_command("mb","ack")
			if (check_obstacle("back",safe_obstacle_dist)):
				if(debug):
					print "Stop"
				break

	#STOP
	send_my_command("ms","ack")	

	#check length
	if((check_obstacle("back",safe_obstacle_dist)) and (check_obstacle("front_and", safe_to_avoid))):		
		if (debug):
			print "Not enough length"
		length_param=False
	else:
		length_param=True

	#check right/left distance
	#turn_side is used for avoid bigger obstacles
	if(length_param):
		if (debug):
			print "Length is ok"

		if(turn_side==0):
			#turn camera Right and get Right distance
			send_my_command("cR","ack")
			
			time.sleep(2)
			right_dist=sonar_serns[4]
			
			#turn camera Left and get Left distance
			send_my_command("cL","ack")
			
			time.sleep(2)
			left_dist=sonar_serns[4]
			
			#reset camera
			send_my_command("cc","ack")
			
			time.sleep(0.5)
		elif(turn_side==1):	#go again right if you can
			send_my_command("cR","ack")
			time.sleep(2)
			right_dist=sonar_serns[4]
			left_dist=0
			
		elif(turn_side==2): #go again left if you can	
			send_my_command("cL","ack")
			time.sleep(2)
			left_dist=sonar_serns[4]
			right_dist=0
		else:
			print "something went wrong (turn_side=",turn_side ,')'
			right_dist=0
			left_dist=0
	
		if(debug):
			print'right:', right_dist, ' and left: ',left_dist
	
	#reasons why an obstacle can not be avoided
	if (not length_param or (right_dist<safe_to_avoid and left_dist<safe_to_avoid)):
		if(debug):
			print "Cannot be avoided"
		avoidance=False
	#avoid, You can do it!	
	else:
		avoidance=True
		#avoid going left
		if (right_dist < left_dist):
			if (debug):
				print"Going left"
			
			turn_side=2
			#look for obstacle
			send_my_command("cL","ack")
			time.sleep(0.2)
			#move Left
			send_my_command("ml","ack")
			time.sleep(0.4)
			send_my_command("ss","ack")
			time.sleep(0.2)
						
			#move forward
			#stop when you find an obstacle or the given time passed
			start=time.time()
			while(duration<mv_threshold):
				#check distance
				if(sonar_serns[4]<safe_to_stop or check_obstacle("front",safe_obstacle_dist)):	
					if(debug):
						print'camera:',sonar_serns[4],'Front:',sonar_serns[0],'-',sonar_serns[1]
					break	
				
				send_my_command("mf","ack")
				ending=time.time()
				duration=ending-start
			#STOP	
			send_my_command("ms","ack")	
			
			#debug
			if (debug):
				if(duration>=mv_threshold):
					print('Duration:',duration)
			
			
			#reset camera
			send_my_command("cc","ack")
			time.sleep(0.2)
			#move right
			send_my_command("mr","ack")
			time.sleep(0.6)
			send_my_command("ss","ack")
			time.sleep(0.2)
			send_my_command("mf","ack")
			time.sleep(1)
			#reset steering wheel
			send_my_command("ml","ack")
			time.sleep(0.2)
			send_my_command("ss","ack")
			time.sleep(0.2)
			# end of algorithm
		#avoid going right	
		else:
			if (debug):
				print"Going right"
			turn_side=1	
			time.sleep(0.2)
			#look for obstacle
			send_my_command("cR","ack")
			time.sleep(0.2)
			#move Right
			send_my_command("mr","ack")
			time.sleep(0.4)
			send_my_command("ss","ack")
			time.sleep(0.2)
			
			#move forward
			#stop when you find an obstacle or the given time passed
			start=time.time()
			while(duration<mv_threshold):
				#check distance
				if(sonar_serns[4]<safe_to_stop or check_obstacle("front",safe_obstacle_dist)):	
					if(debug):
						print'camera:',sonar_serns[4],'Front:',sonar_serns[0],'-',sonar_serns[1]
					break	
					
				send_my_command("mf","ack")
				ending=time.time()
				duration=ending-start
			#STOP	
			send_my_command("ms","ack")	
			#debug
			if (debug):
				if(duration>=mv_threshold):
					print('Duration:',duration)
				
			#reset camera
			send_my_command("cc","ack")
			time.sleep(0.2)
			#move left 
			send_my_command("ml","ack")
			time.sleep(0.6)
			send_my_command("ss","ack")
			time.sleep(0.2)
			send_my_command("mf","ack")
			time.sleep(1)
			#reset steering wheel
			send_my_command("mr","ack")
			time.sleep(0.2)
			send_my_command("ss","ack")
			time.sleep(0.2)
			#end of algorithm
			
	return avoidance
	
def check_obstacle(direction,avoid_dist):
	global sonar_serns
	found_obstacle=False
	#check FR and FL sensors
	if(direction=="front"):
		if ((sonar_serns[0] <= avoid_dist and sonar_serns[0] != 0) or (sonar_serns[1] <= avoid_dist and sonar_serns[1] != 0)):
			found_obstacle=True
	elif (direction=="front_and"):
		if((sonar_serns[0] <= avoid_dist and sonar_serns[0] != 0) and (sonar_serns[1] <= avoid_dist and sonar_serns[1] != 0)):
			found_obstacle=True		
	elif(direction=="back"):
		if ((sonar_serns[2] <= avoid_dist and sonar_serns[2] != 0) or (sonar_serns[3] <= avoid_dist and sonar_serns[3] != 0)):
			found_obstacle=True
	else:
		print"Wrong value in direction. You gave:",direction,"waiting 'front or back' "
	return found_obstacle

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
		send_my_command(cam_com[0],'');
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
		send_my_command(mov_com[0],'');
		mf.seek(0)
		mf.write("0")
		mf.truncate()
	
	return	
		
def followmefun():
	global red,green,ser,serin,rssi,blue
	debug=False
	print("Follow me started!")
	avoid_counter=0 #counts the number of continuesly calls of avoid_fun
	avoid_threshold=3 #max number of allowed continious calls of avoid_fun
	keep_Rolling_threshold=4 #max zeros of sensors (IR) untill the car stops moving
	prev_cmd="NULL"
	ZeroCount=0
	keep_rolling_mode= True #enable/disable keep rolling mode
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
			send_my_command("ms","ack")
			time.sleep(0.1)			
			send_my_command("ss","ack")
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
				send_my_command("ms","ack")
				time.sleep(0.1)			
				send_my_command("ss","ack")
			else:
				##Light off Blue LED
				if (blue):
					GPIO.output(blue_led,False)	
					blue=False

				if (debug):
					print(sensors)
					
				#sensors (IR) 0
				if(sensors == "0"):
					if(red):
							GPIO.output(red_led,False)	#Light off RED LED
							red=False
					#keep rolling mode		
					if (keep_rolling_mode):		
						if(ZeroCount<4):
							if(keep_Rolling):
								send_command1="mf"
							ZeroCount+=1
						else:
							ZeroCount=0;
							keep_Rolling=False
							send_command1="ms"
							send_command2="ss"
					#no keep rolling mode				
					else:
						send_command1="ms"
						send_command2="ss"
		
				else:
					if (keep_rolling_mode):
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
				if(check_obstacle("front",avoid_dist)):
					#if we tried less times than avoid_threshold there's still hope to avoid the obstacle
					#else the car cannot avoid this obstacle avoid it manually ;)
					if (avoid_counter<avoid_threshold):
						print "Avoid algorithm engaged"	
						#time.sleep(2)
						if not(avoid_fun(avoid_counter)):
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
		time.sleep(1)
		if(GPIO.input(green_btn)):
			killFlag=True
			GPIO.output(green_led,True)	#Light off Green LED
			time.sleep(1)
			GPIO.output(green_led,False)	#Light off Green LED
			print("Shuting down...")
			os.system('sudo shutdown -h now')
		elif(GPIO.input(red_btn)):
			killFlag=True
			GPIO.output(red_led,False)	#Light off RED LED
			GPIO.output(green_led,False)	#Light off Green LED
			time.sleep(2)
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
