//Change serial to debug serial
//#include <DHT.h>

//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
//#include <SPI.h>
#include <SoftwareSerial.h>
#include <BlynkSimpleStream.h>

char auth[] = "54a10142143d4854901d5d7bb67de132"; //Insert auth token between the " "

#include <SimpleTimer.h> //here is the SimpleTimer library
SimpleTimer timer; // Create a Timer object called "timer"!

//#include <LiquidCrystal.h>//LCD library
// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

const int trigPin = 12; 
const int echoPin = 13; //distance measurer

const byte switchPin1 = 31; //float switch
const byte switchPin2 = 33; //float switch
const byte switchPin3 = 35; //float switch
const byte switchPin4 = 37; //float switch

long duration;
int distanceCm, volume;

int algaecase; //variable used for determining which pair of algae tanks are being used (1+2 or 3+4)
int turbidity, turbsetpoint, valveoutput;
int turb;


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

//LED widgets:
WidgetLED led1(V8); //Connect a LED widget to Virtual pin 7 in the app
WidgetLED led2(V9); //Connect a LED widget to Virtual pin 7 in the app
WidgetLED led3(V10); //Connect a LED widget to Virtual pin 7 in the app
WidgetLED led4(V11); //Connect a LED widget to Virtual pin 7 in the app
WidgetLED led5(V40); //Connect a LED widget to Virtual pin 7 in the app

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

// Variables: pH set to 10 by default to avoid CO2 valves activting on arduino boot
float Po1;
float pH1 = 10;
float Po2;
float pH2 = 10;
float Po3;
float pH3 = 10;
float Po4;
float pH4 = 10;

#include <NewPing.h> //using NewPing sonar library
 

#define TRIGGER_PIN  12
#define ECHO_PIN     13
#define MAX_DISTANCE 200 //defines max measuring distance

#define LEVEL_DIFFERENCE_FOR_1L 1
int water_adding_valve_pin[] = {2, 3, 4, 5};  // array of pin-numbers, where algae-valves  are connected
int current_tank = 0; //index of the pin from water_adding_valve_pin-array, which is used now for the water-adding
int prev_level ;
int valve_close_timer_ID; // Timer that is responsible for the function of closing of valve

#define VALVE_K_RELAY_PIN 15 //needed to be replaced by actual pin-number

#define FS_A_MIN 5 // defined floatswithces (random pins)
#define FS_A_MAX 6
#define FS_B_MIN 7
#define FS_B_MAX 8

#define PUMP_A 9
#define PUMP_B 10 


NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void turb_control(){ 
    int turbidity = get_turbidity();
    if (turbidity < turbsetpoint){
	prev_level = sonar.ping_cm();
	digitalWrite(water_adding_valve_pin[current_tank], HIGH);
	timer.enable(valve_close_timer_ID);
    }
}

int get_rurbidity(){ // To be continued...
	int turb = analogRead(TURBIDITY_SENSOR_PIN);
	return turb;
}

void close_valve (){
    int curr_level = sonar.ping_cm();
    if ((curr_level - prev_level) >= LEVEL_DIFFERENCE_FOR_1L){
	digitalWrite(water_adding_valve_pin[current_tank], LOW);
	timer.disable(valve_close_timer_ID);
	switch_tank();
    }
}

void switch_tank(){
    if (current_tank == 0)
	current_tank = 1;  
  else if (current_tank == 1)
	current_tank = 0;
  else if (current_tank == 2)
	current_tank = 3;
   else if (current_tank == 3)
	current_tank = 2;
}

void maxlevel_K_tank() { // "void" because we don't expect result in physical world, but only in virtual
   int volume = map(sonar.ping_cm(), 36, 3, 29, 70); //measurement of water level
   if (volume >= 56){
       digitalWrite(VALVE_K_RELAY_PIN, HIGH);  // If I need to open appropriate valve, then I need to apply voltage to the corresponding relay
   }
}

void maxlevel_A_tank() { 
	if (digitalRead(FS_A_MAX) == HIGH) { // If FS read is high then...
		digitalWrite(VALVE_K_RELAY_PIN, LOW);
		digitalWrite(PUMP_A, HIGH);
	}
}

void minlevel_A_tank() {
	if (digitalRead(FS_A_MIN) == LOW) {
		digitalWrite(PUMP_A, LOW);
	}
}

void maxlevel_B_tank() {
	if (digitalRead(FS_B_MAX) == HIGH) {
		digitalWrite(PUMP_B, HIGH);
	}
}

void minlevel_B_tank() {
	if (digitalRead(FS_B_MIN) == LOW) {
		digitalWrite(PUMP_B, LOW);
	}
}


BLYNK_WRITE(0){
  algaecase = param.asInt(); 
}

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


void measure() { //must recode for flow logic

    //if (digitalRead (switchPin) == LOW)
     //{
     //Serial.println ("Switch closed.");
     //delay (1000); 
     //} // end if switchState is LOW
     //if (digitalRead (switchPin) == HIGH)
    // {
    // Serial.println ("Switch OPEN.");
     //delay (1000); 
     // end if


Blynk.virtualWrite(V5, sonar.ping_cm());
volume = map(sonar.ping_cm(), 36, 3, 29, 70);
//Serial.print("Ping: ");
//Serial.print(sonar.ping_cm());
//Serial.println("cm");

//digitalWrite(trigPin, LOW); //make own timer for this?



//lcd.setCursor(0,0); // Sets the location at which subsequent text written to the LCD will be displayed
//lcd.print("Bopper: "); // Prints string "Distance" on the LCD // Bopper mean "floatswitch"
//lcd.print(switchPin); // Prints the distance value from the sensor
//lcd.print(" cm");
//lcd.setCursor(0,1);
//lcd.print("Volume");
//lcd.print(volume);
//lcd.print("L");
//Serial.println(duration);
	

}
// modified map function for float values. Now we don't need an additional variables (pHm1,pHm2,pHm3,pHm4)
float map_float(float value,float fromLow, float fromHigh, float toLow, float toHigh){
	return (toLow + (value - fromLow) * ((toHigh - toLow) / (fromHigh - fromLow)));	
	//float k = (toHigh - toLow) / (fromHigh - fromLow);
	//float len_v = value - fromLow;
	//float new_len = len_v * k;
	//return (toLow + new_len);	
}  

void ph(){
  Po1 = (1023 - analogRead(pHpin1)); // it is done convert value from analogue sensor to 
  Po2 = (1023 - analogRead(pHpin2));
  Po3 = (1023 - analogRead(pHpin3));
  Po4 = (1023 - analogRead(pHpin4));
  //Serial.print(Po1); //This is the raw voltage value for the pH module
  //Serial.print(Po2); //This is the raw voltage value for the pH module
  //Serial.print(Po3); //This is the raw voltage value for the pH module
  //Serial.print(Po4); //This is the raw voltage value for the pH module
   //Calibration values:
   //405@pH7 // These values were checked/calibrated manually (we need to do it to read values from analogue sensors correctly 
   //290@ph4

  //Serial.print(", ph =");
  float pH1 = map_float(Po1, 290, 406, 4, 7);
  float pH2 = map_float(Po2, 290, 406, 4, 7);
  float pH3 = map_float(Po3, 290, 406, 4, 7);
  float pH4 = map_float(Po4, 290, 406, 4, 7);
  //float pHm1 = map(Po1, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  //float pH1 = (pHm1/100);
  //float pHm2 = map(Po2, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  //float pH2 = (pHm2/100);
  //float pHm3 = map(Po3, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  //float pH3 = (pHm3/100);
  //float pHm4 = map(Po4, 290, 406, 400, 700); //maps voltage(Po) from calibration values at 4.00 and 7.00 pH
  //float pH4 = (pHm4/100);
  //Serial.println(pH, 2); 
  //lcd.setCursor(0, 0);
  //lcd.print("pH: ");  //Print pH value to the LCD
  //lcd.setCursor(3, 0);
  //lcd.print(pH);
  //lcd.setCursor(0, 1);
  //lcd.print("Setpoint:");
  //Serial.println(", setpoint=");
  //Serial.println((encoderPosCount1));
  //Serial.println((encoderPosCount2));
  //Serial.println((encoderPosCount3));
  //Serial.println((encoderPosCount4));
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



void blynker1() { //Writes the setpoint value to a gague widget.Connect the gague widget to virtual pin 1: for å vise på skjermen hva setpoint er
    Blynk.virtualWrite(V21, (encoderPosCount1/10));  
    Blynk.virtualWrite(V22, (encoderPosCount2/10));
    Blynk.virtualWrite(V23, (encoderPosCount3/10));
    Blynk.virtualWrite(V24, (encoderPosCount4/10));   
}


void algaetank(){
  
  switch (algaecase) {
    case 1:
     if (turbidity < turbsetpoint);
{
  digitalWrite(relayPin5, HIGH); 
  //finn kode for riktig "delay" ikke delay(): teller mens det går (teller millis)
  ++valveoutput; //må kode for riktig pin relaypin skal være valveoutput
  //led3.on();
}


      break;
    case 2:
      //do something when var equals 2
      break;
    default: 
      // if nothing else matches, do the default
      // default is optional
    break;
  }
}

//alternative to delay()
void Blynk_Delay(int milli){
   int end_time = millis() + milli;
   while(millis() < end_time){
    led5.on();
    led5.off();
       
       yield();
   }
}

void setup(){
  Serial.begin(9600); // initialize serial communications at 9600 bps
  
  pinMode (switchPin1, INPUT_PULLUP); //Float switch pins
  pinMode (switchPin2, INPUT_PULLUP); //Float switch pins
  pinMode (switchPin3, INPUT_PULLUP); //Float switch pins
  pinMode (switchPin4, INPUT_PULLUP); //Float switch pins

  //lcd.begin(16, 2); // set up the LCD's number of columns and rows: 
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  //pinMode(inPin, INPUT_PULLUP);
  //pinMode(13, OUTPUT);
  //if (ethbutton == LOW) {
  Blynk.begin(Serial, auth);
  //Blynk.begin(auth, "blynk-cloud.com");
  timer.setInterval(2100, blynker1); //the number for the timers sets the interval for how frequently the function is called. Keep it above 1000 to avoid spamming the server.
  timer.setInterval(24000, ph);
  timer.setInterval(25000, measure);
  timer.setInterval(120000, algaetank);
  
  //timer.setInterval(2000, setpointwriter);
  valve_close_timer_ID = timer.setInterval(500, close_valve); // Need to get a timer ID which is respnsible for closing of valve
  timer.disable(valve_close_timer_ID);
	
}


void loop(){
  Blynk.run(); 
  timer.run();
  //encoder();
}

