#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Define RX and TX pins for SIM900 module
SoftwareSerial gprsSerial(4, 5); // RX, TX for SIM900

// Define RX and TX pins for GPS module
static const int RXPin = 13; // Connect to GPS TX
static const int TXPin = 12; // Connect to GPS RX
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  gprsSerial.begin(9600);    // Initialize GPRS module
  Serial.begin(9600);        // Initialize Serial Monitor
  ss.begin(GPSBaud);         // Initialize GPS module communication

  Serial.println("Initializing...");
  delay(1000);

  // Initialize SIM900
  gprsSerial.println("AT");
  delay(1000);
  ShowSerialData();

  gprsSerial.println("AT+CFUN=1"); // Set full functionality
  delay(1000);
  ShowSerialData();

  gprsSerial.println("AT+CPIN?"); // Check SIM card status
  delay(1000);
  ShowSerialData();

  gprsSerial.println("AT+CREG?"); // Check network registration
  delay(1000);
  ShowSerialData();

  gprsSerial.println("AT+CGATT?"); // Check GPRS attachment
  delay(1000);
  ShowSerialData();

  // Set APN for Teletalk Bangladesh
  gprsSerial.println("AT+CSTT=\"gpinternet\""); 
  delay(1000);
  ShowSerialData();

  gprsSerial.println("AT+CIICR"); // Start wireless connection
  delay(1000);
  ShowSerialData();

  gprsSerial.println("AT+CIFSR"); // Get local IP address
  delay(1000);
  ShowSerialData();

  Serial.println("Initialization complete. Waiting for GPS signal...");
}

void loop() {
  while (ss.available() > 0) {
    char c = ss.read();    // Read GPS data
    if (gps.encode(c)) {   // Parse GPS data
      if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        // Print latitude and longitude
        Serial.print("Latitude: ");
        Serial.println(latitude, 6);
        Serial.print("Longitude: ");
        Serial.println(longitude, 6);

        // Send GPS data to ThingSpeak
        sendToThingSpeak(latitude, longitude);
      } else {
        Serial.println("Searching for GPS signal...");
      }
    }
  }
}

void sendToThingSpeak(float latitude, float longitude) {
  // Start connection to ThingSpeak
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");
  delay(1000);
  ShowSerialData();

  gprsSerial.println("AT+CIPSEND");
  delay(1000);
  ShowSerialData();

  // Create the HTTP GET request with GPS data
  String str = "GET https://api.thingspeak.com/update?api_key=CD8LNCDYF4WA2ZEQ&field1=" + 
               String(latitude, 6) + "&field2=" + String(longitude, 6);
  gprsSerial.println(str); // Send the GET request
  Serial.println(str);     // Debugging: print the request

  gprsSerial.println((char)26); // Send Ctrl+Z to finish the request
  delay(1000); // Wait for response
  ShowSerialData();

  // Close the connection
  gprsSerial.println("AT+CIPSHUT");
  delay(100);
  ShowSerialData();
}

void ShowSerialData() {
  while (gprsSerial.available() != 0) {
    Serial.write(gprsSerial.read()); // Display SIM900 responses in Serial Monitor
  }
  delay(1000); // Allow time for responses
}
