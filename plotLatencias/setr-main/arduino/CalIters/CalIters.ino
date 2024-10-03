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
unsigned long last = 0;

#define TIME_WAIT_MS      10      // medio segundo
#define TIME_WAIT_MICROS  10000   // 10 milisegundos

long toc,tic

void setup() {
  Serial.begin(9600);
}

void loop() {

  tic_mc = millis();
  
  Serial.println("Latency (micros): " + String(last-tic_mc));
  Serial.println("*********");

  last = tic_mc;

  
}
