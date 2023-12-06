#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <dht.h>

#define LED_PIN 13
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DHT_PIN 7

dht DHT;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String msgBreathing = "";
String msgPatient = "";
float previousHumidity = 0.0; // Variable to store the previous humidity measurement
unsigned long displayTimerBreathing = 0; // Timer to control breathing message display duration
unsigned long displayDurationBreathing = 2000; // Display duration for breathing message in milliseconds
unsigned long displayTimerPatient = 0; // Timer to control patient detection message display duration
unsigned long displayDurationPatient = 2000; // Display duration for patient detection message in milliseconds


void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);  // Set cursor position at the top

  // Display static text
  display.println("BreathEase ControlHub is Detecting...");
  display.display();

  delay(3000); // Allowing sensor detection to be done
}


void loop() {
  int chk = DHT.read11(DHT_PIN);

  // Read humidity and temperature values
  float humidity = DHT.humidity;
  float temperature = DHT.temperature;

  // Send humidity data via Serial
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%\t");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  // Check the difference between the new and previous humidity measurements
  float humidityDifference = humidity - previousHumidity;

  // Display breathing message
  if (humidityDifference > 0 && millis() - displayTimerBreathing >= displayDurationBreathing) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("The patient is breathing");
    display.display();
    digitalWrite(LED_PIN, LOW);
    Serial.println("Breathing");
    displayTimerBreathing = millis(); // Reset the breathing message display timer
  }

  // Display patient detection message
  if (millis() - displayTimerPatient >= displayDurationPatient) {
    if (humidityDifference <= 0) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.println("WARNING: BREATHING IS NOT DETECTED");
      display.display();
      digitalWrite(LED_PIN, HIGH);
      Serial.println("Not Breathing");
      displayTimerPatient = millis(); // Reset the patient detection message display timer
    }
  }

  // Save the current humidity measurement for the next iteration
  previousHumidity = humidity;

  // Delay before the next reading
  delay(2000);

  if (Serial.available() > 0) {
    // Read the incoming message
    String inputMessage = Serial.readStringUntil('$');

    // Parse and process the input
    processInput(inputMessage);
  }

  delay(2000);
}


void processInput(String input) {
  int asciiValue1 = input.toInt();
  char character = char(asciiValue1);
  // Print the message to the Serial Monitor
  Serial.println(character);

  // Check if the received message indicates detection
  if (character == 'Y') {
    // delay(3000);
    digitalWrite(LED_PIN, LOW);
    msgPatient = "The Patient Is Detected";
    displayTimerPatient = millis(); // Reset the patient detection message display timer
  } else {
    msgPatient = "NWARNING: NO PATIENT DETECTED";
    displayTimerPatient = millis(); // Reset the patient detection message display timer
    digitalWrite(LED_PIN, HIGH);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(msgPatient);
  display.display();
}