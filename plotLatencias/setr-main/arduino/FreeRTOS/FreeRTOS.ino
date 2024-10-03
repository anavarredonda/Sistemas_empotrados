/*
* Copyright (C) 2020 by Roberto Calvo-Palomino
*
*
*  This programa is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with RTL-Spec.  If not, see <http://www.gnu.org/licenses/>.
*
*   Authors: Roberto Calvo-Palomino <roberto.calvo [at] urjc [dot] es>
*   
*   based on https://microcontrollerslab.com/use-freertos-arduino/
*/

#include <Arduino_FreeRTOS.h>
#include <Servo.h>

#define PIN_T1 8
#define PIN_T2 9
#define PIN_IDLE 10
#define PIN_SERVO 11

#define PERIODIC_T1 1000   //LED
#define PERIODIC_T2 1000   //LED

#define PERIODIC_T3 100    //SERVO
#define PERIODIC_IDLE 100    //SERVO

#define COMPUTATION_TIME_ON_T1 250
#define COMPUTATION_TIME_ON_T2 250

#define DELAY_IDLE 15

Servo myservo;

void setup()
{
  Serial.begin(9600);
  Serial.println("In Setup function");
  
  pinMode(PIN_T1,OUTPUT);
  pinMode(PIN_T2,OUTPUT);
  pinMode(PIN_IDLE,OUTPUT);
  
  myservo.attach(PIN_SERVO);

  // Creamos las tareas de nuestro sistema con diferentes prioridades.
  xTaskCreate(MyTask1, "TaskLed1", 100, NULL, 3, NULL); 
  xTaskCreate(MyTask2, "TaskLed2", 100, NULL, 2, NULL);  
   
  xTaskCreate(MyTask3, "TaskServo3", 100, NULL, 4, NULL);

  xTaskCreate(MyIdleTask, "IdleTask", 100, NULL, 0, NULL);

}

  
void loop()
{
  // No hay necesidad de añadir instrucciones en el loop
  // Todas las tareas se ejecutan en el kernel FreeRTOS
}


// Task1
static void MyTask1(void* pvParameters)
{
  TickType_t xLastWakeTime, aux;
  
  while(1)
  { 
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;
      
    digitalWrite(PIN_T1,HIGH);
    //digitalWrite(PIN_T2,LOW);
    digitalWrite(PIN_IDLE,LOW);    


    while ( (aux - xLastWakeTime)*portTICK_PERIOD_MS < COMPUTATION_TIME_ON_T1) {
      aux = xTaskGetTickCount();
    }
    digitalWrite(PIN_T1,LOW);
    Serial.println("Task1");   
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_T1 / portTICK_PERIOD_MS ) );
       
  }
}

// Task2
static void MyTask2(void* pvParameters)
{
  TickType_t xLastWakeTime, aux;
  while(1)

  { 
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;
    
    //digitalWrite(PIN_T1,LOW);
    digitalWrite(PIN_T2,HIGH);
    digitalWrite(PIN_IDLE,LOW);    


    while ( (aux - xLastWakeTime)*portTICK_PERIOD_MS < COMPUTATION_TIME_ON_T2) {
      aux = xTaskGetTickCount();
    }

    digitalWrite(PIN_T2,LOW);
    Serial.println("Task2");        
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_T2 / portTICK_PERIOD_MS ) );
       
  }
}

// Task3
static void MyTask3(void* pvParameters)
{
  TickType_t xLastWakeTime, aux;
  int pos = 0;
  int step_servo = 5;
  
  while(1)
  { 
    digitalWrite(PIN_IDLE,LOW);
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;

    myservo.write(pos);
    
    pos = pos + step_servo;    
    if (pos >= 180) step_servo = -5;
    if (pos <= 0)  step_servo = 5;

    Serial.println("Task3");        
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_T3 / portTICK_PERIOD_MS ) );
       
  }
}

// IDLE Task
static void MyIdleTask(void* pvParameters)
{
  TickType_t xLastWakeTime, aux;
  while(1)
   { 
    xLastWakeTime = xTaskGetTickCount();
    digitalWrite(PIN_T1,LOW);
    digitalWrite(PIN_T2,LOW);
    digitalWrite(PIN_IDLE,HIGH);         
    Serial.println("Idle state");
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_IDLE / portTICK_PERIOD_MS ) );
  }  
}
