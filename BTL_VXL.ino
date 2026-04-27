#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "PMS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RXD2 16
#define TXD2 17

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET  -1
#define SCREEN_ADDRESS  0x3C

PMS pms(Serial2);
PMS::DATA data;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void read_bme();
void read_pms();
void oled();

Adafruit_BME280 bme;

void setup(){
  Wire.begin(21, 22);
  Serial.begin(115200);
  delay(500);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //bool status = bme.begin(0x76);
  if(!bme.begin(0x76)){
    Serial.println("Không tìm thấy BME280");
    while(1){};
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Không tìm thấy địa chỉ OLED"));
    while(1){}
  }
  display.clearDisplay();
}

void loop(){
  read_bme();
  read_pms();
  oled();
  //delay(2000);
}


void oled(){
  if (pms.readUntil(data)) {

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

  // dòng 1
    display.setCursor(0, 0);
    display.print("PM1.0: ");
    display.print(data.PM_AE_UG_1_0);
    display.println(" ug/m3");

  // dòng 2
    display.setCursor(0, 16);
    display.print("PM2.5: ");
    display.print(data.PM_AE_UG_2_5);
    display.println(" ug/m3");

    // dòng 3
    display.setCursor(0, 32);
    display.print("PM10 : ");
    display.print(data.PM_AE_UG_10_0);
    display.println(" ug/m3");

    //Đường thẳng ngăn cách
    display.println();
    display.drawLine(0, 42, 128, 42, SSD1306_WHITE);
    display.println();

    //Dữ liệu BME280
    display.setCursor(0, 48);
    display.print("T:");
    display.print(bme.readTemperature(),1);
    display.print("C");

    display.print(" H:");
    display.print(bme.readHumidity(),1);
    display.print("%");

    display.setCursor(0, 57);
    display.print("P:");
    display.print(bme.readPressure() / 100.0,1);
    display.print("hPa");

    display.display();

  delay(2000);
  }
  else{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Không có dữ liệu!!!");
  }
}


void read_bme(){
  Serial.print("Temp = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Hum = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure()/100);
  Serial.println(" hPa");

  Serial.println();
}

void read_pms(){
  if (pms.readUntil(data)) {

    Serial.print("PM1.0: ");
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print("PM2.5: ");
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print("PM10 : ");
    Serial.println(data.PM_AE_UG_10_0);

    Serial.println("------");

  } else {
    Serial.println("No data");
  }
}
