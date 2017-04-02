//Change serial to debug serial // 
//The ethernet button is used to tell the arduino board if it's connected to the internet through the ethernet cable or not. If the button is turned off, then it won't spend time trying to connect to the server.
//int inPin = 22; //Just wire this pin dirrectly to GND if you want to leave ethernet constantly on. If you want to test the code before connecting to the servers, leave the pin disconnected.
//int ethbutton = digitalRead(inPin); 

/// LIBRARIES THAT ARE IN USE (".h" is an abbreviation for header)
//#include <DHT.h> (temperature and humidity sensors)
//#include <SPI.h> // Library for work with devices that support SPI (serial periferal interface) 

//#include <LiquidCrystal.h>//LCD library
// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7); Here we create an object called lcd (object type is LiquidCrystal).

#include <SimpleTimer.h> //here is the SimpleTimer library
#include <NewPing.h> //using NewPing sonar library
#include <FlowMeter.h> //using a library for the flowmeter
#include <SoftwareSerial.h> 
#include <BlynkSimpleStream.h>

/// PINS THAT ARE USED IN THIS PROGRAM (Fritzing!) Most of the pins are random. We need to find out relevant pins.

#define TURBIDITY_SENSOR_PIN A1 //analogue
#define FLOWMETER_PIN 21 // We need to use one of six interrupt pins. For Arduino Mega it is: 2,3,21,20,19,18

#define TRIGGER_PIN  12 // distance measurer
#define ECHO_PIN     13 // distance measurer

#define CO2_VALVE_AL1_PIN 2
#define CO2_VALVE_AL2_PIN 3
#define CO2_VALVE_AL3_PIN 4
#define CO2_VALVE_AL4_PIN 5 //Connect to the relay that will control the power to a pH regulating pump or valve

#define WATER_VALVE_AL1_PIN 6
#define WATER_VALVE_AL2_PIN 7
#define WATER_VALVE_AL3_PIN 8
#define WATER_VALVE_AL4_PIN 9

#define VALVE_K_RELAY_PIN 15 //needed to be replaced by actual pin-number // valve under K-tank

#define FS_A_MIN 5 // defined floatswithces (random pins)
#define FS_A_MAX 6
#define FS_B_MIN 7
#define FS_B_MAX 8
#define FS_AL1_MAX 15 //Floatswitches for algae tanks.
#define FS_AL2_MAX 15
#define FS_AL3_MAX 15
#define FS_AL4_MAX 15

#define PUMP_A 9
#define PUMP_B 10 

#define PH1_PIN A8
#define PH2_PIN A9
#define PH3_PIN A10
#define PH4_PIN A11  // Connect the sensor's Po output to an analogue pin - whichever one you choose
#define PH_K_TANK_PIN A12 //pH sensor in K-tank

/// THINGS THAT ARE RELEVANT TO BLYNK
//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space


char auth[] = "x"; //Insert auth token between the " "
//LED widgets:
WidgetLED led1(V8); //Connect a LED widget to Virtual pin 8 in the app.
WidgetLED led2(V9); //
WidgetLED led3(V10); //
WidgetLED led4(V11); //
WidgetLED led5(V12); //



/// Constants // here we use #define to write down constants. Same is done for pins, but these are constants that are not pins. These are macros.

#define MAX_DISTANCE 200 //defines max measuring distance in K-tank for ultrasonic sensor
#define MAX_WATER_VOLUME_K_TANK 56 // In liters. Than process will start when it is 50+6 liter
#define PERIOD_TO_CHECK_FLOWMETER 500 //check of volume for closing of valve
 
#define TURB_MEASURE_NUMBER 10 // How many times turbidity will be measured before comparing to the value named turbsetpoint
float STANDARD_TAKEOUT_FROM_AL = 1.0;// standard amount of water that is taken from AL-tank before switching to a next one. Can be defined with floating point number
/// VARIABLES 

SimpleTimer timer; // Create a Timer object called "timer"!
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //object controlling ultrasonic distance measuerer
FlowMeter Meter = FlowMeter(FLOWMETER_PIN, FS300A); // Constructor always have the same name as class
//Best to write in specific modell (after pin), namely FS300A 

float encoderPosCount1 = 7.8; //default setpoint at startup set to 7.8 (is correct that it's 78 and not 7.8 because slider can only work with integers (whole numbers)?
float encoderPosCount2 = 7.8; //default setpoint at startup set to 7.8 (in these lines this variable is beeing created)
float encoderPosCount3 = 7.8; 
float encoderPosCount4 = 7.8; 
float encoderPosCount5 = 7.8; //default set point
float encoderPosCount6 = 7.8; 
// We need to find the border values for copepods and set the inn for PosCount5 and PosCount6.

// Variables: pH set to 10 by default to avoid CO2 valves activting on arduino boot
// Here variables were declared
float pH1 = 10;
float pH2 = 10;
float pH3 = 10;
float pH4 = 10;
float pH_K_tank = 10;

int water_adding_valve_pin[] = {WATER_VALVE_AL1_PIN, WATER_VALVE_AL2_PIN, WATER_VALVE_AL3_PIN, WATER_VALVE_AL4_PIN}; // array of pin-numbers, where algae-valves  are connected
int current_tank = 0; //index of the pin from water_adding_valve_pin-array, which is used now for the water-adding.
// int current_tank = 0 or 2 is going to be used once in two weeks (only to initialize the process of tankswitching)
int prev_level;
int valve_close_timer_ID; // Timer that is responsible for the function of closing of valve

int turbsetpoint = 1000;
int turbidity_measure_counter = 0; //I need a variable that is going to serve as a measurments counter
int total_turbidity_measurments = 0; //Here we are going to store a total number of turbidity measurments

boolean allow_work_of_K_valve = true;
int algae_FS_pin[] = {FS_AL1_MAX, FS_AL2_MAX, FS_AL3_MAX, FS_AL4_MAX}; // We create an array of constants that will contain the pins of all four FS in algaetanks



///Functions (there are always brackets after the function, f(x), brackets can also be empty)



void turb_control(){ 
    int turbidity = analogRead(TURBIDITY_SENSOR_PIN);
    if (turbidity_measure_counter < TURB_MEASURE_NUMBER){
    total_turbidity_measurments += turbidity; // += means "add to this variable" there are also -=, /= and *=
    turbidity_measure_counter++; // this is an increment operator "++"
  }
    else {
    if ((total_turbidity_measurments / TURB_MEASURE_NUMBER) < turbsetpoint){ //we get an average of ten measurments and compare it to setpoint
      Meter.reset(); // Here we reset the turn count for flowmeter and in the next line we open the valve.
      digitalWrite(water_adding_valve_pin[current_tank], HIGH);
      timer.enable(valve_close_timer_ID);
    }
    total_turbidity_measurments = 0; //Set value to zero in order to get a new total for next ten measurments
    turbidity_measure_counter = 0; //Set value to zero in order to start a new measurment cycle
  }
}


void close_valve (){
   Meter.tick(PERIOD_TO_CHECK_FLOWMETER); // we call method "tick" from an object "meter" // Tick is prosessing of data. Here signals are converted to liters.
   double current_volume_flowmeter = Meter.getCurrentVolume(); // here we setup a local variable. It's born in this frame. And it's gonna die here too.
   // And this method works because it belongs to an object Meter (which is an instance of a class that is a part of a library (flowmeter.h) that we connected.
    if (current_volume_flowmeter >= STANDARD_TAKEOUT_FROM_AL){
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
	// /Measure water level, and if it is sufficient, open the valve under K-tank
   int volume = map(sonar.ping_cm(), 36, 3, 29, 70); //measurement of water level
   if ((volume >= MAX_WATER_VOLUME_K_TANK) && allow_work_of_K_valve){ // To ampersands mean a logical "and" // It causes both conditions to be fulfilled
       digitalWrite(VALVE_K_RELAY_PIN, HIGH);  // If I need to open appropriate valve, then I need to apply voltage to the corresponding relay
   }
}

void maxlevel_A_tank() { 
  // If the upper float switch in the tank-A is closed, close the valve and switch on the pump in the tank-A
  if (digitalRead(FS_A_MAX) == LOW) { // If FS read is high then...
    digitalWrite(VALVE_K_RELAY_PIN, LOW);
    digitalWrite(PUMP_A, HIGH);
  }
}

void minlevel_A_tank() {
  // If the lower float swtich in the tank-A is opened, turn off the pump in the tank-A
  if (digitalRead(FS_A_MIN) == HIGH) {
    digitalWrite(PUMP_A, LOW);
  }
}

void maxlevel_B_tank() {
  // If the upper float switch in the tank-B is closed, turn on the pump in the tank-B
  if (digitalRead(FS_B_MAX) == LOW) {
    digitalWrite(PUMP_B, HIGH);
  }
}

void minlevel_B_tank() {
  //If the lower float switch in the tank-B is closed, turn off the pump in the tank-B
  if (digitalRead(FS_B_MIN) == HIGH) { // here "high" means "contact has open"
    digitalWrite(PUMP_B, LOW);
  }
}

void maxlevel_algae_tanks() {
	allow_work_of_K_valve = true;
	for(int al_tank_index = 0; al_tank_index < 4; al_tank_index++){ // Is called cycle with a counter.
		// That is, al_tank_index first turn to the tank zero, then consequently turn to the tanks one, two and three (therefore it is written "> 4")
		if (digitalRead(algae_FS_pin[al_tank_index]) == LOW) { // If the upper float switch in the tank-A is closed, close the valve and switch on the pump in the tank-A
			digitalWrite(VALVE_K_RELAY_PIN, LOW);
			digitalWrite(PUMP_A, LOW);
			digitalWrite(PUMP_B, LOW);
			allow_work_of_K_valve = false;
			// These four lines make it so that we consequently shut down the whole bottom part of RAS, and then forbid to turn it on again
		}	
	} 
}


BLYNK_WRITE(66){ //Changes are set by the actions application itself
  STANDARD_TAKEOUT_FROM_AL = param.asFloat(); 
}

BLYNK_WRITE(12){
  encoderPosCount1 = param.asFloat(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
} //param.asInt() Read the parameter as a whole number
BLYNK_WRITE(13){
  encoderPosCount2 = param.asFloat(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}
BLYNK_WRITE(14){
  encoderPosCount3 = param.asFloat(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}
BLYNK_WRITE(15){
  encoderPosCount4 = param.asFloat(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}
BLYNK_WRITE(16){ // Value is higher than MIN value that is set here
  encoderPosCount5 = param.asFloat(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}
BLYNK_WRITE(17){ // Value is lower than MAX value that is set here
  encoderPosCount6 = param.asFloat(); //Connect a slider widget to virtual pin 3 in the app. Slider range should be 0 to 140.
}





// modified map function for float values. Now we don't need an additional variables (pHm1,pHm2,pHm3,pHm4)
float map_float(float value,float fromLow, float fromHigh, float toLow, float toHigh){
  return (toLow + (value - fromLow) * ((toHigh - toLow) / (fromHigh - fromLow))); 
  //Logic //4 float k = (toHigh - toLow) / (fromHigh - fromLow);
      //3 float len_v = value - fromLow;
      //2 float new_len = len_v * k;
      //1 return (toLow + new_len); 
}  

void ph(){
  pH1 = (1023 - analogRead(PH1_PIN)); // it is done convert value from analogue sensor to 
  pH2 = (1023 - analogRead(PH2_PIN));
  pH3 = (1023 - analogRead(PH3_PIN));
  pH4 = (1023 - analogRead(PH4_PIN));
  pH_K_tank = (1023 - analogRead (PH_K_TANK_PIN));
  
  //Serial.print(Po1); //This is the raw voltage value for the pH module
  //Serial.print(Po2); //This is the raw voltage value for the pH module
  //Serial.print(Po3); //This is the raw voltage value for the pH module
  //Serial.print(Po4); //This is the raw voltage value for the pH module
   //Calibration values:
   //405@pH7 // These values were checked/calibrated manually (we need to do it to read values from analogue sensors correctly 
   //290@ph4

  //Serial.print(", ph =");
  pH1 = map_float(pH1, 290, 406, 4, 7);
  pH2 = map_float(pH2, 290, 406, 4, 7);
  pH3 = map_float(pH3, 290, 406, 4, 7);
  pH4 = map_float(pH4, 290, 406, 4, 7);
  pH_K_tank = map_float(pH_K_tank, 290, 406, 4, 7);

 
 
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
  Blynk.virtualWrite(V5, pH_K_tank);
  
   if (pH1 > (encoderPosCount1))
{
  digitalWrite(CO2_VALVE_AL1_PIN, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led1.on();
}
else
{
  digitalWrite(CO2_VALVE_AL1_PIN, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led1.off();      
  
}
   if (pH2 > (encoderPosCount2)) //This if/else statement turns the valve on if the pH value is above the setpoint, or off if it's below the setpoint. Modify according to your need.
{
  digitalWrite(CO2_VALVE_AL2_PIN, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led2.on();
}
else
{
  digitalWrite(CO2_VALVE_AL2_PIN, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led2.off();      
  
}
   if (pH3 > (encoderPosCount3) //This if/else statement turns the valve on if the pH value is above the setpoint, or off if it's below the setpoint. Modify according to your need.
{
  digitalWrite(CO2_VALVE_AL3_PIN, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led3.on();
}
else
{
  digitalWrite(CO2_VALVE_AL3_PIN, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led3.off();      
  
}
   if (pH4 > (encoderPosCount4)) 
{
  digitalWrite(CO2_VALVE_AL4_PIN, HIGH); 
  //lcd.setCursor(8, 0);
  //lcd.print("CO2:ON*");
  led4.on();
}
else
{
  digitalWrite(CO2_VALVE_AL4_PIN, LOW); 
 // lcd.setCursor(8, 0);
  //lcd.print("CO2:OFF");
  led4.off();      
  
}
  if ((pH_K_tank > (encoderPosCount5)) && (pH_K_tank < (encoderPosCount6))) // To show if pH is within reasonable values 
{ 
  led5.off();
}
else
{
  led5.on();      
  
}
}




/* void blynker1() { //Writes the setpoint value to a gague widget.Connect the gague widget to virtual pin 1: to show on the screen what is the setpoint
    Blynk.virtualWrite(V21, (encoderPosCount1/10));  
    Blynk.virtualWrite(V22, (encoderPosCount2/10));
    Blynk.virtualWrite(V23, (encoderPosCount3/10));
    Blynk.virtualWrite(V24, (encoderPosCount4/10));   
}*/

void MeterISR() {
  // let our flow meter count the pulses
  // https://github.com/sekdiy/FlowMeter/blob/master/examples/Simple/Simple.ino 
  Meter.count();
}

void setup(){
  Serial.begin(9600); // initialize serial communications at 9600 bps
  
//pinMode(FLOWMETER_PIN // we have it as attachInterrupt(2, MeterISR, RISING) at the end of void setup()
pinMode(TRIGGER_PIN, OUTPUT);         // distance measurer
pinMode(ECHO_PIN, INPUT);            // distance measurer
pinMode(CO2_VALVE_AL1_PIN, OUTPUT);
pinMode(CO2_VALVE_AL2_PIN, OUTPUT);
pinMode(CO2_VALVE_AL3_PIN, OUTPUT);
pinMode(CO2_VALVE_AL4_PIN, OUTPUT); //Connect to the relay that will control the power to a pH regulating pump or valve
pinMode(WATER_VALVE_AL1_PIN, OUTPUT);
pinMode(WATER_VALVE_AL2_PIN, OUTPUT);
pinMode(WATER_VALVE_AL3_PIN, OUTPUT);
pinMode(WATER_VALVE_AL4_PIN, OUTPUT);
pinMode(VALVE_K_RELAY_PIN, OUTPUT); //needed to be replaced by actual pin-number // valve under K-tank
pinMode(FS_A_MIN, INPUT_PULLUP);          // defined floatswithces (random pins)
pinMode(FS_A_MAX, INPUT_PULLUP);
pinMode(FS_B_MIN, INPUT_PULLUP);
pinMode(FS_B_MAX, INPUT_PULLUP);
pinMode(FS_AL1_MAX, INPUT_PULLUP); //Floatswitches for algae tanks
pinMode(FS_AL2_MAX, INPUT_PULLUP);
pinMode(FS_AL3_MAX, INPUT_PULLUP);
pinMode(FS_AL4_MAX, INPUT_PULLUP);
pinMode(PUMP_A, OUTPUT);
pinMode(PUMP_B, OUTPUT); 
// Analog channels are not configurable for INPUT or OUTPUT. It is only relevant for digital pins
// Howerver, analog pins can be configured to work as digital pins.

 
  //lcd.begin(16, 2); // set up the LCD's number of columns and rows: 
  //if (ethbutton == LOW) {
  Blynk.begin(Serial, auth);
  //Blynk.begin(auth, "blynk-cloud.com");
  //timer.setInterval(2100, blynker1); //This timer has updated a new value of the standard setpoint every two seconds
  //the number for the timers sets the interval for how frequently the function is called. Keep it above 1000 to avoid spamming the server.
  
  timer.setInterval(24000, ph);
  //timer.setInterval(25000, measure);
  timer.setInterval (60000, turb_control); // timer will call a turb_control function every minute
  
  valve_close_timer_ID = timer.setInterval(PERIOD_TO_CHECK_FLOWMETER, close_valve); // Getting a timer ID which is responsible for closing of valve
  //Every half second it calls a function close_valve
  timer.disable(valve_close_timer_ID);


  // enable a call to the 'interrupt service handler' (ISR) on every rising edge at the interrupt pin
  // do this setup step for every ISR you have defined, depending on how many interrupts you use
  attachInterrupt(2, MeterISR, RISING); //If interrupt pin is changed then this constant has to be changed to.
  // sometimes initializing the gear generates some pulses that we should ignore
  Meter.reset();


 //timer.setInterval(120000, algaetank);
 //timer.setInterval(2000, setpointwriter); //timer is triggered (turned on) by the fact that valve open. Then it starts to check every half second if necessary amount of water passed through.
}


void loop(){
  Blynk.run(); 
  timer.run(); //For those functions that are registered in the loop, they are called with each tact, that is we don't need to assign a specific set.interval function
	maxlevel_K_tank(); // However we still need to write those functions in void.loop, namely:
	maxlevel_A_tank(); 
	minlevel_A_tank();
	maxlevel_B_tank();
	minlevel_B_tank();
	maxlevel_algae_tanks();
}


/* void measure() { //must recode for flow logic
  
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
int volume = map(sonar.ping_cm(), 36, 3, 29, 70);
//Serial.print("Ping: ");
//Serial.print(sonar.ping_cm());
//Serial.println("cm");

//digitalWrite(TRIGGER_PIN, LOW); //make own timer for this?

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
*/

//int pinA = 40;  // Connected to CLK on KY-040 Rotary Encoder
//int pinB = 42;  // Connected to DT on KY-040 Rotary Encoder
//SW(button) pin on KY-040 is not used

//int pinALast;  
//int aVal;
//boolean bCW;

