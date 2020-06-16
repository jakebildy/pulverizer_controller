/**************************************************************************
Countdown Device - June 2020
 **************************************************************************/
#include <Encoder.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//The state of the system. 0: Rest, 1: Counting Down, 2: Finished
int STATE = 0;

bool justPressed = false;

//Initial values - 3:00 min on the timer
int seconds = 0;
int minutes = 3;

String timeSec;
String timeMin;
String result;

unsigned long lastButtonPress = 0;

//Rotary Encoder
Encoder myEnc(2, 3);

boolean isButtonPressed = false;
long lastUpdateMillis = 0;

void setup() {
  Serial.begin(9600);

  pinMode(4, INPUT); // Button


  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  pinMode(12, OUTPUT); //Beeper
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  STATE = 0;
  justPressed = false;
}

//A single 'beep' from the buzzer, for starting the countdown.
void beep() {
   digitalWrite(12, HIGH); 
    delay(100);
    digitalWrite(12, LOW); 
}

//Two short beeps in succession, for stopping the timer.
void beepbeep() {
   digitalWrite(12, HIGH); 
    delay(50);
    digitalWrite(12, LOW); 
    delay(50);
    digitalWrite(12, HIGH); 
    delay(50);
    digitalWrite(12, LOW); 
}

void loop() {

     maintainDisplay(); 
     maintainRotaryEncoder();
     Serial.print("BUTTON: ");
      Serial.println(digitalRead(4));
     
    if (digitalRead(4) == LOW) {
      if (STATE == 0) {
     STATE = 1;
     Serial.println("STARTING");
     digitalWrite(11, LOW); // Start Relay
     beep();
     delay(1000);
     digitalWrite(11, HIGH); 
      }
      else if (STATE == 1) {
        STATE = 0;
        minutes = 3;
        seconds = 0;
        digitalWrite(10, LOW);  //Stop Relay
        Serial.println("STOPPING");
        beepbeep();
        delay(1000);
        digitalWrite(10, HIGH); 
        refreshDisplay();
      }
    }
    
 

    
    timeSec = String(seconds);
    timeMin = String(minutes);
    
    if (seconds < 10) {
      result = timeMin + ":0" + timeSec;
    }
    else {
      result = timeMin + ":"+ timeSec;
    }

    if (seconds == 0 && minutes == 0) {
      result = "DONE";
      STATE = 2;
      digitalWrite(10, LOW); // Close Relay
      digitalWrite(12, HIGH); 
      refreshDisplay(); 
      delay(1000);
      digitalWrite(10, HIGH); //Close Relay
      digitalWrite(12, LOW); 
      delay(3000);
      STATE = 0; 
      minutes = 3;
    }
}

//Maintains the display - by not calling delay() allows multiple tasks to take place at once
void maintainDisplay()
{
  static const unsigned long REFRESH_INTERVAL = 500; // ms
  static unsigned long lastRefreshTime = 0;
  
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
  {
    lastRefreshTime += REFRESH_INTERVAL;
   // if (STATE != 0 || initialRefresh == false) {
                refreshDisplay();
              // initialRefresh = true;
  //  }
  }
}

//Refreshes the display with the current time.
void refreshDisplay(void)
{
  if (STATE == 1) {
    seconds--;
    
    if (seconds <= -1) {
      seconds = 59;
      minutes--;
    }
    }
    
  display.clearDisplay();
  display.setCursor(0,0);  
  display.setTextSize(5);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println(result);

  display.display();
}

long position  = -999;

//Reads changes to the Rotary Encoder. 
void maintainRotaryEncoder(){
long newPosition = myEnc.read();
 // Serial.print("Position :");
   // Serial.println(newPosition);
    
  if (newPosition != position) {
    position = newPosition;
    seconds -= newPosition * 5;
  
    if (seconds >= 60) {
      seconds = seconds - 60;
      minutes ++;
    }
    else if (seconds < 0) {
      seconds = seconds + 60;
      minutes --;
    }

    if (minutes < 1 && seconds < 30) {
      minutes = 0;
      seconds = 30;
    }
    else if (minutes >= 9) {
      minutes = 9;
      seconds = 0;
    }
    Serial.println(position);
    refreshDisplay();
  }
  // software debounce
  if (millis() - lastUpdateMillis > 50) {
    lastUpdateMillis = millis();
    // Reset the counter
    myEnc.write(0);
  }
  //delay(1);
}
