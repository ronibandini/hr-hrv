// Stress detection with ECG sensor and Edge Impulse HR/HRV block 
// MIT License, November 2024
// Roni Bandini, @RoniBandini

#include <Heart_Rate_Analysis_inferencing.h>


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
    pinMode(pinLoMax, INPUT); // Setup for leads off detection LO +
    pinMode(pinLoMin, INPUT); // Setup for leads off detection LO -

    Serial.begin(115200);
    Serial.println("Stress detection started");
    Serial.println("Roni Bandini, November 2024");
    Serial.println("------------------------------------");

    digitalWrite(LEDG, LOW);


}


void loop()

{
    
    // ECG sensor
    if((digitalRead(pinLoMax) != 1) and (digitalRead(pinLoMin) != 1)){
          Serial.println(analogRead(A0));
          features[myCounter] = analogRead(A0);
    }

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

            if (float(result.classification[ix].value)>threesold and result.classification[ix].label=="stress")
            {
              ei_printf("STRESS");  
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
