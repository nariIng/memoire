#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define NANO_ADDRESS 8
#define IN1 27
#define IN2 14
//#define IN3 25
//#define IN4 13
#define ENA 26
//#define ENB 

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
WiFiServer server(80); // Serveur HTTP

const char *ssid = "esp_gpci";
const char *password = "gpci2025";

char  buffer[30];
float poids1 = 0, poids2 = 0, pH = 0;

int fast = 100;

void setup() {
  WiFi.softAP(ssid, password);
  
  Serial.begin(115200);
  Wire.begin();
  server.begin();
  lcd.begin(16, 2);
  
  Serial.println("ESP32 Access Point started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Bonjour!!");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Initialisation du moteur : arrêt
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0); // PWM à 0% (moteur arrêté)
  
  delay(2000);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    
    Wire.requestFrom(NANO_ADDRESS, sizeof(buffer));
    if (Wire.available()) {
      size_t len = Wire.readBytesUntil('\0', buffer, sizeof(buffer));
      buffer[len] = '\0'; 
      sscanf(buffer, "%f,%f,%f", &poids1, &poids2, &pH);
      Serial.print("poids_1 : ");
      Serial.println(poids1);
      Serial.print("poids_2 : ");
      Serial.println(poids2);
      Serial.print("poids_pH : ");
      Serial.println(pH);
    }
      // Demander le pH
      if (request.indexOf("GET /ph") != -1) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();
          String ph = String(pH);
          client.println(ph); // Retourner la valeur du pH
      }
      
      // Demander le vol_1
      if (request.indexOf("GET /vol_1") != -1) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();
          String vol_1 = String(poids1);
          client.println(vol_1); // Retourner la valeur de vol_1
      }
      
      // Demander le vol_2
      if (request.indexOf("GET /vol_2") != -1) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();
          String vol_2 = String(poids2);
          client.println(vol_2); // Retourner la valeur de vol_2
      }

      //Envoye des paramatres

      if (request.indexOf("GET /param") != -1) {
          String param = request.substring(request.indexOf("=") + 1, request.length() - 9);
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();
          client.println("Message affiché");
          sendCommand(param[0]);
          printParam(param[0]);
      }
      
      // Afficher un message sur l'écran LCD
      if (request.indexOf("GET /titrant") != -1) {
          String titrant = request.substring(request.indexOf("=") + 1, request.length() - 9);
          Serial.print("titrant : ");
          Serial.println(titrant);
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();
          client.println("Message affiché");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(titrant);
      }
      if (request.indexOf("GET /titrand") != -1) {
          String titrand = request.substring(request.indexOf("=") + 1, request.length() - 9);
          Serial.print("titrE : ");
          Serial.println(titrand);
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();
          client.println("Message affiché");
          lcd.setCursor(0, 1);
          lcd.print(titrand);
      }

      //paramatrage speed pump
      if (request.indexOf("GET /pump") != -1) {
        String pumpSpeed = request.substring(request.indexOf("=") + 1, request.length() - 9);
        fast = pumpSpeed.toInt();
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println();
        client.println("Message affiché");
        lcd.clear();
        lcd.print("pump Set!");
      }

      //activer le moteur
      if (request.indexOf("GET /moteur_high") != -1) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, fast);
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println();
        client.println("Message affiché");
        lcd.clear();
        lcd.print("Titration ...");
      }

      if (request.indexOf("GET /moteur_low") != -1) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 0);
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println();
        client.println("Message affiché");
        lcd.clear();
        lcd.print("Arret !");
      }
    delay(1);
    client.stop();
  }
}

void printParam(char a){
  if (a == '1'){
    lcd.clear();
    lcd.print("tarage balance 1...");
    delay(2000);
    lcd.clear();
    lcd.print("tarage fini!");
  }
  else if (a == '2'){
    lcd.clear();
    lcd.print("tarage balance 2...");
    delay(2000);
    lcd.clear();
    lcd.print("tarage fini!");
  }
  else if (a == '3'){
    lcd.clear();
    lcd.print("etalonnage pH...");
    delay(2000);
    lcd.clear();
    lcd.print("etalonage fini!");
  }
  else if (a == '4'){
    lcd.clear();
    lcd.print("netoyage pompe...");
    delay(3000);
    pump_clean();
    lcd.clear();
    lcd.print("netoyage fini!");
  }
}

void pump_clean(){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, fast);
  delay (20000);
  analogWrite(ENA, 0);
}

void sendCommand(char command) {
  Wire.beginTransmission(NANO_ADDRESS);
  Wire.write(command); // Envoyer la commande
  Wire.endTransmission();
}
