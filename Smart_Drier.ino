//#include <HygrometerSensor.h> // Rain sensor
#include <Wire.h> // Serial Communication
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>//Temperature and Humidity sensor
#include <LiquidCrystal_I2C.h> // LCD display
#include <Keypad.h> // Keypad
#include <DS3231.h>//RTC for reading time
LiquidCrystal_I2C lcd(0x38, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// INTRODUCTION
/* This is a code for a prototype dehydrator
    submitted for the Talented Young Scientists
    of Nigeria (TYSON)
    A display unit will be used to display the
    status of the system.
    When it is first used, the system will be set and initialized.
    The default runing code is the TOMATO code.
    SENSORS and DISPLAYS:
    RTC (Serial A4 & A5)
    LCD (Serial A4 & A5)
    Rain sensor (Digital pin used A0)
    Temperature and Humidity sensor (Two pins)
    KEYPAD DETAILS
    Column keys are high and are connected from PD4 to PD7
    Row keys are low and are connected from PD2 to PD3
*/

/* NOTES
 *  The system uses time to position the lid to the direction of the sun.
 *  When it is raining, the system will tilt the chamber to 10 DEG 
 *  and switch on the heater.
 *  At night, the same condition runs for rain.
 *  
 */
//====================== INITIALIZATION OF VARIABLES ===========================
//RTC ; All RTC variables are in 2 digits.
bool Century = false;
bool h12;
bool PM;
byte Year, Month, Day, Hour, Minute, Second;
DS3231 Clock;

//DHT
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht = DHT (DHTPIN, DHTTYPE);
byte temperature; // stores the temperature in the chamber
byte humidity; // stores the humidity in the chamber

//KEYPAD
const byte ROWS = 2;
const byte COLS = 4;
byte RowPins[ROWS] = {2, 3};
byte ColPins[COLS] = {4, 5, 6, 7};
char key;
char KeyMap[ROWS][COLS] = {
  {'1', '2', '3', '4'},
  {'5', '6', '7', '8'},
};
Keypad board = Keypad(makeKeymap(KeyMap), RowPins, ColPins, ROWS, COLS);

//OTHERS
const int Rain = A1; // The rain sensor (DIGITAL IN)
const int Mclk = 10; // Motor clockwise relay (DIGITAL OUT)
const int Anticlk = 11; //Motor anticlockwise relay (DIGITAL OUT)
const byte Heater = 8; // The heater relay (DIGITAL OUT)
const byte Buzzer = 9; //The buzzer output
const byte light = 1; // Lid light (DIGITAL OUT)
const byte Open = 0; // Lid switch (DIGITAL IN)ACTIVE LOW
long buzz = 2000; //interval for buzzer to sound when food is dry
/* The keypad is connected to PORT D
    Pins used are 2 to 7 (6 pins total)
    Rows are connected to 2 and 3.
    Columns are connected to 4 to 7
*/
byte RainPos = 10; //The angle of the lid when it is raining 

//MOTOR AND POTENTIOMETER
const byte Pot = A0; //Position potentiometer
//byte Motor []={Mclk, Anticlk};
int Pot_Min = 0;// MINIMUM POTENTIOMETER VALUE (DIGITAL VALUE)
int Pot_Max = 1023;//MAXIMUM POTENTIOMETER VALUE (DIGITAL VALUE)
int Min_Angle = -45; //MINIMUM ANGLE (IN DEGREES)
int Max_Angle = 45; //MAXIMUM ANGLE (ANGLE IN DEGREES)
int M_Bound = 7;//Boundary for motor to stop
int posVal = 1;// For displaying positioning on the LCD

//FOOD
int j = 0; //counter for selecting food
int selected_food = 0; //address of selected food
String Food [] = {"1. Tomato    ", "2. Carrot    ", "3. Pepper    ",
                  "4. Cuccumber ", "5. Apple     " , "6. Meat      ", "7. Others     "
                 }; // Contains all the food the heater can handle
//Temperatures are in degree celcius (Maximum temteratures)
byte TomatoTemp = 30;
byte CarrotTemp = 40;
byte PepperTemp = 40;
byte CucTemp = 30;
byte AppleTemp = 30;
byte MeatTemp = 50;
byte DefaultTemp = 35; // For all other foods not covered
byte dryHum = 25; // Dryness humidity


//=================== CONFIGURATION OF PINS AND DEVICES ====================
void setup() {
  for (int i = 2; i <= 7; i++) pinMode(i, INPUT);
  // put your setup code here, to run once:
  pinMode(Rain, INPUT);
  pinMode(Open, INPUT);
  pinMode(light, OUTPUT);
  //for (int i=0; i<=7; i++) pinMode(i, INPUT);
  pinMode(Heater, OUTPUT);
  pinMode (Mclk, OUTPUT);
  pinMode(Anticlk, OUTPUT);
  pinMode (Buzzer, OUTPUT);
  Wire.begin(); // Starts serial communication for I2C devices
  lcd.init(); // initialize the lcd
  lcd.backlight();
  dht.begin();
  lcd.setCursor(1, 0);
  lcd.print("INITIALIZING!!!");
  delay(1000);
  Position(0);
  lcd.clear();
}

//====================== BODY OF CODE ===========================
void loop() {
  // put your main code here, to run repeatedly:
  ReadLid();
}

//====================== SUBROUTINES ===========================

//============= Function that reads the lid switch ===========
void ReadLid() {
  lcd.clear();
  //SWITCH IS ACTIVE LOW.
  while ( digitalRead (Open) == HIGH) { // WHEN THE LID IS OPEN
    lcd.setCursor(4, 0);
    lcd.print("LID OPEN");
    digitalWrite (Heater, LOW); // turn off heater in case it was on
    digitalWrite(Mclk, LOW); // turn off motor in case it was on
    digitalWrite(Anticlk, LOW); // turn off motor in case it was on
    digitalWrite(Buzzer, LOW);//turn off buzzer in case it was on
    digitalWrite(light, HIGH);// Turn chamber light on
    //delay(500);
  }
  lcd.clear();
  while (digitalRead (Open) == LOW) { //THE LID IS CLOSED, THE SYSTEM STARTS FUNCTIONING
    digitalWrite(light, LOW); // Turn off chamber light
    lcd.setCursor(3, 0);
    lcd.print("LID CLOSED");
    lcd.setCursor(1, 1);
    lcd.print("GIVE A COMMAND");
    if (posVal == 1)Position(0);
    posVal = 2;
    char key1 = board.getKey(); //READS THE KEYPAD
    if (key1) {
      key = key1;
    }
    switch (key) {
      case '6':// CORRESPONDS TO NO FOOD COMMAND
        key = '0';
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("LID CLOSED");
        lcd.setCursor(1, 1);
        lcd.print("NO FOOD INSIDE");
        delay(2000);
        lcd.clear();
        while (digitalRead(Open) == LOW) {
          // while the lid is closed and the the open key is not pressed
          if (board.getKey() == '5') Position(0);
          lcd.setCursor(1, 0);
          lcd.print("NO FOOD INSIDE");
          lcd.setCursor(0, 1);
          lcd.print("open to add food");
          digitalWrite (Heater, LOW); // turn off heater in case it was on
          digitalWrite(Mclk, LOW); // turn off motor in case it was on
          digitalWrite(Anticlk, LOW); // turn off motor in case it was on
          digitalWrite(Buzzer, LOW);//turn off buzzer in case it was on
        }
        ReadLid();
      case '7': // Corresponds to the NEW FOOD command
        key = '0';
        SelectFood(); // Let us know which food is inside
        break;
      case '8'://Corresponds to the CONTINUE command
        key = '0';
        Fd_Routine(selected_food);//Continue with the selected food subroutine
      default:
        break;
    }
  }
  ReadLid();
}

//================ Function to select the food inside ===================
void SelectFood() {
  j = 0;
  lcd.clear();
  while (key != '4') { //Key 4 is OK key
    //if the OK key is not pressed, the code remains in this loop
    key = board.getKey();
    if (key == '1') j = 0; //HOME key
    if (key == '2') j = j - 1; //PREV key
    if (key == '3') j = j + 1; //NEXT key
    if (key == '5') ReadLid(); // Go back to the start
    // else j = 0;
    if (j >= (sizeof(Food) / sizeof(String))) j = 0;
    lcd.setCursor(0, 0);
    lcd.print("What's inside?");
    lcd.setCursor(0, 1);
    lcd.print(Food[j]);
  }
  selected_food = j;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(Food[j]);
  lcd.setCursor(0, 1);
  lcd.print("Selected");
  unsigned long rest = millis();
  while (millis() - rest < 2500) {
    key = board.getKey();
    if ( key == '5' ) {
      SelectFood();
      break;
    }
  }
  Fd_Routine( selected_food);//calls the selected food function
}


//=============== Runs the code for the selected food ==================
void Fd_Routine(int selected_food) {
  /*
     This is where the main program for the selected food runs.
      "1. Tomato    ", "2. Carrot    ", "3. Pepper    ","4. Cuccumber ", "5. Apple     " , "6. Meat      ", "7. Others     "

  */

  switch (selected_food) {
    case 0: // RUNS WHEN TOMATO IS SELECTED.
      /* 1. Read RTC and use it to position the heating chamber. If night, position at zero and turn on heater.
         2. Read temperature and humidity sensor and use the readings to determine the
         dryness fraction of the food. If dry, position  away from sun in the day or zero at night and sound buzzer.
         3. Read rain sensor and determine if it is raining or not. if it is raining,
         position at 10 degrees and turn on the heater. Position(10)
      */
      //PosWtTime();
      while (digitalRead(Open) == LOW) { //While the lid is closed
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TOMATO INSIDE");
        DispValues();
        DispTime();
        if (digitalRead(Rain) == HIGH) { //if it is raining
          /*
             Here, it doesn't matter if it is day or night
             just on or off the heater
          */
          Position(RainPos);
          if (temperature > TomatoTemp)digitalWrite(Heater, LOW);
          else digitalWrite(Heater, HIGH);
          if (humidity < dryHum)FoodReady();
        }
        else { //It is not raining
          if ( Hour < 7 || Hour > 17) { //It is night
            Position(0);
            if (temperature > TomatoTemp)digitalWrite(Heater, LOW);
            else digitalWrite(Heater, HIGH);
            if (humidity < dryHum)FoodReady();
          }
          else {//It is day
            digitalWrite(Heater, LOW);
            if (temperature > TomatoTemp) {//Temperature above setpoint
              if (Hour < 12)Position(-40);
              else Position(40);
            }
            else PosWtTime();// temperature okay
            if (humidity < dryHum) {
              if (Hour < 12)Position(-40);
              else Position(40);
              FoodReady();
            }
          }
        }
      }
      ReadLid();
      break;

    case 1: //RUNS WHEN CARROT IS SELECTED
      PosWtTime();
      while (digitalRead(Open) == LOW) { //While the lid is closed
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        if (Hour < 7 || Hour > 17 || digitalRead(Rain) == HIGH) {
          // Hours between 5PM and 7AM or it's raining turn on the heater.
          if (digitalRead(Rain) == HIGH) Position(RainPos);
          else Position(0);
          if ( temperature < CarrotTemp)digitalWrite (Heater, HIGH);
          else digitalWrite (Heater, LOW); //Puts off the heater
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("CARROT INSIDE");
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.setCursor(2, 1);
        lcd.print(temperature, DEC);
        lcd.setCursor(4, 1);
        lcd.print("*C");
        lcd.setCursor(9, 1);
        lcd.print("H:");
        lcd.setCursor(11, 1);
        lcd.print(humidity, DEC);
        lcd.setCursor(13, 1);
        lcd.print("%");
        delay(1500);
        if (temperature > CarrotTemp) {//Temperature is above the limit
          if (digitalRead(Rain) == LOW && Hour >= 7 and Hour <= 17) {
            //if it is not raining and it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else digitalWrite (Heater, LOW); //it is either raining or it's night
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        if (humidity < dryHum) { //If the food is dry enough
          digitalWrite(Heater, LOW); //Turn off heater
          if (Hour > 7 && Hour < 17) {//if it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else {
            if (digitalRead(Rain) == HIGH) {
              Position(RainPos);
            }
            else PosWtTime();
          }
          lcd.clear();
          while ( digitalRead(Open) == LOW) {
            lcd.setCursor(0, 0);
            lcd.print("FOOD IS READY");
            lcd.setCursor(0, 1);
            lcd.print("OPEN TO REMOVE");
            if (board.getKey() == '5') Position(0);
            if (millis() % buzz < 20) {
              //buzzer sounds twice every buzz seconds
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
              delay(500);
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
            }
          }
          ReadLid();
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        DispTime();
      }
      ReadLid();
      break;

    case 2://RUNS WHEN PEPPER IS SELECTED
      PosWtTime();
      while (digitalRead(Open) == LOW) { //While the lid is closed
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        if (Hour < 7 || Hour > 17 || digitalRead(Rain) == HIGH) {
          // Hours between 5PM and 7AM or it's raining turn on the heater.
          if (digitalRead(Rain) == HIGH) Position(RainPos);
          else Position(0);
          if ( temperature < PepperTemp)digitalWrite (Heater, HIGH);
          else digitalWrite (Heater, LOW); //Puts off the heater
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PEPPER INSIDE");
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.setCursor(2, 1);
        lcd.print(temperature, DEC);
        lcd.setCursor(4, 1);
        lcd.print("*C");
        lcd.setCursor(9, 1);
        lcd.print("H:");
        lcd.setCursor(11, 1);
        lcd.print(humidity, DEC);
        lcd.setCursor(13, 1);
        lcd.print("%");
        delay(1500);
        if (temperature > PepperTemp) {//Temperature is above the limit
          if (digitalRead(Rain) == LOW && Hour >= 7 and Hour <= 17) {
            //if it is not raining and it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else digitalWrite (Heater, LOW); //it is either raining or it's night
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        if (humidity < dryHum) { //If the food is dry enough
          digitalWrite(Heater, LOW); //Turn off heater
          if (Hour > 7 && Hour < 17) {//if it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else {
            if (digitalRead(Rain) == HIGH) {
              Position(RainPos);
            }
            else PosWtTime();
          }
          lcd.clear();
          while ( digitalRead(Open) == LOW) {
            lcd.setCursor(0, 0);
            lcd.print("FOOD IS READY");
            lcd.setCursor(0, 1);
            lcd.print("OPEN TO REMOVE");
            if (board.getKey() == '5') Position(0);
            if (millis() % buzz < 20) {
              //buzzer sounds twice every buzz seconds
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
              delay(500);
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
            }
          }
          ReadLid();
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        DispTime();
      }
      ReadLid();
      break;

    case 3: // RUNS WHEN CUCCUMBER IS SELECTED

      PosWtTime();
      while (digitalRead(Open) == LOW) { //While the lid is closed
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        if (Hour < 7 || Hour > 17 || digitalRead(Rain) == HIGH) {
          // Hours between 5PM and 7AM or it's raining turn on the heater.
          if (digitalRead(Rain) == HIGH) Position(RainPos);
          else Position(0);
          if ( temperature < CucTemp)digitalWrite (Heater, HIGH);
          else digitalWrite (Heater, LOW); //Puts off the heater
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("CUCCUMBER INSIDE");
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.setCursor(2, 1);
        lcd.print(temperature, DEC);
        lcd.setCursor(4, 1);
        lcd.print("*C");
        lcd.setCursor(9, 1);
        lcd.print("H:");
        lcd.setCursor(11, 1);
        lcd.print(humidity, DEC);
        lcd.setCursor(13, 1);
        lcd.print("%");
        delay(1500);
        if (temperature > CucTemp) {//Temperature is above the limit
          if (digitalRead(Rain) == LOW && Hour >= 7 and Hour <= 17) {
            //if it is not raining and it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else digitalWrite (Heater, LOW); //it is either raining or it's night
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        if (humidity < dryHum) { //If the food is dry enough
          digitalWrite(Heater, LOW); //Turn off heater
          if (Hour > 7 && Hour < 17) {//if it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else {
            if (digitalRead(Rain) == HIGH) {
              Position(RainPos);
            }
            else PosWtTime();
          }
          lcd.clear();
          while ( digitalRead(Open) == LOW) {
            lcd.setCursor(0, 0);
            lcd.print("FOOD IS READY");
            lcd.setCursor(0, 1);
            lcd.print("OPEN TO REMOVE");
            if (board.getKey() == '5') Position(0);
            if (millis() % buzz < 20) {
              //buzzer sounds twice every buzz seconds
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
              delay(500);
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
            }
          }
          ReadLid();
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        DispTime();
      }
      ReadLid();
      break;

    case 4:// RUNS WHEN APPLE IS SELECTED

      PosWtTime();
      while (digitalRead(Open) == LOW) { //While the lid is closed
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        if (Hour < 7 || Hour > 17 || digitalRead(Rain) == HIGH) {
          // Hours between 5PM and 7AM or it's raining turn on the heater.
          if (digitalRead(Rain) == HIGH) Position(RainPos);
          else Position(0);
          if ( temperature < AppleTemp)digitalWrite (Heater, HIGH);
          else digitalWrite (Heater, LOW); //Puts off the heater
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("APPLE INSIDE");
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.setCursor(2, 1);
        lcd.print(temperature, DEC);
        lcd.setCursor(4, 1);
        lcd.print("*C");
        lcd.setCursor(9, 1);
        lcd.print("H:");
        lcd.setCursor(11, 1);
        lcd.print(humidity, DEC);
        lcd.setCursor(13, 1);
        lcd.print("%");
        delay(1500);
        if (temperature > AppleTemp) {//Temperature is above the limit
          if (digitalRead(Rain) == LOW && Hour >= 7 and Hour <= 17) {
            //if it is not raining and it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else digitalWrite (Heater, LOW); //it is either raining or it's night
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        if (humidity < dryHum) { //If the food is dry enough
          digitalWrite(Heater, LOW); //Turn off heater
          if (Hour > 7 && Hour < 17) {//if it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else {
            if (digitalRead(Rain) == HIGH) {
              Position(RainPos);
            }
            else PosWtTime();
          }
          lcd.clear();
          while ( digitalRead(Open) == LOW) {
            lcd.setCursor(0, 0);
            lcd.print("FOOD IS READY");
            lcd.setCursor(0, 1);
            lcd.print("OPEN TO REMOVE");
            if (board.getKey() == '5') Position(0);
            if (millis() % buzz < 20) {
              //buzzer sounds twice every buzz seconds
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
              delay(500);
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
            }
          }
          ReadLid();
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        DispTime();
      }
      ReadLid();
      break;

    case 5://RUNS WHEN MEAT IS SELECTED

      PosWtTime();
      while (digitalRead(Open) == LOW) { //While the lid is closed
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        if (Hour < 7 || Hour > 17 || digitalRead(Rain) == HIGH) {
          // Hours between 5PM and 7AM or it's raining turn on the heater.
          if (digitalRead(Rain) == HIGH) Position(RainPos);
          else Position(0);
          if ( temperature < MeatTemp)digitalWrite (Heater, HIGH);
          else digitalWrite (Heater, LOW); //Puts off the heater
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MEAT INSIDE");
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.setCursor(2, 1);
        lcd.print(temperature, DEC);
        lcd.setCursor(4, 1);
        lcd.print("*C");
        lcd.setCursor(9, 1);
        lcd.print("H:");
        lcd.setCursor(11, 1);
        lcd.print(humidity, DEC);
        lcd.setCursor(13, 1);
        lcd.print("%");
        delay(1500);
        if (temperature > MeatTemp) {//Temperature is above the limit
          if (digitalRead(Rain) == LOW && Hour >= 7 and Hour <= 17) {
            //if it is not raining and it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else digitalWrite (Heater, LOW); //it is either raining or it's night
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        if (humidity < dryHum) { //If the food is dry enough
          digitalWrite(Heater, LOW); //Turn off heater
          if (Hour > 7 && Hour < 17) {//if it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else {
            if (digitalRead(Rain) == HIGH) {
              Position(RainPos);
            }
            else PosWtTime();
          }
          lcd.clear();
          while ( digitalRead(Open) == LOW) {
            lcd.setCursor(0, 0);
            lcd.print("FOOD IS READY");
            lcd.setCursor(0, 1);
            lcd.print("OPEN TO REMOVE");
            if (board.getKey() == '5') Position(0);
            if (millis() % buzz < 20) {
              //buzzer sounds twice every buzz seconds
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
              delay(500);
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
            }
          }
          ReadLid();
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        DispTime();
      }
      ReadLid();
      break;

    default://RUNS WHEN OTHERS IS SELECTED

      PosWtTime();
      while (digitalRead(Open) == LOW) { //While the lid is closed
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        if (Hour < 7 || Hour > 17 || digitalRead(Rain) == HIGH) {
          // Hours between 5PM and 7AM or it's raining turn on the heater.
          if (digitalRead(Rain) == HIGH) Position(RainPos);
          else Position(0);
          if ( temperature < DefaultTemp)digitalWrite (Heater, HIGH);
          else digitalWrite (Heater, LOW); //Puts off the heater
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("OTHERS INSIDE");
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.setCursor(2, 1);
        lcd.print(temperature, DEC);
        lcd.setCursor(4, 1);
        lcd.print("*C");
        lcd.setCursor(9, 1);
        lcd.print("H:");
        lcd.setCursor(11, 1);
        lcd.print(humidity, DEC);
        lcd.setCursor(13, 1);
        lcd.print("%");
        delay(1500);
        if (temperature > DefaultTemp) {//Temperature is above the limit
          if (digitalRead(Rain) == LOW && Hour >= 7 and Hour <= 17) {
            //if it is not raining and it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else digitalWrite (Heater, LOW); //it is either raining or it's night
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        if (humidity < dryHum) { //If the food is dry enough
          digitalWrite(Heater, LOW); //Turn off heater
          if (Hour > 7 && Hour < 17) {//if it is day
            if (analogRead(Pot) > (Pot_Max + Pot_Min) / 2)Position(-40);
            else Position(40);
          }
          else {
            if (digitalRead(Rain) == HIGH) {
              Position(RainPos);
            }
            else PosWtTime();
          }
          lcd.clear();
          while ( digitalRead(Open) == LOW) {
            lcd.setCursor(0, 0);
            lcd.print("FOOD IS READY");
            lcd.setCursor(0, 1);
            lcd.print("OPEN TO REMOVE");
            if (board.getKey() == '5') Position(0);
            if (millis() % buzz < 20) {
              //buzzer sounds twice every buzz seconds
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
              delay(500);
              digitalWrite(Buzzer, HIGH);
              delay(1000);
              digitalWrite(Buzzer, LOW);
            }
          }
          ReadLid();
        }
        else {
          if (digitalRead(Rain) == HIGH) {
            Position(RainPos);
          }
          else PosWtTime();
        }
        DispTime();
      }
      ReadLid();
      break;
  }
}

//============ tilts the chamber to a specified position ===================
void Position (int pos) {//pos is an angle in degrees
  int Pot_Pos = map(pos, Min_Angle, Max_Angle, Pot_Min, Pot_Max);
  int Pot_New = constrain(Pot_Pos, Pot_Min, Pot_Max);
  int Pot_Old = analogRead(Pot);
  int diff = Pot_New - Pot_Old;
  lcd.clear();
  do {
    int Pot_Old = analogRead(Pot);
    int diff = Pot_New - Pot_Old;
    if (diff > M_Bound) {
      digitalWrite(Anticlk, LOW);
      digitalWrite(Mclk, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("ROTATING TO");
      lcd.setCursor(12, 0);
      lcd.print(pos);
      lcd.setCursor(0, 1);
      lcd.print("PLEASE WAIT.");
    }
    if (diff < -M_Bound) {
      digitalWrite(Mclk, LOW);
      digitalWrite(Anticlk, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("ROTATING TO");
      lcd.setCursor(12, 0);
      lcd.print(pos);
      lcd.setCursor(0, 1);
      lcd.print("PLEASE WAIT.");
    }
    if (diff >= -M_Bound && diff <= M_Bound) {
      digitalWrite(Mclk, LOW);
      digitalWrite(Anticlk, LOW);
      break;
    }
  } while (abs(diff) >= 5);
  digitalWrite(Mclk, LOW);
  digitalWrite(Anticlk, LOW);
  lcd.clear();
}

// ============ POSITION CHAMBER WITH HOUR OF THE DAY =============
void PosWtTime() {
  Hour = Clock.getHour(h12, PM);
  switch (Hour) {
    case 7:
      Position(35);
      break;
    case 8:
      Position(30);
      break;
    case 9:
      Position(25);
      break;
    case 10:
      Position(20);
      break;
    case 11:
      Position(15);
      break;
    case 12:
      Position(0);
      break;
    case 13:
      Position(-15);
      break;
    case 14:
      Position(-20);
      break;
    case 15:
      Position(-25);
      break;
    case 16:
      Position(-35);
      break;
    case 17:
      Position(-40);
      break;
    default: // There is no sun
      Position(0);
      break;
  }
}
void DispTime() {
   Year = Clock.getYear();
   Month = Clock.getMonth(Century);
   Day = Clock.getDate();
   Hour = Clock.getHour(h12, PM);
   Minute = Clock.getMinute();
   Second = Clock.getSecond();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print ("Date:");
  lcd.setCursor(6, 0);
  lcd.print(Day / 10);
  lcd.setCursor(7, 0);
  lcd.print(Day % 10);
  lcd.setCursor(8, 0);
  lcd.print("-");
  lcd.setCursor(9, 0);
  lcd.print(Month / 10);
  lcd.setCursor(10, 0);
  lcd.print(Month % 10);
  lcd.setCursor(11, 0);
  lcd.print("-");
  lcd.setCursor(12, 0);
  lcd.print("20");
  lcd.setCursor(14, 0);
  lcd.print(Year / 10);
  lcd.setCursor(15, 0);
  lcd.print(Year % 10);
  lcd.setCursor(0, 1);
  lcd.print("Time:");
  lcd.setCursor(6, 1);
  lcd.print(Hour / 10); //24-hr
  lcd.setCursor(7, 1);
  lcd.print(Hour % 10);
  lcd.setCursor(8, 1);
  lcd.print(":");
  lcd.setCursor(9, 1);
  lcd.print(Minute / 10);
  lcd.setCursor(10, 1);
  lcd.print(Minute % 10);
  lcd.setCursor(11, 1);
  lcd.print(":");
  lcd.setCursor(12, 1);
  lcd.print(Second / 10);
  lcd.setCursor(13, 1);
  lcd.print(Second % 10);
  delay(1500);
}
void FoodReady() {
  while ( digitalRead(Open) == LOW) {
    digitalWrite(Heater, LOW);
    lcd.setCursor(0, 0);
    lcd.print("FOOD IS READY");
    lcd.setCursor(0, 1);
    lcd.print("OPEN TO REMOVE");
    if (board.getKey() == '5') Position(0);
    if (millis() % buzz < 20) {
      //buzzer sounds twice every buzz seconds
      digitalWrite(Buzzer, HIGH);
      delay(1000);
      digitalWrite(Buzzer, LOW);
      delay(500);
      digitalWrite(Buzzer, HIGH);
      delay(1000);
      digitalWrite(Buzzer, LOW);
    }
  }
  ReadLid();
}
void DispValues() {
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.setCursor(2, 1);
  lcd.print(temperature, DEC);
  lcd.setCursor(4, 1);
  lcd.print("*C");
  lcd.setCursor(9, 1);
  lcd.print("H:");
  lcd.setCursor(11, 1);
  lcd.print(humidity, DEC);
  lcd.setCursor(13, 1);
  lcd.print("%");
  delay(1500);
}
