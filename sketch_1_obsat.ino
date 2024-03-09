/*************************************************************************** 

 
 

***************************************************************************/ 

// Bibliotecas utilizadas no código 

#include <WiFi.h> 

#include <HTTPClient.h> 

#include <NTPClient.h> 

#include <WiFiUdp.h> 

#include <Wire.h> 

#include <SPI.h> 

#include <Adafruit_BMP280.h> 

#include <Adafruit_MPU6050.h> 

#include <ArduinoJson.h> 

#include <Arduino.h> 

#include "FS.h" 

#include "SD.h" 

#include "SPI.h" 

#include <iostream> 

#include <cmath> 

 
 

// Definição dos pinos para comunicação com o sensor BMP280 

#define BMP_SCK (13) 

#define BMP_MISO (12) 

#define BMP_MOSI (11) 

#define BMP_CS (10) 

 
 

// Inicialização dos objetos da biblioteca Adafruit para o BMP280 e o MPU6050 

Adafruit_MPU6050 mpu; 

Adafruit_BMP280 bmp; 

 
 

// Sensor UV 

int UVOUT = 36; /* Pino D36 do ESP32 conetado ao Out do sensor */ 

int REF = 39;   /* Pino D34 do ESP32 conectado ao EN do sensor */ 

 
 

//Pino analógico 4 para entrada do sensor_MQ7 

int MQ7_input = 4; 

// Constantes de calibração - substitua pelos valores reais obtidos na calibração 

const float valor_analogico_zero = 200;  // Valor analógico em um ambiente sem CO (ppm = 0) 

const float ppm_zero = 0.0;              // Concentração de CO correspondente ao valor analógico zero 

const float valor_analogico_max = 800;   // Valor analógico em um ambiente com concentração conhecida de CO (por exemplo, 10 ppm) 

const float ppm_max = 10.0;              // Concentração de CO correspondente ao valor analógico máximo 

// Função para converter valor analógico em ppm 

float analogToPPM(float valor_analogico) { 

  // Interpolação linear 

  float slope = (ppm_max - ppm_zero) / (valor_analogico_max - valor_analogico_zero); 

  float intercept = ppm_zero - (slope * valor_analogico_zero); 

 
 

  return (slope * valor_analogico) + intercept; 

} 

 


const int MQ131_input = 34;       // Pino analógico 34 para o sensor MQ-131 

const float RLOAD = 10.0;         // Valor do resistor de carga (ohms) 

const float RL_ZERO = 76.63;      // Resistência do sensor em ppm zero (Rs/R0 em ppm zero) 

const float RL_CLEAN_AIR = 7.34;  // Resistência do sensor no ar limpo (Rs/R0 no ar limpo) 

 
 

// Pino utilizado para leitura da voltagem da bateria 

const int batteryPin = 35; 

 
 

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

 
 

// NTP configurações 

WiFiUDP ntpUDP; 

NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600); 

 
 

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

 
 

// Função para escrever um arquivo no sistema de arquivos 

void writeFile(fs::FS &fs, const char *path, const char *message) { 

  Serial.printf("Escrevendo arquivo: %s\n", path); 

 
 

  File file = fs.open(path, FILE_WRITE); 

  if (!file) { 

    Serial.println("Falha ao abrir o arquivo para escrita"); 

    return; 

  } 

  if (file.print(message)) { 

    Serial.println("Arquivo escrito"); 

  } else { 

    Serial.println("Falha na escrita"); 

  } 

  file.close(); 

} 

 
 

// Função para adicionar conteúdo a um arquivo existente no sistema de arquivos (versão com mensagem de debug) 

void appendFile(fs::FS &fs, const char *path, const char *message) { 

 
 

  File file = fs.open(path, FILE_APPEND); 

  if (!file) { 

    Serial.println("Falha ao abrir o arquivo para adicionar conteúdo"); 

    return; 

  } 

  if (file.print(message)) { 

 
 

  } else { 

    Serial.println("Falha ao adicionar conteúdo"); 

  } 

  file.close(); 

} 

 
 

void appendFileF(fs::FS &fs, const char *path, const char *message) { 

  Serial.printf("Adicionando ao arquivo: %s\n", path); 

 
 

  File file = fs.open(path, FILE_APPEND); 

  if (!file) { 

    Serial.println("Falha ao abrir o arquivo para adicionar conteúdo"); 

    return; 

  } 

  if (file.print(message)) { 

    Serial.println("Conteúdo adicionado"); 

  } else { 

    Serial.println("Falha ao adicionar conteúdo"); 

  } 

  file.close(); 

} 

 
 

// Configuração inicial do programa 

void setup() { 

  Serial.begin(115200); 

 
 

  pinMode(buzzerPin, OUTPUT); 

  // Tocar uma melodia inicial 

  for (int i = 0; i < sizeof(melodyINICIAL) / sizeof(melodyINICIAL[0]); i++) { 

    playTone(melodyINICIAL[i], noteDurationsINICIAL[i]); 

    delay(noteDurationsINICIAL[i] * 1.30);  // Pausa entre as notas 

  } 

 
 

  // Conexão com a rede WiFi 

  WiFi.begin(ssid, password); 

  Serial.println("Conectando ao WiFi"); 

  while (WiFi.status() != WL_CONNECTED) { 

    delay(1000); 

    Serial.print("."); 

  } 

  Serial.println("\nConectado"); 

  SomVirgula(); 

 
 

  while (!Serial) delay(100);  // aguardar conexão USB nativa 

  Serial.println(F("Teste BMP280")); 

  Serial.println("Teste Adafruit MPU6050!"); 

 
 

  // Inicialização do NTP (Network Time Protocol) 

  timeClient.begin(); 

  timeClient.update(); 

 
 

  unsigned status; 

  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID); 

  status = bmp.begin(0x76, 0x58); 

  if (!status) { 

    Serial.println(F("Não foi possível encontrar um sensor BMP280 válido, verifique a conexão ou tente um endereço diferente!")); 

    Serial.print("SensorID: 0x"); 

    Serial.println(bmp.sensorID(), 16); 

    Serial.print("ID de 0xFF provavelmente indica um endereço inválido, BMP 180 ou BMP 085\n"); 

    Serial.print("ID de 0x56-0x58 representa um BMP 280,\n"); 

    Serial.print("ID de 0x60 representa um BME 280.\n"); 

    Serial.print("ID de 0x61 representa um BME 680.\n"); 

    while (1) delay(10); 

  } 

 
 

  /* Configurações padrão do datasheet. */ 

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Modo de operação. */ 

                  Adafruit_BMP280::SAMPLING_X2,     /* Oversampling de temperatura */ 

                  Adafruit_BMP280::SAMPLING_X16,    /* Oversampling de pressão */ 

                  Adafruit_BMP280::FILTER_X16,      /* Filtro. */ 

                  Adafruit_BMP280::STANDBY_MS_500); /* Tempo de espera. */ 

 
 

  // Verificar se o MPU6050 está presente 

  if (!mpu.begin()) { 

    Serial.println("Falha ao encontrar o chip MPU6050"); 

    while (1) { 

      delay(10); 

    } 

  } 

  Serial.println("MPU6050 Encontrado!"); 

 
 

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); 

  Serial.print("Faixa do acelerômetro configurada para: "); 

  switch (mpu.getAccelerometerRange()) { 

    case MPU6050_RANGE_2_G: 

      Serial.println("+-2G"); 

      break; 

    case MPU6050_RANGE_4_G: 

      Serial.println("+-4G"); 

      break; 

    case MPU6050_RANGE_8_G: 

      Serial.println("+-8G"); 

      break; 

    case MPU6050_RANGE_16_G: 

      Serial.println("+-16G"); 

      break; 

  } 

  mpu.setGyroRange(MPU6050_RANGE_500_DEG); 

  Serial.print("Faixa do giroscópio configurada para: "); 

  switch (mpu.getGyroRange()) { 

    case MPU6050_RANGE_250_DEG: 

      Serial.println("+- 250 deg/s"); 

      break; 

    case MPU6050_RANGE_500_DEG: 

      Serial.println("+- 500 deg/s"); 

      break; 

    case MPU6050_RANGE_1000_DEG: 

      Serial.println("+- 1000 deg/s"); 

      break; 

    case MPU6050_RANGE_2000_DEG: 

      Serial.println("+- 2000 deg/s"); 

      break; 

  } 

 
 

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); 

  Serial.print("Largura de banda do filtro configurada para: "); 

  switch (mpu.getFilterBandwidth()) { 

    case MPU6050_BAND_260_HZ: 

      Serial.println("260 Hz"); 

      break; 

    case MPU6050_BAND_184_HZ: 

      Serial.println("184 Hz"); 

      break; 

    case MPU6050_BAND_94_HZ: 

      Serial.println("94 Hz"); 

      break; 

    case MPU6050_BAND_44_HZ: 

      Serial.println("44 Hz"); 

      break; 

    case MPU6050_BAND_21_HZ: 

      Serial.println("21 Hz"); 

      break; 

    case MPU6050_BAND_10_HZ: 

      Serial.println("10 Hz"); 

      break; 

    case MPU6050_BAND_5_HZ: 

      Serial.println("5 Hz"); 

      break; 

  } 

 
 

  // Verificar se o cartão SD está presente 

  if (!SD.begin()) { 

    Serial.println("Falha ao montar o cartão SD"); 

    return; 

  } 

  uint8_t cardType = SD.cardType(); 

 
 

  if (cardType == CARD_NONE) { 

    Serial.println("Nenhum cartão SD conectado"); 

    return; 

  } 

 
 

  Serial.print("Tipo do Cartão SD: "); 

  if (cardType == CARD_MMC) { 

    Serial.println("MMC"); 

  } else if (cardType == CARD_SD) { 

    Serial.println("SDSC"); 

  } else if (cardType == CARD_SDHC) { 

    Serial.println("SDHC"); 

  } else { 

    Serial.println("DESCONHECIDO"); 

  } 

 
 

  uint64_t cardSize = SD.cardSize() / (1024 * 1024); 

  Serial.printf("Tamanho do Cartão SD: %lluMB\n", cardSize); 

  appendFile(SD, "/DadosCUBESAT.txt", "\n"); 

 
 

  Serial.println(""); 

} 

 
 

/// Loop principal do programa 

void loop() { 

  Serial.println(F("============================================")); 

 
 

  // Obter a data e hora atual 

  timeClient.update(); 

  String dateTime = timeClient.getFormattedTime(); 

 
 

  // Ler a voltagem da bateria 

  int batteryValue = analogRead(batteryPin); 

  float batteryVoltage = batteryValue * 0.0011224; 

  Serial.print("Bateria = "); 

  Serial.print(batteryVoltage); 

  Serial.println(" %"); 

 
 

  // Realizar leituras dos sensores 

  Leitura_BMP280(); 

  Leitura_MPU5060(); 

  Leitura_Sensor_MQ7(); 

  Leitura_Sensor_MQ131(); 

  Leitura_Sensor_UV(); 

  Serial.println(" "); 

  Serial.println(" "); 

 
 

  // Criar objeto JSON e preencher campos 

  DynamicJsonDocument jsonDoc(512); 

  jsonDoc["equipe"] = 9876; 

  jsonDoc["bateria"] = round((analogRead(batteryPin) * 0.0011224) * 100 / 5); 

  jsonDoc["temperatura"] = bmp.readTemperature(); 

  jsonDoc["pressao"] = bmp.readPressure() / 100; 

 
 

  sensors_event_t a, g, temp; 

  mpu.getEvent(&a, &g, &temp); 

  JsonArray giroscopioArray = jsonDoc.createNestedArray("giroscopio"); 

  giroscopioArray.add(g.gyro.x); 

  giroscopioArray.add(g.gyro.y); 

  giroscopioArray.add(g.gyro.z); 

 
 

  JsonArray acelerometroArray = jsonDoc.createNestedArray("acelerometro"); 

  acelerometroArray.add(a.acceleration.x); 

  acelerometroArray.add(a.acceleration.y); 

  acelerometroArray.add(a.acceleration.z); 

 
 

  JsonObject payloadObject = jsonDoc.createNestedObject("payload"); 

  payloadObject["MonoxidoCarbono"] = 1; 

  payloadObject["Ozonio"] = analogRead(MQ131_input); 

  payloadObject["UV"] = analogRead(MQ131_input); 

 
 
 
 
 

  // Serializar o JSON para uma string 

  String jsonString; 

  serializeJson(jsonDoc, jsonString); 

  Serial.print("Json "); 

  Serial.println(jsonString); 

 
 

  // Fazer a requisição HTTP POST 

  HTTPClient http; 

  http.begin(serverUrl); 

  http.addHeader("Content-Type", "application/json"); 

 
 

  int httpResponseCode = http.POST(jsonString); 

  if (httpResponseCode > 0) { 

    Serial.println("Resposta do servidor: "); 

    Serial.println(http.getString()); 

  } else { 

    Serial.print("Erro na requisição HTTP: "); 

    Serial.println(httpResponseCode); 

  } 

 
 

  // Salvar o LOG de informações no cartão de memória 

  File dataFile = SD.open("/DadosCUBESAT.txt", FILE_APPEND); 

  if (dataFile) { 

    dataFile.println(timeClient.getFormattedTime().c_str()); 

    dataFile.println(jsonString); 

    dataFile.println();  // Adicione uma linha em branco para separar os registros 

    dataFile.close(); 

    Serial.println("JSON salvo no cartão de memória."); 

  } else { 

    Serial.println("Erro ao abrir o arquivo no cartão de memória."); 

  } 

 
 

  // Tocar um tom no buzzer de leitura 

  playTone(500, 100); 

  delay(2000); 

} 

 
 
 

// Função para realizar a leitura do sensor BMP280 

void Leitura_BMP280() { 

  Serial.print(F("Pressão = ")); 

  Serial.print(bmp.readPressure() / 100); 

  Serial.println(" Pa"); 

  Serial.print(F("Altitude aproximada = ")); 

  Serial.print(bmp.readAltitude(1022)); /* Ajustado para a previsão local! */ 

  Serial.println(" m"); 

  Serial.println(); 

} 

 
 

// Função para realizar a leitura do sensor MPU6050 

void Leitura_MPU5060() { 

  /* Obter novos eventos do sensor com as leituras */ 

  sensors_event_t a, g, temp; 

  mpu.getEvent(&a, &g, &temp); 

 
 

  Serial.print("Aceleração X: "); 

  Serial.print(a.acceleration.x); 

  Serial.print(", Y: "); 

  Serial.print(a.acceleration.y); 

  Serial.print(", Z: "); 

  Serial.print(a.acceleration.z); 

  Serial.println(" m/s^2"); 

  Serial.print("Rotação X: "); 

  Serial.print(g.gyro.x); 

  Serial.print(", Y: "); 

  Serial.print(g.gyro.y); 

  Serial.print(", Z: "); 

  Serial.print(g.gyro.z); 

  Serial.println(" rad/s"); 

  Serial.println(" "); 

} 

 
 

void Leitura_Sensor_MQ7() { 

  int sensor_MQ7 = analogRead(MQ7_input);  //Função de leitura do valor analógico sensor_MQ7 

  float ppm_CO = analogToPPM(sensor_MQ7);  // Converter para ppm 

  Serial.print("CO em ppm: "); 

  Serial.println(ppm_CO);  //Imprime o valor lido em ppm 

} 

 
 

void Leitura_Sensor_UV() { 

  int uvLevel = analogRead(UVOUT); /* Armazena a leitura analógica do pino OUT */ 

  int refLevel = analogRead(REF);  /* Armazena a leitura analógica do pino EN */ 

  /* Use 3.3V como referencia no calculo de tensão */ 

  float outputVoltage = 3.3 / refLevel * uvLevel;               /* Indica a tensão de saída do sensor */ 

  float uvIntensity = map(outputVoltage, 0.99, 2.9, 0.0, 15.0); /* Intensidade raios UV */ 

 
 

  Serial.print(" Intensidade UV: "); 

  Serial.println(uvIntensity); 

} 

 
 

void Leitura_Sensor_MQ131() { 

  int sensorValue = analogRead(MQ131_input);     // Leitura do valor analógico do sensor MQ-131 

  float voltage = sensorValue * (3.3 / 4095.0);  // Conversão do valor analógico para tensão (3.3V é a tensão de referência do ESP32) 

  float rs_over_r0 = voltage / (3.3 - voltage);  // Cálculo da relação Rs/R0 

 
 

  // Cálculo da resistência do sensor (Rs) usando a relação Rs/R0 e o valor do resistor de carga 

  float sensorResistance = RLOAD * rs_over_r0; 

 
 

  // Cálculo da concentração de ozônio (O3) em ppm usando a resistência do sensor no ar limpo (RL_CLEAN_AIR) 

  float ppm_Ozonio = sensorResistance / RL_CLEAN_AIR; 

 
 

  Serial.print("Tensão (V): "); 

  Serial.print(voltage, 4);  // Exibe a tensão com 4 casas decimais 

  Serial.print(", Resistência do sensor (Rs): "); 

  Serial.print(sensorResistance, 2);  // Exibe a resistência do sensor com 2 casas decimais 

  Serial.print(", Concentração de Ozônio (ppm): "); 

  Serial.println(ppm_Ozonio, 2);  // Exibe a concentração de ozônio com 2 casas decimais 

 
 

  delay(1000);  // Aguarda 1 segundo 

} 

 
 

// Função para tocar a melodia da vírgula no buzzer 

void SomVirgula() { 

  for (int i = 0; i < sizeof(melodyVIRGULA) / sizeof(melodyVIRGULA[0]); i++) { 

    playTone(melodyVIRGULA[i], noteDurationsVIRGULA[i]); 

    delay(noteDurationsVIRGULA[i] * 1.30);  // Pausa entre as notas 

  } 

} 

 