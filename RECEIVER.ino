#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 10, 9, 8, 7); // Initialize the LCD
SoftwareSerial mySerial(4, 5); //TX, RX
// (Send and Receive)

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  lcd.begin(16, 2); // Initialize the LCD columns and rows
}

void loop() {
  
  
  if(Serial.available() > 0){//Read from serial monitor and send over  OSOYOO UART LoRa wireless module
    String input = Serial.readString();
    mySerial.println(input);    
  }
 
  if(mySerial.available() > 1){//Read from  OSOYOO UART LoRa wireless module and send to serial monitor
    String input = mySerial.readString();
    Serial.println(input);

    // Parse GPS coordinates
    int commaIndex = input.indexOf(',');
    if (commaIndex != -1) {
      String latitude = input.substring(10, 19);
      String longitude = input.substring(20,30);
      
      // Display GPS coordinates on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lat: ");
      lcd.print(latitude);
      lcd.setCursor(0, 1);
      lcd.print("Lon: ");
      lcd.print(longitude);
    }    
  }
  delay(20);
}