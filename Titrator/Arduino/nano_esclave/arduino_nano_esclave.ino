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
  Serial.begin(57600);
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  start_loadCell();
}

void loop() {
  pounds();
  ph_value();
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
  delay(1000);
  tare1();
  tare2();
}

void ph_value() {
  int buffer_arr[30];
  unsigned long int avgval = 0;

  // Lire les valeurs analogiques
  for (int i = 0; i < 30; i++) {
    buffer_arr[i] = analogRead(A0);
//   delay(30);
  }

  // Tri à bulles
  for (int i = 0; i < 29; i++) {
    for (int j = 0; j < 29 - i; j++) {
      if (buffer_arr[j] > buffer_arr[j + 1]) {
        int temp = buffer_arr[j];
        buffer_arr[j] = buffer_arr[j + 1];
        buffer_arr[j + 1] = temp;
      }
    }
  }

  // Calcul de la moyenne des valeurs médianes
  for (int i = 10; i < 21; i++) {
    avgval += buffer_arr[i];
  }
  
  float volt = (float)avgval * 5.0 / 1024 / 12;
  ph_act = 2.88 * volt - 0.31;
  if (ph_act <= 0){
    ph_act = 0;
  }
  else if (ph_act >= 14){
    ph_act = 14;
  }
}


float estab_pound(int a){
  float buffer_arr[30];
  float avgval = 0;

  if (a == 1){
    for (int i = 0; i < 30; i++) {
      LoadCell1.update();
      buffer_arr[i] = LoadCell1.getData();
    }
  }
  else if (a == 2){
    for (int i = 0; i < 30; i++) {
      LoadCell2.update();
      buffer_arr[i] = LoadCell2.getData();
    }
  }

  // Tri à bulles
  for (int i = 0; i < 29; i++) {
    for (int j = 0; j < 29 - i; j++) {
      if (buffer_arr[j] > buffer_arr[j + 1]) {
        float temp = buffer_arr[j];
        buffer_arr[j] = buffer_arr[j + 1];
        buffer_arr[j + 1] = temp;
      }
    }
  }

  // Calcul de la moyenne des valeurs médianes
  for (int i = 0; i < 30; i++) {
    avgval += buffer_arr[i];
  }
  return (avgval/30);
}

void pounds() {
  poids1 = estab_pound(1);
  Serial.print("poids1  :");
  Serial.println(poids1);
  if (poids1 <= 0){
   poids1 = 0;
  }
  poids2 = estab_pound(2);
  Serial.print("poids2  :");
  Serial.println(poids2);
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
