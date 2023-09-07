//Lector NFC
#include <SPI.h>
#include <MFRC522.h>


//Pantalla
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//Libreria keyboard bluetooth low energy
#include <BleKeyboard.h>
BleKeyboard tecladoFantasma("tecladoFantasma", "by Reber28", 80);                     //80 es el nivel de bateria que se mostrara


//Declaraciones NFC
#define SS_PIN 5
#define RST_PIN 2
MFRC522 mfrc522(SS_PIN, RST_PIN);                                                     // Instance of the class
MFRC522::MIFARE_Key key;



// Temporizador
unsigned long marcaTiempoDate = 0;
unsigned long tiempoRefreshDate = 1000;

// Variables almacena tiempo millis
int dias;
int horas;
int minutos;
int segundos;

// Cadena para almacenar texto formateado
char cadenaTiempo[16];



byte LecturaUID[4];                                                                       // crea array para almacenar el UID leido
byte Usuario1[4] = {0x99, 0x59, 0x8C, 0x9D} ;                                             // Cambiar el UID de tarjeta leido en programa 1. Al pasar la tarjeta deseada
                                                                                          // se imprime en Monitor Serie de manera Hexadecimal
String password = "**********";                                                           // Aquí tu contraseña de Windows
boolean btConnect = false;


//Declaraciones pantalla OLED
#define ANCHO_PANTALLA 128                                                                // Ancho de la pantalla OLED
#define ALTO_PANTALLA 64                                                                  // Alto de la pantalla OLED

#define OLED_RESET     -1                                                                 // Pin reset incluido en algunos modelos de pantallas (-1 si no disponemos de pulsador). 
#define DIRECCION_PANTALLA 0x3C                                                           //Dirección de comunicacion: 0x3D para 128x64, 0x3C para 128x32

Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, OLED_RESET);

#define LOGO_WIDTH    90
#define LOGO_HEIGHT   90



void setup() {

  Serial.begin(9600);                                                                     // inicializa comunicacion por monitor serie a 9600 bps
  SPI.begin();                                                                            // inicializa bus SPI
  mfrc522.PCD_Init();                                                                     // inicializa modulo lector
  tecladoFantasma.begin();                                                                //inicializar el teclado fantasma
  Serial.println("Listo ");                                                               // Muestra texto Listo

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, DIRECCION_PANTALLA)) {
    Serial.println(F("Fallo en la asignacion de SSD1306"));
  }


  while (!btConnect) {
    conexionBLE();
  }

  display.clearDisplay();
  delay(1500);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(18, 15);
  display.println("Esperando tarjeta      para iniciar          la sesion");
  display.display();
  display.drawLine(0, 10, 128, 10, WHITE);
  display.drawLine(0, 42, 128, 42, WHITE);
  display.display();
}



void loop() {

  if ( ! mfrc522.PICC_IsNewCardPresent())                                                 // si no hay una tarjeta presente
    return;                                                                               // retorna al loop esperando por una tarjeta

  if ( ! mfrc522.PICC_ReadCardSerial())                                                   // si no puede obtener datos de la tarjeta
    return;                                                                               // retorna al loop esperando por otra tarjeta

  Serial.print("UID:  ");                                                                 // muestra texto UID:
  Serial.println("");
  /* bucle recorre de a un byte por vez el UID.
     si el byte leido es menor a 0x10 imprime espacio en blanco y numero cero
     sino imprime un espacio en blanco*/
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      Serial.print(" 0");
    } else {

      Serial.print(" ");
    }

    Serial.print(mfrc522.uid.uidByte[i], HEX);                                             // imprime el byte del UID leido en hexadecimal
    LecturaUID[i] = mfrc522.uid.uidByte[i];                                                // almacena en array el byte del UID leido
  }

  Serial.print("\t");
  Serial.println("");
  if (comparaUID(LecturaUID, Usuario1))                                                   // llama a funcion comparaUID con Usuario1
    Serial.println("Bienvenido Usuario 1");                                               // si retorna verdadero muestra texto bienvenida

  /*  else if (comparaUID(LecturaUID, Usuario2))                                          // llama a funcion comparaUID con Usuario2*/
  /* Serial.println("Bienvenido Usuario 2");                                              // si retorna verdadero muestra texto bienvenida*/
  else           // si retorna falso
    Serial.println("No te conozco");                                                      // muestra texto equivalente a acceso denegado




  tarjetaCorrecta();
  escribir();



  mfrc522.PICC_HaltA();                                                                   // detiene comunicacion con tarjeta
  delay(3000);
  imprimir();
  tiempo();
}




boolean comparaUID(byte lectura[], byte usuario[])                                      // funcion comparaUID
{
  for (byte i = 0; i < mfrc522.uid.size; i++) {                                         // bucle recorre de a un byte por vez el UID
    if (lectura[i] != usuario[i])                                                       // si byte de UID leido es distinto a usuario
      return (false);                                                                   // retorna falso
  }
  return (true);                                                                        // si los 4 bytes coinciden retorna verdadero
}



void conexionBLE() {
  if (tecladoFantasma.isConnected()) {
    display.setTextSize(1, 3);
    display.setTextColor(WHITE);
    display.setCursor(8, 20);
    display.clearDisplay();
    display.display();
    display.print("Bluetooth vinculado");
    display.display();
    delay(1000);
    btConnect = true;
  } else {
    display.clearDisplay();
    delay(1000);
    display.setTextSize(1, 2);
    display.setTextColor(WHITE);
    display.setCursor(8, 22);
    display.display();
    display.print("BT no vinculado");
    display.display();
    // Mover texto de derecha a izquierda
    display.startscrollleft(0x00, 0x0F);
    delay(5000);
    display.stopscroll();
    btConnect = false;
  }
}





void imprimirMensajeTarjeta() {

  display.clearDisplay();
  delay(1500);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(18, 15);
  display.println("Esperando tarjeta      para iniciar          la sesion");
  display.display();
  display.drawLine(0, 10, 128, 10, WHITE);
  display.drawLine(0, 42, 128, 42, WHITE);
  display.display();
}

/*
Escribe el password deja pasar dos segundos,
y presiona de manera automática enter
*/
void escribir() {
  tecladoFantasma.write(KEY_RETURN);
  delay(3500);
  tecladoFantasma.print(password);
  delay(2000);
  tecladoFantasma.press(KEY_RETURN);
  tecladoFantasma.releaseAll();
}




void tarjetaCorrecta() {
  if (comparaUID(LecturaUID, Usuario1) == true) {
    delay(1000);
    display.setTextSize(1, 5);
    display.setTextColor(WHITE);
    display.setCursor(18, 15);
    display.clearDisplay();
    display.display();
    display.print("Bienvenido Marc");
    display.display();
    display.drawLine(0, 4, 128, 4, WHITE);
    display.display();
    display.drawLine(0, 54, 128, 54, WHITE);
    display.display();
    tecladoFantasma.write(KEY_RETURN);
    delay(3500);
    tecladoFantasma.print(password);
    delay(2000);
    tecladoFantasma.press(KEY_RETURN);
    tecladoFantasma.releaseAll();
  } else {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(18, 23);
    display.clearDisplay();
    display.display();
    display.println("usuario no \n registrado!");
    display.display();
  }
}


void imprimir() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(28, 28);
  display.clearDisplay();
  display.display();
  display.println("PRUEBAS");
  display.display();

  // Mover texto de derecha a izquierda
  display.startscrollleft(0x00, 0x0F);
  delay(5000);
  display.stopscroll();
  // Mover texto de izquierda a derecha
  display.startscrollright(0x00, 0x0F);
  delay(5000);
  display.stopscroll();
  // Mover texto de derecha a izquierda
  display.startscrollleft(0x00, 0x0F);
  delay(2500);
  display.stopscroll();
  // Mover texto de izquierda a derecha
  display.startscrollright(0x00, 0x0F);
  delay(2500);
  display.stopscroll();

}




void impresionPuertoSerial() {
  // retorna al loop esperando por otra tarjeta

  Serial.print("UID:  ");                                                         // muestra texto UID:
  Serial.println("");
  // bucle recorre de a un byte por vez el UID. si el byte leido es menor a 0x10 imprime
  // espacio en blanco y numero cero
  //sino imprime un espacio en blanco
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      Serial.print(" 0");
    } else {
      Serial.print(" ");
    }

    Serial.print(mfrc522.uid.uidByte[i], HEX);                                    // imprime el byte del UID leido en hexadecimal
    LecturaUID[i] = mfrc522.uid.uidByte[i];                                       // almacena en array el byte del UID leido
  }

  Serial.print("\t");                                                             // imprime un espacio de tabulacion
  Serial.println("");
  if (comparaUID(LecturaUID, Usuario1))                                           // llama a funcion comparaUID con Usuario1
    Serial.println("Bienvenido Usuario 1"); // si retorna verdadero muestra texto bienvenida

  /*  else if (comparaUID(LecturaUID, Usuario2)) // llama a funcion comparaUID con Usuario2*/
  /* Serial.println("Bienvenido Usuario 2"); // si retorna verdadero muestra texto bienvenida*/
  else           // si retorna falso
    Serial.println("No te conozco");                                              // muestra texto equivalente a acceso denegado
}



/*
  Función que convierte millis() a segundos, minutos, horas y días
  Almacena la información en variables globales
*/
void millisToTiempo(unsigned long valMillis) {
  // Se obtienen los segundos
  valMillis = valMillis / 1000;

  segundos = valMillis % 60; // se divide entre segundos por minuto y te quedas con el resto
  minutos = valMillis / 60; // Se convierte a minutos
  minutos = minutos % 60; // se divide entre minutos por hora y te quedas con el resto
  horas = (valMillis / 60) / 60; // Se convierte en horas
  horas = horas % 24; // se divide entre horas al día y te quedas con el resto
  dias = ((valMillis / 60) / 60) / 24; // Se convierte en días
#ifdef __DEBUG__
  Serial.print("Segundos = ");
  Serial.println(valMillis);
  Serial.print(dias);
  Serial.print(":");
  Serial.print(horas);
  Serial.print(":");
  Serial.print(minutos);
  Serial.print(":");
  Serial.println(segundos);
#endif
}

void tiempo() {
  // Protección overflow
  if (millis() < marcaTiempoDate) {
    marcaTiempoDate = millis();
  }

  // Comprobar is hay que actualizar temperatura
  if (millis() - marcaTiempoDate >= tiempoRefreshDate)
  {
    // Actualizar variables de tiempo
    millisToTiempo(millis());
    // Componer cadena con la información del tiempo formateada
    sprintf(cadenaTiempo, "%02d:%02d:%02d", horas, minutos, segundos);
    // Marca de tiempo
    marcaTiempoDate = millis();
  }

  // Limpiar buffer pantalla
  display.clearDisplay();
  // Dibujar línea horizontal
  display.drawLine(0, 4, display.width(), 4, SSD1306_WHITE);
display.drawLine(0, 60, display.width(), 60, SSD1306_WHITE);
  // Dibujar texto tiempo
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 26);
  display.println(cadenaTiempo);
  display.display();
  display.startscrollleft(0x00, 0x0F);
}
