---------------------------------------------------------------------------

  // Bibliotecas em uso

  #include <Wifi.h>

  #include <HTTPClient.h>

  #include <NTPClient.h>

  #include <WifiUdp.h>

  #include <Wire.h>

  #include <ArduinoJson.h>

  #include <Arduino.h>

  #include <SPI.h>

  #include "FS.h"

  #include "SD.h"

  #include "SPI.h"

  #include <iostream>

  #include <cmath>

  #include <Adafruit_BMP280.h> 

  #include <Adafruit_MPU6050.h> 



  // Pinos de comunicação do sensor BMP280

  #define BMP_SCK ()

  #define BMP_MISO ()

  #define BMP_MOSI ()

  #define BMP_CS ()


  // Iniatialize da biblioteca Adafruit para o BMP280 e o MPU6050

  Adafruit_MPU6050 mpu;

  Adafruit_BMP280 bmp;


  // Sensor de Metano

  int 


  // Sensor de CO2

  int


  // Sensor de ...

  int 


  // Pino da bateria pra ver a tensão

  const int batteryPin = 35


  // Credenciais de rede WiFi 

  const char* ssid = "ESTEVAO_2G";

  const char* password = "14082006ian";

  //const char* ssid = "Winterfel"; 

  //const char* password = "panqueca123"; 

  //const char *ssid = "OBSAT ETEVI"; 

  //const char *password = "panqueca"; 

  //const char *ssid = "Rede dos Templarios"; 

  //const char *password = "23571113"; 

  const char *serverUrl = "https://obsat.org.br/teste_post/envio.php"; 


  // Configuração do buzzer 

  const int buzzerPin = 12; 

  const int melodyINICIAL[] = { 

  //392, 392, 392, 311, 466, 392, 311, 466, 392 //ofc
   // 300, 200, 300, 400, 500, 600, 700, 500, 900
   392, 440, 468, 589, 663, 701, 589, 663, 526
  }; 

  const int noteDurationsINICIAL[] = { 

  100, 100, 200, 100, 100, 200, 100, 100, 100 //ofc
    //500, 500, 200, 500, 500, 200, 500, 500, 200
   

  }; 

  const int melodyVIRGULA[] = { 

  //587, 523, 466, 392 //ofc
  200, 100, 200, 300

  }; 

  const int noteDurationsVIRGULA[] = { 

  100, 100, 100, 100 

  }; 


  // Função para reproduzir um tom no buzzer 

  void playTone(int frequency, int duration) { 

    unsigned long startTime = millis(); 

    while (millis() - startTime < duration) { 

      digitalWrite(buzzerPin, HIGH); 

      delayMicroseconds(1000000 / frequency / 2); 

      digitalWrite(buzzerPin, LOW); 

      delayMicroseconds(1000000 / frequency / 2); 

  } 
} 


  // NTP configurações

  WiFiUDP ntpUDP;

  NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600);


  // Função para escrever um arquivo no sistema de arquivos

  void WriteFile(fs::FS &fs, const char *path, const char *message) {

    Serial.printf("Escrevendo arquivo...", path);


  File file = fs.open(path, FILE_WRITE);

  if (!file) {

    Serial.print("Falha ao abrir o arquivo para escrita");

    return;

  }

  if (file.print(message))

    Serial.println("Arquivo escrito com sucesso");

  } else {

    Serial.printIn("Falha na escrita");

  }

  file.close();

}


  void appendFileF(fs::FS &fs, const char *path, const char *message) { 

    Serial.printf("Adicionando ao arquivo...", path); 


  File file = fs.open(path, FILE_APPEND); 

  if (!file) { 

    Serial.println("Falha ao abrir o arquivo para adicionar conteúdo"); 

    return; 

  } 

  if (file.print(message)) { 

    Serial.println("Conteúdo adicionado com sucesso"); 

  } else { 

    Serial.println("Falha ao adicionar conteúdo"); 

  } 

  file.close(); 

} 


---------------------------------------------------------------------------------------------

//Configuração inicial do código

void setup() {
