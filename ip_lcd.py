#!/usr/bin/python
import Adafruit_CharLCD as LCD
from subprocess import *
from time import sleep
# Raspberry Pi pin configuration:
lcd_rs        = 27  # Note this might need to be changed to 21 for older revision Pi's.
lcd_en        = 22
lcd_d4        = 25
lcd_d5        = 24
lcd_d6        = 23
lcd_d7        = 18
lcd_backlight = 4

# Define LCD column and row size for 16x2 LCD.
lcd_columns = 16
lcd_rows    = 2

# Initialize the LCD using the pins above.
lcd = LCD.Adafruit_CharLCD(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7,
                           lcd_columns, lcd_rows, lcd_backlight)

cmd = "ip addr show eth0 | grep inet | head -1 |awk '{print $2}' | cut -d / -f1"
cmd1 = "ip addr show wlan0 | grep inet | head -1 |awk '{print $2}' | cut -d / -f1"



def run_cmd(cmd):
        p = Popen(cmd, shell=True, stdout=PIPE)
        output = p.communicate()[0]
        return output

while 1:
		lcd.clear()
		sleep(0.05)
		ipaddr = run_cmd(cmd)
		if(ipaddr.find(".") <0):
			ipaddr = run_cmd(cmd1)
			if(ipaddr.find(".") >0):
				cmd=cmd1
			
		lcd.message('IP %s' % ( ipaddr ) )
		sleep(2)

		
