// Code for the shock sensor.
// Caique Trivelato 
// 7/24/2024

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <BfButton.h>
#include <DHT.h>

#define DHTPIN D7     // Pin where the DHT sensor is connected
#define DHTTYPE DHT11 // Change to DHT11 if you are using a DHT11 sensor

const char* ssid = "Internet Connection";        // Your WiFi SSID
const char* password = "******";  // Your WiFi Password

// IP of your local server (where PHP script is hosted)
const char* serverIP = "IP address";  // Replace with your server IP
const int serverPort = 80;  // HTTP port

int machine_id = 1;
int node_id = 1;

DHT dht(DHTPIN, DHTTYPE);

int btnPin = D3;
int dT = D4;
int cLK = D5;
BfButton btn(BfButton::STANDALONE_DIGITAL, btnPin, true, LOW);

int counting = 0; //counts the little twists of the rotary encoder
int laps = 0; //counts revolutions
int angle = 0; 
int aState;
int aLastState;
bool lapChange;

int shock_pin = D1;
bool lastMove;
bool moving = false;
int counter = 0;

WiFiClient wifiClient;

// Variable to store button click status
String button_status = "No push";  // 0 = not pressed, 1 = pressed

//Button press handling function
void pressHandler(BfButton* btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial.println("Single push");
      button_status = "Single"; // Set status to 1 when button is pressed
      break;

    case BfButton::DOUBLE_PRESS:
      Serial.println("Double push");
      button_status = "Double"; // Can be any value based on how you want to track the type of press
      break;

    case BfButton::LONG_PRESS:
      Serial.println("Long push");
      button_status = "Long";
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(shock_pin, INPUT);
  Serial.println(angle);
  pinMode(cLK, INPUT_PULLUP);
  pinMode(dT, INPUT_PULLUP);
  aLastState = digitalRead(cLK);

  dht.begin();

  btn.onPress(pressHandler)
    .onDoublePress(pressHandler)
    .onPressFor(pressHandler, 1000);

  // Connecting to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
}

void loop() {
  // 30 second count of approx how many vibrations
  for (int i = 0; i <= 10000; i++) {
    delay(1);
    int pin_state = digitalRead(shock_pin);

    if (pin_state == LOW && lastMove == HIGH) {
      counter++;
      Serial.println("The vibration is high.");
    }
    lastMove = pin_state;

    // Read button states
    btn.read();

    aState = digitalRead(cLK);

    // Encoder rotation tracking
    if (aState != aLastState) {
      if (digitalRead(dT) != aState) {
        counting++;
        angle = 12 * counting;
      } else {
        counting--;
        angle = 12 * counting;
      }
      if (counting == 30 || counting == -30) {
        lapChange = true;
        counting = 0;
        laps++;
      }

      Serial.print("Angle: ");
      Serial.println(angle);

      if (lapChange == true) {
        lapChange = false;
        Serial.print("Laps: ");
        Serial.println(laps);
      }
    }
    aLastState = aState;
  }

  // Read temperature and humidity
  int humidity = dht.readHumidity();
  int temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Prepare to send the sensor data to the server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Construct the URL for the request
    String url = "http://" + String(serverIP) + "/FilePath.../webserver_publishing.php";

    // Start the HTTP request with the WiFiClient object
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Data to send (sensor values)
    String postData = "vibration=" + String(counter) + "&laps=" + String(laps) + "&button=" + String(button_status) + "&machine_id=" + String(machine_id) + "&node_id=" + String(node_id) + "&temperature=" + String(temperature) + "&humidity=" + String(humidity);

    // Send POST request
    int httpResponseCode = http.POST(postData);

    // Check the response code and print it to the serial monitor
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    // End the HTTP request
    http.end();
  }

  // Reset button status after sending data
  button_status = "No push";
  counter = 0;

  // Wait for 30 seconds before sending data again
  delay(20);
}
