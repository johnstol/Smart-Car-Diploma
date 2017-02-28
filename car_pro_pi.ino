#include <Servo.h>
#include <NewPing.h>

#define TRIGGER_PIN_FR  13  // Arduino pin tied to trigger pin on the ultrasonic sensor at frond right side of the car.
#define ECHO_PIN_FR     A0  // Arduino pin tied to echo pin on the ultrasonic sensor at frond right side of the car.

#define TRIGGER_PIN_FL  12  // Arduino pin tied to trigger pin on the ultrasonic sensor at frond left side of the car.
#define ECHO_PIN_FL     A1  // Arduino pin tied to echo pin on the ultrasonic sensor at frond left side of the car.

#define TRIGGER_PIN_BR  11  // Arduino pin tied to trigger pin on the ultrasonic sensor at back right side of the car.
#define ECHO_PIN_BR     A2  // Arduino pin tied to echo pin on the ultrasonic sensor at back right side of the car.

#define TRIGGER_PIN_BL  10  // Arduino pin tied to trigger pin on the ultrasonic sensor at back left side of the car.
#define ECHO_PIN_BL     A3  // Arduino pin tied to echo pin on the ultrasonic sensor at back left side of the car.

#define TRIGGER_PIN_CM  9  // Arduino pin tied to trigger pin on the ultrasonic sensor mounted on camera.
#define ECHO_PIN_CM     A4  // Arduino pin tied to echo pin on the ultrasonic sensor mounted on camera.


#define wheel A5

#define bumper 8


#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define stop_distance 40 //set distance for automated stop


//wheel value
int wheel_val=0;
bool bumper_val;

// NewPing setup of pins and maximum distance.
NewPing sonarFR(TRIGGER_PIN_FR, ECHO_PIN_FR, MAX_DISTANCE); 
NewPing sonarFL(TRIGGER_PIN_FL, ECHO_PIN_FL, MAX_DISTANCE); 
NewPing sonarBR(TRIGGER_PIN_BR, ECHO_PIN_BR, MAX_DISTANCE); 
NewPing sonarBL(TRIGGER_PIN_BL, ECHO_PIN_BL, MAX_DISTANCE); 
NewPing sonarCM(TRIGGER_PIN_CM, ECHO_PIN_CM, MAX_DISTANCE); 
	
//serial input for new orders
int serial_input=0;		

//servo steps
int angle_mov1=2;			

//Servos for camera movement
Servo servo1;
Servo servo2;

// Motor 1	car movement
int mo1f = 2;
int mo1b = 3;

//Motor 2	Steering wheel
int mo2f = 4;
int mo2b = 7;

int servo1_pin = 5;			//servo1 tied to pin 5
int servo2_pin = 6;			//servo2 tied to pin 6
int angle1=90;				
int angle2=90;

//Timer
int start = millis();
int myend=0;

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


//Sonar time varaiables
unsigned int timeSonFR,timeSonFL,timeSonBR,timeSonBL,timeSonCM; 

//Sonar distance varaiables
int DistSonFR = 0,
    DistSonFL = 0,
    DistSonBR = 0,
    DistSonBL = 0,
    DistSonCM = 0;

//allow moving flags 
bool moveFflag=true,moveBflag=true;

void setup() {

//Initializing Values

  
  servo1.attach(servo1_pin);
  servo1.write(90);
  
  servo2.attach(servo2_pin);
  servo2.write(90);

  //setup moving DC motor
  pinMode(mo1f,OUTPUT);
  pinMode(mo1b,OUTPUT);
  
  //setup streering DC motor
  pinMode(mo2f,OUTPUT);
  pinMode(mo2b,OUTPUT);

  //setup bumper
  pinMode(bumper,INPUT);

  Serial.begin(19200);  
  
}

void loop(){
  //check bumper
  bumper_val=digitalRead(bumper);
  if(bumper_val==HIGH && moveFflag==false){ //not crashed anymore
    allowmoving();  //refresh moving flags
  }
  else if(bumper_val==LOW){  //we have crashed - stop moving
    digitalWrite(mo1f, LOW);
    digitalWrite(mo1b, LOW);
    allowmoving();  //refresh moving flags
  }

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
			if(moveFflag==true){		//if distance is bigger than 45cm its ok move
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
      if(moveBflag==true){    //if distance is bigger than 45cm its ok move
  			digitalWrite(mo1f, LOW);
  			digitalWrite(mo1b, HIGH);
      }
      else{          //else stop
        digitalWrite(mo1f, LOW);
        digitalWrite(mo1b, LOW);  
      }
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
	if(myend-start>=1000){
		
		// Send ping, get ping time in microseconds (uS).
	
    timeSonFR = sonarFR.ping();
  
    timeSonFL = sonarFL.ping();
  
    timeSonBR = sonarBR.ping();
  
    timeSonBL = sonarBL.ping();
  
    timeSonCM = sonarCM.ping();
		
		
		// Convert ping time to distance in cm and print result (0 = outside set distance range)
		DistSonFR = (timeSonFR / US_ROUNDTRIP_CM);
    DistSonFL = (timeSonFL / US_ROUNDTRIP_CM);
    DistSonBR = (timeSonBR / US_ROUNDTRIP_CM);
    DistSonBL = (timeSonBL / US_ROUNDTRIP_CM);
    DistSonCM = (timeSonCM / US_ROUNDTRIP_CM);

    allowmoving();  //refresh moving flags

		Serial.print(DistSonFR);
    Serial.print("-");
		Serial.print(DistSonFL);
    Serial.print("-");
		Serial.print(DistSonBR);
    Serial.print("-");
		Serial.print(DistSonBL);
    Serial.print("-");
		Serial.print(DistSonCM);
    Serial.println("-");

    wheel_val = analogRead(wheel);
    if(wheel_val> 35 && wheel_val< 50){   //wheel is turned right
      //Serial.println("Right");
        //do something
      }
    else if(wheel_val> 65 && wheel_val< 80){  //wheel is in the middle
      //Serial.println("Middle");
        //do something
      }
     
    else if(wheel_val> 95 && wheel_val< 110){  //wheel is turned left
      //Serial.println("Left");
        //do something
      } 
    
		start=millis();
	}
}

bool allowmoving(){ //check sensors/automated stop
   if((DistSonFR>0 && DistSonFR<=stop_distance) || bumper_val==LOW || (DistSonFL>0 && DistSonFL<=stop_distance)){
      digitalWrite(mo1f, LOW);
      digitalWrite(mo1b, LOW);
      moveFflag=false;
   }
   else{
      moveFflag=true;
   }

   if((DistSonBR>0 && DistSonFR<=stop_distance) || (DistSonBL>0 && DistSonBL<=stop_distance)){
      digitalWrite(mo1f, LOW);
      digitalWrite(mo1b, LOW);
      moveBflag=false;
   }
   else{
      moveBflag=true;
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
