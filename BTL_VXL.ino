
/*========================== Thư viện =============================*/
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "PMS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>                /* Giao tiếp SPI cho SD card */
#include <SD.h>  
#include <RTClib.h>

/*=========== Cấu hình cho các linh kiện ===========*/
/*------ PMS7003 ------*/
#define RXD2 16
#define TXD2 17
/*------ Chân CS của SD card -----*/
#define SD_CS 5
/*------- LED --------*/
#define LED_PIN  13
/*---------------- OLED --------------*/
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET  -1
#define SCREEN_ADDRESS  0x3C


/* ================= Thời gian đọc ghi ================= */
unsigned long lastRead = 0;
unsigned long lastSave = 0;

const unsigned long READ_INTERVAL = 1000;  // 1 giây
const unsigned long SAVE_INTERVAL = 5000;  // 5 giây


/* ================= Biến lưu data ================= */
float temp = 0;
float hum = 0;
float press = 0;


/* ================= LED cảnh báo ================= */
uint8_t pm25_alarm_count = 0;


/* ================= Khai báo cảm biến ================= */

/*--------- PMS ---------*/
PMS pms(Serial2);
PMS::DATA data;

/*--------- BME ---------*/
Adafruit_BME280 bme;

/*--------- RTC ---------*/
RTC_DS3231 rtc;
DateTime t;

/*--------- OLED --------*/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*========= KHAI BÁO HÀM =========*/
void read_rtc();
void read_bme();
void read_pms();
void oled();
void saveData(); // lưu dữ liệu

/*========= SETUP =========*/
void setup(){
  Wire.begin(21, 22);
  Serial.begin(115200);
  delay(500);
  /*------- PMS -------*/
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  /*------- BME -------*/
  //bool status = bme.begin(0x76);
  if(!bme.begin(0x76)){
    Serial.println("Không tìm thấy BME280");
    while(1){};
  }

  /*-------------- RTC --------------*/
  if (!rtc.begin()) {
    Serial.println("Không tìm thấy DS3231");
    while (1);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //set thời gian, chỉ dùng 1 lần

  /*-------------- OLED --------------*/
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Không tìm thấy địa chỉ OLED"));
    while(1){}
  }
  display.clearDisplay();

  /*-------------- SD Card --------------*/
  if (!SD.begin(SD_CS)) {
    Serial.println("Không tìm thấy SD Card");
    while (1);
  }

  Serial.println("SD Card OK");

  // Tạo file CSV và tiêu đề nếu chưa có
  if (!SD.exists("/data.csv")) {

    File file = SD.open("/data.csv", FILE_WRITE);

    if (file) {

      file.println("Date,Time,PM1.0,PM2.5,PM10,Temperature,Humidity,Pressure");

      file.close();

      Serial.println("Tạo file data.csv");
    }
  }
  /*-------------- LED --------------*/
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN,LOW);
}

void loop()
{
    unsigned long now = millis();

    /* ===== Đọc cảm biến mỗi 1 giây ===== */
    if (now - lastRead >= READ_INTERVAL)
    {
        lastRead = now;

        t = rtc.now();

        temp  = bme.readTemperature();
        hum   = bme.readHumidity();
        press = bme.readPressure() / 100.0;

        if (pms.readUntil(data))
        {
            read_rtc();
            read_bme();
            read_pms();

            /* ===== LED cảnh báo ===== */
            /* Nếu 5 mẫu liên tục đều lớn hơn 32ug thì LED hiện sáng*/
            if (data.PM_AE_UG_2_5 >= 32)
            {
                pm25_alarm_count++;
            }
            else
            {
                pm25_alarm_count = 0;
            }

            if (pm25_alarm_count >= 5)
            {
                digitalWrite(LED_PIN, HIGH);
            }
            else
            {
                digitalWrite(LED_PIN, LOW);
            }

            oled();
        }
    }

    /* ===== Ghi SD mỗi 5 giây ===== */
    if (now - lastSave >= SAVE_INTERVAL)
    {
        lastSave = now;
        saveData();
    }
}

/*===================CÁC HÀM ===================*/
void read_rtc(){
  Serial.print(t.day());
  Serial.print("/");

  Serial.print(t.month());
  Serial.print("/");

  Serial.print(t.year());
  Serial.print("  ");

  Serial.print(t.hour());
  Serial.print(":");

  Serial.print(t.minute());
  Serial.print(":");

  Serial.println(t.second());
}

void read_bme()
{
  Serial.print("Temp = ");
  Serial.print(temp);
  Serial.println(" °C");

  Serial.print("Hum = ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(press);
  Serial.println(" hPa");

  Serial.println();
}

void read_pms()
{
    Serial.print("PM1.0: ");
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print("PM2.5: ");
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print("PM10 : ");
    Serial.println(data.PM_AE_UG_10_0);
}

void oled()
{

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
    display.print(temp,1);
    display.print("C");

    display.print(" H:");
    display.print(hum,1);
    display.print("%");

    display.setCursor(0, 57);
    display.print("P:");
    display.print(press,1);
    display.print("hPa");

    display.display();
}

void saveData() 
{

  File file = SD.open("/data.csv", FILE_APPEND);

  if (!file) {

    Serial.println("Không mở được file");

    return;
  }

  // ===== Ghi dữ liệu CSV =====
  //RTC
  file.print(t.day());
  file.print("-");

  file.print(t.month());
  file.print("-");

  file.print(t.year());
  file.print(",");

  file.print(t.hour());
  file.print(":");

  file.print(t.minute());
  file.print(":");

  file.print(t.second());
  file.print(",");

  //PMS
  file.print(data.PM_AE_UG_1_0);
  file.print(",");

  file.print(data.PM_AE_UG_2_5);
  file.print(",");

  file.print(data.PM_AE_UG_10_0);
  file.print(",");

  //BME
  file.print(temp);
  file.print(",");

  file.print(hum);
  file.print(",");

  file.println(press);

  file.close();

  Serial.println("Đã lưu dữ liệu vào SD");
  Serial.println("------");
}

