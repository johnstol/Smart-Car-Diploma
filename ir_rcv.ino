#include <IRremote.h>

#define RECEIVERS 4
#define message_length 9

IRrecv *irrecvs[RECEIVERS];

decode_results results;


char data[RECEIVERS][message_length];
char id = "John";
int counter[RECEIVERS];
int i, j;
int command;

void setup() {
  Serial.begin(9600);

  irrecvs[0] = new IRrecv(2); // Receiver #0: pin 2
  irrecvs[1] = new IRrecv(3); // Receiver #1: pin 3
  irrecvs[2] = new IRrecv(4); // Receiver #2: pin 4
  irrecvs[3] = new IRrecv(5); // Receiver #3: pin 5

  for (i = 0; i < RECEIVERS; i++) {
    irrecvs[i]->enableIRIn();
    counter[i] = 0;
  }
}


int read_receivers() {
  command = B0000;
  for (j = 0; j < 6826; j++) {
    for (i = 0; i < RECEIVERS; i++) {
      if (irrecvs[i]->decode(&results)) {
        data[i][counter[i]] = results.value, HEX;
        counter[i] += 1;
        irrecvs[i]->resume();
        if (counter[i] == message_length) {
          //Serial.print("\nres-");
          //Serial.print(i);
          //Serial.print(":");
          if (strstr(data[i], id) != NULL) {
            Serial.print("Fou");
            //delay(1);
            if (i == 0) {
              command = command | B0001;
            }
            else if (i == 1) {
              command = command | B0010;
            }
            else if (i == 2) {
              command = command | B0100;
            }
            else {
              command = command | B1000;
            }
          }
          memset(data[i], 0, sizeof(data[i]));
          counter[i] = 0;
        }
      }
    }
  }
  //Serial.println(command,BIN);
  return command;
}



void loop() {

  //--------Read Receivers-----------------
  command = read_receivers();
  Serial.print("#");
  Serial.print(command, BIN);
  Serial.println("#");
  /*
  if(commander > B0 and commander<B100){
    Serial.println("#Midle#");
  }
  else if(commander > B11 and commander <B1000){
    Serial.println("#Right#");
  }
  else if(commander > B111){
    Serial.println("#Left#");
  }
*/

}
