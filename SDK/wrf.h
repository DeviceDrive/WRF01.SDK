/*	Copyright 2017 DeviceDrive AS
*
*	Licensed under the Apache License, Version 2.0 (the "License");
*	you may not use this file except in compliance with the License.
*	You may obtain a copy of the License at
*
*	http ://www.apache.org/licenses/LICENSE-2.0
*
*	Unless required by applicable law or agreed to in writing, software
*	distributed under the License is distributed on an "AS IS" BASIS,
*	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*	See the License for the specific language governing permissions and
*	limitations under the License.
*
*/

/**@file
 *	
 *	@brief		WRF01 module
 *	@details	This module provieds an abstraction from the WRF01 and allows the deviceloper 
 *				to handle objects instead of strings sent from the wrf. 
 *				We strongly recommend tha user of this module to take a look at 
 *				WRF01 serial specification which can be found at
 *				https://devicedrive.com/downloads/
 */

#ifndef WRF_H__
#define WRF_H__

#include <stdint.h>
#include <stdbool.h>

#define STX_CHAR ((char)0x02)
#define ETX_CHAR ((char)0x03)
#define EOT_CHAR  ((char)0x04)

#define WRF_EOT         EOT_CHAR
#define WRF_SEND_ONLY   ETX_CHAR

#define WRF_EOT_STR ((unsigned char*)"\x04")
#define WRF_SEND_ONLY_STR ((unsigned char*)"\x03")

#pragma region Sizes
#define WRF_DEFAULT_CONFIG_SIZE 11
#define DEFAULT_OTA_PARAM_SIZE 5
#define DEFAULT_SEND_FILE_PARAM_SIZE 2
#define WRF_ERROR_MESSAGE_SIZE 256

#define WRF_MAC_STR_SIZE 13
#define WRF_PARAM_SIZE 64
#define WRF_SSID_PREFIX_SIZE 19
#define WRF_SSID_LENGTH_SIZE 32
#define WRF_PASSWORD_LENGHT_SIZE 63
#define WRF_TOKEN_SIZE 64
#define WRF_PRODUCT_KEY_SIZE 64
#define WRF_VERSION_SIZE 16
#define WRF_MASTER_URL_SIZE 256
#define WRF_OTA_MODULE_SIZE 2
#define WRF_CRC_STRING_SIZE 4
#define WRF_FILE_PACKET_OVERHEAD_SIZE 10
#pragma endregion

#pragma region Strings
/*	@brief	WRF01 strings for building JSON commands	*/
#define WRF_COMMAND_STR "command"
#define WRF_COMMAND_CLEAR_STR "clear"
#define WRF_COMMAND_DEEP_SLEEP_STR "deep_sleep"
#define WRF_COMMAND_FACTORY_RESET_STR "factory_reset"
#define WRF_COMMAND_CHECK_UPGRADE_STR "check_upgrade"
#define WRF_COMMAND_GET_UPGRADE_STR "get_upgrade"
#define WRF_COMMAND_INTROSPECT_STR "introspect"
#define WRF_COMMAND_REBOOT_STR "reboot"
#define WRF_COMMAND_STATUS_STR "status"
#define WRF_COMMAND_SETUP_STR "setup"
#define WRF_COMMAND_UPGRADE_STR "upgrade"
#define WRF_COMMAND_SMART_LINKUP_STR "smart_linkup"
#define WRF_COMMAND_GET_TIME_STR "get_time"
#define	WRF_COMMAND_UNKNOWN_STR	"COMMAD UNKNWON"

#define WRF_SETUP_DEBUG_MODE_STR "debug_mode"
#define WRF_SETUP_ERROR_MODE_STR "error_mode"
#define WRF_SETUP_SSID_PREFIX_STR "ssid_prefix"
#define WRF_SETUP_VISIBILITY_STR "visibility"
#define WRF_SETUP_SSL_ENABLE_STR "ssl_enabled"
#define WRF_SETUP_SSID_STR "network_ssid"
#define WRF_SETUP_PWD_STR "network_pwd"
#define WRF_SETUP_SILENT_CONNECT_STR "silent_connect"
#define WRF_SETUP_TOKEN_STR "token"
#define WRF_SETUP_PRODUCT_KEY_STR "product_key"
#define WRF_SETUP_VERSION_STR "version"
#define WRF_SETUP_MASTER_URL_STR "master_url"

#define WRF_OTA_MODULE_STR "module"
#define WRF_OTA_WRF01_STR "WRF01"
#define WRF_OTA_CLIENT_STR "CLIENT"
#define WRF_SIZE_STR "length"

#define WRF_COMMAND_SEND_FILE_STR "send_file"
#define WRF_COMMAND_SEND_FILE_MAX_PACKET_SIZE_STR "max_packet_size"

#define WRF_OTA_FILENO_STR "file_no"
#define WRF_OTA_DELAY_STR "delay"
#define WRF_OTA_PIN_TOGGLE_STR "pin_toggle"

#define WRF_OTA_PROTOCOL_STR "protocol"
#define WRF_OTA_PROTOCOL_RAW_STR "RAW"
#define WRF_OTA_PROTOCOL_ARDUINO_STR "ARDUINO_ZERO"
#define WRF_OTA_PROTOCOL_HANDSHAKE_STR "HANDSHAKE"

#define WRF_SEND_FILE_LENGTH_STR "length"
#define WRF_SEND_FILE_FILE_NAME_STR "file_name"

#define WRF_SECONDS_STR "seconds"
#define WRF_TIMEOUT_STR "timeout"

#define WRF_OFF_STR "off"
#define	WRF_ON_STR "on"
#define	WRF_MODE_NONE_STR "none"
#define	WRF_MODE_ALL_STR "all"
#define	WRF_MODE_LOCAL_STR "local"
#define	WRF_MODE_REMOTE_STR "remote"

#define DEFAULT_SSID_PREFIX "DeviceDrive"

#define WRF_LOCAL_RESPONSE_STR "devicedrive"
#define WRF_REMOTE_RESPONSE_STR "DeviceDrive"

#define WRF_RESULT_STR "result"
#define WRF_RESULT_OK_STR "OK"
#define WRF_RESULT_EMPTY_STR "EMPTY"
#define WRF_RESULT_SENT_STR "SENT"
#define WRF_RESULT_FILE_SENT_STR "FILE_SENT"
#define WRF_RESULT_FILE_CANCEL_STR "FILE_TRANSFER_CANCELED"
#define WRF_RESULT_TIME_STR "time"

#define WRF_CONFIG_STR "configuration"
#define WRF_CONFIG_MAC_STR "mac"
#define WRF_CONFIG_DEVICE_STATE_STR "device_state"
#define WRF_CONFIG_RSSI_STR "rssi"

#define WRF_ERROR_STR "error"
#define WRF_REMOTE_ERROR_CODE_STR "ErrorCode"
#define WRF_ERROR_NONE_STR "NONE"
#define WRF_ERROR_SYSTEM_BUSY_STR "SYSTEM_BUSY"
#define WRF_ERROR_SEND_REQUEST_FAILED_STR "SEND_REQUEST_FAILED"
#define WRF_ERROR_REVEICE_REQUEST_FAILED_STR "RECEIVE_REQUEST_FAILED"
#define WRF_ERROR_MASTER_REQUEST_FAILED_STR "MASTER_REQUEST_FAILED"
#define WRF_ERROR_NOT_ONLINE_STR "NOT_ONLINE"
#define WRF_ERROR_REMOTE_ERROR_STR "REMOTE_ERROR"
#define WRF_ERROR_RX_OVERFLOW_STR "RX_OVERFLOW"
#define WRF_ERROR_SET_AP_MODE_FAILED_STR "SET_AP_MODE_FAILED"
#define WRF_ERROR_UPGRADE_STR "UPGRADE_ERROR"
#define WRF_ERROR_COMMAND_FAILED_STR "COMMAND_FAILED"

#define WRF_ERROR_UNDEFINED_ERROR_STR "UNDEFINED_ERROR"
#define WRF_ERROR_INVALID_JSON_STR "INVALID_JSON"
#define WRF_ERROR_INVALID_INTROSPECT_STR "INVALID_INTROSPECT"
#define WRF_ERROR_INVALID_HEADER_STR "INVALID_HEADER"
#define WRF_ERROR_FORWARDING_ERROR_STR "FORWARDING_ERROR"
#define WRF_ERROR_MISSING_HEADER_STR "MISSING_HEADER"
#define WRF_ERROR_INVALID_TOKEN_STR "INVALID_TOKEN"
#define WRF_ERROR_INVALID_APP_STR "INVALID_APP"
#define WRF_ERROR_SMARTLINKUP_FAILED_STR "SMART_LINKUP_FAILED"
#define WRF_ERROR_NO_TIME_STR "NO_TIME"

#define WRF_IDLE_STR "IDLE"
#define WRF_CONNECTING_STR "CONNECTING"
#define WRF_GOT_IP_STR "GOT_IP"
#define WRF_CONNECTION_FAILED_STR "CONNECTION_FAILED"
#define WRF_DNS_FAILED_STR "DNS_FAILED"
#define WRF_NO_AP_FOUND_STR "NO_AP_FOUND"
#define WRF_WRONG_PASSWORD_STR "WRONG_PASSWORD"

#define LIB_ERROR_PARSE_STATUS_STR "ERROR_PARSE_STATUS"
#define LIB_ERROR_PARSE_UPGRADE_STR "ERROR_PARSE_UPGRADE"
#define LIB_ERROR_UNKNOWN_OBJECT_STR "ERROR_UNKNOWN_OBJECT"
#define LIB_ERROR_PARSE_TIME_STR "ERROR_PARSE_TIME"

#define WRF_ERROR_UNKNOWN_STR "Wrf SDK does not recognize message"

#define WRF_JSON_START "{\"devicedrive\":{"
#define WRF_JSON_END "}}"

#pragma endregion

#pragma region Enums

/*	@brief	WRF01 modes. */
typedef enum
{ 
	WRF_OFF,
	WRF_ON,
	WRF_MODE_NONE,
	WRF_MODE_ALL,
	WRF_MODE_LOCAL,
	WRF_MODE_REMOTE,
}wrf_mode;

/*	@brief	WRF01 Commands*/
typedef enum
{ 
	WRF_COMMAND_CLEAR,
	WRF_COMMAND_DEEP_SLEEP,
	WRF_COMMAND_FACTORY_RESET,
	WRF_COMMAND_CHECK_UPGRADE,
	WRF_COMMAND_GET_UPGRADE,
	WRF_COMMAND_INTROSPECT,
	WRF_COMMAND_REBOOT,
	WRF_COMMAND_STATUS,
	WRF_COMMAND_SETUP,
	WRF_COMMAND_SMARTLINKUP,
	WRF_COMMAND_UPGRADE,
	WRF_COMMAND_SEND_FILE,
	WRF_COMMAND_GET_TIME,
}wrf_command;

/*	@brief	WRF01 Error codes*/
typedef enum
{
	WRF_ERROR_NONE,
	WRF_ERROR_SYSTEM_BUSY,
	WRF_ERROR_SEND_REQUEST_FAILED,
	WRF_ERROR_REVEICE_REQUEST_FAILED,
	WRF_ERROR_MASTER_REQUEST_FAILED,
	WRF_ERROR_NOT_ONLINE,
	WRF_ERROR_REMOTE_ERROR,
	WRF_ERROR_RX_OVERFLOW,
	WRF_ERROR_SET_AP_MODE_FAILED,
	WRF_ERROR_UPGRADE,
	WRF_ERROR_COMMAND_FAILED,
	WRF_ERROR_UNDEFINED_ERROR,
	WRF_ERROR_INVALID_JSON,
	WRF_ERROR_INVALID_INTROSPECT,
	WRF_ERROR_INVALID_HEADER,
	WRF_ERROR_FORWARDING_ERROR,
	WRF_ERROR_MISSING_HEADER,
	WRF_ERROR_INVALID_TOKEN,
	WRF_ERROR_INVALID_APP,
	WRF_ERROR_SMARTLINKUP_FAILED,
	LIB_ERROR_RESULT_UNKNOWN,
	LIB_ERROR_PARSE_STATUS, 
	LIB_ERROR_PARSE_UPGRADE,
	LIB_ERROR_UNKNOWN_OBJECT,
	LIB_ERROR_PARSE_TIME,
	WRF_ERROR_NO_TIME
}wrf_error_code;

/*	@brief		Connection status codes 
*	@note		These codes are used in Status struct. 
*/
typedef enum {
	WRF_IDLE,				// WRF01 is idle, not trying to connect.
	WRF_CONNECTING,			// WRF01 is trying to connect to the configured network.
	WRF_GOT_IP,				// WRF01 got an IP from the local network and is online.
	WRF_CONNECTION_FAILED,	// Problem occured while trying to connect to the given SSID.Still trying to connect.
	WRF_DNS_FAILED,			// WRF01 is online on the local network but has problems resolving the DNS address of the master cloud agent.
	WRF_NO_AP_FOUND,		// WRF01 can not find the given SSID, but is still scanning
	WRF_WRONG_PASSWORD,		// WRF01 can not connect to the given network due to wrong password.
	WRF_UNKNOWN				// Incomming string is unknown as a wrf_connection_status
}wrf_connection_status;

/*	@brief		OTA module codes*/
typedef enum {
	OTA_WRF01,
	OTA_CLIENT,
	OTA_UKNOWN,
}wrf_ota_module;

/*	@brief		Protocol codes  */
typedef enum {
	PROTOCOL_RAW,
	PROTOCOL_ARDUINO,
	PROTOCOL_HANDSHAKE
}wrf_ota_protocol;

/*	@brief		WRF respons codes
*
*	@note		Results from WRF, use @ref wrf_handle_response to fire callbacks
*/
typedef enum {
	WRF_EMPTY,
	WRF_OK,
	WRF_SENT,
	WRF_STATUS,
	WRF_CONFIG,
	WRF_UPGRADE_PENDING,
	WRF_UPGRADE_PACKAGE,
	WRF_LOCAL_ERROR,
	WRF_REMOTE_ERROR,
	WRF_MESSAGE,
	WRF_SEND_FILE,
	WRF_FILE_SENT,
	WRF_FILE_CANCEL,
	WRF_TIME,
}wrf_result_code;

#pragma endregion

#pragma region Structs

/*	@brief	wrf_config contains information to set up WRF01.
*
*	@note	Use	@ref DEFAULT_WRF_CONFIG(M_CONFIG) to get the default config to send.
*/
typedef struct 
{
	wrf_mode debug_mode;
	wrf_mode error_mode;
	char* ssid_prefix;
	int visibility;
	bool ssl_enabeled;
	char* network_ssid;
	char* network_pwd;
	bool silent_connect;
	char* product_key;
	char * token;
	char* version;
}wrf_config;

/*	@brief	wrf_status contains infromation about the status of the WRF01
*
*	@note	Status from the WRF01 is retrieved by @ref wrf_ask_status and waiting
*			for a resonse from WRF01 on uart. This response can be handeled with
*			@ref wrf_handle_response, and it will create this object and fire a 
*			@ref wrf_callback.
*/
typedef struct
{
	wrf_connection_status connection_status;
	char ip_addr[WRF_PARAM_SIZE]; // TODO: decrease the size!
	bool visibility_status;
	wrf_error_code last_error_code;
	char last_error_msg[WRF_MASTER_URL_SIZE]; // TODO: check sizes
	int successful_transfer_count;
}wrf_status;

typedef struct
{
	wrf_result_code code;
	char* msg;
}wrf_send_file_status;

/*	@brief	wrf_device_state contains information about the connection and device
*
*	@note	To receive this @ref silent_connect must be set to false.
*/
typedef struct {
	char mac[WRF_MAC_STR_SIZE];
	int rssi; 
}wrf_device_state;

/*	@brief	Building block for creating commands.
*/
typedef struct 
{
	char* name;
	char* str_value;
	int i_value;
} wrf_param;

/*	@brief	Object containg information about how to start an OTA.
*/
typedef struct {
	wrf_ota_module module;
	int file_no;
	int delay;
	char* pin_toggle;
	wrf_ota_protocol protocol;
} ota_params;

/*	@brief	Object describing the ota packet received.
*/
typedef struct {
	int size;
	int crc;
}ota_packet;

/*	@brief	Object holding information about pending upgrades-
*	@note	Call @ref wrf_check_upgrade to get this object. 
*/
typedef struct{
	wrf_ota_module modules[WRF_OTA_MODULE_SIZE];
	int size;
}wrf_module_list;

/*	@brief	An WRF01 error with describing message
*/
typedef struct {
	wrf_error_code code;
	char* msg;
} wrf_error;

typedef struct {
    uint64_t timestamp; 
    int week_day;   // 0-6 (Mon-Sun)
    int day;        
    int month;      // 1-12 (Jan-Dec)
    int year;   
    int hour;       // 0-24
    int minute;     // 0-60
    int second;     // 0-60
    int timezone;  // -11 - 13 where 0 is GMT
    int dst;        // 1 is on, 0 is off
} wrf_time; 

#pragma endregion

#pragma region Defined Functions

/*	@brief	Initializes a default wrf config struct
*/
#define DEFAULT_WRF_CONFIG(M_CONFIG)						\
	M_CONFIG.debug_mode = WRF_MODE_NONE;					\
	M_CONFIG.error_mode = WRF_MODE_ALL;						\
	M_CONFIG.ssid_prefix = (char* const)DEFAULT_SSID_PREFIX;\
	M_CONFIG.visibility = WRF_OFF;							\
	M_CONFIG.ssl_enabeled = WRF_ON;							\
	M_CONFIG.silent_connect = WRF_ON;						\
	M_CONFIG.network_ssid = NULL;							\
	M_CONFIG.network_pwd = NULL;							\
	M_CONFIG.token = NULL;									\
	M_CONFIG.product_key = NULL;							\
	M_CONFIG.version = NULL	

/*	@brief	Initializes a defailt ota_params struct
*/
#define DEFAULT_OTA_PARAMS(M_OTAPARAM)				\
	M_OTAPARAM.module = OTA_WRF01;					\
	M_OTAPARAM.file_no = -1;						\
	M_OTAPARAM.delay = -1;							\
	M_OTAPARAM.pin_toggle = NULL;					\
	M_OTAPARAM.protocol = PROTOCOL_RAW

/*  Next defines are helper methods	*/
#define CHECK_STR(CODE_STR, VALUE)					\
	(strcmp(CODE_STR, VALUE->u.string.ptr)==0)		

#define CHECK_NAME(CODE_STR, VALUE)					\
	(strcmp(CODE_STR, VALUE->u.object.values[0].name)==0)		

#define INIT_ERROR(CODE, MSG)						\
	{ error.code = CODE; error.msg = MSG;}

#define JSON_PAIR_STR(NAME,VALUE,DEST)				\
    char param_str[WRF_PARAM_SIZE];					\
	sprintf(param_str,"\"%s\":\"%s\"",NAME, VALUE);	\
	strcat(DEST, param_str)		

#define JSON_PAIR_INT(NAME,VALUE,DEST)				\
    char param_str[WRF_PARAM_SIZE];					\
	sprintf(param_str,"\"%s\":%d",NAME, VALUE);		\
	strcat(DEST, param_str)

#pragma endregion

#pragma region Function definitions
/*	@brief		Function for writing charcaters to WRF01.
*	
*	@details	This function must be implemented, and should write the 
*				incomming string to the uart that is connected to WRF01
*	@note		Do not add anything to the incoming string. The string is 
*				built from the library. 
*
*	@param[in]	string	String to be written to WRF01.
*/
typedef uint32_t(*wrf_write_string)(unsigned char* buffer, int buflen);

/*	@brief		Function defenition for handling responses from WRF01
*	
*	@details	A function must be implmented to handle the respnonse objects
*				created from the library.
*	@note		For this function to be called it must be registrated with 
*				@ref wrf_on_response and the method @ref wrf_handle_response 
*				must be used when characters are available from the uarte connected 
*				to the WRF01.
*
*	@param[in]	code	The type of result 
*	@param[in]  object	A pointer to the response object. Must be casted based on @ref wrf_result_code
*/
typedef void(*wrf_callback)(wrf_result_code code, void* object);

#pragma endregion

/*	@brief		Function for initiate the library.
*
*	@details	This function setes the function for communicating with the physical WRF01.
*
*	@param[in]	write_uart	pointer to the method writing to the uart connected to WRF01.
*	@note		write_uart must be implemented on the microcontroller used. 
*/
void wrf_init(wrf_write_string write_uart);

/*	@brief		Fuction for setting response callback.
*
*	@details	This function registrates the response callback.
*	@note		To get library to fire this callback use @ref wrf_handle_response.
*
*	@param[in]	callback function to write to WRF01 over uart. 
*/
void wrf_on_response(wrf_callback callback);

#pragma region Basic Methods
/*	@brief		Function for sending messages to WRF01
*
*	@details	This message adds a EOT charcater to the message and uses the 
*				@ref wrf_write_string to write message to the WRF01
*	@note		Before calling this method the WRF must have been initiated by @ref wrf_init
*
*	param[in]	message		message to be sent to WRF01
*/
void wrf_send_message(char* message);

/*	@brief		Function for sending commands to WRF01.
*
*	@details	This functions creates the the JSON string for sending 
*				the giving command to WRF01 and uses @ref wrf_send_string to send it.
*	@note		Before calling this method the WRF must have been initiated by @ref wrf_init.
*	
*	param[in]	cmd		type of WRF01 command.
*	param[in]	params	array of parameters to be sendt with command.
*	param[in]	size	number of parameters in @ref params.
*/
void wrf_send_command(wrf_command cmd, wrf_param* params, int size);

/*	@brief		Function for polling messages from the cloud/app to the WRF01.
*
*	@note		Before calling this method the WRF must have been initiated by @ref wrf_init.
*/
void wrf_receive_message();

/*	@brief		Function for sending commands to WRF01 without expecting something to receive.
*
*	@details	This functions creates the the JSON string for sending
*				the giving command to WRF01 and uses @ref wrf_send_string to send it.
*	@note		Before calling this method the WRF must have been initiated by @ref wrf_init.
*
*	param[in]	msg		message to send
*/
void wrf_send_without_receive(char* msg);

/*	@brief		Function for sending config.
*
*	@details	This function creates and sends a JSON command based on the given config. 
*	@note		Before calling this method the WRF must have been initiated by @ref wrf_init.
*
*	@param[in]	config	Config for the WRF01.
*/
void wrf_send_config(wrf_config *config);

/*	@brief		Function for sending introspect to WRF01.
*
*	@details	This functions takes a JSON formated string wich describes the device 
*				capabilities, se WRF01 serial documentation, and sends it with  @ref wrf_send_message
*	@note		Before calling this method the WRF must have been initiated by @ref wrf_init.
*
*	@param[in]	introspect	JSON fomrated string that describes device capabilities.
*/
void wrf_send_introspect(char* introspect);

/*	@brief		Function for setting WRF01 in AccessPoint mode.
*
*	@details	This function sends JSON formated string to WRF using @ref wrf_send_message
*				setting WRF01 visible.
*
*	param[in]	seconds		number of seconds WRF01 should show Access Point, -1 is infinite.
*/
void wrf_set_visible(int seconds);

/*	@brief		Function for enableing SmartLinkUp on the WRF01
*
*	@details	Enable the SmartLinkup function for WRF01. This function shortens the linkup time and
*				allows for the WRF01 to be linked up without using the Access Point
*
*	param[in]	seconds		Number of seconds the WRF01 should stay in SmartLinkup mode. Should be between 15 seconds and 3 minutes
*/
void wrf_smart_linkup(int seconds);

/*	@brief		Function for triggering WRF01 to try to connect.
*
*	@details	This function sends JSON formated string to WRF using @ref wrf_send_message
*				forceing a connect.
*
*	param[in]	silent		if WRF01 not should send rssi and mac when it connects.
*/
void wrf_connect(bool silent);

/*	@brief		Function for rebooting WRF01.
*/
void wrf_reboot();

/*	
*	@brief		Fuction for setting WRF01 in deep sleep.
*
*	@param		duration	secconds is should sleep, 
*							0 for indefinitly sleep until wakeup from external pin.
*/
void wrf_deep_sleep(int duration);

/*	@brief		Function to check if WRF01 has a pending upgrade.
*/
void wrf_check_upgrade();

/*	@brief		Function for starting an upgrade.
*
*	@note		Call @ref wrf_check_upgrade to check if WRF01 has pending upgrade. 
*
*	@param[in]	upgrade_params	Defines the upgrad details. 
*/
void wrf_get_upgrade(ota_params *upgrade_params);

/*	@brief		Function for starting the send file feature
*
*	@param[in]	file_name	Name of the saved file in the cloud
*				size		The size of the file
*/
void wrf_init_send_file(char* file_name, int size);

/*	@brief		Fuction for clearing wifi information from the WRF01.
*/
void wrf_clear();

/*	@brief		Function ser setting WRF01 back to factory settings.
*/
void wrf_factory_reset();

/*	@brief		Function for asking status from WRF01.
*
*	@note		Handle the response in @ref wrf_callback.
*/
void wrf_ask_status();


/*	@brief		Function for requesting time from WRF01.
*
*	@note		Handle the response in @ref wrf_callback.
*/
void wrf_get_time();

/*	@brief		Function for parsing WRF01 JSON strings and fire callbacks.
*
*	@details	This function parses a string and converts it to wrf_sctructs and fires
				@ref wrf_callback with correct code and object. 
*	@note		For this to work @ref wrf_on_response must have been called.
*/
void wrf_handle_response(char* msg);
#pragma endregion

#pragma region Helper Methods

/*	@brief		Function for parsing a wrf_param object to string
*
*	@details	This function takes a wrf_param and creates a JSON formated string.
*	@note		This function is used by the library, and recomed to use wrf_send_command
*				instead. 
*	
*	@param[in]	param	Wrf parameter.
*	@param[in]	dest	char array to write the JSON formatted string. 
*/
void add_cmd_param(wrf_param param, char* dest);

/*	@brief		Function for getting wrf_command(enum) as string.
*
*	@param[in]	cmd		enum to translate.
*/
char* get_cmd_str(wrf_command cmd);

/*	@brief		Function for getting wrf_mode(enum) as string.
*
*	@param[in]	mode	enum to translate.
*	
*	@retval		wrf_command as string.
*/
char* get_mode_str(wrf_mode mode);

/*	@brief		Function for getting wrf_ota_module(enum) as string.
*
*	@param[in]	mode	enum to translate.
*
*	@retval		wrf_mode as string.
*/
char* get_ota_module_str(wrf_ota_module module);

/*	@brief		Function for getting wrf_ota_protocol(enum) as string.
*
*	@param[in]	mode	enum to translate.
*
*	@retval		wrf_ota_module as string.
*/
char* get_ota_protocol_str(wrf_ota_protocol protocol);

/*	@breif		Function for parsing a string as a wrf_ota_module.
*
*	@param[in]	string to handle.
*
*	@retval		OTA_WRF01	if string is WRF01.
*	@retval		OTA_CLIENT	if string is CLIENT.
*	@retval		OTA_UKNOWN	if the string is unknown.
*/
wrf_ota_module get_ota_module(char* module);

/*	@breif		Function for parsing a string as a wrf_connection_status.
*
*	@param[in]	string to handle.
*
*	@retval		@ref wrf_connection_status
*/
wrf_connection_status get_status(char* status_str);

/*	@breif		Function for parsing a string for wrf_error_code.
*
*	@param[in]	String to handle.
*
*	@retval		@ref wrf_error_code
*/
wrf_error_code get_error_code(char* code_str);

#pragma endregion

#endif