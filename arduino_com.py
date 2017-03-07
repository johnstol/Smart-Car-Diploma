import serial 
import time
import thread
import threading


#open serial
ser = serial.Serial('/dev/ttyACM0', 9600)
#open files
sf=open("/home/onram/sonar.txt","w");	#sf ---> sonar file
cf=open("/home/onram/camera.txt","r+"); #cf ---> camera file
mf=open("/home/onram/movement.txt","r+"); #mf ---> movement file


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
	while 1:
		sonar=ser.readline()
		sf.seek(0)
		sf.write(sonar)
		sf.truncate()
		time.sleep(0.02)
		

#create thread
sonarthread= mythread(1, "Sonar_thread")

#start thread
sonarthread.start()

while 1:
	cf.seek(0)
	camera= cf.readline()
	cam_com= camera.split()
	if not cam_com:
		cf.seek(0)
		cf.write("0")
	elif cam_com[0] != "0":
		ser.write(cam_com[0])
		cf.seek(0)
		cf.write("0")
		cf.truncate()
		
		
	mf.seek(0)
	move=mf.readline()
	mov_com=move.split()
	if not mov_com:
		mf.seek(0)
		mf.write("0")
	elif mov_com[0] != "0":
		ser.write(mov_com[0])
		mf.seek(0)
		mf.write("0")
		mf.truncate()

	time.sleep(0.01)
