/*************************************************************************************************
  ESP32 Web Server | Angie Recinos 23294 - Edvin Paiz 23072
  Webserver Proyecto 3
**************************************************************************************************/
//************************************************************************************************
// Librerías
//************************************************************************************************
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

//************************************************************************************************
// Variables globales
//************************************************************************************************
// SSID & Password
const char* ssid = "Edvin";  // Enter your SSID here
const char* password = "paiz moscoso";  //Enter your Password here

WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)

bool estadoGlobal = false; // false = libre, true = ocupado
bool parqueo[16] = {false}; // false = libre, true = ocupado

#define I2CSlaveAddress1 0x18
#define I2CSlaveAddress2 0x27

#define I2C_SDA 21
#define I2C_SCL 22

uint32_t lastI2C = 0;
const uint32_t I2C_PERIOD_MS = 300;

uint8_t error = 0;

//External Wire.h equivalent error Codes
typedef enum{
  I2C_ERROR_OK=0,
  I2C_ERROR_DEV,
  I2C_ERROR_ACK,
  I2C_ERROR_TIMEOUT,
  I2C_ERROR_BUS,
  I2C_ERROR_BUSY,
  I2C_ERROR_MEMORY,
  I2C_ERROR_CONTINUE,
  I2C_ERROR_NO_RETURN
}i2c_err_t;

//************************************************************************************************
// Configuración
//************************************************************************************************
void setup()
{
  Serial.begin(115200);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  // Connect to your wi-fi modem
  WiFi.begin(ssid, password);

  // Check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32 IP on serial

  server.on("/", handle_OnConnect); // Directamente desde e.g. 192.168.0.8
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);

  Wire.begin(I2C_SDA, I2C_SCL);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(50000);
  Wire.setTimeOut(50);

  i2cScanner();
}

//************************************************************************************************
// loop principal
//************************************************************************************************
void loop()
{
  server.handleClient();

  if (Serial.available()) {
    int num = Serial.parseInt();

    if (num >= 1 && num <= 16) {
      parqueo[num - 1] = !parqueo[num - 1];

      Serial.print("Parqueo ");
      Serial.print(num);
      Serial.print(" ahora: ");
      Serial.println(parqueo[num - 1] ? "OCUPADO" : "LIBRE");
    }
  }

  if (millis() - lastI2C >= I2C_PERIOD_MS)
  {
    lastI2C = millis();
    leerSensoresI2C();
  }
}

//************************************************************************************************
// Scanner de dispositivos I2C
//************************************************************************************************
void i2cScanner()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning ... ");
  nDevices = 0;
  for (address = 1; address < 127; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)

      Serial.print("0");

      Serial.println(address, HEX);
      nDevices++;
    }
    else if(error == 4)
    {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
      {
      Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
  {
    Serial.println("No I2C Devices found\n");
  }
  else
  {
    Serial.print("done\n");
  }
}

//************************************************************************************************
// Reseteo en caso de error del I2C
//************************************************************************************************
void resetI2CBus()
{
  Serial.println("Recuperando bus I2C...");

  Wire.end();
  delay(10);

  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
  delay(10);

  // Si SDA quedó atorado en bajo, pulsar SCL ayuda a liberar al esclavo
  pinMode(I2C_SCL, OUTPUT_OPEN_DRAIN);
  pinMode(I2C_SDA, INPUT_PULLUP);

  for (int i = 0; i < 9; i++)
  {
    digitalWrite(I2C_SCL, HIGH);
    delayMicroseconds(10);
    digitalWrite(I2C_SCL, LOW);
    delayMicroseconds(10);
  }

  // Generar condición STOP manual
  pinMode(I2C_SDA, OUTPUT_OPEN_DRAIN);

  digitalWrite(I2C_SDA, LOW);
  delayMicroseconds(10);

  digitalWrite(I2C_SCL, HIGH);
  delayMicroseconds(10);

  digitalWrite(I2C_SDA, HIGH);
  delayMicroseconds(10);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(50000);
  Wire.setTimeOut(50);

  delay(10);

  Serial.println("Bus I2C reiniciado");
}

//************************************************************************************************
// Handler de Inicio página
//************************************************************************************************
void handle_actualizar(){
  Serial.println("Actualizar pagina");
  server.send(200, "text/html", SendHTML());
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
}

//************************************************************************************************
// Procesador de HTML
//************************************************************************************************
String SendHTML() {

  String verde_claro = "https://raw.githubusercontent.com/angierecinos/imagenes_esp32_proy3/main/led_v_claro.png";
  String verde_oscuro = "https://raw.githubusercontent.com/angierecinos/imagenes_esp32_proy3/main/led_v_oscuro.png";

  String rojo_claro = "https://raw.githubusercontent.com/angierecinos/imagenes_esp32_proy3/main/led_r_claro.png";
  String rojo_oscuro = "https://raw.githubusercontent.com/angierecinos/imagenes_esp32_proy3/main/led_r_oscuro.png";

  String img[16];

  // Generar estado de cada parqueo
  for(int i=0; i<16; i++){
    if(parqueo[i]){ // ocupado
      img[i] = rojo_claro;
    } else { // libre
      img[i] = verde_claro;
    }
  }

  String ptr = "<!DOCTYPE html><html>";
  
  ptr += "<head>";
  ptr += "<meta charset='UTF-8'>";
  ptr += "<meta http-equiv='refresh' content='1'>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  ptr += "<style>";

  ptr += "body{ text-align:center; font-family:Arial; margin:0; }";

  ptr += ".titulo{ width:100%; height:auto; display:block; }";

  ptr += ".mapa{ position:relative; width:500px; margin:0; }";

  ptr += ".fondo{ width:100%; display:block; }";
  ptr += ".contenedor{ display:flex; align-items:flex-start; }";

  ptr += ".tabla{ margin-left:20px; border-collapse:collapse; font-size:14px; width:800px; }";

  ptr += ".tabla th, .tabla td{ border:1px solid #ccc; padding:15px; text-align:center; }";

  ptr += ".libre{ background-color: rgba(0,255,0,0.2); }";
  ptr += ".ocupado{ background-color: rgba(255,0,0,0.2); }";

  ptr += ".led{ position:absolute; width:25px; }";

  // posiciones
  ptr += ".p1{ top:20px; left:80px; }";
  ptr += ".p2{ top:20px; left:188px; }";
  ptr += ".p3{ top:20px; left:290px; }";
  ptr += ".p4{ top:20px; left:401px; }";
  ptr += ".p5{ top:340px; left:80px; }";
  ptr += ".p6{ top:340px; left:188px; }";
  ptr += ".p7{ top:340px; left:290px; }";
  ptr += ".p8{ top:340px; left:401px; }";

  ptr += "</style>";
  ptr += "</head>";

  ptr += "<body>";

  // título
  ptr += "<img class='titulo' src='https://raw.githubusercontent.com/angierecinos/imagenes_esp32_proy3/main/titulo.png'>";
  ptr += "<div class='contenedor'>";
  ptr += "<div class='mapa'>";

  // fondo
  ptr += "<img class='fondo' src='https://raw.githubusercontent.com/angierecinos/imagenes_esp32_proy3/main/parqueo222.png'>";

  // LEDs
  ptr += "<img class='led p1' src='" + img[0] + "'>";
  ptr += "<img class='led p2' src='" + img[1] + "'>";
  ptr += "<img class='led p3' src='" + img[2] + "'>";
  ptr += "<img class='led p4' src='" + img[3] + "'>";

  ptr += "<img class='led p5' src='" + img[4] + "'>";
  ptr += "<img class='led p6' src='" + img[5] + "'>";
  ptr += "<img class='led p7' src='" + img[6] + "'>";
  ptr += "<img class='led p8' src='" + img[7] + "'>";

  ptr += "</div>";
  // TABLA
  ptr += "<table class='tabla'>";
  ptr += "<tr><th># Parqueo</th><th>Estado</th></tr>";

  for(int i=0; i<8; i++){  

    if(parqueo[i]){ // ocupado
      ptr += "<tr class='ocupado'><td>" + String(i+1) + "</td><td>🚫 Ocupado</td></tr>";
    } else {
      ptr += "<tr class='libre'><td>" + String(i+1) + "</td><td>🚗 Disponible</td></tr>";
    }

  }

  ptr += "</table>";
  ptr += "</div>";
  ptr += "</body></html>";

  return ptr;
}

//************************************************************************************************
// Handler de not found
//************************************************************************************************
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

//************************************************************************************************
// Actualizar parqueos
//************************************************************************************************
void actualizarParqueosWeb(uint8_t sensor_state)
{
  for (int i = 0; i < 8; i++)
  {
    parqueo[i] = (sensor_state & (1 << i)) != 0;
  }
}

void leerSensoresI2C()
{
  uint8_t comando = 'S';

  Wire.beginTransmission(I2CSlaveAddress1);
  Wire.write(comando);
  uint8_t error1 = Wire.endTransmission(true);

  Serial.print("Nucleo 1 endTransmission: ");
  Serial.println(error1);

  if (error1 != 0)
  {
    Serial.println("Error escribiendo comando a Nucleo 1");

    if (error1 == 5)
    {
      Serial.println("Timeout I2C, recuperando bus");
      resetI2CBus();
    }

    return;
  }

  delay(2);

  uint8_t bytesReceived = Wire.requestFrom(I2CSlaveAddress1, 1, true);

  Serial.print("requestFrom Nucleo 1: ");
  Serial.println(bytesReceived);

  if (bytesReceived != 1)
  {
    Serial.println("No se recibio dato de Nucleo 1");

    resetI2CBus();
    return;
  }

  uint8_t sensor_state = Wire.read();

  Serial.print("Estado sensores: ");
  Serial.println(sensor_state, BIN);

  actualizarParqueosWeb(sensor_state);

  delay(2);

  Wire.beginTransmission(I2CSlaveAddress2);
  Wire.write(sensor_state);
  uint8_t error2 = Wire.endTransmission(true);

  Serial.print("Nucleo 2 endTransmission: ");
  Serial.println(error2);

  if (error2 != 0)
  {
    Serial.println("Error enviando a Nucleo 2");

    if (error2 == 5)
    {
      Serial.println("Timeout con Nucleo 2, recuperando bus");
      resetI2CBus();
    }
  }
}

