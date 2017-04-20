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

#include  "wrf.h"
#include "json.h"
#include  <string.h>
#include <stdio.h>

#define WRF_MESSAGE_MAX_SIZE 1024

static wrf_write_string _write_string = NULL;
static wrf_callback on_response_cb = NULL;

void wrf_init(wrf_write_string write_string)
{
	_write_string = write_string;
}

void wrf_send_message(char* msg)
{
	int length = strlen(msg);
	char* buffer = malloc(length+2);
	memcpy(buffer, msg, length);
	buffer[length] = (char)WRF_EOT;
	buffer[length + 1] = 0x0; //Zero terminate

	_write_string(buffer, strlen(buffer));
	free(buffer);
}

#pragma region Callbacks

void wrf_on_response(wrf_callback callback)
{
	on_response_cb = callback;
}

#pragma endregion

#pragma region Send Commands

void wrf_send_command(wrf_command cmd, wrf_param* params, int size)
{
	char msg[WRF_MESSAGE_MAX_SIZE] = WRF_JSON_START;
	JSON_PAIR_STR(WRF_COMMAND_STR, get_cmd_str(cmd), msg);

	if (params)
		for(int i = 0; i< size; i++)
			add_cmd_param(*(params+i), msg);	
	
	strcat(msg, WRF_JSON_END);
	wrf_send_message(msg);
}

void wrf_send_introspect(char* introspect) 
{
	char msg[WRF_MESSAGE_MAX_SIZE] = WRF_JSON_START;
	JSON_PAIR_STR(WRF_COMMAND_STR, WRF_COMMAND_INTROSPECT_STR, msg);
	strcat(msg, ", ");
	strcat(msg, introspect);
	strcat(msg, WRF_JSON_END);

	wrf_send_message(msg);
}

void wrf_send_config(wrf_config *config)
{
	wrf_param params[WRF_DEFAULT_CONFIG_SIZE];
	params[0].name = WRF_SETUP_DEBUG_MODE_STR;
	params[0].str_value = get_mode_str(config->debug_mode);
	params[1].name = WRF_SETUP_ERROR_MODE_STR;
	params[1].str_value = get_mode_str(config->error_mode);
	params[2].name = WRF_SETUP_SSID_PREFIX_STR;
	params[2].str_value = config->ssid_prefix;

	char v_buffer[10];
	char s_buffer[10];
	char c_buffer[10];
	params[3].name = WRF_SETUP_SILENT_CONNECT_STR;
	sprintf(c_buffer, "%d", config->silent_connect);
	params[3].str_value = c_buffer;
	params[4].name = WRF_SETUP_VISIBILITY_STR;
	sprintf(v_buffer, "%d", config->visibility);
	params[4].str_value = v_buffer;
	params[5].name = WRF_SETUP_SSL_ENABLE_STR;
	sprintf(s_buffer, "%d", config->ssl_enabeled);
	params[5].str_value = s_buffer;

	int size = 6;

	if (config->network_ssid)
	{
		params[size].name = WRF_SETUP_SSID_STR;
		params[size].str_value = config->network_ssid;
		size++;
	}
	if (config->network_pwd)
	{
		params[size].name = WRF_SETUP_PWD_STR;
		params[size].str_value = config->network_pwd;
		size++;
	}
	if (config->token)
	{
		params[size].name = WRF_SETUP_TOKEN_STR;
		params[size].str_value = config->token;
		size++;
	}
	if (config->product_key)
	{
		params[size].name = WRF_SETUP_PRODUCT_KEY_STR;
		params[size].str_value = config->product_key;
		size++;
	}
	if (config->version) {
		params[size].name = WRF_SETUP_VERSION_STR;
		params[size].str_value = config->version;
		size++;
	}

	wrf_send_command(WRF_COMMAND_SETUP, params, size);
}


void wrf_set_visible(int seconds) 
{
	char seconds_str[WRF_PARAM_SIZE];
	sprintf(seconds_str, "%d", seconds);
	wrf_param param = {WRF_SETUP_VISIBILITY_STR, seconds_str};
	wrf_send_command(WRF_COMMAND_SETUP, &param, 1);
}

void wrf_connect(bool silent) 
{
	char silent_str[WRF_PARAM_SIZE];
	sprintf(silent_str, "%d", silent);
	wrf_param params[1] = {
		{WRF_SETUP_SILENT_CONNECT_STR, silent_str}
	};
	
	wrf_send_command(WRF_COMMAND_SETUP, params, 1);
}

void wrf_reboot() 
{
	wrf_send_command(WRF_COMMAND_REBOOT, NULL, 0);
}

void wrf_deep_sleep(int duration) 
{
	char duration_str[WRF_PARAM_SIZE];
	sprintf(duration_str, "%d", duration);
	wrf_param param = {WRF_SECONDS_STR, duration_str};
	wrf_send_command(WRF_COMMAND_DEEP_SLEEP, &param, 1);
}

void wrf_check_upgrade() {
	wrf_send_command(WRF_COMMAND_CHECK_UPGRADE, NULL, 0);
}

void wrf_get_upgrade(ota_params *upgrade_params) {
	wrf_param params[DEFAULT_OTA_PARAM_SIZE];
	int size = 0;
	if (upgrade_params->module >= 0) {
		params[size].name = WRF_OTA_MODULE_STR;
		params[size].str_value = get_ota_module_str(upgrade_params->module);
		size++;
	}
	if (upgrade_params->file_no>=0) {
		params[size].name = WRF_OTA_FILENO_STR;
		params[size].str_value = NULL;
		params[size].i_value = upgrade_params->file_no;
		size++;
	}
	if (upgrade_params->delay>=0) {
		params[size].name = WRF_OTA_DELAY_STR;
		params[size].str_value = NULL;
		params[size].i_value = upgrade_params->delay;
		size++;
	}
	if (upgrade_params->pin_toggle) {
		params[size].name = WRF_OTA_PIN_TOGGLE_STR;
		params[size].str_value = upgrade_params->pin_toggle;
		size++;
	}
	if ((int)upgrade_params->protocol >= 0) {
		params[size].name = WRF_OTA_PROTOCOL_STR;
		params[size].str_value = get_ota_protocol_str(upgrade_params->protocol);
		size++;
	}

	wrf_send_command(WRF_COMMAND_GET_UPGRADE, params, size);
}

void wrf_init_send_file(char* file_name, int size) {
	char size_str[WRF_PARAM_SIZE];
	sprintf(size_str, "%d", size);

	wrf_param params[DEFAULT_SEND_FILE_PARAM_SIZE];

	params[0].name = WRF_SEND_FILE_LENGTH_STR;
	params[0].str_value = size_str;
	params[1].name = WRF_SEND_FILE_FILE_NAME_STR;
	params[1].str_value = file_name;

	wrf_send_command(WRF_COMMAND_SEND_FILE, params, 2);
}

void wrf_clear() 
{
	wrf_send_command(WRF_COMMAND_CLEAR, NULL, 0);
}

void wrf_factory_reset()
{
	wrf_send_command(WRF_COMMAND_FACTORY_RESET, NULL, 0);
}

void wrf_ask_status() 
{
	wrf_send_command(WRF_COMMAND_STATUS, NULL, 0);
}

#pragma endregion

#pragma region Response Handelers

static void send_response(wrf_result_code code, void* object) 
{
	if (on_response_cb)
		on_response_cb(code, object);
}

static void handle_result(json_value* value)
{
	if (CHECK_STR(WRF_RESULT_OK_STR, value))
		send_response(WRF_OK, NULL);
	else if (CHECK_STR(WRF_RESULT_SENT_STR, value))
		send_response(WRF_SENT, NULL);
	else if (CHECK_STR(WRF_RESULT_EMPTY_STR, value))
		send_response(WRF_EMPTY, NULL);
	else if (CHECK_STR(WRF_RESULT_FILE_SENT_STR, value))
		send_response(WRF_FILE_SENT, NULL);
	else if (CHECK_STR(WRF_RESULT_FILE_CANCEL_STR, value))
		send_response(WRF_FILE_CANCEL, NULL);
	else {
		wrf_error error = { LIB_ERROR_RESULT_UNKNOWN, WRF_ERROR_UNKNOWN_STR };
		send_response(WRF_LOCAL_ERROR, &error);
	}
}

static void handle_local_error(json_value* value)
{
	wrf_error error;
	if CHECK_STR(WRF_ERROR_SYSTEM_BUSY_STR, value)
		INIT_ERROR(WRF_ERROR_SYSTEM_BUSY, WRF_ERROR_SYSTEM_BUSY_STR)

	else if CHECK_STR(WRF_ERROR_SEND_REQUEST_FAILED_STR, value)
		INIT_ERROR(WRF_ERROR_SEND_REQUEST_FAILED, WRF_ERROR_SEND_REQUEST_FAILED_STR)

	else if CHECK_STR(WRF_ERROR_REVEICE_REQUEST_FAILED_STR, value)
		INIT_ERROR(WRF_ERROR_REVEICE_REQUEST_FAILED, WRF_ERROR_REVEICE_REQUEST_FAILED_STR)

	else if CHECK_STR(WRF_ERROR_MASTER_REQUEST_FAILED_STR, value)
		INIT_ERROR(WRF_ERROR_MASTER_REQUEST_FAILED, WRF_ERROR_MASTER_REQUEST_FAILED_STR)

	else if CHECK_STR(WRF_ERROR_NOT_ONLINE_STR, value)
		INIT_ERROR(WRF_ERROR_NOT_ONLINE, WRF_ERROR_NOT_ONLINE_STR)

	else if CHECK_STR(WRF_ERROR_REMOTE_ERROR_STR, value)
		INIT_ERROR(WRF_ERROR_REMOTE_ERROR, WRF_ERROR_REMOTE_ERROR_STR)

	else if CHECK_STR(WRF_ERROR_RX_OVERFLOW_STR, value)
		INIT_ERROR(WRF_ERROR_RX_OVERFLOW, WRF_ERROR_RX_OVERFLOW_STR)

	else if CHECK_STR(WRF_ERROR_SET_AP_MODE_FAILED_STR, value)
		INIT_ERROR(WRF_ERROR_SET_AP_MODE_FAILED, WRF_ERROR_SET_AP_MODE_FAILED_STR)

	else if CHECK_STR(WRF_ERROR_UPGRADE_STR, value)
		INIT_ERROR(WRF_ERROR_UPGRADE, WRF_ERROR_UPGRADE_STR)

	else if CHECK_STR(WRF_ERROR_COMMAND_FAILED_STR, value)
		INIT_ERROR(WRF_ERROR_COMMAND_FAILED, WRF_ERROR_COMMAND_FAILED_STR)

	else INIT_ERROR(LIB_ERROR_RESULT_UNKNOWN, WRF_ERROR_UNKNOWN_STR)

	send_response(WRF_LOCAL_ERROR, &error);
}

static void handle_status(json_value* _value) {
	
	if (_value->u.object.length != 6) {
		wrf_error error = { LIB_ERROR_PARSE_STATUS, LIB_ERROR_PARSE_STATUS_STR };
		send_response(WRF_LOCAL_ERROR, &error);
	}
	else {
		wrf_status status;
		status.connection_status = get_status((_value->u.object.values[0].value->u.string.ptr));
		memcpy(status.ip_addr, _value->u.object.values[1].value->u.string.ptr, _value->u.object.values[1].value->u.string.length);
		status.ip_addr[_value->u.object.values[1].value->u.string.length] = '\x0'; // Null terminate string
		status.visibility_status = strcmp(_value->u.object.values[2].value->u.string.ptr, "ON") == 0 ? true : false;
		status.last_error_code = get_error_code((_value->u.object.values[3].value->u.string.ptr));
		memcpy(status.last_error_msg, _value->u.object.values[4].value->u.string.ptr, (size_t)_value->u.object.values[4].value->u.string.length);
		status.last_error_msg[_value->u.object.values[4].value->u.string.length] = '\x0'; // Null terminate string
		status.successful_transfer_count = atoi(_value->u.object.values[5].value->u.string.ptr);

		send_response(WRF_STATUS, &status);
	}
}

static void handle_remote_error(json_value* value) 
{
	wrf_error error;

	if CHECK_STR(WRF_ERROR_UNDEFINED_ERROR_STR, value)
		INIT_ERROR(WRF_ERROR_UNDEFINED_ERROR, WRF_ERROR_UNDEFINED_ERROR_STR)

	else if CHECK_STR(WRF_ERROR_INVALID_JSON_STR, value)
		INIT_ERROR(WRF_ERROR_INVALID_JSON, WRF_ERROR_INVALID_JSON_STR)

	else if CHECK_STR(WRF_ERROR_INVALID_INTROSPECT_STR, value)
		INIT_ERROR(WRF_ERROR_INVALID_INTROSPECT, WRF_ERROR_INVALID_INTROSPECT_STR)

	else if CHECK_STR(WRF_ERROR_INVALID_HEADER_STR, value)
		INIT_ERROR(WRF_ERROR_INVALID_HEADER, WRF_ERROR_INVALID_HEADER_STR)

	else if CHECK_STR(WRF_ERROR_FORWARDING_ERROR_STR, value)
		INIT_ERROR(WRF_ERROR_FORWARDING_ERROR, WRF_ERROR_FORWARDING_ERROR_STR)

	else if CHECK_STR(WRF_ERROR_MISSING_HEADER_STR, value)
		INIT_ERROR(WRF_ERROR_MISSING_HEADER, WRF_ERROR_MISSING_HEADER_STR)

	else if CHECK_STR(WRF_ERROR_INVALID_TOKEN_STR, value)
		INIT_ERROR(WRF_ERROR_INVALID_TOKEN, WRF_ERROR_INVALID_TOKEN_STR)

	else if CHECK_STR(WRF_ERROR_INVALID_APP_STR, value)
		INIT_ERROR(WRF_ERROR_INVALID_APP, WRF_ERROR_INVALID_APP_STR)

	else INIT_ERROR(LIB_ERROR_RESULT_UNKNOWN, WRF_ERROR_UNKNOWN_STR)

	send_response(WRF_REMOTE_ERROR, &error);
}

static void handle_configuartion(json_value* value) 
{
	wrf_device_state state;
	memcpy(state.mac, value->u.object.values[0].value->u.string.ptr, value->u.object.values[0].value->u.string.length);
	state.mac[value->u.object.values[0].value->u.string.length] = '\x0'; // Nullterminate string!
	json_value* device_state = value->parent->u.object.values[1].value;
	state.rssi = atoi(device_state->u.object.values[0].value->u.string.ptr);
	
	send_response(WRF_CONFIG, &state);
}

static void handle_upgrade(json_value* value) 
{
	if (value->type == json_array) 
	{
		wrf_module_list list;
		list.size = value->u.array.length;
		for (int i = 0; i < list.size; i++)
			list.modules[i] = get_ota_module(value->u.array.values[i]->u.string.ptr);
		
		send_response(WRF_UPGRADE_PENDING, &list);
	}
	else if (value->type == json_object && value->u.object.length >0 && strcmp(WRF_SIZE_STR,value->u.object.values[0].name)==0)
	{
		ota_packet packet; 
		packet.size = (int)value->u.object.values[0].value->u.integer;
		memcpy(packet.crc, value->u.object.values[1].value->u.string.ptr, value->u.object.values[1].value->u.string.length);
		packet.crc[WRF_CRC_STRING_SIZE] = '\x0';

		send_response(WRF_UPGRADE_PACKAGE, &packet);
	}
	else
	{
		wrf_error error;
		INIT_ERROR(LIB_ERROR_PARSE_UPGRADE, LIB_ERROR_PARSE_UPGRADE_STR)
		send_response(WRF_LOCAL_ERROR, &error);
	}
}

static void handle_send_file(json_value* value) {
	char size_str[WRF_PARAM_SIZE];
	memcpy(size_str, value->u.string.ptr, value->u.string.length);
	send_response(WRF_SEND_FILE, size_str);
}

static void process_local_response(json_value* value) 
{
	if CHECK_NAME(WRF_RESULT_STR, value)
		handle_result(value->u.object.values[0].value);
	else if CHECK_NAME(WRF_ERROR_STR, value)
		handle_local_error(value->u.object.values[0].value);
	else if CHECK_NAME(WRF_COMMAND_STATUS_STR, value)
		handle_status(value->u.object.values[0].value);
	else if CHECK_NAME(WRF_COMMAND_UPGRADE_STR, value)
		handle_upgrade(value->u.object.values[0].value);
	else if CHECK_NAME(WRF_COMMAND_SEND_FILE_MAX_PACKET_SIZE_STR, value)
		handle_send_file(value->u.object.values[0].value);
	else{
		wrf_error error = { LIB_ERROR_UNKNOWN_OBJECT, LIB_ERROR_UNKNOWN_OBJECT_STR };
		send_response(WRF_LOCAL_ERROR, &error);
	}
}

static void process_remote_response(json_value* value) 
{
	if CHECK_NAME(WRF_REMOTE_ERROR_CODE_STR, value)
		handle_remote_error(value->u.object.values[0].value);
	else{
		wrf_error error;
		INIT_ERROR(LIB_ERROR_RESULT_UNKNOWN, WRF_ERROR_UNKNOWN_STR)
		send_response(WRF_REMOTE_ERROR, &error);
	}
}

static bool process_object(json_value* value)
{
	bool handeled_by_dd = true;
	if CHECK_NAME(WRF_LOCAL_RESPONSE_STR, value)
		process_local_response(value->u.object.values[0].value);
	else if CHECK_NAME(WRF_REMOTE_RESPONSE_STR, value)
		process_remote_response(value->u.object.values[0].value);
	else if CHECK_NAME(WRF_CONFIG_STR, value)
		handle_configuartion(value->u.object.values[0].value);
	else
		handeled_by_dd = false;

	return handeled_by_dd;
}

void wrf_handle_response(char* msg) {
	json_char* json;
	json_value* value;

	int len = strlen(msg);    
	if(msg[len-1] == WRF_EOT)
		len--;
		
	json = (json_char*)msg;
	value = json_parse(json, len);
	bool msg_proccesed = false;

	if (value && value->type == json_object)
		msg_proccesed = process_object(value);
	
	if(!msg_proccesed)
		send_response(WRF_MESSAGE, msg);

	json_value_free(value);
}
#pragma endregion

#pragma region Helper Methods

void add_cmd_param(wrf_param param, char* dest)
{
	strcat(dest,",");
	if (param.str_value) {
		JSON_PAIR_STR(param.name, param.str_value, dest);
	}
	else {
		JSON_PAIR_INT(param.name, param.i_value, dest);
	}
}

char* get_cmd_str(wrf_command cmd)
{
	switch (cmd)
	{
		case WRF_COMMAND_CLEAR:
			return WRF_COMMAND_CLEAR_STR;
		case WRF_COMMAND_DEEP_SLEEP:
			return WRF_COMMAND_DEEP_SLEEP_STR;
		case WRF_COMMAND_FACTORY_RESET:
			return WRF_COMMAND_FACTORY_RESET_STR;
		case WRF_COMMAND_CHECK_UPGRADE:
			return WRF_COMMAND_CHECK_UPGRADE_STR;
		case WRF_COMMAND_GET_UPGRADE:
			return WRF_COMMAND_GET_UPGRADE_STR;
		case WRF_COMMAND_INTROSPECT:
			return WRF_COMMAND_INTROSPECT_STR;
		case WRF_COMMAND_REBOOT:
			return WRF_COMMAND_REBOOT_STR;
		case WRF_COMMAND_STATUS:
			return WRF_COMMAND_STATUS_STR;
		case WRF_COMMAND_SETUP:
			return WRF_COMMAND_SETUP_STR;
		case WRF_COMMAND_UPGRADE:
			return WRF_COMMAND_UPGRADE_STR;
		case WRF_COMMAND_SEND_FILE:
			return WRF_COMMAND_SEND_FILE_STR;
		default:
			return WRF_COMMAND_UNKNOWN_STR;
	}	
}

char* get_mode_str(wrf_mode mode)
{
	switch (mode)
		{
			case WRF_OFF:
				return WRF_OFF_STR;
			case WRF_ON:
				return WRF_ON_STR;
			case WRF_MODE_NONE:
				return WRF_MODE_NONE_STR;
			case WRF_MODE_ALL:
				return WRF_MODE_ALL_STR;
			case WRF_MODE_LOCAL:
				return WRF_MODE_LOCAL_STR;
			case WRF_MODE_REMOTE:
				return WRF_MODE_REMOTE_STR;
			default:
				return NULL;
		}
}

char* get_ota_module_str(wrf_ota_module module) {
	switch (module)
	{
	case OTA_WRF01:
		return WRF_OTA_WRF01_STR;
	case OTA_CLIENT:
		return WRF_OTA_CLIENT_STR;
	default:
		return NULL;
	}
}

wrf_ota_module get_ota_module(char* module) {
	if (strcmp(module, WRF_OTA_WRF01_STR) == 0)
		return OTA_WRF01;
	else if (strcmp(module, WRF_OTA_CLIENT_STR) == 0)
		return OTA_CLIENT;
	else
		return OTA_UKNOWN;
}

char* get_ota_protocol_str(wrf_ota_protocol protocol) {
	switch (protocol)
	{
		case PROTOCOL_RAW:
			return WRF_OTA_PROTOCOL_RAW_STR;
		case PROTOCOL_ARDUINO:
			return WRF_OTA_PROTOCOL_ARDUINO_STR;
		default:
			return NULL;
	}
}

wrf_connection_status get_status(char* status_str) 
{
	if (strcmp(status_str, WRF_IDLE_STR)== 0)
		return WRF_IDLE;
	else if (strcmp(status_str, WRF_CONNECTING_STR) == 0)
		return WRF_CONNECTING;	
	else if (strcmp(status_str, WRF_GOT_IP_STR) == 0)
		return WRF_GOT_IP;
	else if (strcmp(status_str, WRF_CONNECTION_FAILED_STR) == 0)
		return WRF_CONNECTION_FAILED;
	else if (strcmp(status_str, WRF_DNS_FAILED_STR) == 0)
		return WRF_DNS_FAILED;
	else if (strcmp(status_str, WRF_NO_AP_FOUND_STR) == 0)
		return WRF_NO_AP_FOUND;
	else if (strcmp(status_str, WRF_WRONG_PASSWORD_STR) == 0)
		return WRF_WRONG_PASSWORD;
	else return WRF_UNKNOWN;
}

wrf_error_code get_error_code(char* code_str) {
	if (strcmp(code_str, WRF_ERROR_NONE_STR) == 0)
		return WRF_ERROR_NONE;
	else if (strcmp(code_str, WRF_ERROR_SYSTEM_BUSY_STR) == 0)
		return WRF_ERROR_SYSTEM_BUSY;
	else if (strcmp(code_str, WRF_ERROR_SEND_REQUEST_FAILED_STR) == 0)
		return WRF_ERROR_SEND_REQUEST_FAILED;
	else if (strcmp(code_str, WRF_ERROR_REVEICE_REQUEST_FAILED_STR) == 0)
		return WRF_ERROR_REVEICE_REQUEST_FAILED;
	else if (strcmp(code_str, WRF_ERROR_MASTER_REQUEST_FAILED_STR) == 0)
		return WRF_ERROR_MASTER_REQUEST_FAILED;
	else if (strcmp(code_str, WRF_ERROR_NOT_ONLINE_STR) == 0)
		return WRF_ERROR_NOT_ONLINE;
	else if (strcmp(code_str, WRF_ERROR_REMOTE_ERROR_STR) == 0)
		return WRF_ERROR_REMOTE_ERROR;
	else if (strcmp(code_str, WRF_ERROR_RX_OVERFLOW_STR) == 0)
		return WRF_ERROR_RX_OVERFLOW;
	else if (strcmp(code_str, WRF_ERROR_SET_AP_MODE_FAILED_STR) == 0)
		return WRF_ERROR_SET_AP_MODE_FAILED;
	else if (strcmp(code_str, WRF_ERROR_COMMAND_FAILED_STR) == 0)
		return WRF_ERROR_COMMAND_FAILED;
	else if (strcmp(code_str, WRF_ERROR_UPGRADE_STR) == 0)
		return WRF_ERROR_UPGRADE;
	else if (strcmp(code_str, WRF_ERROR_UNDEFINED_ERROR_STR) == 0)
		return WRF_ERROR_UNDEFINED_ERROR;
	else if (strcmp(code_str, WRF_ERROR_INVALID_JSON_STR) == 0)
		return WRF_ERROR_INVALID_JSON;
	else if (strcmp(code_str, WRF_ERROR_INVALID_INTROSPECT_STR) == 0)
		return WRF_ERROR_INVALID_INTROSPECT;
	else if (strcmp(code_str, WRF_ERROR_INVALID_HEADER_STR) == 0)
		return WRF_ERROR_INVALID_HEADER;
	else if (strcmp(code_str, WRF_ERROR_FORWARDING_ERROR_STR) == 0)
		return WRF_ERROR_FORWARDING_ERROR;
	else if (strcmp(code_str, WRF_ERROR_MISSING_HEADER_STR) == 0)
		return WRF_ERROR_MISSING_HEADER;
	else if (strcmp(code_str, WRF_ERROR_INVALID_TOKEN_STR) == 0)
		return WRF_ERROR_INVALID_TOKEN;
	else if (strcmp(code_str, WRF_ERROR_INVALID_APP_STR) == 0)
		return WRF_ERROR_INVALID_APP;
	else return LIB_ERROR_RESULT_UNKNOWN;
}

#pragma endregion
