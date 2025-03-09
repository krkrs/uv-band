#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "LowPower.h"
#define I2C_ADDRESS 0x3C
#define rtc 0x68
#define uv A0
#define buttonInterrupt1 2
#define buttonInterrupt2 3
#define AltitudeDef 130
#define moto 6
#define AT 7 //Stan wysoki włącza komendy AT
#define test 5
#define rxPin 8
#define txPin 9
#define SDCard 10

SSD1306AsciiAvrI2c oled;
Adafruit_BME280 bme;
SoftwareSerial mySerial(rxPin, txPin);
File myFile;

static uint8_t convertValueIN(uint8_t value);
static uint8_t convertValueOUT(uint8_t value);

typedef struct timeParameters
{
  uint8_t ss ;
  uint8_t mm ;
  uint8_t hh ;
  uint8_t dy ;
  uint8_t d ;
  uint8_t m ;
  uint8_t y ;
};

timeParameters foo;

void readTime(timeParameters *timeVals)
{
  Wire.beginTransmission(rtc);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(rtc, (byte) sizeof(timeParameters));

  timeVals->ss = convertValueIN(Wire.read());
  timeVals->mm = convertValueIN(Wire.read());
  timeVals->hh = convertValueIN(Wire.read());
  Wire.read();
  timeVals->d = convertValueIN(Wire.read());
  timeVals->m = convertValueIN(Wire.read());
  timeVals->y = convertValueIN(Wire.read());
  sleep(15);
}

/*void setTime(timeParameters *timeVals)
  {

   //  From the datasheet
   //  0 - seconds
   //  1 - minutes
   //  2 - hours
   //  3 - day
   //  4 - date
   //  5 - month
   //  6 - year

  Wire.beginTransmission(rtc) ;
  Wire.write(0x00) ;

  Wire.write(convertValueOUT(timeVals->ss)) ;
  Wire.write(convertValueOUT(timeVals->mm)) ;
  Wire.write(convertValueOUT(timeVals->hh)) ;
  Wire.write(0) ;
  Wire.write(convertValueOUT(timeVals->d)) ;
  Wire.write(convertValueOUT(timeVals->m)) ;
  Wire.write(convertValueOUT(timeVals->y)) ;
  Wire.endTransmission() ;
  sleep(15) ;

  }*/

static uint8_t convertValueIN(uint8_t value)
{
  uint8_t convertedVal = value - 6 * (value >> 4);
  return convertedVal ;
}

static uint8_t convertValueOUT(uint8_t value)
{
  uint8_t convertedVal = value + 6 * (value / 10);
  return convertedVal ;
}

byte timeOutV, off, file , screen;
byte buttonOneV, buttonTwoV, save, skin = 0;
float pressure;
String fileName;
char str[11] = {0};
unsigned long counter = 0;
float averageUVIndex = 0;
//float uvi[60];

void buttonOne()
{
  buttonOneV = 1;
  sleep(500);
}

void buttonTwo()
{
  buttonTwoV = 1;
  sleep(500);
}

void setup() {
  pinMode(buttonInterrupt1, INPUT_PULLUP);
  pinMode(buttonInterrupt2, INPUT_PULLUP);
  pinMode(moto, OUTPUT);
  pinMode(uv, INPUT);
  pinMode(test, OUTPUT);
  pinMode(AT, OUTPUT);
  if (!SD.begin(SDCard))
    Serial.println("QRWA");
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  Serial.begin(38400);
  mySerial.begin(38400);
  bme.begin(0x76); //inicjuje czujnik BME280
  Wire.begin();
  Wire.beginTransmission(rtc); // address DS3231
  Wire.write(0x0E); // select register
  Wire.write(0b00011100); // write register bitmap, bit 7 is /EOSC
  Wire.endTransmission();
  attachInterrupt(digitalPinToInterrupt(buttonInterrupt1), buttonOne, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonInterrupt2), buttonTwo, FALLING);
  motor(30);
  sleep(30);
  motor(15);
  ui(0);
  sleep(1000);
  ui(1);
  timeOut(12);
}

void loop() {
  readTime(&foo);
  /*if (foo.ss != save2)
    {
    uvi [foo.ss] = uvRead();
    save2 = foo.ss;
    uviavr = 0;
    int m = 0;
    for (int i = 0; i < 60 ; i++)
    {
      if (uvi[i] > 0)
        m++;
      uviavr += uvi[i];
    }
    if (m == 0)
      uviavr /= 1;
    else
      uviavr /= m;
    if (uvRead() - uviavr > 1 && odliczanie == 60)
    {
      odliczanie = foo.ss;
      uvZapis = uvRead();
    }
    if (odliczanie == foo.ss && uvRead() == uvZapis)
      ui(31);
    else
    {
      uvZapis = 255;
      odliczanie = 60;
    }
    }*/
  while (Serial.available())
  {
    dataTransfer(Serial.readString());
    mySerial.print(Serial.readString());
  }
  while (mySerial.available())
  {
    dataTransfer(mySerial.readString());
    Serial.print(mySerial.readString());
  }
  if (foo.mm != save)
  {
    makeName();
    myFile = SD.open(fileName.c_str(), FILE_WRITE);
    myFile.print("r");
    if (foo.hh < 10)
      myFile.print("0");
    myFile.print(foo.hh);
    myFile.print("x");
    if (foo.mm < 10)
      myFile.print("0");
    myFile.print(foo.mm);
    myFile.print("t");
    if (bme.readTemperature() < 10)
      myFile.print("0");
    myFile.print(bme.readTemperature());
    myFile.print("h");
    if (bme.readHumidity() < 10)
      myFile.print("0");
    myFile.print(bme.readHumidity());
    myFile.print("p");
    if (pressure < 1000)
      myFile.print("0");
    myFile.print(pressure);
    myFile.print("a");
    if (averageUVIndex < 10);
    myFile.print("0");
    myFile.print(averageUVIndex);
    myFile.print("i");
    if (uvRead() < 10);
    myFile.print(0);
    myFile.print(uvRead());
    myFile.print("y");
    myFile.close();
    Serial.print("Zapisane");
    save = foo.mm;
  }
  if (buttonOneV == 1 && screen == 1)
  {
    oled.set2X();
    sleep(1000);
    buttonTwoV = 0;
    buttonOneV = 0;
    byte x = 11;
    ui(11);
    for (int i = 0; i < 1; i--)
    {
      sleep(250);
      if (buttonOneV == 1)
      {
        if (buttonTwoV == 1)
        {
          switch (x)
          {
            case 11:
              x = 12;
              ui(x);
              buttonTwoV = 0;
              break;
            case 12:
              x = 13;
              ui(x);
              buttonTwoV = 0;
              break;
            case 13:
              x = 14;
              ui(x);
              buttonTwoV = 0;
              break;
            case 14:
              x = 11;
              ui(x);
              buttonTwoV = 0;
              break;
          }
        }
      }
    }
    sleep(500);
    buttonOneV = 0;
    buttonTwoV = 0;
    oled.set1X();
  }
  else if (buttonTwoV == 1)
  {
    /*timeParameters example_vals = {
      00,
      50,
      13,
      1,
      25,
      1,
      1,
      } ;
      setTime(&example_vals) ; // Sets the time*/
    buttonOneV = 0;
    buttonTwoV = 0;
    screen = 1;
    ui(1);
    timeOut(12);
  }
  else if (timeOutV == foo.ss)
  {
    oled.clear();
    screen = 0;
  }
  sleep(250);
}

void makeName ()
{
  fileName = String(foo.y + 18);
  if (foo.m < 10)
    fileName += "0";
  fileName += String(foo.m);
  if (foo.d < 10)
    fileName += "0";
  fileName += String(foo.d);
  fileName += ".txt";
  fileName.toCharArray(str, 11);
}
/*
  boolean IOData(String n)
  {
  if (n.length() == 13 || n.length() == 14)
  {

    dataTransfer();
  }
  else if (n.length() == 5 || n.length() == 3)
    dataTransfer();
  else
    sendData();
  //Serial.println(n.length());
  myFile.close();
  }
*/

void dataTransfer(String n)
{
  n.toCharArray(str, 11);
  Serial.println(n.length());
  Serial.println(n);
  myFile = SD.open(str);
  char czas [1] {0};
  while (myFile.available())
  {
    czas [0] = myFile.read();
    Serial.print(czas [0]);
    mySerial.print(czas [0]);
    if (czas [0] == 'y')
    {
      Serial.print("\n");
      mySerial.print("\n");
    }
  }
  //mySerial.print(czas [0]);
  /*
    while (myFile.available())
    {
    czas [0] = myFile.read();
    if (czas[0] == 'z')
    {
      Serial.print("\n");
      mySerial.print("\n");
    }
    else
    {
      Serial.print(czas [0]);
      mySerial.print(czas [0]);
    }
    }*/
}

void sendData ()
{
  mySerial.println(foo.ss);
  mySerial.println(foo.mm);
  mySerial.println(foo.hh);
  mySerial.println(foo.d);
  mySerial.println(foo.m);
  mySerial.println(foo.y);
  mySerial.println(bme.readTemperature());
  mySerial.println(bme.readHumidity());
  mySerial.println(bme.seaLevelForAltitude(AltitudeDef, bme.readPressure()) / 100.0F);
}

boolean uvRead ()
{
  if (analogRead(uv) <= 27)
    return 0;
  else if (analogRead(uv) <= 55)
    return 1;
  else if (analogRead(uv) <= 84)
    return 2;
  else if (analogRead(uv) <= 112)
    return 3;
  else if (analogRead(uv) <= 141)
    return 4;
  else if (analogRead(uv) <= 169)
    return 5;
  else if (analogRead(uv) <= 197)
    return 6;
  else if (analogRead(uv) <= 225)
    return 7;
  else if (analogRead(uv) <= 253)
    return 8;
  else if (analogRead(uv) <= 282)
    return 9;
  else if (analogRead(uv) <= 311)
    return 10;
}

boolean motor(int i)
{
  digitalWrite(moto, HIGH);
  sleep(i);
  digitalWrite(moto, LOW);
}

boolean timeOut(byte i)
{
  timeOutV = foo.ss + i;
  if (timeOutV >= 60 - i)
    timeOutV = abs(timeOutV - 60);
}

boolean sleep(int i)
{
  switch (i)
  {
    case 250:
      LowPower.idle(SLEEP_250MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_ON, TWI_OFF);
      break;
    case 15:
      LowPower.idle(SLEEP_15MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_ON, TWI_OFF);
      break;
    case 30:
      LowPower.idle(SLEEP_30MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_ON, TWI_OFF);
      break;
    //case 120:
    //  LowPower.idle(SLEEP_120MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_ON, TWI_OFF);
    //  break;
    case 500:
      LowPower.idle(SLEEP_500MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_ON, TWI_OFF);
      break;
    case 1000:
      LowPower.idle(SLEEP_1S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_ON, TWI_OFF);
      break;
    case 4000:
      LowPower.idle(SLEEP_4S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_ON, TWI_OFF);
      break;
  }
}

boolean ui(byte mode)
{
  switch (mode)
  {
    case 0:
      oled.clear();
      oled.set2X();
      oled.setCursor(25, 9);
      oled.print("UV band");
      sleep(1000);
      sleep(1000);
      oled.set1X();
      break;
    case 1:
      oled.clear();
      oled.print(bme.readTemperature());
      oled.print("*C");
      oled.print("  UV index ");
      oled.println(uvRead());
      oled.print(bme.readHumidity());
      oled.print(" %  ");
      oled.print(foo.y + 2018);
      oled.print(".");
      if (foo.m < 10)
        oled.print("0");
      oled.print(foo.m);
      oled.print(".");
      if (foo.d < 10)
        oled.print("0");
      oled.println(foo.d);
      oled.print(bme.seaLevelForAltitude(AltitudeDef, bme.readPressure()) / 100.0F);
      oled.print(" hPa");
      if (foo.hh < 10)
        oled.print(" ");
      oled.print(foo.hh);
      oled.print(":");
      if (foo.mm < 10)
        oled.print("0");
      oled.print(foo.mm);
      oled.print(":");
      if (foo.ss < 10)
        oled.print("0");
      oled.print(foo.ss);
      break;
    case 11:
      oled.clear();
      oled.print("WHITE");
      break;
    case 12:
      oled.clear();
      oled.print("MULAT");
      break;
    case 13:
      oled.clear();
      oled.print("BROWN");
      break;
    case 14:
      oled.clear();
      oled.print("BLACK");
      break;
    case 31:
      oled.clear();
      oled.set1X();
      oled.println("Slonce mocniej swieci");
      oled.println("Indeks ");
      oled.println(uvRead());
      mySerial.print("uvtohi");
      motor(500);
  }
}
