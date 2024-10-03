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

int LED_PIN=12;

void setup() {

    // Necesario para inicializar la velocidad
    // del puerto seria para mostrar mensaje de depuración
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);

}

void loop() {
    int sleep = 5;
    
    int tic=millis();
    delay(sleep);
    int toc=millis();
    Serial.println((toc-tic)-sleep);
    
    //Serial.println("Inicio loop");
    //digitalWrite(LED_PIN, HIGH);
    //delay(1000);
    //digitalWrite(LED_PIN, LOW);
    //delay(1000);

}
