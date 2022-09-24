#include <Arduino.h>

//Start: HTTP Client (MotorBoard)
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
const char *ssid = "ESP32-Access-Point";
const char *password = "123456789";
//Your IP address or domain name with URL path
const char *serverNameTemp = "http://192.168.4.1/Moein";
String Moein_pass;
unsigned long previousMillis = 0;
int interval = 10000;
//End: HTTP Client (MotorBoard)

String ServerMessage;
// Temperature
char buffT[100];
// Relative Humidity
char buffRH[100];
// Lux
char buffL[100];
// Pump Status
char buffP[100];
double Temp, RH, Lux;
int PumpStatus;

#include <ESP32Encoder.h>
//Encoder and motor setup
ESP32Encoder encoder;
#define SLEEP 16  // Used to idle the motor and make sure the drive is working, no need to change
#define PMODE 27  // Used to set the drive mode for the motor controller, leave as defined below
#define EN_PWM 32 // PWM to set motor "torque_speed, etc..."
#define DIR 33    // Change the direction of motor trave

//Start: HTTP Client (MotorBoard)
String
httpGETRequest(const char *serverName)
{
  HTTPClient http;
  // Your IP address with path or Domain name with URL path
  http.begin(serverName);
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  String payload = "--";
  if (httpResponseCode > 0)
  {
    // Serial.print("HTTP Response code: ");
    // Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}
//End: HTTP Client (MotorBoard)

void setup()
{
  Serial.begin(9600);
  //Start: HTTP Client (MotorBoard)
  // put your setup code here, to run once:
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }
  //End: HTTP Client (MotorBoard)

  //Start: Presenting Orientation Value on Motor
  ESP32Encoder::useInternalWeakPullResistors = true;
  encoder.setCount(37);
  /*Create a half resolution quadrature encoder using the internal counter*/
  encoder.attachHalfQuad(25, 26);
  encoder.clearCount();
  /* setup the pins for the motor control */
  pinMode(SLEEP, OUTPUT);
  pinMode(PMODE, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite(SLEEP, HIGH);
  digitalWrite(PMODE, LOW);
  digitalWrite(DIR, LOW);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(EN_PWM, 0);
  //End: Presenting Orientation Value on Motor
}

void loop()
{
  //Start: Reading data from sensor board
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      ServerMessage = httpGETRequest(serverNameTemp);
      previousMillis = currentMillis;
      Serial.println(ServerMessage);
      //Start: Data decoding
      sscanf(ServerMessage.c_str(), "%[^,]%*c%[^,]%*c%[^,]%*c%s", buffT, buffRH, buffL, buffP);
      Temp = atof(buffT);
      RH = atof(buffRH);
      Lux = atof(buffL);
      PumpStatus = atof(buffP);
      //End: Data decoding
    }
  }
  //End: Reading data from sensor board

  if (PumpStatus > 0)
  {
    ledcWrite(0, 100);
    delay(PumpStatus);
    ledcWrite(0, 0);
  }
  else
  {
    ledcWrite(0, 0);
  }
}