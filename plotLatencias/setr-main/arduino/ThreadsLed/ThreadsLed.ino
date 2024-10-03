/*
* Copyright (C) 2022 by Roberto Calvo-Palomino
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
*/

// Subject: Sistemas Empotrados y de Tiempo Real
// Universidad Rey Juan Carlos, Spain

#include <Thread.h>
#include <StaticThreadController.h>
#include <ThreadController.h>

#define PIN_LED 8

class LedThread: public Thread {

public:
  int pin;
  bool state;
  
  LedThread(int _pin): Thread() {
    pin = _pin;
    state = true;
    
    pinMode(pin, OUTPUT);
  }
  
  bool shouldRun(unsigned long time) {
    return Thread::shouldRun(time);
  }
  
  void run(){
    
    Thread::run();
    digitalWrite(pin, state ? HIGH : LOW);
    state = !state;
  }
};

ThreadController controller = ThreadController();



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(100);

  LedThread* ledThread = new LedThread(LED_BUILTIN);
  ledThread->setInterval(1000);
  controller.add(ledThread);
  
  LedThread* ledThread2 = new LedThread(PIN_LED);
  ledThread2->setInterval(500);
  controller.add(ledThread2);
}
  


void loop() {
  
  controller.run();
  
}
