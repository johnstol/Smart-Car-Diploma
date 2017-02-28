#!/bin/sh

gpio mode 0 out

echo NULL > /home/onram/camera.txt
echo NULL > /home/onram/movement.txt
echo NULL > /home/onram/sonar.txt

chmod 777 /home/onram/camera.txt
chmod 777 /home/onram/movement.txt
chmod 777 /home/onram/sonar.txt

chmod 777 /dev/ttyACM0

python /home/pi/scripts/arduino_com.py
