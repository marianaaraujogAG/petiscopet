#include <string.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "RTClib.h"

// variaveis de conexão wifi
const char* ssid = "Indisponivel";
const char* password = "naotemsenha";

// variaveis de conexão MQTT
const char* mqtt_server = "test.mosquitto.org";//"broker.mqtt-dashboard.com";
int BROKER_PORT = 1883;
#define ID_MQTT  "Alimentador_Mariana_01" 
#define TOPIC_CMD  "AlimentadorMQTT_MARI_CMD" 
#define TOPIC_STAT  "AlimentadorMQTT_MARI_STAT" 
#define Motor 12

//cria os objetos de conexão
WiFiClient wifiClient;
PubSubClient client(wifiClient);

RTC_DS1307 rtc; //Objeto rtc da classe DS1307

//variaveis de uso geral
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int hora[4], minuto[4], semana[7];//variaveis globais do despertador
int xsemana, xdia;//variaveis globais do despertador
int PinTrigger = 12; // Pino usado para disparar os pulsos do sensor
int PinEcho = 14; // pino usado para ler a saida do sensor
const float VelocidadeSom_mpors = 340; // em metros por segundo
const float VelocidadeSom_mporus = 0.000340; // em metros por microsegundo



//FUNÇÃO que verifica o estado das conexões e se tiver caido reestabelece
void mantemConexoes() {
    if (!client.connected()) {
       conectaMQTT(); 
    }
    if (WiFi.status() != WL_CONNECTED) {
     conectaWiFi();
  } 
}

//FUNÇÃO que faz a conexão do wifi
void conectaWiFi() {
  delay(10);  
  //prints para depuração serial
  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(ssid);
  //executa o método de conexão
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { //aguarda conexão
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  //prints para depuração serial
  Serial.println();
  Serial.print("Conectado com sucesso, na rede: ");
  Serial.print(ssid);  
  Serial.print(" IP obtido: ");
  Serial.println(WiFi.localIP());
}

//FUNÇÃO que faz a conexão do host MQTT
void conectaMQTT() {
  while (!client.connected()) { //executa método de conexão mqtt 
    Serial.print("Conectando no MQTT...");
    if (client.connect(ID_MQTT)) {//se a conexão tiver sido estabelecida 
      Serial.println("conectado");
      client.subscribe(TOPIC_CMD); //faz a subscrição do tópico onde será escrito os comandos
    } else {
      //prints para depuração serial
      Serial.print("Falhou, erro=");
      Serial.print(client.state());
      Serial.println(" tentar novamente em 5 seg");
      delay(5000);
    }
  }
}

//FUNÇÃO que recebe o pacote, decodifica o payload e monta o comando
void recebePacote(char* topic,  byte* payload, unsigned int length)
{
  //declarações
  String msg;
  char *input ;
  //prints para depuração serial
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  
  //Converte PAYLOAD
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
       char c = (char)payload[i];
       msg += c;
    } Serial.println();//pula a linha do final

  //Separa comando
  if(msg == "Handshake") {//se for apenas um handshake devolve a conexão
    client.publish(TOPIC_STAT, "HandshakeBack");
    MostraNivel(CalculaNivel());
    MostraDespertador();
    Serial.println("HandshakeBack");
  }
  else {//caso seja um comando complexo separa o comando da informa 
    int i= msg.indexOf('_');

    if ( msg.substring(0,i) == "Motor")
      comandaMotor(msg.substring(i+1));
    else if ( msg.substring(0,i) == "Despertador"){
      configuraDespertador(msg.substring(i+1));
      MostraDespertador();
    }
    else if ( msg.substring(0,i) == "Nivel"){
      MostraNivel(CalculaNivel());
    }
    }  
}

void MostraNivel(int nivel )
{
  String msg= "Nivel_";
  msg  += nivel; 
  int a=msg.length();
  char payload[a];
  msg.toCharArray(payload,a);
  client.publish(TOPIC_STAT, payload);
  Serial.print("o dispenser está com o nível : ");
  Serial.print(msg);
  Serial.println(" % ");
}

//FUNÇÃO QUE LIGA O MOTOR
void comandaMotor(String comando)
{
  //executa o comando
  if (comando == ("liga")) {
    digitalWrite(BUILTIN_LED, LOW);
    digitalWrite(Motor,HIGH );
  } 
  else if (comando == "desliga"){
      digitalWrite(BUILTIN_LED, HIGH);
      digitalWrite(Motor, LOW);
  }
  //envia feedback
  if (digitalRead(Motor)){               //se led ligado(estado low)
    client.publish(TOPIC_STAT, "Motor_ligado");      //publica no tópico de status
    Serial.println("STATUS LED ligado. Payload enviado.");//prints para depuração serial
  }
  else{
    client.publish(TOPIC_STAT, "Motor_desligado");
    Serial.println("STATUS LED desligado. Payload enviado.");
  }  
}

void MostraDespertador()
{
  String msg= "Despertador_";
  msg  += "1234567";//esse é referente aos dias da semana (possibilidade de deixar variavel
  for(int i=0; i<4;i++)
  {  
    msg += "&";
    msg += hora[i];
    msg += "&";
    msg += minuto[i];
  }
  
  int a=msg.length();
  char payload[a];
  msg.toCharArray(payload,a);
  client.publish(TOPIC_STAT, payload);
  Serial.print("Mensagem enviada: ");
  Serial.print(msg);
}

//FUNÇÃO que configura a data e horario que o alimentador liga
void configuraDespertador(String buff)
{
  Serial.println("Depertador configurado para os dias da semana: ");
  //encontra os dias da semana de alimentar
  xsemana =buff.indexOf('&'); 
  for(int i =0; i<xsemana;i++){
       semana[i]=buff.substring(i,i+1).toInt();//popula o array de dias da semana
        Serial.print(semana[i]);
        Serial.print(" ");
       }

  Serial.print("às ");
  // encontra as horas de alimentar 
  int oldCaract = buff.indexOf('&');
  int i=1;
  while(oldCaract <  buff.lastIndexOf('&'))
  {
    //popula o array das horas
    int newCaract =buff.indexOf('&',oldCaract+i);
    hora[i-1]=buff.substring(oldCaract+1,newCaract).toInt(); 
    //popula o array dos minutos    
    oldCaract = newCaract;
    if(oldCaract <  buff.lastIndexOf('&'))
      newCaract =buff.indexOf('&',oldCaract+i);   
    else
      newCaract = buff.length();      
    minuto[i-1]=buff.substring(oldCaract+1,newCaract).toInt();
    oldCaract = newCaract;
    i++;  
  Serial.print(hora[i-1]);  
  Serial.print(":");
  Serial.print(minuto[i-1]);
  Serial.print(" , "); 
  }
  xdia = i;
}

//FUNÇÃO que confere se está na hora de ligar o alimentador
void ConfereDespertador()
{
  DateTime t = rtc.now();                             // Faz a leitura de dados de data e hora
  for (int i=0;i<xsemana;i++)
  {
    if(t.dayOfTheWeek()== semana[i])
    {
      for (int j=0;j<xdia;j++)
      {
        if(t.hour() == hora[j] && t.minute() == minuto[j] && t.second() < 1)
        {
          comandaMotor("liga");
          Serial.println("Motor ligou"); 
          delay(500);
          comandaMotor("desliga");
        }
      }
    }
  }
}

// Funçao para enviar o pulso de trigger
void DisparaPulsoUltrassonico(){
  // Para fazer o HC-SR04 enviar um pulso ultrassonico, nos temos
  // que enviar para o pino de trigger um sinal de nivel alto
  // com pelo menos 10us de duraçao
  digitalWrite(PinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(PinTrigger, LOW);
  }
  
// Função para calcular a distancia em metros
int CalculaNivel(){
  float TempoEcho = 0, dist;//variavel que mede o tempo de eco
  for(int i=0;i<=3;i++)//calcula a média de 'i' medições para evitar oscilações
  {
    DisparaPulsoUltrassonico();// Mede o tempo de duração do sinal no pino de leitura(us)  
    TempoEcho = TempoEcho + pulseIn(PinEcho, HIGH);  
    delay(20);
  } 
  dist=((TempoEcho/3)*VelocidadeSom_mporus)/2*100;//distancia em centímetros
  return((15.8-dist)/15)*100;//retorna o percentual que o nivel está cheio
  //return(dist);
}

//Configura ESP
void setup() {
  pinMode(PinTrigger, OUTPUT);// Configura pino de Trigger como saída 
  digitalWrite(PinTrigger, LOW);//e inicializa com nível baixo
  pinMode(PinEcho, INPUT); // configura pino ECHO como entrada
  pinMode(BUILTIN_LED, OUTPUT); 
  pinMode(Motor, OUTPUT); 
  Serial.begin(115200);
  conectaWiFi();
  client.setServer(mqtt_server, BROKER_PORT);
  client.setCallback(recebePacote);
  if (! rtc.begin()) {                         //Se o RTC nao for inicializado, faz
    Serial.println("RTC NAO INICIALIZADO");    //Imprime o texto
    while (1);                                 //Trava o programa
  }  
  if (!rtc.isrunning()) {
    Serial.println("RTC parado, vou ajustar com a hora da compilacao...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }  
}

void loop() {
  mantemConexoes();  
  ConfereDespertador();
  client.loop();
}
