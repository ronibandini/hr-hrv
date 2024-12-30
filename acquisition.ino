// Data Forwarding acquisition script
// Nov-2024 MIT License
// Roni Bandini, @RoniBandini

const int pinLoMax=3;
const int pinLoMin=2;

#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

static unsigned long last_interval_ms = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(pinLoMax, INPUT); // leads off detection LO +
  pinMode(pinLoMin, INPUT); // leads off detection LO -

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

      if((digitalRead(pinLoMax) != 1) and (digitalRead(pinLoMin) != 1)){
          digitalWrite(LEDG, LOW); 
          digitalWrite(LEDR, HIGH);   
          Serial.println(analogRead(A0));
      }
      else{
        digitalWrite(LEDR, LOW); 
        digitalWrite(LEDG, HIGH);  
      }

  }
}