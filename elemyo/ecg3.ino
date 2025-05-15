// Stress detection with ECG sensor and Edge Impulse HR/HRV block 
// MIT License, January 2025
// Roni Bandini, @RoniBandini
// Arduino Portenta H7 
// ECG Hardware Elemyo Muscle Sensor https://elemyo.com/support/start_info/myo_v1_5_arduino
// Note: this is an adaptation of the original code I wrote for Adafruit AD8232

#include <arduino-hrv.h>
#include <ELEMYO.h>

#define   CSpin         7
#define   sensorInPin   A1     
int signalReference = 524;    // reference of signal, 2.5 V for MYO, MYO-kit, BPM, BPM-kit

ELEMYO MyoSensor(CSpin);


// Axis array
float features[2000];

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

// settings
int   myCounter=0;
float threesold=0.85;
int myDelay=50;

const int pinLoMax=3;
const int pinLoMin=2;

void setup()
{

    pinMode(LEDG, OUTPUT);
    pinMode(LEDR, OUTPUT);

    MyoSensor.gain(x1);           // initial value of gain: x1; x2; x4; x5; x8; x10; x16; x32
    pinMode(sensorInPin, INPUT);   // initialisation of sensorInPin

    Serial.begin(115200);
    Serial.println("Stress detection started");
    Serial.println("Roni Bandini, May 2025");
    Serial.println("------------------------------------");

    digitalWrite(LEDG, LOW);


}


void loop()

{
    
  int sensorValue = analogRead(sensorInPin);              
  sensorValue = MyoSensor.BandStop(sensorValue, 50, 4);   // notch 50 Hz filter with band window 4 Hz
  sensorValue = MyoSensor.BandStop(sensorValue, 100, 6);  // notch 100 Hz (one of 50 Hz mode) filter with band window 6 Hz
  
  // moving average transformation with 0.8 smoothing constant.
  int sensorValueMA = MyoSensor.movingAverage(sensorValue, signalReference,  0.8); 
  
  Serial.print(sensorValue);    
  Serial.print(" ");
  Serial.println(sensorValueMA + signalReference); 
  
  features[myCounter] = sensorValue;
    
  myCounter=myCounter+1;

    if (myCounter==2000) myCounter=0;
           

        if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
            delay(1000);
            return;
        }
    
        ei_impulse_result_t result = { 0 };
    
        // the features are stored into flash, and we don't want to load everything into RAM
        signal_t features_signal;
        features_signal.total_length = sizeof(features) / sizeof(features[0]);
        features_signal.get_data = &raw_feature_get_data;
    
        // invoke the impulse
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
        ei_printf("run_classifier returned: %d\n", res);
    
        if (res != 0) return;
    
        // print predictions
        ei_printf("Predictions ");
        ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");
        ei_printf("[");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("%.5f", result.classification[ix].value);
          #if EI_CLASSIFIER_HAS_ANOMALY == 1
                  ei_printf(", ");
          #else
                  if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
                      ei_printf(", ");
                  }
          #endif
              }
          #if EI_CLASSIFIER_HAS_ANOMALY == 1
              ei_printf("%.3f", result.anomaly);
          #endif
              ei_printf("]\n");
    
          // human-readable predictions
          for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
              ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);

            // check threshold
            if (float(result.classification[ix].value)>threesold and result.classification[ix].label=="stress")
            {
              ei_printf("----------------------------------------- STRESS");  
	            digitalWrite(LEDR, LOW); 
              digitalWrite(LEDG, HIGH);  

             
            }
            else
            {
              ei_printf("-");        
              digitalWrite(LEDG, LOW);       
              digitalWrite(LEDR, HIGH);                

            } 
       } //for
                      

          
          #if EI_CLASSIFIER_HAS_ANOMALY == 1
              ei_printf("    anomaly score: %.3f\n", result.anomaly);
          #endif            
           
    delay(myDelay);

}

void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0) {
        Serial.write(print_buf);
    }
}