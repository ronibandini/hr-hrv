// Data Forwarding acquisition script
// Nov-2024 MIT License
// Roni Bandini, @RoniBandini
// Board Arduino Portenta H7 
// ECG Hardware Elemyo Muscle Sensor https://elemyo.com/support/start_info/myo_v1_5_arduino
// Note: this is an adaptation of the original code I wrote for Adafruit AD8232

#include <ELEMYO.h>

#define   CSpin         7
#define   sensorInPin   A1     
int signalReference = 524;    // reference of signal, 2.5 V for MYO, MYO-kit, BPM, BPM-kit

ELEMYO MyoSensor(CSpin);

#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

static unsigned long last_interval_ms = 0;

void setup() {

  Serial.begin(115200);

  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);

 MyoSensor.gain(x1);           // initial value of gain: x1; x2; x4; x5; x8; x10; x16; x32
  pinMode(sensorInPin, INPUT);   // initialisation of sensorInPin

  digitalWrite(LEDG, LOW);  
  delay(1000);  
  digitalWrite(LEDG, HIGH);  
  delay(1000);

  digitalWrite(LEDR, LOW);  
  delay(1000);  
  digitalWrite(LEDR, HIGH);  
  delay(1000);  

}

void loop() {
  

  if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();
      
        digitalWrite(LEDG, LOW); 
        digitalWrite(LEDR, HIGH);   
        int sensorValue = analogRead(sensorInPin);              
        sensorValue = MyoSensor.BandStop(sensorValue, 50, 4);   // notch 50 Hz filter with band window 4 Hz
        sensorValue = MyoSensor.BandStop(sensorValue, 100, 6);  // notch 100 Hz (one of 50 Hz mode) filter with band window 6 Hz
          
        Serial.println(sensorValue); 
 
      

  }
}