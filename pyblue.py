from bt_proximity import BluetoothRSSI
import time
import sys

btf=open("/home/onram/blue.txt","w");	#btf ---> bluetooth file

BT_ADDR = '98:D3:31:80:15:9C'  # You can put your Bluetooth address here 
NUM_LOOP = 1

def print_usage():
	print "Usage: python test_address.py <bluetooth-address> [number-of-requests]"


def main():
	addr = BT_ADDR
	btrssi = BluetoothRSSI(addr=addr)
    #for i in range(0, num):
	while True:
		rssi=str(btrssi.get_rssi())
		btf.seek(0)
		btf.write(rssi)
		btf.truncate()
		#print btrssi.get_rssi()
		time.sleep(0.5)


if __name__ == '__main__':
    main()
