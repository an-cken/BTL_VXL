#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>

void read_bme();
void lcd_setup();
void lcd_bme();

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(0x27,16,2);


void setup(){
  Wire.begin(21, 22);
  Serial.begin(115200);

  lcd_setup();
  
  bool status = bme.begin(0x76);
  if(!status){
    Serial.println("Không tìm thấy BME280");
    while(1){};
  }
}

void loop(){
  read_bme();
  lcd_bme();
  delay(2000);
}
void lcd_setup(){
  lcd.init();
  lcd.backlight();
}

void lcd_bme(){
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(bme.readTemperature());

  
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(bme.readHumidity());
}


void read_bme(){
  Serial.print("Temp = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure()/100);
  Serial.println(" hPa");

  Serial.print("Hum = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();

}