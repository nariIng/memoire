#include <Wire.h>
#include <EEPROM.h>
#include <HX711_ADC.h>

#define HX711_DATA_PIN_1  2
#define HX711_CLOCK_PIN_1 3
#define HX711_DATA_PIN_2  4
#define HX711_CLOCK_PIN_2 5

#define I2C_ADDRESS 8

HX711_ADC LoadCell1(HX711_DATA_PIN_1, HX711_CLOCK_PIN_1);
HX711_ADC LoadCell2(HX711_DATA_PIN_2, HX711_CLOCK_PIN_2);

float poids1 = 0, poids2 = 0, ph_act = 0;
float calfactor_1, calfactor_2;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  start_loadCell();
}

void loop() {
  pounds();
  ph_value();
  delay(500);
}

void requestEvent() {
  // Formater les poids avec 2 décimales
  char buffer[30];
  char buffer1[10];
  char buffer2[10];
  char buffer3[10];
  dtostrf(poids1, 6, 1, buffer1);
  dtostrf(poids2, 6, 1, buffer2);
  dtostrf(ph_act, 6, 2, buffer3);
  snprintf(buffer, sizeof(buffer), "%s,%s,%s", buffer1, buffer2, buffer3);

  // Débogage des valeurs envoyées
  Serial.print("Envoi: ");
  Serial.println(buffer);

  // Envoyer les données sous forme de chaîne
  Wire.write((const uint8_t*)buffer, strlen(buffer) + 1); // Inclure le caractère null '\0'
}

void receiveEvent(int numBytes){
  char  command = Wire.read();
  if (command == '1'){
    tare1();
  }
  else if (command == '2'){
    tare2();
  }
  else if (command == '3'){
    calibPH();
  }
}

void start_loadCell() {
  LoadCell1.begin();
  LoadCell2.begin();
  LoadCell1.start(2000);
  LoadCell2.start(2000);

  EEPROM.get(0, calfactor_1);
  EEPROM.get(4, calfactor_2);
  LoadCell1.setCalFactor(calfactor_1);
  LoadCell2.setCalFactor(calfactor_2);
}

void ph_value() {
  int buffer_arr[10];
  unsigned long int avgval = 0;

  // Lire les valeurs analogiques
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(A0);
    delay(30);
  }

  // Tri à bulles
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9 - i; j++) {
      if (buffer_arr[j] > buffer_arr[j + 1]) {
        int temp = buffer_arr[j];
        buffer_arr[j] = buffer_arr[j + 1];
        buffer_arr[j + 1] = temp;
      }
    }
  }

  // Calcul de la moyenne des valeurs médianes
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }
  
  float volt = (float)avgval * 5.0 / 1024 / 6;
  ph_act = 2.88 * volt - 0.31;
  if (ph_act <= 0){
    ph_act = 0;
  }
  else if (ph_act >= 14){
    ph_act = 14;
  }
}


void pounds() {
  LoadCell1.update();
  LoadCell2.update();
  poids1 = LoadCell1.getData();
  if (poids1 <= 0){
   poids1 = 0;
  }
  poids2 = LoadCell2.getData();
  if (poids2 <= 0){
   poids2 = 0;
  }
}

void tare1(){
  LoadCell1.tare(); // Définir la tare pour LoadCell1
  Serial.println("Tare1 completed");
}

void tare2(){
  LoadCell2.tare(); // Définir la tare pour LoadCell2
  Serial.println("Tare2 completed");
}

void calibPH(){
  Serial.println("calibration du pHmetre");
}
