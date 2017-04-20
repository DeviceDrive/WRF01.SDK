# WRFArduinoLib

WRFArduinoLib is an easy to use wrapper for Arduino Zero with the DeviceDrive WRF01 Arduino Zero shield connected.
This library abstracts all elements of Wifi operations and onboarding functions with simple commands.
To get started fast, just implement your own message callbacks, send data to the cloud and control your device from our mobile app, designed for iOS and Android.

Check out the examples section for more information and tips / tricks.

NOTE:
The shield can be obtained from [DeviceDrive](https://devicedrive.com/).

## Getting started

Create a WRFArduino object and get get the instance for WRF. The WrfArduinoLib is built on a singleton from the generic WRF sdk's for multiple platforms.

### Initialize WRF

For the arduino you can get the object like this

	WRFArduino &wrf = WRFArduino::getInstance();
	
The serial port that communicates with the WRF01 is being handled by the library, so you don't need to setup the serial ports in the setup routine. The library uses Serial1 port for communication with the WRF01.

The basic setup routine for arduino should look like this:

	WRFArduino &wrf = WRFArduino::getInstance();

	void setup() {
		Serial.begin(115200); //For debugging and printing to your computer
		wrf.setup();
	}
	
See [WRF config](#wrfConfig) to see how to configure the WRF with the setup you want.

### Arduino Loop

In the Arduino loop, just add the WRF handler.

	void loop() {
		wrf.handle();
		delay(1);
	}
	
	NOTE: We add 1ms delay to the routine just to stabelize the arduino.

### Introspection document

The Introspect is a JSON object that describes your device and its capabilities. See the [Serial Specification](https://devicedrive.com/downloads/).

	  #define INTROSPECTION_INTERFACES "\"interfaces\":[[\"com.devicedrive.light\",
	  				\"@status>s\",
					\"@version>s\",
					\"@power=b\"]]"

This is used by the cloud and the app to keep track of what your product is capable of. 

### Send and receive messages

The library handles the messages you want to send in a queue, so you do not need to wait for response in your client code. Both messages and commands uses this queue.

##### Sending Message to App/Cloud
Your messages to the cloud and app must match the introspect defined in the setup routine of your program. The reason is that the mobile app uses the introspect to identify the fields and commands available in the app and presented to the user.
To send a status message, you must create a valid JSON formatted string with the correct parameters and values, then issue

	 wrf.send(String message);

To build a status message, you can use either Strings or the ArduinoJSON library. See this example from (LightSwitch.ino):

	#define INTERFACE_NAME "com.devicedrive.light"
	#define STATUS_PARAM "status"
	#define POWER_PARAM "power"

	int power = 0;
	String state = "Off";

	...

	void sendStatus() {
		String message = "{\"com.devicedrive.light\":{"state\": \"Off\",\"power\": 1}}"
		wrf.send(message);
	}

Based on the Introspection Document, the parameters are shown either as strings, numbers or buttons with a boolean value.

##### Sending large files

> NOTE: At this time, there is no public endpoint to retreive the files uploaded. This is a planned feature!

The WRF can send large files to the cloud and store them there. By using the generic libraries you can load segments of data from
memory and send through the WRF, but for convinience in the Arduino library, we assume that you have the full file in memory.
to send the file from Arduino, use the sendFile function.

    wrf.sendFile(String file_name, int size, unsigned char* data);

If your package is bigger than 500bytes, the WRF will segment it and send it as individual packages.

Example:
    
    const char* file = "Hello World";
    char*  file_name = "HelloWorld.txt";
    wrf.sendFile(file_name, 11, (unsigned char*)file);

##### Receiving message
To receive a message, the wrf.handle(); must be present in your loop function, and you must either call

	wrf.poll();

in the loop, or you can call the following function somewhere once in your code:

	wrf.startPoll(milliseconds ms);
	
To stop the polling, issue:
	
	wrf.stopPoll();

startPoll(milliseconds) will enable the auto poll feature, and make a poll request to the cloud every given second.
A recommended value for this is around 5 seconds (5000ms).
Each time a message is received, your callback defined in onMessageReceived(callback) will be triggered.
This might be handled like this:

	void onMessageReceived(char* msg) {
		Serial.println("++ onMessageReceived ++");
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(String(msg));
		JsonObject& dd = root["com.devicedrive.light"];
		
		if(dd.containsKey("power")){
			String val = dd["power"].as<String>();
			setActive(val.toInt()); 
			wrf.checkPendingUpgrades();
		}
	}

Notice that we check for upgrades every time we receive a message.
The upgrade information is sent from the cloud along with every message, so this does not trigger a separate cloud communication.
	
### <a name="wrfConfig"></a>WRF Config

To configure the WRFShield, you need to setup an WRFConfig object.
The WRFConfig object holds all the necessary information about the configuration of the shield and your product for the communication with the DeviceDrive cloud systems. 

There are two important configuration settings that needs to be defined; Client version and Product key.

The client version tells the cloud system what version of the client code that is currently running. The product key grants the device access to the cloud and identifies your product with the product type defined. Read more at [here](#productKey) or at [DeviceDrive](https://devicedrive.com/downloads).

The library has a method for setting up a default configuration structure. See the example below on how to setup your config.

	//Example of config:
	#define VERSION "1.0"
	#define PRODUCT_KEY "<Your product key here>"

	WRFArduino &wrf = WRFArduino::getInstance();
	WRFConfig wrfConfig;
	
	void setup() {
		Serial.begin(115200);
		wrf.setup(); // Setting up wrf object.
		DEFAULT_WRF_CONFIG(config); // Setting up the default config
		config.product_key = PRODUCT_KEY;
		config.version = VERSION;
	}
	
The WRFConfig contains the following properties, and default values:

	debug_mode			= "none"
	error_mode			= "all"
	ssid_prefix			= "DeviceDrive"
	visibility			= 0
	ssl_enabled			= 1
	silent_connect		= 1
	network_ssid		= NULL
	network_pwd			= NULL
	token				= NULL
	product_key			= NULL
	version				= NULL
	
##### Debug mode

The debug mode defines what should be printed out on the logport of the WRF01 Shield. The log port is the Mini USB port on the shield. There are 2 available options, either:
	
	WRF_MODE_NONE	No debug
	WRF_MODE_ALL	All debug messages
	
If NONE is set, no output will be printed on the logport, ALL will print everything on the debug port on the WRF01 Shield (Mini USB marked "Log").

##### Error mode

The error mode defines what kind of messages that you will receive on the host port if an error should occur. By default all error messages are enabled. The different kinds of messages are as follows:
	
	WRF_MODE_ALL	All messages are printed
	WRF_MODE_NONE	No messages are printed
	WRF_MODE_LOCAL	Only local messages from the WRF01
	WRF_MODE_REMOTE	Only messages from the cloud
	
##### Ssid prefix

When the WRF01 goes into linkup mode it will broadcast an SSID that your phone or computer can connect to for running LinkUp sequence or to send web based communication settings.
The ssid prefix determines what the SSID will look like. If you are going to use the DeviceDrive playground app ([iOS](https://itunes.apple.com/us/app/devicedrive-playground/id1108807229?mt=8) or [Android](https://play.google.com/store/apps/details?id=com.homedrive.droid.app), the prefix needs to be "DeviceDrive".

##### Visibility

A part of the config object, but should not be set each time the WRF01 starts. On first startup, if the WRF01 can't connect to a network, set this to number of seconds you want the network to be visible, or use -1 to keep it on until restart.

| Value | Comment |
|-------|---------|
| 0		| Turn AP off |
| -1	| AP on until off or restart |
| xx	| Number of seconds (e.g 90) | 

##### Ssl enabled

By default SSL is always enabled, but you can turn it off, if you really want to. By setting the WRFConfig->ssl_enabled to 0 or false

| Value | Comment |
|-------|---------|
| 0		| SSL is disabled |
| 1		| SSL is enabled | 

##### Silent connect

When the WRF01 has connected to a valid WiFi network, a connection string can be printed on the host port. This string will contain the RSSI signal strength of the router you are currently connected to and the local IP address of the device. This connection string is enabled by default by this library. 

| Value | Comment |
|-------|---------|
|	0	| Connection string is enabled  |
|	1	| Connection string is disabled	|

##### Network ssid

This is the network SSID you want the device to connect to. This is usually set through the LinkUp
library for iOS and Android.

##### Network pwd

This is the network password for the network you want the device to connect to. Usually set through the
LinkUp library for iOS and Android.

##### Token

Token is the identificator for the device in the cloud to assure that a user has access to communicate with the device. A valid token is only possible through the LinkUp tools if the product is defined as an internal product in the DeviceDrive cloud system. If the product is an Forwarding product, then you can use this field as you please as an identificator.

##### <a name="productKey"></a>Product key

The product key is the primary identifier for you product in the cloud system. This key can be obtained in the DeviceDrive cloud services when you register an product.
		
## Linkup

DeviceDrive has developed it's own form of onboarding, called Linkup. From the client code, you can tell the WRF to enter "visibility mode", where the WRF broadcasts a local network, on which an app or a browser can connect and transfer SSID, password, and device token. The device token is issued by the DeviceDrive Cloud.
All of this logic is abstracted away from the client.


## Callbacks

There are several callbacks that you can register that the WRF01 will signal back on in case of the registerd events.

Setup the callbacks in the setup routine of your arduino sketch. You don't need all of them, but they can be advantageous

	Example:
		
		WRFArduino &wrf = WRFArduino::getInstance();
	
		void setup() {
			Serial.begin(115200);
			wrf.setup(); // Setting up wrf object.
			
			wrf.onConnected(onConnectedCallback);
			wrf.onNotConnected(onDisconnected);
		}
		
		void onConnectedCallback() {
			Serial.println("You are connected to the internet");
		}
		
		void onDisconnected() {
			Serial.println("You got disconnected!");
		}

This is a description of all the callbacks available from the WRF01 that will be available to your arduino code.

##### onError
If an error should occur on the WRF01 and the error_mode setting is enabled, the current error will be thrown to this callback.
		
	The Error callback will return with an error code and error message.
	void onError(wrf_error* error) {
		Serial.println(error->msg);
	}
		
##### onPowerUp
When the WRF01 has started up and is ready to start communicating, this callback will be called.
	
##### onConnected
This callback will be fired when the devic e receives an IP address from the selected network. The return callback will also give you the RSSI value and the MAC address of the device.
		
	void onConnected(wrf_device_state* state) {
		Serial.println(state->rssi);
		Serial.println(state->mac);
	}
	
##### onNotConnected
If network connection is dropped or an error in communication happens, this callback will be fired.
	
##### onMessageSent
Fired when the WRF01 has successfully sent a message to the cloud.
		
##### onMessageReceived
When the device checks the cloud for new messages, this callback will be fired with the new content available.
		
	void onMessageReceived(char* msg) {
		Serial.println(msg);
	}
		
##### onPendingUpgrades
If there are OTA upgrades available for your device in the cloud, this callback will be fired. Here you will receive
a list of potential upgrades available for your device.
		
	typedef struct{
		wrf_ota_module modules[WRF_OTA_MODULE_SIZE];
		int size;
	}wrf_module_list;
	
	void onPendingUpgrades(wrf_module_list* list) {
		Serial.println("++ onPendingUpgrades ++");
		for(int i = 0; i <list->size; i++){
			if(list->modules[i] == OTA_CLIENT){
				Serial.println("Starting Client Upgrade");
				wrf.startClientUpgrade(1000);
			}
		}
	}
		
##### onStatusReceived
A message can be sent to the WRF to request a system status. The status message will contain information about your connection to the local wifi, your IP address, the status of the local AP and the last sent error message.
		
	typedef struct {
		wrf_connection_status connection_status;
		char ip_addr[WRF_PARAM_SIZE]; // TODO: decrease the size!
		bool visibility_status;
		wrf_error_code last_error_code;
		char last_error_msg[WRF_MASTER_URL_SIZE]; // TODO: check sizes
		int successful_transfer_count;
	}wrf_status;
	
	typedef void WrfStatusReceivedCallback(wrf_status* status);
		
## Pin definitions
	
Version 1.3 of the WRF01 Zero Shield uses these pins which are not available for further use.

| Arduino pin | WRF01 Function |
| ----------- | -------------- |
| D0 | UART TX |
| D1 | UART RX |
| D5 | GPIO0 (Board pullup) |
| D6 | GPIO13 |
| D8 | GPIO15 (Board pulldown, used in boot) |
| D9 | RESET (Wake the WRF01 from deepsleep or reset |

	
