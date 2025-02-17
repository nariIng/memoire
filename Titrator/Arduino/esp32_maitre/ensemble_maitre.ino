#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define NANO_ADDRESS 8

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
char buffer[30];
float poids1 = 0, poids2 = 0, pH = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initialisation...");
}

void loop() {
  Wire.requestFrom(NANO_ADDRESS, sizeof(buffer));
  if (Wire.available()) {
    size_t len = Wire.readBytesUntil('\0', buffer, sizeof(buffer));
    buffer[len] = '\0';  // Assurez-vous que la chaîne est bien terminée

    sscanf(buffer, "%f,%f,%f", &poids1, &poids2, &pH);
    Serial.println(buffer);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1:");
    lcd.print(poids1, 2);
    lcd.print(" g ");
    lcd.print("2:");
    lcd.print(poids2, 2);
    lcd.print(" g");
    lcd.setCursor(0, 1);
    lcd.print("pH : ");
    lcd.print(pH, 2);
  }
  delay(1000);
}
