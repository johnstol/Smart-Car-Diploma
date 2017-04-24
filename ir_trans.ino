//Send PINS 3,9
#include <IRremote.h>
unsigned long timer,start,ending;

IRsend irsend;

void setup()
{
  Serial.begin(9600);
  int i=0;
}

void loop() {
  String AEnvoyer = "John";  
    
  for (int i = 0; i < AEnvoyer.length(); i++){
    irsend.sendRC5(AEnvoyer.charAt(i), 12); 
    delay(50);
  }

  delay(100);

} 







