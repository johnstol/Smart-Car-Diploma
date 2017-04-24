#!/bin/sh

echo NULL > /home/onram/camera.txt
echo NULL > /home/onram/movement.txt
echo NULL > /home/onram/sonar.txt
echo NULL > /home/onram/blue.txt

chmod 777 /home/onram/camera.txt
chmod 777 /home/onram/movement.txt
chmod 777 /home/onram/sonar.txt
chmod 777 /home/onram/blue.txt

chmod 777 /dev/ttyACM0
chmod 777 /dev/ttyUSB0

/usr/local/bin/gpio export 17 out
/usr/local/bin/gpio export 18 out

python /home/pi/scripts/arduino_com.py
