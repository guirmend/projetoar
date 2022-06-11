//Bibliotecas a serem incluidas
#include <OneWire.h>
#include <DallasTemperature.h>
/*#include <Wire.h>         // Comunicação I2C*/
#include <WiFi.h>         // Biblioteca para utilizar o Wi-Fi
#include <PubSubClient.h>           // Biblioteca do MQTT

// Variavéis para o sensor de Corrente Elétrica.

int pino_sensor = 35;
int menor_valor;
int valor_lido;
int menor_valor_acumulado = 0;
int ZERO_SENSOR = 0;
float corrente_pico;
float corrente_eficaz;
double maior_valor = 0;
double corrente_valor = 0;

// Definição do sensor DS18B20
#define DS18B20      15 // OK

/***INSTANCIANDO OBJETOS***********************************************************************************************************************************/
// Sensor de Temperatura DS18B20

//Instacia o Objeto oneWire e Seta o pino do Sensor para iniciar as leituras
OneWire oneWire(DS18B20);

//Repassa as referencias do oneWire para o Sensor Dallas (DS18B20)
DallasTemperature Sensor(&oneWire);

// Variavel para Armazenar os dados de Leitura
float leitura;


//Definição dos topicos
#define TOPICO_PUBLISH_TEMPERATURA   "topico_sensor_temperatura"
#define TOPICO_PUBLISH_CORRENTE       "topico_sensor_corrente" 



//Dados para conectar no Wi-Fi
const char* ssid = "Rafael";  //Nome do Wi-Fi
const char* password = "17112006"; //Senha do Wi-Fi

//Dados do MQTT Broker
const char* mqtt_server = "grmprojetos.duckdns.org"; //Localização do MQTT Broker
const char* mqtt_username = "admin"; //Usuario do Broker
const char* mqtt_password = "guiramos07"; //Usuario do Broker
const int mqtt_port = 1883; //Porta de acesso do Broker

WiFiClient espClient;
PubSubClient client(espClient);

void init_wifi() { //Função para inicializar o Wi-Fi
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  reconnect_wifi();
}

void init_mqtt(){ //Função para inicializar o MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void reconnect_wifi(){ //Função para conectar o Wi-Fi
  if (WiFi.status() == WL_CONNECTED)
        return;
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {//Loop que mostra a tentativa de se conectar no Wi-Fi
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
}

void reconnect_mqtt() { //Função para conectar o mqtt
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");                         // Attempt to connect
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}

void verificaConexao(){
   if (!client.connected()) {
    reconnect_mqtt();
  }
  reconnect_wifi();
}


void setup() {
  Serial.begin(9600); //Inicializa o serial
  init_wifi(); //Inicializa o Wi_Fi
  init_mqtt(); //Inicializa o MQTT
  
  // Inicia o Sensor
  Sensor.begin();

  pinMode(pino_sensor, INPUT);
  delay(3000);
  //Fazer o AUTO-ZERO do Sensor
  Serial.println("Fazendo o Auto ZERO do Sensor...");

  menor_valor = 4095;

  for(int i = 0; i < 10000; i++){
    valor_lido = analogRead(pino_sensor);
    if(valor_lido < menor_valor){
      menor_valor = valor_lido;
    }
    delayMicroseconds(1);
  }
  ZERO_SENSOR = menor_valor;
  Serial.print("Zero do Sensor: ");
  Serial.println(ZERO_SENSOR);
  delay(3000);
}

void loop() {
  //Variaveis locais
  char temperatura_str[10] = {0};
  //double temperatura = Sensor.requestTemperatures();
  char corrente_str[10]    = {0};

  // Leitura do Sensor  DS18B20  //////////////
  Sensor.requestTemperatures();

  // Armazerna na variavel o valor da Leitura
  leitura          = Sensor.getTempCByIndex(0);


 // Imprime na Tela a Leitura
  Serial.print(leitura);
  Serial.println("ºC"); 
  Serial.println("--------------------------------------"); 


  // ----------------------------------------------------
  // Zerar valores
  menor_valor = 4095;

  for(int i = 0; i < 1600; i++){
    valor_lido = analogRead(pino_sensor);
    if(valor_lido < menor_valor){
      menor_valor = valor_lido;
    }
    delayMicroseconds(10);
  }


  Serial.print("Menor Valor: ");
  Serial.println(menor_valor);


  //Transformar em corrente de pico
  corrente_pico = ZERO_SENSOR - menor_valor;
  corrente_pico = corrente_pico*0.805;
  corrente_pico = corrente_pico/185;


  Serial.print("Corrente de Pico: ");
  Serial.print(corrente_pico);
  Serial.print(" A");
  Serial.print(" --- ");
  Serial.print(corrente_pico*1000);
  Serial.println(" mA");


  //Converter para corrente eficaz
  corrente_eficaz = corrente_pico/1.4;
  Serial.print("Corrente Eficaz: ");
  Serial.print(corrente_eficaz);
  Serial.print(" A");
  Serial.print(" --- ");
  Serial.print(corrente_eficaz*1000);
  Serial.println(" mA");

  delay(5000);
  // ----------------------------------------------------
  
  verificaConexao();
  client.loop();

dtostrf(Sensor.getTempCByIndex(0), 2, 2, temperatura_str); //temperatura - leitura para o dashboard
client.publish("TOPICO_PUBLISH_TEMPERATURA", temperatura_str); //publish para do mqtt para o dashboard

dtostrf(corrente_eficaz, 1, 2, corrente_str);
client.publish("TOPICO_PUBLISH_CORRENTE", corrente_str);

  delay(2000);
  } 
