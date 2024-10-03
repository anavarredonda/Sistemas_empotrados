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


ThreadController controller = ThreadController();
Thread myThread = Thread();
Thread myThread2 = Thread();


void callback_thread1() {
  Serial.println("Hello TH1 " + String(millis()));  
  delay(100);

}

void callback_thread2() {
  Serial.println("Hello TH2 " + String(millis()));  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(100);

  
  myThread2.enabled = true;
  myThread2.setInterval(300);  
  myThread2.onRun(callback_thread2);
  

  myThread.enabled = true;
  myThread.setInterval(300);  
  myThread.onRun(callback_thread1);
  
  controller.add(&myThread);
  controller.add(&myThread2);

  
}


void loop() {
  
  controller.run();
  
}
