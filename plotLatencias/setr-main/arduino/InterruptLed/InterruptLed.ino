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

const int INT_PIN = 2;

volatile byte state = LOW;
long startTime = 0;
const int timeThreshold = 200;

void blink() {
  if (millis() - startTime > timeThreshold) {
    startTime = millis();
    Serial.println("Interruption: " + String(millis()));
    state = !state;
    digitalWrite(LED_BUILTIN, state);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);    
  
  pinMode(LED_BUILTIN, OUTPUT);  
  pinMode(INT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), blink, CHANGE);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  Serial.println(state);
}
