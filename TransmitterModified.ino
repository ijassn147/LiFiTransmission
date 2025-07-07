#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "moto g60"
#define STAPSK "12456789"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

WiFiServer server(80);  // Web server on port 80

const int ledPin = 4;  // GPIO4 (D2 on NodeMCU)
const int bitDelay = 250;  // Bit transmission delay

String receivedMessage = "";  // Stores received message

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  delay(2000);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();  // Start the web server
}

void loop() {
  WiFiClient client = server.available();  // Check if a client is connected

  if (client) {
    Serial.println("\nNew Client Connected!");
    String request = client.readStringUntil('\r');  // Read the request
    client.flush();
    
    // Extract message from the request
    if (request.indexOf("GET /send?msg=") >= 0) {
      int start = request.indexOf("msg=") + 4;
      int end = request.indexOf(" HTTP", start);  // Find the end of the message
      if (end < 0) end = request.length();
      receivedMessage = request.substring(start, end);
      
      receivedMessage.replace("+", " ");  // Convert '+' to space
      receivedMessage.replace("%20", " ");  // Handle '%20' encoding
      receivedMessage.replace("%40", "@");  // Handle '@' symbol
      receivedMessage.replace("%23", "#");  // Handle '#' symbol
      receivedMessage.replace("%24", "$");  // Handle '$' symbol
      receivedMessage.replace("%25", "%");  // Handle '%' symbol
      receivedMessage.replace("%26", "&");  // Handle '&' symbol
      receivedMessage.replace("%2A", "*");  // Handle '*' symbol

      Serial.print("✅ Received Message: ");
      Serial.println(receivedMessage);

      // Transmit the message via LED
      transmitMessage(receivedMessage);
    }

    // Send webpage response with last sent message
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html><head><title>ESP8266 Web Chat</title></head>");
    client.println("<body>");
    client.println("<h1>Send a Message</h1>");
    client.println("<form action='/send'>");
    client.println("Message: <input type='text' name='msg'>");
    client.println("<input type='submit' value='Send'>");
    client.println("</form>");
    client.println("<p><b>Last Sent Message:</b> " + receivedMessage + "</p>");  // Show last message
    client.println("</body></html>");
    
    client.println();
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

// Function to transmit message via LED
void transmitMessage(String message) {
  Serial.println("🔵 Transmitting Message via LED:");

  for (int i = 0; i < message.length(); i++) {
    char c = message[i];
    String binary = String(c, BIN);  // Convert character to binary

    // Ensure 8-bit format
    while (binary.length() < 8) {
      binary = "0" + binary;
    }

    Serial.print(c);
    Serial.print(": ");
    Serial.println(binary);

    transmitWithStartStopBit(binary);  // Send via LED
    delay(2000);  // Faster transmission
  }

  Serial.println("✅ Transmission Complete!\n");
}

// Function to send each character with start and stop bits
void transmitWithStartStopBit(String binary) {
  // Start bit (1)
  digitalWrite(ledPin, HIGH);
  delay(210);

  // Send the 8 data bits
  for (int i = 0; i < 8; i++) {
    digitalWrite(ledPin, binary[i] == '1' ? HIGH : LOW);
    delay(bitDelay);
  }

  // Stop bit (1)
  digitalWrite(ledPin, HIGH);
  delay(210);
  digitalWrite(ledPin, LOW);
}
