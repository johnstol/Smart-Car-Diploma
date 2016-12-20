#include <Servo.h>
#include <NewPing.h>

#define TRIGGER_PIN  10  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     9  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

//Blue and White Front Car Lights
#define Bluelight_pin 12
#define light_pin 11

// NewPing setup of pins and maximum distance.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 
	
//serial input for new orders
int serial_input=0;		

//servo steps
int angle_mov1=2;			

//Servos for camera movement
Servo servo1;
Servo servo2;

// Motor 1	car movement
int mo1f = 7;
int mo1b = 8;

//Motor 2	Steering wheel
int mo2f = 3;
int mo2b = 4;

int servo1_pin = 5;			//servo1 tied to pin 5
int servo2_pin = 6;			//servo2 tied to pin 6
int angle1=90;				
int angle2=90;

//Timer
int start = millis();
int myend=0;

int sonar_value = 0;

//define communication codes
int
ss=737,
mf=664,
mb=660,
mr=676,
ml=670,
ms=677,
cu=579,
cd=562,
cc=561,
cr=576,
cl=570;






void setup() {

//Initializing Values
	
  servo1.attach(servo1_pin);
  servo1.write(90);
  
  servo2.attach(servo2_pin);
  servo2.write(90);
  
  pinMode(mo1f,OUTPUT);
  pinMode(mo1b,OUTPUT);

  pinMode(mo2f,OUTPUT);
  pinMode(mo2b,OUTPUT);
  
  pinMode(Bluelight_pin,OUTPUT);
  pinMode(light_pin,OUTPUT);	
  digitalWrite(light_pin, LOW);
  digitalWrite(Bluelight_pin, LOW);

  Serial.begin(19200);  
  
}

void loop(){

	if(serial_input!=0){
		angle1=servo1.read();
		angle2=servo2.read();


		//Camera Up cu command: 
		//when cu command arrives, servo 1 moves up by "angle_mov1"
		if(serial_input==cu && servo1.read()<180){ //cu
			serial_input=0;
			angle1+=angle_mov1;
			servo1.write(angle1);
		}	//cu
		
		//Likewise, Camera Down (cd)
		else if(serial_input==cd && servo1.read()>0){ //cd
			serial_input=0;
			angle1-=angle_mov1;
			servo1.write(angle1);
		}	//cd
		
		//Camera Left (cl)
		if(serial_input==cl && servo2.read()<160){ //cl
			serial_input=0;
			angle2+=angle_mov1;
			servo2.write(angle2);
		}	//cl
		
		//Camera Right (cr)
		else if(serial_input==cr && servo2.read()>20){ //cr
			serial_input=0;
			angle2-=angle_mov1;
			servo2.write(angle2);
		}	//cr
		
		//Camera Center (cc) (Up/Down AND Left/Right)
		else if(serial_input==cc){ 				//cc
			serial_input=0;
			servo1.write(90);
			servo2.write(90);
		}	//cc
		
		//Move Forward (mf) if there are no obstacles
		if(serial_input==mf){		//mf
			if(sonar_value>45){		//if distance is bigger than 45cm its ok move
				digitalWrite(mo1f, HIGH);
				digitalWrite(mo1b, LOW);
			}
			else{					//else stop
				digitalWrite(mo1f, LOW);
				digitalWrite(mo1b, LOW);	
			}

		}		//mf
		
		//Move Backward (mb)
		else if(serial_input==mb){	//mb
			digitalWrite(mo1f, LOW);
			digitalWrite(mo1b, HIGH);
		}		//mb
		
		//Steer Left (ml)
		if(serial_input==ml){		//ml
			digitalWrite(mo2f, HIGH);
			digitalWrite(mo2b, LOW);
		}		//ml
		
		//Steer Right (mr)
		else if(serial_input==mr){		//mr
			digitalWrite(mo2f, LOW);
			digitalWrite(mo2b, HIGH);
		}		//mr
		
		//Stop Moving (ms)
		if (serial_input==ms){		//ms
			digitalWrite(mo1f, LOW);
			digitalWrite(mo1b, LOW);
		}		//ms
		
		//Stop Steering (ss)
		if (serial_input==ss){		//ss
			digitalWrite(mo2f, LOW);
			digitalWrite(mo2b, LOW);
		}		//ss
		

	
	}
	myend = millis();
	if(myend-start>=500){
		
		// Send ping, get ping time in microseconds (uS).
		unsigned int uS = sonar.ping(); 
		Serial.print("-");
		
		// Convert ping time to distance in cm and print result (0 = outside set distance range)
		sonar_value = (uS / US_ROUNDTRIP_CM);
		Serial.print(uS / US_ROUNDTRIP_CM);
		Serial.println("-");
		
		start=millis();
	}
}


int serialEvent()
{
	serial_input = 0;
	// add serial input to "serial_input"
	while (Serial.available() > 0)
	{
		//Convert to ascii encoding
		serial_input *= 10;
		serial_input += (Serial.read() - '0');
		delay(2);
	}
	
}
