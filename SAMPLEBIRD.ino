#define BLYNK_TEMPLATE_ID "TMPL63Vu3xv19"
#define BLYNK_TEMPLATE_NAME "Bird Detection"
#define BLYNK_AUTH_TOKEN "bFsUiZnfnYmC-J6BovKx5-fTJwQj1KqQ"
#define DATABASE_URL "https://bird-detection-490ea-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyBvHtzUIpmlEtJVa33fxpL8WAOI7Zrjr6o"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <FirebaseESP32.h>
#include <stdlib.h> // Standard library for random number generation
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth authData;
bool signupOK = false;
char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "PLDTHOMEFIBRJyB4k";
char pass[] = "PLDTWIFIE4c4e";

// Define the PIR sensor pin
#define pirPin 2   // PIR sensor input pin

SoftwareSerial softwareSerial(17, 16); // RX, TX
DFRobotDFPlayerMini player;

unsigned long motionDetectedTime = 0;
bool isPlaying = false;
BlynkTimer timer;

// Track number
int currentTrack = 1;
const int totalTracks = 6; // Total number of tracks on the SD card

// Declare the blynkRun function before using it
void blynkRun();

void setup() {
  // Init USB serial port for debugging
  Serial.begin(9600);

  // Init serial port for DFPlayer Mini
  softwareSerial.begin(9600);

  // Start communication with DFPlayer Mini
  if (player.begin(softwareSerial)) {
    Serial.println("DFPlayer Mini initialized.");
    // Set volume to maximum (0 to 30).
    player.volume(20);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }

  // Initialize the PIR sensor input
  pinMode(pirPin, INPUT);
  Serial.println("PIR sensor initialized.");

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);

  // Set up a timer to run Blynk.run() every second
  timer.setInterval(1000L, blynkRun);
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &authData, "", "")){
    Serial.println("signUp OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; // Provide the token generation process callback function
  Firebase.begin(&config, &authData);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Run Blynk
  Blynk.run();
  
  // Run timer
  timer.run();

  // Check the PIR sensor state
  int pirState = digitalRead(pirPin);

  if (pirState == HIGH) {
    // Motion detected
    if (!isPlaying) {
      Serial.println("Motion detected!");

      // Play the current track
      Serial.print("Playing track number: ");
      Serial.println(currentTrack);
      player.play(currentTrack);

      // Increment the track number for the next detection
      currentTrack++;
      if (currentTrack > totalTracks) {
        currentTrack = 1; // Reset to the first track if the last track was played
      }

      isPlaying = true;
      motionDetectedTime = millis();  // Record the time motion was detected
      Blynk.logEvent("bird_detected", "BIRD DETECTED!");

      if (Firebase.ready()) {
        String motionDetectedTimeString = String(motionDetectedTime);
        Firebase.pushString(firebaseData, "/motion_detected_times/" + String(millis()), motionDetectedTimeString);
      }
    }
  }

  // Stop the sound after 20 seconds if it was playing
  if (isPlaying && (millis() - motionDetectedTime >= 20000)) {
    Serial.println("Stopping playback.");
    player.stop();  // Stop the sound
    isPlaying = false;
  }
}

void blynkRun() {
  Blynk.run();
}
