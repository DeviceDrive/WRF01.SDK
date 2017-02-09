#include <ArduinoJson.h>
#include <wrfarduinolib.h>

typedef void ButtonPressCallback();							
ButtonPressCallback *button_press_cb = NULL;
bool button_was_pressed; 
int button_pressed_counter; 
void onButtonPress(ButtonPressCallback * callback);

#define VERSION "1.0"
#define PRODUCT_KEY "<Your product key here>"
#define INTERFACE_NAME "com.devicedrive.light"
#define STATUS_PARAM "status"
#define POWER_PARAM "power"
#define VERSION_PARAM "version"
#define INTROSPECTION_INTERFACES "\"interfaces\":[[\"com.devicedrive.light\",\"@status>s\",\"@version>s\",\"@power=b\"]]"

#define POLL_INTERVALL 5000

#define TEXT_ON "On"
#define TEXT_OFF "Off"

#define LED_PIN 10		// Define our light at pin 10
#define PUSH_PIN 4		// Define our button at pin 4

WRFArduino &wrf = WRFArduino::getInstance();
wrf_config config;

int power = 0;				// Indicates if the light is on or off
String state = TEXT_OFF;	// Status text to be sent to app

void setup() 
{
	Serial.begin(115200);	// For printing information

	// First we set up our WRF01 and define its config. 
	wrf.setup();
	DEFAULT_WRF_CONFIG(config);
	config.product_key = PRODUCT_KEY;
	config.silent_connect = false;
	config.version = VERSION;
	config.debug_mode= WRF_MODE_ALL;

	// Then we registrate the callbacks we want to handle
	wrf.onPowerUp(onStart);
	wrf.onConnected(onConnected);
	wrf.onNotConnected(onDisconnected);
	wrf.onMessageReceived(onMessageReceived);
	wrf.onPendingUpgrades(onPendingUpgrades);
	wrf.onError(onError);

	button_press_cb = onPress;

	pinMode(LED_PIN, OUTPUT);			// LED
	pinMode(PUSH_PIN, INPUT);			// Button

	Serial.println("Done Setup");
	Serial.println("------------------------------------");
	wrf.reboot(); // Restart WRF01 when Arduino is setup
}

// The loop function runs over and over again until power down or reset.
void loop()
{
	wrf.handle();		
	handle_button();
	delay(1);
}

void onStart() 
{
	Serial.println("++ OnStart ++");
	// When the WRF01 starts we set the config. 
	wrf.send_config(config);
}

void onConnected(wrf_device_state* state)
{
	Serial.println("++ OnConnected ++");
	Serial.println("    -> Sending introspect");
	// When we connect, we tell the world what our devcie can by sending its introspect
	wrf.sendIntrospect(String(INTROSPECTION_INTERFACES));
	// Then we tell the what our current status is
	sendStatus();
	// Then we start polling 
	wrf.startPoll(POLL_INTERVALL);
}


void onDisconnected()
{
	Serial.println("++ OnDisconnected ++");
	// When we loose network we stop polling and show ourselvs as an access point
	wrf.stopPoll();
	wrf.setVisibility(-1, true);
}

void setLight(bool mode) {
	//Turn on or off LED
	digitalWrite(LED_PIN, mode);
	power = mode;
	state = (mode) ?TEXT_ON : TEXT_OFF;
	// Tell the wolrd that our status has changed!
	sendStatus();
}

void sendStatus() {
	Serial.println("    -> Sending status");

	String status_str = state;
	String power_str = String(power, DEC);

	String msg = String("{\"com.devicedrive.light\":{\"status\":\"" + status_str + "\",\"version\":\""+ VERSION +"\",\"power\":" + power_str + "}}");
	wrf.send(msg);
}

void onMessageReceived(char* msg)
{
	Serial.println("++ onMessageReceived ++");

	//When we received a message, we check that we suppot the introspect and handle it. 
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(String(msg));
	JsonObject& dd = root[INTERFACE_NAME];
	if (dd.containsKey(POWER_PARAM)) {
		String val = dd[POWER_PARAM].as<String>();
		// If it is a power message we set the light to correct value
		setLight(val.toInt());
		// Then we check if there are any pending upgrades
		wrf.checkPendingUpgrades();
	}
}

void onPendingUpgrades(wrf_module_list* list)
{
	//If there are any upgrades pending, we upgrade!
	Serial.println("++ onPendingUpgrades ++");
	for (int i = 0; i <list->size; i++) {
		if (list->modules[i] == OTA_WRF01){
			Serial.println("Start WRF01 Upgrade");
			wrf.startWrfUpgrade();
		}
		else if (list->modules[i] == OTA_CLIENT) {
			Serial.println("Starting Client Upgrade");
			wrf.startClientUpgrade(1000);
		}
	}
}

void onError(wrf_error* error)
{
    // Her we handle the possible errors.
      Serial.println("++ onError ++");
      Serial.println("    -> " + String(error->msg));
}

void onPress() 
{
  Serial.println("++ onPress ++");
	// When button is pressed we toggle the ligth
	setLight(!power);
}

/** An easy Button implementation for arduino. */
#define SHORTPRESS 50


void onButtonPress(ButtonPressCallback * callback)
{
	button_press_cb = callback;
}

void handle_button()
{
	int button_now_pressed = !digitalRead(PUSH_PIN); // pin low -> pressed
													 // Trigger current event
	if (!button_now_pressed && button_was_pressed) {
		if (button_pressed_counter > SHORTPRESS) {
			if (button_press_cb != NULL)
				button_press_cb();
		}
	}
	// Tracking button push
	if (button_now_pressed) {
		button_pressed_counter++;
	}
	else {
		button_pressed_counter = 0;
	}
	button_was_pressed = button_now_pressed;
}
