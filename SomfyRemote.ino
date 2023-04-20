#include <EEPROM.h>
#include <EEPROMRollingCodeStorage.h>
#define DEBUG
#include <SomfyRemote.h>

#include <AceButton.h>
using namespace ace_button;

#define EMITTER_GPIO 2
#define EEPROM_ADDRESS 0
#define REMOTE 0x5184c8



EEPROMRollingCodeStorage rollingCodeStorage(EEPROM_ADDRESS);
SomfyRemote somfyRemote(EMITTER_GPIO, REMOTE, &rollingCodeStorage);

// Physical pin numbers attached to the buttons.
const int BUTTON1_PIN = 3;
const int BUTTON2_PIN = 4;

#define LED_PIN LED_BUILTIN

// LED states. Some microcontrollers wire their built-in LED the reverse.
const int LED_ON = HIGH;
const int LED_OFF = LOW;

// Both buttons automatically use the default System ButtonConfig. The
// alternative is to call the AceButton::init() method in setup() below.
AceButton button1(BUTTON1_PIN);
AceButton button2(BUTTON2_PIN);

// Forward reference to prevent Arduino compiler becoming confused.
void handleEvent(AceButton*, uint8_t, uint8_t);


void setup() {
	Serial.begin(115200);

	somfyRemote.setup();
    // Initialize built-in LED as an output.
  pinMode(LED_PIN, OUTPUT);
    // Buttons use the built-in pull up register.
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
    // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);

  // Check if the button was pressed while booting
  if (button1.isPressedRaw()) {
    Serial.println(F("setup(): button 1 was pressed while booting"));
  }
  if (button2.isPressedRaw()) {
    Serial.println(F("setup(): button 2 was pressed while booting"));
  }

  Serial.println(F("setup(): ready"));

#if defined(ESP32)
	if (!EEPROM.begin(4)) {
		Serial.println("failed to initialise EEPROM");
		delay(1000);
	}
#elif defined(ESP8266)
	EEPROM.begin(4);
#endif
}

void loop() {
  button1.check();
  button2.check();
	if (Serial.available() > 0) {
		const String string = Serial.readStringUntil('\n');
		const Command command = getSomfyCommand(string);
		somfyRemote.sendCommand(command);
#ifdef DEBUG
		Serial.println("finished sending");
#endif
	}
}


// The event handler for both buttons.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
/*
  // Print out a message for all events, for both buttons.
  Serial.print(F("handleEvent(): pin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
*/
  // Control the LED only for the Pressed and Released events of Button 1.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  switch (eventType) {
    case AceButton::kEventPressed:
      if (button->getPin() == BUTTON1_PIN) {
        digitalWrite(LED_PIN, LED_ON);
      }
      break;
    case AceButton::kEventReleased:
      if (button->getPin() == BUTTON1_PIN) {
        digitalWrite(LED_PIN, LED_OFF);
      }
      break;
    case AceButton::kEventDoubleClicked:
      if (button->getPin() == BUTTON1_PIN) {
        Serial.println(F("Button 1 DoubleClicked!"));
        somfyRemote.sendCommand(Command::Prog, 4);
      }
      if (button->getPin() == BUTTON2_PIN) {
        Serial.println(F("Button 2 DoubleClicked!"));
        somfyRemote.sendCommand(Command::My, 4);
      }
      break;
    case AceButton::kEventLongPressed:
      if (button->getPin() == BUTTON1_PIN) {
        Serial.println(F("Button 1 LongPressed!"));
        somfyRemote.sendCommand(Command::MyUp, 4);       
      }
      if (button->getPin() == BUTTON2_PIN) {
        Serial.println(F("Button 2 LongPressed!"));
        somfyRemote.sendCommand(Command::MyDown, 4);       
      }
      break;                
    case AceButton::kEventClicked:
      if (button->getPin() == BUTTON2_PIN) {
        Serial.println(F("Button 2 clicked!"));
        somfyRemote.sendCommand(Command::Down, 4);
      }
      if (button->getPin() == BUTTON1_PIN) {
        Serial.println(F("Button 1 Clicked!"));
		    somfyRemote.sendCommand(Command::Up, 4);        
      }
      break;
  }
}

/*
///Available commands
Name	    Description	                                    HEX code
My	      The My button pressed	                          1
Up	      The Up button pressed	                          2
MyUp	    The My and Up button pressed at the same time	  3
Down	    The Down button pressed	                        4
MyDown	  The My and Down button pressed at the same time	5
UpDown	  The Up and Down button pressed at the same time	6
Prog	    The Prog button pressed	                        8
SunFlag	  Enable sun and wind detector	                  9
Flag	    Disable sun detector	                          A
*/