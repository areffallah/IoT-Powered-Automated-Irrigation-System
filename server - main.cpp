#include <Arduino.h>
#include <Wire.h>
#include <Si7020.h>
#include <AsyncAPDS9306.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <string>

Si7020 sensor;
AsyncAPDS9306 sensor2;

const APDS9306_ALS_GAIN_t again = APDS9306_ALS_GAIN_1;
const APDS9306_ALS_MEAS_RES_t atime = APDS9306_ALS_MEAS_RES_16BIT_25MS;

const char *ssid = "ESP32-Access-Point";
const char *password = "123456789";
AsyncWebServer server(80);

float Temp, RH, Lux = 0;
double PumpRun = 0;
int PumpStatus = 0;

double Activation(int PStatus)
{
  // PS is required pump status
  double out;
  if (PStatus == 1)
  {
    // Calculate how much you want the pump to run: eg. 1000
    out = 1000;
    return out; //have function return the PID output
  }
  else
  {
    out = 0;
    return out; //have function return the PID output
  }
}

String readSenStr()
{
  String WriteString = "";
  sensor.begin();
  // To change resolution of the sensor use
  // sensor.changeResolution(int i) where i=[0-3],
  sensor.changeResolution(3);
  // Measure Temperature
  Temp = sensor.getTemp();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement instead with readTemp()
  Serial.println();
  Serial.print("Temp:");
  Serial.print(Temp, 2);
  delay(10);
  // Measure RH
  RH = sensor.getRH();
  Serial.println();
  Serial.print("RH(%):");
  Serial.print(RH, 1);
  //Luminosity measurement
  unsigned long startTime;
  unsigned long duration;
  startTime = millis();
  AsyncAPDS9306Data data = sensor2.syncLuminosityMeasurement();
  delay(2000);
  duration = millis() - startTime;
  Lux = data.calculateLux();
  Serial.println("Luminosity : ");
  Serial.print(Lux, 2);
  Serial.println("lux");
  //Pump Activation: Activation(1) means always on, 1 is a signal that needs to be received by cellphone
  // Pump on time needs to be calculated in Activation function
  double PumpRun = Activation(PumpStatus);
  //PumpStatus = 10;
  WriteString = String(Temp) + "," + String(RH) + "," + String(Lux) + "," + String(PumpRun);
  return WriteString;
}

void setup()
{
  Serial.begin(9600);

  //Start Server
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  //server.on("/PumpOn", HTTP_GET, [](AsyncWebServerRequest *request) {
  //  request->send_P(200, "text/plain", "OK");
  //  PumpStatus = 1;
  //});

  server.on("/PumpOn", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", "Pump is turning On");
    PumpStatus = 1;
  });

  server.on("/PumpOff", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", "Pump is turning Off");
    PumpStatus = 0;
  });

  server.on("/Moein", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readSenStr().c_str());
  });

  delay(1000);

  server.begin();
  //End: Server
}

void loop()
{
}