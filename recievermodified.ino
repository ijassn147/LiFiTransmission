#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

const int ldrPin = A0;  // LDR sensor connected to analog pin A0
const int threshold = 600;  // Light threshold for detecting HIGH (1) and LOW (0)
const int bitDelay = 250;  // MUST match the transmitter bit delay

LiquidCrystal_PCF8574 lcd(0x27);
String receivedBinary = "";  // Stores the received binary sequence
int cursor = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ldrPin, INPUT);
  
  // Initialize 16x2 LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Received text:");
  lcd.setCursor(0, 1);
}

void loop() {
  int ldrValue = analogRead(ldrPin);

  // Detect start bit (HIGH)
  if (ldrValue < threshold) {
    Serial.println("✅ Start bit detected");
    delay(210);  // Wait for start bit stabilization

    receivedBinary = "";  // Clear previous binary data

    // Read 8 data bits
    for (int i = 0; i < 8; i++) {
      delay(bitDelay / 2);  // Allow stable reading
      ldrValue = analogRead(ldrPin);

      // Read bit twice and take majority vote
      int ldrValue2 = analogRead(ldrPin);
      char bit = ((ldrValue < threshold) && (ldrValue2 < threshold)) ? '1' : '0';

      receivedBinary += bit;
      delay(bitDelay / 2);  // Maintain correct bit timing
    }

    // Read stop bit (HIGH)
    delay(210);
    ldrValue = analogRead(ldrPin);
    if (ldrValue < threshold) {
      Serial.println("🛑 Stop bit detected.");

      // Convert binary string to ASCII character
      char decodedChar = binaryToChar(receivedBinary);
      
      if (decodedChar != '#') {  // '#' is a reset marker
        lcd.setCursor(cursor, 1);
        lcd.print(decodedChar);
        Serial.print("📩 Received: ");
        Serial.println(decodedChar);
        cursor += 1;
      } else {  // Reset LCD on receiving '#'
        cursor = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Received text:");
        lcd.setCursor(0, 1);
      }
    } else {
      Serial.println("⚠️ Error: Stop bit not detected correctly.");
    }

    delay(2000);  // Match transmission delay
  }
}

// Improved function to convert binary string to character
char binaryToChar(String binary) {
  char result = 0;
  if (binary.length() == 8) {  // Ensure full 8-bit data
    for (int i = 0; i < 8; i++) {
      if (binary.charAt(i) == '1') {
        result |= (1 << (7 - i));  // Convert binary to ASCII correctly
      }
    }
  }
  return result;
}
