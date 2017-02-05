#include <DHT.h>

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>

char auth[] = "427329468d844399b8631b11f4af91bd"; //Insert auth token between the " "

#include <SimpleTimer.h> //here is the SimpleTimer library
SimpleTimer timer; // Create a Timer object called "timer"!

//#include <LiquidCrystal.h>//LCD library
// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

const byte pHpin1 = A8;
const byte pHpin2 = A9;
const byte pHpin3 = A10;
const byte pHpin4 = A11;// Connect the sensor's Po output to an analogue pin - whichever one you choose
const int relayPin1 = 2;
const int relayPin2 = 3;
const int relayPin3 = 4;
const int relayPin4 = 5;//Connect to the relay that will control the power to a pH regulating pump or valve
const int relayPin5 = 6;
const int relayPin6 = 7;
const int relayPin7 = 8;

//The ethernet button is used to tell the arduino board if it's connected to the internet through the ethernet cable or not. If the button is turned off, then it won't spend time trying to connect to the server.
//int inPin = 22; //Just wire this pin dirrectly to GND if you want to leave ethernet constantly on. If you want to test the code before connecting to the servers, leave the pin disconnected.
//int ethbutton = digitalRead(inPin); 

WidgetLED led1(V8); //Connect a LED widget to Virtual pin 7 in the app
WidgetLED led2(V9); //Connect a LED widget to Virtual pin 7 in the app
WidgetLED led3(V10); //Connect a LED widget to Virtual pin 7 in the app
WidgetLED led4(V11); //Connect a LED widget to Virtual pin 7 in the app

//int pinA = 40;  // Connected to CLK on KY-040 Rotary Encoder
//int pinB = 42;  // Connected to DT on KY-040 Rotary Encoder
//SW(button) pin on KY-040 is not used
float encoderPosCount1 = 78; //default setpoint at startup set to 8.0
float encoderPosCount2 = 78; //default setpoint at startup set to 8.0
float encoderPosCount3 = 78; //default setpoint at startup set to 8.0
float encoderPosCount4 = 78; //default setpoint at startup set to 8.0
//int pinALast;  
//int aVal;
//boolean bCW;

// Variables:-
float Po1;
float pH1 = 10;
float Po2;
float pH2 = 10;
float Po3;
float pH3 = 10;
float Po4;
float pH4 = 10;

BLYNK_WRITE(12){
  encoderPosCount1 = param.asInt(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}
BLYNK_WRITE(13){
  encoderPosCount2 = param.asInt(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}
BLYNK_WRITE(14){
  encoderPosCount3 = param.asInt(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}
BLYNK_WRITE(15){
  encoderPosCount4 = param.asInt(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}

void ph(){
  Po1 = (1023 - analogRead(pHpin1));
  Po2 = (1023 - analogRead(pHpin2));
  Po3 = (1023 - analogRead(pHpin3));
  Po4 = (1023 - analogRead(pHpin4));
  Serial.print(Po1); //This is the raw voltage value for the pH module
  Serial.print(Po2); //This is the raw voltage value for the pH module
  Serial.print(Po3); //This is the raw voltage value for the pH module
  Serial.print(Po4); //This is the raw voltage value for the pH module
   //Calibration values:
   //405@pH7
   //290@ph4

  Serial.print(", ph =");
  float pHm1 = map(Po1, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  float pH1 = (pHm1/100);
  float pHm2 = map(Po2, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  float pH2 = (pHm2/100);
  float pHm3 = map(Po3, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  float pH3 = (pHm3/100);
  float pHm4 = map(Po4, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  float pH4 = (pHm4/100);
  //Serial.println(pH, 2); 
  //lcd.setCursor(0, 0);
  //lcd.print("pH: ");  //Print pH value to the LCD
  //lcd.setCursor(3, 0);
  //lcd.print(pH);
  //lcd.setCursor(0, 1);
  //lcd.print("Setpoint:");
  //Serial.println(", setpoint=");
  Serial.println((encoderPosCount1));
  Serial.println((encoderPosCount2));
  Serial.println((encoderPosCount3));
  Serial.println((encoderPosCount4));
  Blynk.virtualWrite(V1, pH1);
  Blynk.virtualWrite(V2, pH2);
  Blynk.virtualWrite(V3, pH3);
  Blynk.virtualWrite(V4, pH4);
  
   if (pH1 > (encoderPosCount1/10))
{
  digitalWrite(relayPin1, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led1.on();
}
else
{
  digitalWrite(relayPin1, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led1.off();      
  
}
   if (pH2 > (encoderPosCount2/10)) //This if/else statement turns the valve on if the pH value is above the setpoint, or off if it's below the setpoint. Modify according to your need.
{
  digitalWrite(relayPin2, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led2.on();
}
else
{
  digitalWrite(relayPin2, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led2.off();      
  
}
   if (pH3 > (encoderPosCount3/10)) //This if/else statement turns the valve on if the pH value is above the setpoint, or off if it's below the setpoint. Modify according to your need.
{
  digitalWrite(relayPin3, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led3.on();
}
else
{
  digitalWrite(relayPin3, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led3.off();      
  
}
   if (pH4 > (encoderPosCount4/10)) 
{
  digitalWrite(relayPin4, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led4.on();
}
else
{
  digitalWrite(relayPin4, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led4.off();      
  
}
}



void blynker1() { //Writes the setpoint value to a gague widget.Connect the gague widget to virtual pin 1
    Blynk.virtualWrite(V21, (encoderPosCount1/10));   
}
void blynker2() { //Writes the setpoint value to a gague widget.Connect the gague widget to virtual pin 1
    Blynk.virtualWrite(V22, (encoderPosCount2/10));
}
void blynker3() { //Writes the setpoint value to a gague widget.Connect the gague widget to virtual pin 1
    Blynk.virtualWrite(V23, (encoderPosCount3/10));
}
void blynker4() { //Writes the setpoint value to a gague widget.Connect the gague widget to virtual pin 1
    Blynk.virtualWrite(V24, (encoderPosCount4/10));   
}


void setup(){
  Serial.begin(9600); // initialize serial communications at 9600 bps
  //lcd.begin(16, 2); // set up the LCD's number of columns and rows: 
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  timer.setInterval(5000, ph);
  //pinMode(inPin, INPUT_PULLUP);
  //pinMode(13, OUTPUT);
  //if (ethbutton == LOW) {
  Blynk.begin(auth, "blynk-cloud.com");
  timer.setInterval(2100, blynker1);
  timer.setInterval(2200, blynker2);
  timer.setInterval(2300, blynker3);
  timer.setInterval(2300, blynker4);//the number for the timers sets the interval for how frequently the function is called. Keep it above 1000 to avoid spamming the server.
  timer.setInterval(24000, ph);
  //timer.setInterval(2000, setpointwriter);
}


void loop(){
  Blynk.run(); 
  timer.run();
  //encoder();
}

