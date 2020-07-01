//Configuracao do Sensor de Temperatura e Umidade
#include <DHT.h>
#define DHTPIN 2          //Pino onde o DHT11 está conectado
#define DHTTYPE DHT11     //Define o tipo de Sensor como DHT11
 
DHT dht(DHTPIN, DHTTYPE);

#include <ESP8266WiFi.h >

//Configuracao da Conexao WiFi
const char* ssid = "Kramer";
const char* password = "11971923040";
int ModbusTCP_port = 502;

//Configuracao de IP Estático
IPAddress staticIP(192, 168, 100, 115); //ESP static ip
IPAddress gateway(192, 168, 100, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS

const char* deviceName = "Kramer";

//////// Required for Modbus TCP / IP /// Requerido para Modbus TCP/IP /////////
#define maxInputRegister 20
#define maxHoldingRegister 20

#define MB_FC_NONE 0
#define MB_FC_READ_REGISTERS 3 //implemented
#define MB_FC_WRITE_REGISTER 6 //implemented
#define MB_FC_WRITE_MULTIPLE_REGISTERS 16 //implemented
//
// MODBUS Error Codes
//
#define MB_EC_NONE 0
#define MB_EC_ILLEGAL_FUNCTION 1
#define MB_EC_ILLEGAL_DATA_ADDRESS 2
#define MB_EC_ILLEGAL_DATA_VALUE 3
#define MB_EC_SLAVE_DEVICE_FAILURE 4
//
// MODBUS MBAP offsets
//
#define MB_TCP_TID 0
#define MB_TCP_PID 2
#define MB_TCP_LEN 4
#define MB_TCP_UID 6
#define MB_TCP_FUNC 7
#define MB_TCP_REGISTER_START 8
#define MB_TCP_REGISTER_NUMBER 10

byte ByteArray[260];
unsigned int MBHoldingRegister[maxHoldingRegister];

//////////////////////////////////////////////////////////////////////////

WiFiServer MBServer(ModbusTCP_port);

void setup() {

 pinMode(14, OUTPUT);

 dht.begin();
 
 Serial.begin(9600);
 delay(100) ;
 WiFi.begin(ssid, password);
 delay(100) ;

 WiFi.hostname(deviceName);      // DHCP Hostname (useful for finding device for static lease)
 WiFi.config(staticIP, subnet, gateway, dns);
 
 WiFi.mode(WIFI_STA); //WiFi mode station (connect to wifi router only
  
 Serial.println(".");
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 MBServer.begin();
 Serial.println("Connected ");
 Serial.print("ESP8266 Slave Modbus TCP/IP ");
 Serial.print(WiFi.localIP());
 Serial.print(":");
 Serial.println(String(ModbusTCP_port));
 Serial.println("Modbus TCP/IP Online");
 
}


void loop() {


 // Check if a client has connected // Modbus TCP/IP
 WiFiClient client = MBServer.available();
 if (!client) {
 return;
 }
 

 boolean flagClientConnected = 0;
 byte byteFN = MB_FC_NONE;
 int Start;
 int WordDataLength;
 int ByteDataLength;
 int MessageLength;
 
 // Modbus TCP/IP
 while (client.connected()) {
 
 if(client.available())
 {
 flagClientConnected = 1;
 int i = 0;
 while(client.available())
 {
 ByteArray[i] = client.read();
 i++;
 }

 client.flush();

// ----------------- Codigo -----------------------------------

float h = dht.readHumidity();
float t = dht.readTemperature();
bool sensorStatus = 0;

if (isnan(h) || isnan(t)) 
{
Serial.println("Falha na Leitura do Sensor DHT11");
h = 0;
t = 0;
sensorStatus = 1;
//return;
}

Serial.println(h);
Serial.println(t);
Serial.println(sensorStatus);

// ------------------------------------------------------------

 ///////// Holding Register [0] A [9] = 10 Holding Registers Escritura
 ///////// Holding Register [0] A [9] = 10 Holding Registers Writing
 
 MBHoldingRegister[0] = h;
 MBHoldingRegister[1] = t;
 MBHoldingRegister[2] = sensorStatus;
 MBHoldingRegister[3] = random(0,12);
 MBHoldingRegister[4] = random(0,12);
 MBHoldingRegister[5] = random(0,12);
 MBHoldingRegister[6] = random(0,12);
 MBHoldingRegister[7] = random(0,12);
 MBHoldingRegister[8] = random(0,12);
 MBHoldingRegister[9] = random(0,12);

 


 ///////// Holding Register [10] A [19] = 10 Holding Registers Lectura
 ///// Holding Register [10] A [19] = 10 Holding Registers Reading
 
 int Temporal[10];
 
 Temporal[0] = MBHoldingRegister[10];
 Temporal[1] = MBHoldingRegister[11];
 Temporal[2] = MBHoldingRegister[12];
 Temporal[3] = MBHoldingRegister[13];
 Temporal[4] = MBHoldingRegister[14];
 Temporal[5] = MBHoldingRegister[15];
 Temporal[6] = MBHoldingRegister[16];
 Temporal[7] = MBHoldingRegister[17];
 Temporal[8] = MBHoldingRegister[18];
 Temporal[9] = MBHoldingRegister[19];

 /// Enable Output 14
 digitalWrite(14, MBHoldingRegister[14] );


 //// debug

 for (int i = 0; i < 10; i++) {

 Serial.print("[");
 Serial.print(i);
 Serial.print("] ");
 Serial.print(Temporal[i]);
 
 }
 Serial.println("");




//// end code - fin 
 

 //// rutine Modbus TCP
 byteFN = ByteArray[MB_TCP_FUNC];
 Start = word(ByteArray[MB_TCP_REGISTER_START],ByteArray[MB_TCP_REGISTER_START+1]);
 WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
 }
 
 // Handle request

 switch(byteFN) {
 case MB_FC_NONE:
 break;
 
 case MB_FC_READ_REGISTERS: // 03 Read Holding Registers
 ByteDataLength = WordDataLength * 2;
 ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
 ByteArray[8] = ByteDataLength; //Number of bytes after this one (or number of bytes of data).
 for(int i = 0; i < WordDataLength; i++)
 {
 ByteArray[ 9 + i * 2] = highByte(MBHoldingRegister[Start + i]);
 ByteArray[10 + i * 2] = lowByte(MBHoldingRegister[Start + i]);
 }
 MessageLength = ByteDataLength + 9;
 client.write((const uint8_t *)ByteArray,MessageLength);
 
 byteFN = MB_FC_NONE;
 
 break;
 
 
 case MB_FC_WRITE_REGISTER: // 06 Write Holding Register
 MBHoldingRegister[Start] = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
 ByteArray[5] = 6; //Number of bytes after this one.
 MessageLength = 12;
 client.write((const uint8_t *)ByteArray,MessageLength);
 byteFN = MB_FC_NONE;
 break;
 
 case MB_FC_WRITE_MULTIPLE_REGISTERS: //16 Write Holding Registers
 ByteDataLength = WordDataLength * 2;
 ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
 for(int i = 0; i < WordDataLength; i++)
 {
 MBHoldingRegister[Start + i] = word(ByteArray[ 13 + i * 2],ByteArray[14 + i * 2]);
 }
 MessageLength = 12;
 client.write((const uint8_t *)ByteArray,MessageLength); 
 byteFN = MB_FC_NONE;
 
 break;
 }
 }


 

}
