#include <Servo.h>

#define PIN_T1 8
#define PIN_T2 9
#define PIN_SERVO 11

#define PERIODIC_T1 1000   //LED
#define PERIODIC_T2 1000   //LED

#define PERIODIC_T3 100    //SERVO

#define COMPUTATION_TIME_ON_T1 250
#define COMPUTATION_TIME_ON_T2 250


Servo myservo;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("In Setup function");
  
  pinMode(PIN_T1,OUTPUT);
  pinMode(PIN_T2,OUTPUT);
  
  myservo.attach(PIN_SERVO);

}

int last_T1=0;
int last_T2=0;
int last_T3=0;
int pos = 0;
int step_servo = 5;

void loop() {
  // put your main code here, to run repeatedly:
  
  if ((millis() - last_T3) > PERIODIC_T3) {    
    last_T3 = millis();
    
    myservo.write(pos);
    
    pos = pos + step_servo;    
    if (pos >= 175) step_servo = -5;
    if (pos <= 5)  step_servo = 5;
  }

  if ((millis() - last_T2) > PERIODIC_T2) {    
    last_T2 = millis(); 
    digitalWrite(PIN_T1,LOW);
    digitalWrite(PIN_T2,HIGH);
    delay(COMPUTATION_TIME_ON_T1);
    digitalWrite(PIN_T2,LOW);
  }

  if ((millis() - last_T1) > PERIODIC_T1) {
    last_T1 = millis(); 
    digitalWrite(PIN_T1,HIGH);
    digitalWrite(PIN_T2,LOW);
    delay(COMPUTATION_TIME_ON_T2);
    digitalWrite(PIN_T1,LOW);
  }
  

}
