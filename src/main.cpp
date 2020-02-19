#define BLYNK_PRINT Serial
#include <FS.h> //this needs to be first, or it all crashes and burns...

#include <Arduino.h>
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h> //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>

//needed for library
#include <DNSServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>

#include <Wire.h>        // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"

SSD1306Wire display(0x3c, D7, D6);
BlynkTimer timer;           //clean 为电压传感器引用计时器，名称为timer

const int voltagePin = A0; //电压传感器引脚
float voltage1 = 0;        //接受模拟借口的变量，类型为整数
float R1 = 30000;          //声明R1电阻值，即为：30KΩ
float R2 = 7500;           //声明R2电阻值，即为：7.5KΩ

const int LEDPin = LED_BUILTIN;
static const uint8_t pwm_A = 5;
static const uint8_t pwm_B = 4;
static const uint8_t dir_A = 0;
static const uint8_t dir_B = 2;

int motor_speed = 0;
int motor_speed_V;

const int ResetButton = D5; //设置用于自定义重置WIFI存储信息的按键引脚
int ResetButtonState = digitalRead(ResetButton);

char blynk_token[34] = "e57b6ae1609e461c9fe5f46714b7b00a";

bool shouldSaveConfig = false;

void Blink();

void STOP()
{ //刹车
  analogWrite(pwm_A, 0);
  analogWrite(pwm_B, 0);
}

void backward()
{ //后退

  analogWrite(pwm_A, motor_speed);
  analogWrite(pwm_B, motor_speed);
  digitalWrite(dir_A, HIGH);
  digitalWrite(dir_B, HIGH);
}

void forward()
{ //前进
  analogWrite(pwm_A, motor_speed);
  analogWrite(pwm_B, motor_speed);
  digitalWrite(dir_A, LOW);
  digitalWrite(dir_B, LOW);
}

void turnleft()
{ //左转
  analogWrite(pwm_A, motor_speed);
  analogWrite(pwm_B, motor_speed);
  digitalWrite(dir_A, LOW);
  digitalWrite(dir_B, HIGH);
}

void turnright()
{ //右转
  analogWrite(pwm_A, motor_speed);
  analogWrite(pwm_B, motor_speed);
  digitalWrite(dir_A, HIGH);
  digitalWrite(dir_B, LOW);
}

void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void voltageRead()
{                                                                        //电压传感器自定义函数
  int VoltageState = analogRead(voltagePin);                             //暂存A0的数值，类型为整数（0-1023）
  float Votage = (3.313 * VoltageState * (R1 + R2)) / (1024 * R2) - 0.5; //暂存A0的数值，类型为浮点
  Blynk.virtualWrite(V0, Votage);                                        //写入blynk的V0
}

BLYNK_WRITE(V8) //APP的调速杆插件,最低启动值600-1023
{
  int vel = param.asInt();
  //int motor_speed_V;
  motor_speed_V = map(vel, 0, 1023, 600, 1023);
  motor_speed = motor_speed_V;
}

BLYNK_WRITE(V1)
{
  int FowardButton = param.asInt();
  if (FowardButton == 1)
  {
    forward();
  }
  else
  {
    STOP();
  }
}

BLYNK_WRITE(V2)
{
  int BackwardButton = param.asInt();
  if (BackwardButton == 1)
  {
    backward();
  }
  else
  {
    STOP();
  }
}

BLYNK_WRITE(V3)
{
  int TurnleftButton = param.asInt();
  if (TurnleftButton == 1)
  {
    turnleft();
  }
  else
  {
    STOP();
  }
}

BLYNK_WRITE(V4)
{
  int TurnrightButton = param.asInt();
  if (TurnrightButton == 1)
  {
    turnright();
  }
  else
  {
    STOP();
  }
}

BLYNK_CONNECTED()
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Blynk server");
  display.drawString(0, 15, "is conneted...");
  display.display();
  STOP();
  Blynk.syncAll();
  Blink();
  digitalWrite(LEDPin, LOW);
}
void Blink()
{
  digitalWrite(LEDPin, HIGH);
  delay(500);
  digitalWrite(LEDPin, LOW);
  delay(500);
  digitalWrite(LEDPin, HIGH);
  delay(500);
  digitalWrite(LEDPin, LOW);
  delay(500);
  digitalWrite(LEDPin, HIGH);
  delay(500);
  digitalWrite(LEDPin, LOW);
}

void setup()
{
  Serial.begin(115200);

  // initial settings for motors off and direction forward
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Device is booting");
  display.display();

  pinMode(pwm_A, OUTPUT); // PMW A
  pinMode(pwm_B, OUTPUT); // PMW B
  pinMode(dir_A, OUTPUT); // DIR A
  pinMode(dir_B, OUTPUT); // DIR B
  pinMode(LEDPin, OUTPUT);
  pinMode(ResetButton, INPUT);
  digitalWrite(LEDPin, HIGH);

  //清空FS,保留测试用
  //SPIFFS.format();

  //从FS json中读取配置
  Serial.println("mounting FS...");

  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      //未见存在提取并读取
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // 分配一个缓冲区来存储文件的内容。
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success())
        {
          Serial.println("\nparsed json");
          //strcpy(server, json["server"]);
          //strcpy(port, json["port"]);
          strcpy(blynk_token, json["blynk_token"]);
        }
        else
        {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  //WiFiManagerParameter custom_server("server", "server", server, 40);
  //WiFiManagerParameter custom_port("port", "port", port, 6);
  WiFiManagerParameter custom_blynk_token("blynk", "序列号", blynk_token, 32);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //设置静态IP
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //在这里增加参数
  //wifiManager.addParameter(&custom_server);
  //wifiManager.addParameter(&custom_port);
  wifiManager.addParameter(&custom_blynk_token);

  /*if (digitalRead(SET_PIN) == LOW) {
    wifiManager.resetSettings();
    }*/

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  if (ResetButtonState == HIGH)
  {
    Serial.println("Getting Reset ESP Wifi-Setting.......");
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Getting Reset for ");
    display.drawString(0, 15, "ESP Wifi-Setting.......");
    display.display();
    wifiManager.resetSettings();
    delay(5000);
    Blink();
    Serial.println("Formatting FS......");
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Formatting FS......");
    display.display();
    SPIFFS.format();
    delay(5000);
    Blink();
    Serial.println("Done,Reboot.");
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Done,Reboot.");
    display.display();
    Blink();
    ESP.reset();
  }

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Blynk_WIFI_Car_MK.II.V", ""))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  //if you get here you have connected to the WiFi
  Serial.println("WIFI is connected");

  //read updated parameters
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig)
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  ArduinoOTA.setHostname("Blynk_WIFI_Car_MK.II.V");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Blynk.config(blynk_token, "www.chrisxs.com", 8080);
  timer.setInterval(1000L, voltageRead);
}

void loop()
{
  int VoltageState = analogRead(voltagePin);                             //暂存A0的数值，类型为整数（0-1023）
  float Votage = (3.313 * VoltageState * (R1 + R2)) / (1024 * R2) - 0.5; //暂存A0的数值，类型为浮点
  float Percent = Votage;
  //Percent = map(Percent,0, 8.4, 0, 100);
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "IP: " + String(WiFi.localIP().toString()));
  display.drawString(0, 10, "MAC: " + String(WiFi.macAddress()));
  display.drawString(0, 20, "SSID: " + String(WiFi.SSID()));
  display.drawString(0, 30, "RSSI: " + String(WiFi.RSSI()) + " dB");
  display.drawString(0, 40, "Votage: " + String(Votage, 2) + " V");
  display.drawString(0, 50, " Connection Activated .");
  display.display();
  Blynk.run();
  timer.run();
  ArduinoOTA.handle();
  Blynk.virtualWrite(V5, "IP地址:", WiFi.localIP().toString());
  Blynk.virtualWrite(V6, "MAC地址:", WiFi.macAddress());
  Blynk.virtualWrite(V7, "SSID:", WiFi.SSID(), "   RSSI:", WiFi.RSSI(), " dB");
}