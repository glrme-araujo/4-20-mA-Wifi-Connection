// Nome : Tunel anemômetria 

/* esse projeto é a criação de uma estação(Cliente) partir de um ESP32
O esp controlara a rotação do inversor de frequencia(Tunel) a partir de dados recebidos pelo servidor via TCP
O esp recebera comdandos como frases e valores numericos e fará a tarefa correspondente ao comando enviado.

O Processador 1 (loop) é dedicado a reconexão
O processador 0 é dedicado a receber os dados e administrar o buffer

 // IP SERVER 192.168.4.1
 // Ip do TUNEL 192.168.4.2
/*---------------------------------------*/


#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebSocketsClient.h>


/*-----------------VARIAVEIS-E-OUTROS---------------------*/
#define ledboard 2     // LED DO ESP
#define ledverde 25    // LED TESTE
#define BUFFERLEN 10   // Tamanho em bytes do buffer que armazena a mensagem recebida         
#define PORTA 80       // PORTA DO SERVER
#define PERIODO 1000   // Periodo de reconexao e update em ms
char mensagemTcpIn[64] = ""; //variavel global com a mensagem recebiada via TCP
char mensagemclean[64] = "";  // Limpando BUFFER apos receber msg
TickType_t taskDelay = 5 / portTICK_PERIOD_MS; // ciração do delay em ms para tasks
/*---------------------------------------*/



/*-------------PARAMENTROS WIFI--------*/       
const char* SSID = "gvento";    //Nome da rede
const char* PASS =  "anemometria";  // Senha da rede
WiFiClient Server;                  // CLIENTE NOMEADO COMO SERVER PARA FACILITAR A LEITURA
/*---------------------------------------*/


/*------------------TASKS---------------------*/
void taskTcpCode(void * parameter); // faz a comunicação via socket
/*---------------------------------------*/


/*-----------------FUNÇOES----------------------*/
void setupPins();       // inicialização das saidas digitais e do SPI
void setupWireless();   // inicialização do wireless 
void launchTasks();     // Dispara as tasks.
void checkValue();      // avalia a mensagem recebida via tcp e ajusata as saidas
void connectClient();   // Inicialização da conexão ao Server
void Clean();           // Limpa buffer
/*---------------------------------------*/


/*----------------SETUP-----------------------*/
void setup() {
  Server.setTimeout(100);  // Tempo para considerar a conexão como perdida
  Serial.begin(115200);    // Iniciando a serial
  setupPins();             // Chamando a função dos parametros dos pinos
  setupWireless();         // Chamando a função dos parametros do Wiriless 
  connectClient();         // Chamando a função de conexão ao server
  launchTasks();           // Lançando as tasks
}
/*---------------------------------------*/




/*-----------------LOOP----------------------*/
void loop() {
  if(WiFi.status() != WL_CONNECTED){
    setupWireless();
    Server.stop();
    }
 if(WiFi.status() == WL_CONNECTED){
     digitalWrite(ledboard,HIGH); 
 }
delay(100);
}
/*---------------------------------------*/




/*----------------TASKS-----------------------*/
void taskTcpCode(void * parameters) {   // LEITURA DOS DADOS RECEBIDOS DO SERVER
 while(true) {
        while(Server.connected()){  // Equanto estinver conectado
        if(Server.available()>0){   // se o servidor estiver mandando dados
        
        int i = 0;                  
        char bufferEntrada[BUFFERLEN] = "";
        while(Server.available()>0){   //enquanto o servidor estiver mandando algo
        char mensagem = Server.read(); 
          bufferEntrada[i] = mensagem;
            i++;
        }
    
        strncpy(mensagemTcpIn,bufferEntrada,i); // copiando o que foi recebido(buffer) a variavel de leitura
        checkValue();  // função que ira reconhecer a mensagem  
       
        }
        vTaskDelay(taskDelay); 
 }
 while(!Server.connected())  {               // Se não estiver conectado com o server, mandamos conectar
           connectClient();                  // Função de conexão ao server
        }
 vTaskDelay(taskDelay);
}
}
/*--------------------------------------------------*/


/*----------------FUNÇOES-----------------------*/
void setupPins(){
  pinMode(ledboard,OUTPUT);
  pinMode(ledverde,OUTPUT);
  digitalWrite(ledverde, LOW);
  digitalWrite(ledboard,LOW);
}

void setupWireless(){   // Parametros Wireless
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledboard, HIGH);
    delay(100);
    digitalWrite(ledboard, LOW);
    delay(100); 
  }
}

void launchTasks(){   // Lançandos as tasks
  xTaskCreatePinnedToCore(taskTcpCode,"TRAFEGO TCP",10000,NULL,1,NULL,0);
   delay(500);
}

void checkValue()   //Checar valor e fazer tarefa
 {
   if(strcmp(mensagemTcpIn,"ligar") ==0){
            digitalWrite(ledverde,HIGH);
            Clean();
              }  
            if(strcmp(mensagemTcpIn,"desligar") ==0){
            Clean();
              digitalWrite(ledverde,LOW);  
        }
 }

void connectClient(){ // Conexão ao server
while (!Server.connect(WiFi.gatewayIP(), PORTA)) {return;}
}

void Clean(){ // Limpabndo buffer após realizar a tarefa
strncpy(mensagemTcpIn,mensagemclean,64);
}
/*---------------------------------------*/