/*
FOR SINGLE PHASE USERS ONLY
*This code is for automatic phase selector and changeover.
*It selects the best voltage out of the three phases so that you 
*will always have the best of the voltages.
*If the voltage in all the phases goes low, you have an option of 
*using the voltage as it is or to on your generator. A buzzer will
*be activaed for this.
*If the light goes off entirely, the change over changes to your
*generator input.
*The generator stops automatically if the utility is restored.
*You can also stop the generator at will if you wish by pressing 
*a button.
*There will be a master switch for turning the while system off at
*will.
*NOTE THAT THE SYSTEM WILL NOT START YOUR GENERATOR AUTOMATICALLY.
*/
#include <LiquidCrystal.h>
const int red = A0; // Reads red phase (Analog)
const int yellow = A1; // Reads yellow phase (Analog)
const int blue = A2; // Reads blue phase (Analog)
const int gen = A3; // Reads generator output (Analog)
const int use = A4; // (Digigal in) Input switch to use the utility when it is low 
const int buzzer = A5; // (Digital out) Activates when voltage is not within range (rings 30 times)
const int startSwitch = 0; // (Digital in) switch for starting gen when voltage is not acceptable
const int stopSwitch = 1; // (Digital in) switch for stoping gen when it is running
LiquidCrystal lcd (2,3,4,5,6,7); // LCD pins
const int R = 8; // (Digital out) selects red phase
const int Y = 9; // (Digital out) selects yellow phase
const int B = 10; // (Digital out) selects blue phase
const int G = 11; // (Digital out) selects generator output
const int stopGen = 12; // (Digital out) stops generator when it is running
const int powerOn = 13; // (Digital out) shows that the controller is working
void setup() {
  // put your setup code here, to run once:
for (int i = 8; i<=13; i++) pinMode(i,OUTPUT);
pinMode (buzzer, OUTPUT);
for (int i=0; i<=1; i++) pinMode(i,INPUT);
pinMode (use,INPUT);
digitalWrite (powerOn, HIGH);
lcd.begin(16,2);
lcd.clear();
lcd.setCursor(1,0);
lcd.print ("YOU'RE WELCOME");
lcd.clear();
lcd.setCursor(1,0);
lcd.print("INITIALIZING");
delay (3000);
}

void loop() {
  // put your main code here, to run repeatedly:
normalPhase();
}
// ============FUNCTION TO SELECT NORMAL PHASES============
void normalPhase (){
  //Reading the phases
int r=analogRead (red);
int y=analogRead (yellow);
int b=analogRead (blue);
int g=analogRead (gen);
//WHEN ALL THE PHASES ARE HIGH, DO THIS.
if ( (r>893) && (y>893) && (b>893)){
digitalWrite(R,LOW); // DEACTIVATE RED PHASE
digitalWrite(Y,LOW); // DEACTIVATE YELLOW PHASE
digitalWrite(B,LOW); // DEACTIVATE BLUE PHASE
lcd.clear(); //CLEAR LCD
lcd.setCursor(2,0); // SET LCD CURSOR TO COLUMN 3, ROW 1
lcd.print ("ALL PHASES"); // DISPLAY THE STRING ON THE LCD
lcd.setCursor(3,1);
lcd.print("ARE HIGH");
delay (3000); // DELAY FOR 3 SECONDS
ActivateBuzzer(); // CALL THE BUZZER FUNCTION TO SOUND BUZZER
ReadSwitches();
}
// IF THE VOLTAGES ARE TOO LOW, DO THE FOLLOWING.
if ( (r<372) && (y<372) && (b<372)){
digitalWrite(R,LOW);
digitalWrite(Y,LOW);
digitalWrite(B,LOW);
lcd.clear();
lcd.setCursor(2,0);
lcd.print ("ALL PHASES");
lcd.setCursor(3,1);
lcd.print("ARE LOW");
delay (3000);
ActivateBuzzer();
ReadSwitches();
}
if ((r<200)&&(y<200)&&(b<200))startGen ();
// SELECTING THE RED PHASE
if ( (r>y) && (r>b) && (r>372)&&(r<893)){
digitalWrite(Y,LOW);
digitalWrite(B,LOW);
digitalWrite(G,LOW);
digitalWrite(stopGen, LOW);
delay (10);
digitalWrite(R,HIGH);
lcd.clear();
lcd.setCursor(3,0);
lcd.print ("RED PHASE");
lcd.setCursor(3,1);
lcd.print("IS IN USE");
delay (3000);
}
// SELECTING THE YELLOW PHASE
if ( (y>r) && (y>b) && (y>372)&&(y<893)){
digitalWrite(R,LOW);
digitalWrite(B,LOW);
digitalWrite(G,LOW);
digitalWrite(stopGen, LOW);
delay (10);
digitalWrite(Y,HIGH);
lcd.clear();
lcd.setCursor(1,0);
lcd.print ("YELLOW PHASE");
lcd.setCursor(3,1);
lcd.print("IS IN USE");
delay (3000);
}
// SELECTING THE BLUE PHASE
if ( (b>r) && (b>y) && (b>372)&&(b<893)){
digitalWrite(R,LOW);
digitalWrite(Y,LOW);
digitalWrite(G,LOW);
digitalWrite(stopGen, LOW);
delay (10);
digitalWrite(B,HIGH);
lcd.clear();
lcd.setCursor(3,0);
lcd.print ("BLUE PHASE");
lcd.setCursor(3,1);
lcd.print("IS IN USE");
delay (3000);
}
} // END OF THE NORMAL PHASES FUNCTION

//================== SOUNDING THE BUZZER FUNCTION ==============
void ActivateBuzzer (){
digitalWrite (buzzer, HIGH);
delay (1000);
digitalWrite (buzzer,LOW);
}

//============== STARTING THE GENERATOR ===========
void startGen (){
int r=analogRead (red);
int y=analogRead (yellow);
int b=analogRead (blue);
int g=analogRead (gen);
if (g<100){
lcd.clear();
lcd.setCursor(1,0);
lcd.print ("SWITCHING TO");
lcd.setCursor(3,1);
lcd.print("GENERATOR");
delay (3000);
}
if ((r<372) && (y<372)&&(b<372)){
digitalWrite (stopGen, HIGH);
digitalWrite (R, LOW);
digitalWrite (Y, LOW);
digitalWrite (B, LOW);
if ((g>100)){
digitalWrite (G, HIGH);
lcd.clear();
lcd.setCursor(3,0);
lcd.print ("GENERATOR");
lcd.setCursor(3,1);
lcd.print("IS IN USE");
delay (3000);
}
}
else {
normalPhase();
}
}
// =============== STOPPING THE GENERATOR ================
  void StopGen(){
  digitalWrite (G, LOW);
  delay (20);
  digitalWrite (stopGen,LOW);
  }
  // ==============RUNNING ON ABNORMAL PHASES=============
  void abnormalPhases (){
  int r=analogRead (red);
int y=analogRead (yellow);
int b=analogRead (blue);
int g=analogRead (gen);
if ((r<372)&&(y<372)&&(b<372)&&(g<372)){
  if ((r>y)&&(r>b)){
  digitalWrite(Y,LOW);
digitalWrite(B,LOW);
digitalWrite(G,LOW);
digitalWrite(stopGen, LOW);
delay (10);
digitalWrite(R,HIGH);
lcd.clear();
lcd.setCursor(3,0);
lcd.print ("RED PHASE");
lcd.setCursor(3,1);
lcd.print("IS IN USE");
delay (3000);
}
 if ((y>r)&&(y>b)){
  digitalWrite(R,LOW);
digitalWrite(B,LOW);
digitalWrite(G,LOW);
digitalWrite(stopGen, LOW);
delay (10);
digitalWrite(Y,HIGH);
lcd.clear();
lcd.setCursor(1,0);
lcd.print ("YELLOW PHASE");
lcd.setCursor(3,1);
lcd.print("IS IN USE");
delay (3000);
}
 if ((b>y)&&(b>r)){
  digitalWrite(Y,LOW);
digitalWrite(R,LOW);
digitalWrite(G,LOW);
digitalWrite(stopGen, LOW);
delay (10);
digitalWrite(B,HIGH);
lcd.clear();
lcd.setCursor(3,0);
lcd.print ("BLUE PHASE");
lcd.setCursor(3,1);
lcd.print("IS IN USE");
delay (3000);
}
}
  }
  //============== READING THE SWITCHES =================
  void ReadSwitches (){
  int USE = digitalRead (use);
  int START =digitalRead (startSwitch);
  int STOP =digitalRead (stopSwitch);
  if (USE==HIGH){
  abnormalPhases();
  }
  if (START==HIGH){
  startGen();
  }
  if (STOP==HIGH){
  StopGen();
  }
  }
