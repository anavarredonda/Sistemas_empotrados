# Blog practica 3 Sistemas Empotrados y de tiempo real
------------------------------------------------------
# Esquemático del circuito

# Video de funcionamiento
https://urjc-my.sharepoint.com/:v:/g/personal/a_navarredonda_2022_alumnos_urjc_es/EU0ftQJQI_hEpcBTk1Hhcf8BtisPNhXg_mremefl9S028A?nav=eyJyZWZlcnJhbEluZm8iOnsicmVmZXJyYWxBcHAiOiJPbmVEcml2ZUZvckJ1c2luZXNzIiwicmVmZXJyYWxBcHBQbGF0Zm9ybSI6IldlYiIsInJlZmVycmFsTW9kZSI6InZpZXciLCJyZWZlcnJhbFZpZXciOiJNeUZpbGVzTGlua0NvcHkifX0&e=nR07Vo

# Explicacion de mi solucion

Para empezar decir que al principio del fichero estan declarados todos las variables globales que se necesitaran en el resto del codigo, asi como los distintos pines para los distintos componentes.

En el bucle principal del programa he implementado una maquina de estados, en la que cada estado implementa una de las distantas funcionalidades del sistema, a continuacion explicare el funcionamiento de los distintos estados.

Para implementar el funcionamiento del boton, he usado interrupciones, usando una resistencia software PULLUP, asi como el uso de tiempos para evitar el rebote del boton, esto se aplica tambien a el boton del joystick que he implementado tambien a traves de una interrupcion.
En cuanto al propio joystick he usado un thread de arduino para llamar a la funcion que se ocupa de leer los valores del joystick cada 30 ms.

Empezando ya con la explicion propia de la maquina de estados del bucle principal, tenemos el primer estado, el de arranque, en este estado el sistema se limita a imprimir en el lcd CARGANDO... y encender y apagar un led cada segundo durante 3 segundos, esto lo hara siempre al arrancar el arduino. Una vez que termine la secuencia de 3 segundos, pasara al estado SERVICIO_A, que implementa el comportamiento del apartado servicio A del enunciado.

En el estado SERVICIO_A el sistema estara a la espera de la llegada de un cliente, para ello se imprime en la lcd ESPERANDO CLIENTE, mientras que se usa el ultrasonidos para lanzar pulsos y detectar obstaculos cercanos, si el ultrasonidos detecta obstaculos a menos de 100 cm significa que hay un cliente presente, por lo que pasara a SERVICIO_B, en caso contrario permarecera en SERVICIO_A.

En el estado SERVICIO_B, el sistema lee la informacion del sensor de temperatura y humedad, y la muestra a traves del lcd durante 5 segundos, en caso de cambio durante esos 5 segundos, el valor se actualiza en la lcd. Pasados los 5 segundos, el sistema pasa el estado MENU, que representa el menu principal para selecionar el producto que se quiere comprar.

En el estado MENU, el sistema imprimira en la lcd los productos y sus respectivos precios de dos en dos, y nos desplazaremos por el menu gracias al joystick, para navegar la funcion que recoge el valor del joystick, pasado unos umbrales, aumentara el indice, que es la variable que contiene el producto que se esta seleccionando en ese momento, en el menu se usara esa variable como estado de una maquina de estados, en la que cada estado es la representacion en la lcd del producto seleccionado, y el siguiente. Para seleccionar el producto que queramos, pulsaremos el boton del joystick, tras lo cual se imprimira en la lcd PREPARANDO CAFE, mientras que enciende un led de manera progresiva, esto con una duracion aleatorioa entre 4000ms y 8000ms.

Si mantenemos el boton pulsado entre 2 segundos y 3, reiniciaremos a el estado SERVICIO_A, repitiendo la secuencia desde ahi, en caso de mantener el boton pulsado durante mas de 5 segundos entraremos en el menu de administracion, representado por el estado ADMIN.

En el estado ADMIN, el sistema, usando la misma logica que en el menu principal, imprimira las distantas opciones a mostrar, que se seleccionaran igual que en el menu principal, la unica diferencia con el menu principal es que en este caso, cada caso al tratarse de comportamientos no simples, representan un estado en la maquina de estados principal.

Al seleccionar ver temperatura en el menu de administrador, entraremos en el estado VER_TEMP, que mostrara la temperatura y humedad del habiente, de manera continua hasta que movamos el joystick hacia la izquierda, para volver al menu de administracion, comportamiento que se repetira en las distintas opciones del menu de administrador.

La siguiente opcion que podemos seleccionar en el menu de administrador es ver la distacia que esta detenctando el sensor de ultrasonidos, represnetado por el estado VER_DIST, en este estado se imprimira el numero de centimetros al obstaculo mas cercano, y al igual que en el estado anterior, para volver al menu de administrador debemos de mover el joystick hacia la izquierda.

La tercera opcion en el menu de administrador es la de ver el contador de segundos, representado con el estado VER_CONTADOR, en este estado imprimiremos en la lcd el numero de segundos que lleva la placa encendida, para esto aproveche la funcion millis que devuelve el numero de milisegundos desde que la placa esta encendidad, y lo converti a segundo antes de mostrarlo. Al igual que los estados anteriores, para volver al menu de adminstrador moveremos el joystick hacia la izquierda.

La ultima opcion del menu de administracion es la de modificar los precios de los productos, representado con el estado MODIFICAR_PRECIOS, la logica de este estado es la misma que en el menu principal, pues se imprimi lo mismo y navegamos igual a traves del menu, la unica difererncia es que en este estado, una vez selecionado un producto con el boton del joystick, usaremos el joystick hacia arriba o hacia abajo para aumentar o disminuir el precio del producto, y lo fijaremos pulsando de nuevo el boton del joystick, los cambios se haran de 5 centimos en 5 centimos, y se reflejan en los precios de los productos en el menu principal.

Para salir del menu de administrador, pulsaremos el boton durante 5 segundos, para volver al estado SERVICIO_A, en el que se seguira el comportamiento descrito anteriormente.

Por ultimo, decir que he implementado un watchdog con un intervalo de 8 segundos que se reincia tras la maquina de estados principal, y que reiniciara la placa en caso de que el microcontrolador tarde mas de 8 segundos en volver a reinciar el watchdog, he elegido 8 segundos debido a que hay algunos estados que depender del tiempo, como el estado SERVICIO_B que tardara un minimo de 5 segundos en ejecutarse, por lo que me parecio razonable elegir 8 segundos para darle un poco de margen en caso de que el microcontrador tarde un poco mas en ejecutar.
