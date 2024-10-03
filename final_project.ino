#include <SoftwareSerial.h>
#include <Keypad.h>

// GSM module serial communication pins
SoftwareSerial mySerial(3, 2);

// Ultrasonic sensor pins
#define trigPin 8
#define echoPin 9

// LED pin
int led = 4;

char password[] = "3333"; // Change the password here
char enteredPassword[5] = "";  // Buffer to store entered password
int passwordLength = 4;   // Length of the password

// Keypad setup
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {7, 6, 5};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

bool motionDetected = false;
unsigned long motionDetectedTime;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led, OUTPUT);
  mySerial.begin(9600); // GSM module baud rate
  Serial.begin(9600); // Serial monitor baud rate

  // Initialize the GSM module
  mySerial.println("AT");
  delay(1000);
  mySerial.println("AT+CMGF=1"); // Set GSM module to SMS mode
  delay(1000);
}

void loop() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1; // Convert to centimeters

  if (distance >= 4) { // If someone is detected within 10 cm
   
    digitalWrite(led, HIGH); // Turn on LED as an indicator
    motionDetected = true;
    
    motionDetectedTime = millis(); // Store the time when motion was detected
  }

  if (motionDetected) {
    // Keep detecting keys only for 30 seconds after motion is detected
    if (millis() - motionDetectedTime <= 30000) {
      char key = keypad.getKey();

      if (key != NO_KEY) {
        Serial.print("Key pressed: ");
        Serial.println(key);
        if (key == '#') {
          if (checkPassword()) {
            digitalWrite(led, LOW);
            delay(20000);
            Serial.println("Password correct!"); // Print to serial monitor

          } else {
            Serial.println("Incorrect password."); // Print to serial monitor
            makeCall();
          }
          clearPassword();
          motionDetected = false; // Reset motion detection
        } else {
          appendKey(key);
        }
      }
    } else {
      // Reset motion detection if no key is pressed within 30 seconds
      Serial.println("No password entered within 30 seconds.");
      motionDetected = false;
      clearPassword();
    }
  } else {
    digitalWrite(led, LOW); // Turn off LED
  }

  delay(100);
}

void makeCall() {
  Serial.println("Calling...");
  mySerial.println("ATD+94761598703;"); // Replace with the desired phone number
  delay(20000); // Duration of the call (20 seconds)
  mySerial.println("ATH"); // Hang up
  Serial.println("Call ended");
  digitalWrite(led, LOW);
}

  


void clearPassword() {
  memset(enteredPassword, 0, sizeof(enteredPassword));
}

bool checkPassword() {
  return strcmp(enteredPassword, password) == 0;
}

void appendKey(char key) {
  int len = strlen(enteredPassword);
  if (len < passwordLength) {
    enteredPassword[len] = key;
    enteredPassword[len + 1] = '\0'; // Null-terminate the string
    Serial.print("Entered: ");
    Serial.println(enteredPassword);
  }
}
