const int relayPin1 = 4; //the base of the transistor attach to
int floatswitch = 7;

// lowest and highest sensor readings:
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum

void setup() {
pinMode(floatswitch,INPUT_PULLUP);
pinMode(relayPin1, OUTPUT);
Serial.begin(9600);

}

void loop() {
 int x = digitalRead(floatswitch);
 Serial.println(x);
 if(x>0.5)
 {
 digitalWrite(relayPin1, LOW);
 delay(10000); 
 }
 else
 {
  digitalWrite(relayPin1, HIGH);
 }
 
 // read the sensor on analog AX:
  int sensorReading = analogRead(A7);
  // map the sensor range (four options):
  // ex: 'long int map(long int, long int, long int, long int, long int)'
  int range = map(sensorReading, sensorMin, sensorMax, 0, 3);
  
  // range value:
  switch (range) {
 case 0:    // Sensor getting wet
    Serial.println("Flood");
    break;
 case 1:    // Sensor getting wet
    Serial.println("Rain Warning");
    break;
 case 2:    // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
    Serial.println("Not Raining");
    break;
  }
  delay(1);  // delay between reads
}
