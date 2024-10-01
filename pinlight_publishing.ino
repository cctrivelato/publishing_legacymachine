#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "WIFICONNECTION";        // Your WiFi SSID
const char* password = "yourpassword";  // Your WiFi Password

// IP of your local server (where PHP script is hosted)
const char* serverIP = "192.168.1.100";  // Replace with your server IP
const int serverPort = 80;  // HTTP port

int ledPin = D1;  // GPIO pin connected to the LED
int ledState = LOW;

WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Connecting to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
}

void loop() {
  // Toggle LED state
  ledState = !ledState;
  digitalWrite(ledPin, ledState);

  // Prepare to send the LED status (ON or OFF) to the server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Construct the URL for the request
    String url = "http://" + String(serverIP) + "/NodeMCU Publish/mytest.php";
    String ledStatus = (ledState == HIGH) ? "ON" : "OFF";
    
    // Start the HTTP request with the WiFiClient object
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Data to send (LED status)
    String postData = "led_status=" + ledStatus;
    
    // Send POST request
    int httpResponseCode = http.POST(postData);
    
    // Check the response code and print it to the serial monitor
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }
    
    // End the HTTP request
    http.end();
  }

  // Wait for 5 seconds before toggling again
  delay(5000);  
}

