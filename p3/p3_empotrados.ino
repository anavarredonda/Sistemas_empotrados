#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Thread.h>
#include <StaticThreadController.h>
#include <ThreadController.h>
#include <avr/wdt.h>

//TODO bloqueo dentro de temp_admin

#define DISTANCE_ACTIVATE 100
#define DHTTYPE    DHT11     // DHT 11

#define MIN_TIME_RESET 2000
#define MAX_TIME_RESET 3000
#define TIME_ADMIN_MENU 5000

enum state {
  ARRANQUE,
  SERVICIO_A,
  SERVICIO_B,
  MENU,
  ADMIN ,
  VER_TEMP,
  VER_DIST,
  VER_CONTADOR,
  MODIFICAR_PRECIOS
};

//Pines lcd
const int rs = 7, en = 8, d4 = 10, d5 = 11, d6 = 12, d7 = 13;

//Pines joystick
const int pinJoyX = A0, pinJoyY = A1, pinJoyButton = 3;

//Pines led
const int green_led = 6, red_led = 5;

//Pin boton
const int pin_button = 2;

// Pines ultrasonidos
const int pin_echo = 4, pin_trigger = A5;

//Pin temperatura/humedad
const int pin_dh11 = 9;

long ini_temp_t = 0, fin_temp_t = 0;
int x_joystick, y_joystick;

// 0 =>Joystick no activado, 1 => arriba/derecha, -1 => abajo/izquierda
int cursor_x = 0, cursor_y = 0;
const int timeThreshold3 = 250; //cada cuanto tiempo se admite nueva lectura joystick
long startTime3 = 0; // tiempo al que se ha dejado entrar una lectura del joystick

int servicio = 0; //Estado de la maquinas de estado de loop
int selection = 0;  //Seleccion en cualquiera de los menus

//0 ==> se entra en menu admin tras 5s de boton pulsado, 1 ==> se entra en servicio tras 5s de boton pulsado
int flag_admin_menu = 0;

//incremento que se usara para aumentar o disminuir el precio con el joystick
int increase = 0;
//Incremento en iteracion anterior
int old_increase = 0;

//Estados interrupciones
volatile byte joystick_buttom_state = LOW;
volatile byte buttom_state = HIGH;
//Tiempos iniciales de las interrupciones
long startTime1 = 0;
long pressStartTime = 0;
long time = 0;

long pressDuration = 0;//Duracion de pulsacion de boton

//Cada cuanto tiempo se admite una nueva lectura en las interrupciones
const int timeThreshold1 = 200;
const int timeThreshold2 = 100;

int randomNumber;

float precios[5] = {1.00, 1.10, 1.25, 1.50, 2.00};
char *products[5] = {"Cafe Solo", "Cafe Cortado", "Cafe Doble", "Cafe Premium", "Chocolate"};

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
DHT dht(pin_dh11, DHTTYPE);
ThreadController controller = ThreadController();

void setup() {
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // set up joystick
  pinMode(pinJoyButton , INPUT_PULLUP);

  // setup button
  pinMode(pin_button , INPUT_PULLUP);

  // set up ultrasound
  pinMode(pin_trigger, OUTPUT); //pin como salida
  pinMode(pin_echo, INPUT);

  // set up temp and humidity sensor
  dht.begin();

  // set up thread of joystick
  Thread joystickThread = Thread();
  joystickThread.enabled = true;
  joystickThread.setInterval(30);
  joystickThread.onRun(joystick_function);
  controller.add(&joystickThread);

  // set up interruptions
  attachInterrupt(digitalPinToInterrupt(pinJoyButton), joystick_buttom_interr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_button), buttom_interr, CHANGE);

  // setup watchdog
  wdt_disable();
  wdt_enable(WDTO_8S);

}

void joystick_buttom_interr() {
  if (millis() - startTime1 > timeThreshold1) {
    startTime1 = millis();
    joystick_buttom_state = !joystick_buttom_state;
  }
}

void buttom_interr() {
  if (millis() - pressStartTime > timeThreshold2) {
    buttom_state = !buttom_state;
    // boton pulsado
    if (!buttom_state) {
      time = millis();
    // boton sin pulsar
    } else {
      pressDuration = millis() - time;
      time = 0;
    }
    pressStartTime = millis();
  }
}

int press_button(int estado) {

  if (pressDuration > 0) {
    // reiniciar
    if (pressDuration > 2000 && pressDuration < 3000) {
      pressDuration = 0;
      return SERVICIO_A;
      
    //menu admin
    } else if (pressDuration > 5000) {
      pressDuration = 0;
      if (flag_admin_menu == 0) {
        flag_admin_menu = 1;
        return ADMIN;

      } else {
        flag_admin_menu = 0;
        return SERVICIO_A;
      }
    }
  }
  pressDuration = 0;

  return estado;
}

int see_temp() {
  sensors_event_t event;

  // Get temperature event and show its value.
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp:    ");
    lcd.print(t);
    lcd.print((char)223);
    lcd.print("C");
  }
  
  // Get humidity event and show its value.
  float h = dht.readHumidity();
  if (isnan(h)){
    Serial.println(F("Error reading humidity!"));
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Hum:     ");
    lcd.print(h);
    lcd.print("% ");
  }

  if (cursor_y != -1) {
    return VER_TEMP;

  // se ha usado joystick hacia abajo
  } else {
    // se actualzan variables para poder salir
    joystick_buttom_state = LOW;
    cursor_y = 0;
    return ADMIN;
  }
}

int see_dist_sensor() {

  long t; //tiempo que demora en llegar el eco
  long d; //distancia en centimetros

  digitalWrite(pin_trigger, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(pin_trigger, LOW);

  t = pulseIn(pin_echo, HIGH); //obtenemos el ancho del pulso
  d = t/59;                //escalamos el tiempo a una distancia en cm

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(d);
  lcd.setCursor(14, 0);
  lcd.print("cm");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  delayMicroseconds(10000);

  if (cursor_y != -1) {
    return VER_DIST;

  } else {
    joystick_buttom_state = LOW;
    cursor_y = 0;
    return ADMIN;
  }

}

int see_counter() {
  int counter;
  counter = millis() / 1000;
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(counter);

  delayMicroseconds(10000);

  if (cursor_y != -1) {
    return VER_CONTADOR;
  } else {
    joystick_buttom_state = LOW;
    cursor_y = 0;
    return ADMIN;
  }
}

int modify_prices() {

  //No se ha seleccionado nada
  if (joystick_buttom_state == LOW) {
    switch(selection){
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(products[0]);
      lcd.print("   ");
      lcd.print(precios[0]);

      lcd.setCursor(0, 1);
      lcd.print(products[1]);
      lcd.print(precios[1]);
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print(products[1]);
      lcd.print(precios[1]);

      lcd.setCursor(0, 1);
      lcd.print(products[2]);
      lcd.print("  ");
      lcd.print(precios[2]);
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print(products[2]);
      lcd.print("  ");
      lcd.print(precios[2]);

      lcd.setCursor(0, 1);
      lcd.print(products[3]);
      lcd.print(precios[3]);
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print(products[3]);
      lcd.print(precios[3]);

      lcd.setCursor(0, 1);
      lcd.print(products[4]);
      lcd.print("   ");
      lcd.print(precios[4]);
      break;

    case 4:
      lcd.setCursor(0, 0);
      lcd.print(products[4]);
      lcd.print("   ");
      lcd.print(precios[4]);

      lcd.setCursor(0, 1);
      lcd.print("                 ");
      break;
    }
    
  // Se ha seleccionado un producto
  } else {
    if (increase > old_increase) {
      precios[selection] += 0.05;
    } else if (increase < old_increase) {
      precios[selection] -= 0.05;
      //No permitir valores por debajo de 0
      if (precios[selection] < 0){
        precios[selection] = 0.00;
      }
    }

    old_increase = increase;
  }

  if (cursor_y != -1) {
    return MODIFICAR_PRECIOS;
  } else {
    joystick_buttom_state = LOW;
    cursor_y = 0;
    return ADMIN;
  }

  joystick_buttom_state = LOW;
}

int menu_admin () {

  digitalWrite(red_led, HIGH);
  digitalWrite(green_led, HIGH);

  //No se ha seleccionado nada
  if (joystick_buttom_state == LOW) {
    switch(selection) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Ver temperatura ");

      lcd.setCursor(0, 1);
      lcd.print("Ver distancia   ");
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Ver distancia   ");

      lcd.setCursor(0, 1);
      lcd.print("Ver contador    ");
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Ver contador    ");

      lcd.setCursor(0, 1);
      lcd.print("Modificar precio");
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Modificar precio");

      lcd.setCursor(0, 1);
      lcd.print("                ");
      break;
    }
  // Se ha seleccionado una opcion
  } else {
    switch(selection) {
    case 0:
      return VER_TEMP;
      break;
    
    case 1:
      return VER_DIST;
      break;

    case 2:
      return VER_CONTADOR;
      break;

    case 3:
      return MODIFICAR_PRECIOS;
      break;
    }
  }
  return ADMIN;
}

int menu() {
  int intensity = 0;
  int i = 0;
  int interval_time;

  //No se ha seleccionado nada
  if (joystick_buttom_state == LOW) {
    switch(selection){
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(products[0]);
      lcd.print("   ");
      lcd.print(precios[0]);

      lcd.setCursor(0, 1);
      lcd.print(products[1]);
      lcd.print(precios[1]);
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print(products[1]);
      lcd.print(precios[1]);

      lcd.setCursor(0, 1);
      lcd.print(products[2]);
      lcd.print("  ");
      lcd.print(precios[2]);
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print(products[2]);
      lcd.print("  ");
      lcd.print(precios[2]);

      lcd.setCursor(0, 1);
      lcd.print(products[3]);
      lcd.print(precios[3]);
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print(products[3]);
      lcd.print(precios[3]);

      lcd.setCursor(0, 1);
      lcd.print(products[4]);
      lcd.print("   ");
      lcd.print(precios[4]);
      break;

    case 4:
      lcd.setCursor(0, 0);
      lcd.print(products[4]);
      lcd.print("   ");
      lcd.print(precios[4]);

      lcd.setCursor(0, 1);
      lcd.print("                 ");
      break;
    }
    
  // Se ha seleccionado un producto
  } else {
    ini_temp_t = millis();
    randomNumber = random(4000,8000);

    lcd.setCursor(0, 0);
    lcd.print("Preparando Cafe ");
    lcd.setCursor(0, 1);
    lcd.print("...             ");
    // cambio de rango de 4000-8000 a 0-255
    interval_time = (int) (randomNumber/255);

    while ((fin_temp_t - ini_temp_t) < randomNumber) {
      i++;

      // la intensidad aumentara en funcion del tiempo entre 0 y 255
      if (i == interval_time && intensity < 255) {
        analogWrite(green_led, ++intensity);
        i = 0;
        delayMicroseconds(15000);
       }
      fin_temp_t = millis();
    }

    analogWrite(green_led, 0);
    joystick_buttom_state = LOW;
  }

  return MENU;//Seguir en menu
}

void joystick_function() {
  if (millis() - startTime3 > timeThreshold3) {
    startTime3 = millis();
    x_joystick = analogRead(pinJoyX);
    y_joystick = analogRead(pinJoyY);

    //Variable de seleccion en los menus
    if (x_joystick > 1000 && selection > 0) {
      selection--;
    } else if (x_joystick < 100 && selection < 4) {
      selection++;
    }

    //Variable de seleccion de aumento en modificar precio
    if (x_joystick > 1000) { 
      increase--;
    } else if (x_joystick < 100) {
      increase++;
    }

    if (y_joystick > 1000) {
      cursor_y = 1;
    } else if (y_joystick < 100) {
      cursor_y = -1;
    }
  }
}

int arranque() {
  lcd.print("CARGANDO ...");

  digitalWrite(red_led, HIGH);
  delay(1000);
  digitalWrite(red_led, LOW);
  delay(1000);
  digitalWrite(red_led, HIGH);
  delay(1000);
  digitalWrite(red_led, LOW);

  lcd.clear();
  return SERVICIO_A; //Avanzar al siguiente estado
}

int servicio_a() {
  long t; //tiempo que demora en llegar el eco
  long d; //distancia en centimetros
  int servicio_activo;

  //Apagar leds en caso de venir de menu admin
  digitalWrite(red_led, LOW);
  digitalWrite(green_led, LOW);
 
  digitalWrite(pin_trigger, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(pin_trigger, LOW);

  t = pulseIn(pin_echo, HIGH); //obtenemos el ancho del pulso
  d = t/59;                //escalamos el tiempo a una distancia en cm

  if (d < DISTANCE_ACTIVATE) {
    servicio_activo = SERVICIO_B; // ir a siguiente estado
  } else {
    lcd.print("ESPERANDO       ");
    lcd.setCursor(0, 1);
    lcd.print("CLIENTE         ");
    lcd.setCursor(0, 0);
    servicio_activo = SERVICIO_A;  //seguir esperando
  }
  return servicio_activo;
}

int servicio_b() {
  sensors_event_t event;

  // Get temperature event and show its value.
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp:    ");
    lcd.print(t);
    lcd.print((char)223);
    lcd.print("C");
  }
  
  // Get humidity event and show its value.
  float h = dht.readHumidity();
  if (isnan(h)){
    Serial.println(F("Error reading humidity!"));
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Hum:     ");
    lcd.print(h);
    lcd.print("% ");
  }
}

void loop() {
  controller.run();

  servicio = press_button(servicio);

  switch (servicio){
  case ARRANQUE:
    servicio = arranque();
    break;

  case SERVICIO_A:
    servicio = servicio_a();
    break;

  case SERVICIO_B:
    ini_temp_t = millis();

    while ((fin_temp_t - ini_temp_t) < 5000) {
      servicio_b();
      fin_temp_t = millis();
    }

    servicio = MENU;
    break;

  case MENU:
    servicio = menu();
    break;

  case ADMIN:
    servicio = menu_admin();
    break;

  case VER_TEMP:
    servicio = see_temp();
    break;

  case VER_DIST:
    servicio = see_dist_sensor();
    break;

  case VER_CONTADOR:
    servicio = see_counter();
    break;

  case MODIFICAR_PRECIOS:
    servicio = modify_prices();
    break;
  }

  wdt_reset();
}
