/*
Fresh Air Intake System based on ESP8266
by Bitosaur
Distributed under MIT License 2019.
Absolutely no Warrantees. Use at your own risk.
The software is provided with all its faults. No claim is made for suitability in any particular situation or fitness for a particular purpose.

Problem: When outside weather condition is such that the HVAC system will benefit from importing air from outside to 
suppliment its heating or cooling operation, a fresh air intake valve actuator will be opened while the HVAC fan is on so outside air can be drawn in. 
The intake valve actuator must be closed at other times so as to not lose indoor air or suck in outdoor air that is not going to aid the HVAC syste.
The project assumes:
1) A fresh air intake valve that is "normally closed".
2) The existance of a home automation system, in this case a MiCasaVera which communicates with indoor thermostat for sensor reading as well as for 
calling for heat, cooling and fan.
3) A weather.com API account to read outside weather conditions from close proximity of the HVAC location.
4) An ESP8266 module that commnucates with Wifi Network.
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

struct CurrentWeather {
int IndoorTemp;
int IndoorHumidity;
int TargetTemp;
boolean FanStatus;
int OutdoorTemp;
int OutdoorHeatIndex;
int OutdoorHumidity;
};

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 6; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  
  // Initialize Control Pins
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an 
  pinMode(2, OUTPUT);               // Initialize GPIO2
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  digitalWrite(2, 0);               // Turn Relay Off

  
  // Initialize WIFI
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("your_wifi_ssid", "your_wifi_password");  // USE YOR WIFI SSID AND PASSWORD

}

void loop() {
  CurrentWeather cW;
  cW=getWeather();
  Serial.println("Indoor Conditions:");
  Serial.printf("Current Indoor Temparature is %dF\n",cW.IndoorTemp);   
  Serial.printf("Current Target Temparature is %dF\n",cW.TargetTemp); 
  Serial.printf("Fan Status:%s\n",cW.FanStatus ? "On" : "Off");
  
  Serial.println("Outdoor Conditions:");
  Serial.printf("Current Outdoor Temparature is %dF\n",cW.OutdoorTemp); 
  Serial.printf("Current Outdoor Humidity is %d%%\n",cW.OutdoorHumidity); 
  Serial.printf("Current Outdoor Heat Index is %d\n",cW.OutdoorHeatIndex); 
  delay(100000);
}

CurrentWeather getWeather(){

  CurrentWeather cW;
  
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    int pos;
    
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://YOUR_LAN_IPADDRESS_OF_IOT_HUB_OR_THERMOSTAT:PortNumber/data_request?id=status&output_format=xml&DeviceNum=24")) {  // CHANGE THIS API LINK APPROPRIATELY TO GET A READING OF THE DEVICE


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
          Serial.printf("Length of payload: %d\n",payload.length());
          
          // Get current temparature
          pos = payload.indexOf("CurrentTemperature");
          cW.IndoorTemp = payload.substring(pos+27,pos+29).toInt(); 

           // Get fan status
          pos = payload.indexOf("FanStatus");
          if(payload.substring(pos+18,pos+20).equals("On")){
            cW.FanStatus=true;
            }
            else{
              cW.FanStatus=false;
              };            

           //Get target temparature          
          pos = payload.indexOf("ModeStatus");
          
          if(payload.substring(pos+19,pos+25).equals("CoolOn")){
          //Serial.printf("ModeStatus:CoolOn\n"); 
          pos = payload.indexOf("TemperatureSetpoint1_Cool");
          cW.TargetTemp = payload.substring(pos+61,pos+63).toInt();            
            }
            else if(payload.substring(pos+19,pos+25).equals("HeatOn")){
            //Serial.printf("ModeStatus:HeatOn\n"); 
            pos = payload.indexOf("TemperatureSetpoint1_Heat");
            cW.TargetTemp = payload.substring(pos+61,pos+63).toInt();                 
              }
            else{
            Serial.printf("ModeStatus:Not coded yet\n");  
              };
          
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    };

    //------ END OF FIRST HTTP -------------

  
    std::unique_ptr<BearSSL::WiFiClientSecure> Sclient(new BearSSL::WiFiClientSecure);

    //Sclient->setFingerprint(fingerprint);
    Sclient->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*Sclient, "https://api.weather.com/v2/pws/observations/current?stationId=KCTSTAMF61&format=xml&units=e&apiKey=USE_YOUR_API_KEY")) {  // You need an API key from weather.com

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);

          pos = payload.indexOf("<temp>");
          cW.OutdoorTemp = payload.substring(pos+6,pos+8).toInt();

          pos = payload.indexOf("<humidity>");
          cW.OutdoorHumidity = payload.substring(pos+10,pos+12).toInt();

          pos = payload.indexOf("<heatIndex>");
          cW.OutdoorHeatIndex = payload.substring(pos+11,pos+13).toInt();
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }

    // -- END OF SECOND HTTP ------------
      
    }
    Serial.flush();
  return cW;
}
