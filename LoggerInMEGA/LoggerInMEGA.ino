//OBS: Se desligar os fios do MISO e do CS do Módulo SDcard, não acontece nada, isto é, é impossível de identificar o erro visualmente.
//OBS: D -> A4   C -> A5

#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>       // Bibl. da Memória EEPROM
#include <Wire.h>         // Bibl. do RTC
#include "RTClib.h"

#include <NewPing.h>      // Bibl. para o Sensor Ultrassônico

RTC_DS1307 rtc;

const int chipSelect = 53;        // Pino conectado ao SC do Módulo SD.
const int redLed = 11;             // SDLed de Inicialização.
const int greenLed = 12;           // SDLed de Leitura do Arquivo.
const int pinButton = 13;          // pino do botão.
const int pinSonar_TRIGGER = 5;   // Pino Trig do sensor ultrassônico.
const int pinSonar_ECHO = 6;      // Pino Echo do sensor ultrassônico.
int i = 0;
//const int quantAnalogPin = 3;   // Quantidade de Pinos analógicos q irão receber os dados coletados pelos sensores.
const int posFile = 0;            // Váriável para guardar o endereço da Memória EEPROM à ser o número do último arquivo criado.
String fileName = String(EEPROM[posFile]) + "_BMB.txt";       // Variável que vai receber o nome do aquivo a ser salvo os dados.
String dataString;
String DATE;
long beginRunTime = millis();

//PINO DOS SENSORES
const int sonar_Echo = 6;
const int sonar_Trig = 5;

//MÉTODO PARA LER O CARTÃO SD:
void readSD() {
  while (true) {
    if (!SD.begin(chipSelect)) {
      Serial.println("O Cartão está ausente ou com defeito!");
      digitalWrite(redLed, HIGH);       // Acende o LED Vermelho
      digitalWrite(greenLed, LOW);      // Apaga o LED Verde
    } else break;
  }
}

//MÉTODO PARA CRIAR UM ARQUIVO NO CARTÃO SD APENAS QUANDO O BUTÃO FOR PRESSIONADO:
void createFile() {
  long beginTime = 0;
  long finalTime = 0;
  //EEPROM[posFile] = 0;                                  // Zera o Número dos Arquivos, apenas remova os cometários quando quiser zera-los.
  if (digitalRead(pinButton)) {                           // Isto é, se o botão for pressinado, ele executa:
    beginTime = millis();
    while (finalTime - beginTime < 2500) {
      blinkTwoLed();
      finalTime = millis();
    }
    EEPROM[posFile]++;                                    // Gera um novo Número para ser adicionado ao nome do arquivo date.
    fileName = String(EEPROM[posFile]) + "_BMB.txt";      // Atribui à variável o nome do arquivo a ser gerado.
    i = 0;                                                  // Para que possa escrever a data do arquivo quando executar o comando saveDatesSD();
  }
}

//MÉTODO PARA MONTAGEM DOS DADOS EM UMA VARIÁVEL:
void getDates() {
  dataString = ""; //Variável para montagem dos dados registrados

  rtcTime();
  

  //---------- Sensor de distância ultrassônico ----------//
  int maxDistance = 200;     //Distância máxima que o sensor pode alcançar.
  int distByte;
  int distCm;
  long distTime;

  NewPing sonar(pinSonar_TRIGGER, pinSonar_ECHO, maxDistance); //NewPing objeto('pino do Trig', 'pino do Echo', 'Distância máxima')
  delay(50);
  distByte = sonar.ping();
  distCm = sonar.ping_cm();
  distTime = millis() - beginRunTime;
  dataString += String(distByte) + ",";
  dataString += String(distCm) + ",";
  dataString += String(distTime);

  Serial.print("Ping: ");
  Serial.print(distByte);
  Serial.println("bytes");

  Serial.print("Ping: ");
  Serial.print(distCm);
  Serial.println("cm");
  //------------------------------------------------------//

  //Ler os sensores alocados ao arduino e converte-os em uma String:
  /*for(int analogPin = 0; analogPin < quantAnalogPin; analogPin++){
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if(analogPin < (quantAnalogPin - 1)){
      dataString += ",";
    }
    }*/
}

//MÉTODO PARA SALVAR OS DADOS AQUISITADOS NO CARTÃO SD:
void saveDatesSD() {
  //Abrir o arquivo. Lembrando que apenas um arquivo pode ser aberto de cada vez,
  //então você precisa fechar o que está em uso antes de abrir um outro.
  File dataFile = SD.open(fileName, FILE_WRITE); //SD.open("NOME_DO_ARQUIVO", OQUE DESEJAS FAZER COM ELE)

  //Se o arquivo se encontra disponível, então escreva nele e acende o LED:
  if (dataFile && EEPROM[posFile] <= 254) {
    if (i == 0) {
      dataFile.print("DATA DO ARQUIVO:" );              //APENAS ESCREVE A DATA ATUAL NO ARQUIVO UMA SÓ VEZ.
      dataFile.println(DATE);
      i = 1;
    }
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);         // Também printa na porta serial
    digitalWrite(greenLed, HIGH);       // Acende o LED Verde
    digitalWrite(redLed, LOW);          // Apaga o LED Vermelho
  } else {
    //Isto é, se o aquivo não abrir, mostra uma mensagem de ERRO e pisca o led:
    Serial.print("ERRO EM ABRIR O ARQUIVO "); Serial.println(fileName);
    digitalWrite(greenLed, LOW);
    if (EEPROM[posFile] > 254){
      while(true) {
        blinkOneLed(greenLed); // Piscar o LED Verde
      }
    }
  }
}

//MÉTODO QUE TRABALHA COM O TEMPO E DATA (MÓDULO RTC):
void rtcTime() {
  while (true) {
    if (rtc.begin() && !rtc.isrunning()) {         //Se o RTC iniciar e não estiver execultando, ele pisca os LEDs
      Serial.println("Módulo RTC não encontrado!");
      blinkOneLed(redLed);
      blinkOneLed(greenLed);
    } else break;
  }

  //rtc.adjust(DateTime(2020, 03, 10, 15, 38, 0));    //Ajustar data e hora
  DateTime nowTime = rtc.now();

  DATE = String(nowTime.day()) + "/" + String(nowTime.month()) + "/" + String(nowTime.year());
  String HOUR = String(nowTime.hour());
  String MINUTE = String(nowTime.minute());
  String SECOND = String(nowTime.second());
  dataString += HOUR + ":" + MINUTE + ":" + SECOND + ",";
  Serial.println(dataString);
}

//MÉTODO PARA PISCAR UM LED, TENDO COMO PARÂMETRO O PINO DO LED A SER PISCADO:
void blinkOneLed(int pinLed) {
  digitalWrite(pinLed, HIGH);
  delay(200);
  digitalWrite(pinLed, LOW);
  delay(200);
}

void blinkTwoLed() {
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, HIGH);
  delay(100);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
  delay(100);
}

void setup() {
  
  //----INICIALIZANDO A PINAGEM----//
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(pinButton, INPUT);
  Serial.begin(9600);
  //-------------------------------//

  Serial.print("Inicializando SD Card...");
  readSD();
  Serial.println("SD card Inicializado!");
  
}

void loop() {

  readSD();         //Ler o cartão SD.
  createFile();     //Cria um novo arquivo para salvar os dados caso o botão estiver presionado.
  getDates();       //Pega a váriável que guarda os dados obtidos pelos sensores.
  saveDatesSD();    //Salva os dados no cartão SD.

}
