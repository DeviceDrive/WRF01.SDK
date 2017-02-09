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

#include "wrf_sdk.h"
#include <stdio.h>

#pragma region MessageQueue Implementation

Queue::Queue(int max_queue_size){
	_count = 0;
	_size = max_queue_size;
	_first = _last = 0;
	_data = (char**)malloc(_size * sizeof(char*));
}

Queue::~Queue(){
	clear();
	free(_data);
}

bool Queue::push(char* str){
	if (_count >= _size) return false;

	int len = strlen(str);
	_data[_last] = (char*)malloc(len+1);
	strcpy(_data[_last++], str);
	_count++;
	if (_last >= _size) _last = 0;
	return true;
}
char* Queue::peek(){
	return _data[_first];
}

void Queue::pop(){
	free(_data[_first++]);
	_count--;
	if (_first >= _size) _first = 0;
}

void  Queue::clear(){
	while (!empty())
		pop();
}
bool Queue::empty() {
	return _count == 0;
}

int  Queue::count(){
	return _count;
}

#pragma endregion

#pragma region WRF_SDK Implementation

WRF *WRF::instance;

WRF::WRF() {};

WRF::WRF(wrf_write_string writer, int receive_buffer_size, int queue_size)
{
	init_instance(writer, receive_buffer_size, queue_size);
}

WRF::~WRF()
{
	clearQueue();
	free(_receive_buffer.data);
	delete _queue;
}

void WRF::init_instance(wrf_write_string writer, int receive_buffer_size, int queue_size)
{
	_uart_writer = writer;
	_uart_log = NULL;
	_queue = new Queue(queue_size);
	_receive_buffer.allocated = receive_buffer_size;
	_receive_buffer.length = 0;
	_receive_buffer.data = (char*)malloc(_receive_buffer.allocated);
	_is_sending = false;
}

#pragma region Init Singlton

void WRF::setInstance(WRF * new_instance)
{
	if (instance)
		freeInstance();
	instance = new_instance;
	wrf_init((wrf_write_string)add_message_to_queue);
	wrf_on_response(handle_response);
}

void WRF::set_handle_response_override(pre_handle_response handler)
{
	response_handler_override = handler;
}

WRF* WRF::getInstance(wrf_write_string writer, int receive_buffer_size, int queue_size)
{
	if (!instance) {
		instance = new WRF(writer, receive_buffer_size, queue_size);
		wrf_init((wrf_write_string)add_message_to_queue);
		wrf_on_response(handle_response);
	}
	return instance;
}

void WRF::freeInstance()
{
	delete(instance);
	instance = NULL;
}

#pragma endregion

#pragma region Handle responses and queue

void WRF::handle_response(wrf_result_code code, void * object)
{
	bool has_been_handeled = false;

	if (instance->response_handler_override)
		has_been_handeled = instance->response_handler_override(code, object);

	if (!has_been_handeled) {
		bool is_busy = false;
		switch (code)
		{
			case WRF_MESSAGE:
				if (instance->_message_received_cb)
					instance->_message_received_cb((char*)object);
				break;
			case WRF_LOCAL_ERROR:
			case WRF_REMOTE_ERROR:
				if (((wrf_error*)object)->code == WRF_ERROR_NOT_ONLINE && instance->_not_connected_cb)
					instance->_not_connected_cb();
				if (instance->_error_cb)
					instance->_error_cb((wrf_error*)object);
				if (((wrf_error*)object)->code == WRF_ERROR_SYSTEM_BUSY)
					is_busy = true;
				break;
			case WRF_CONFIG:
				if (instance->_connect_cb)
					instance->_connect_cb((wrf_device_state*)object);
				break;
			case WRF_STATUS:
				if (instance->_status_received_cb)
					instance->_status_received_cb((wrf_status*)object);
				break;
			case WRF_SENT:
				if (instance->_message_sent_cb)
					instance->_message_sent_cb();
				break;
			case WRF_UPGRADE_PENDING:
				if (instance->_pending_upgrades_cb)
					instance->_pending_upgrades_cb((wrf_module_list*)object);
				break;

			case WRF_EMPTY:
			case WRF_OK:
			case WRF_UPGRADE_PACKAGE:
				break;
		}
		if (instance->_is_sending && !is_busy && !instance->_queue->empty())
			instance->_queue->pop();
		if (is_busy && instance->_is_sending)
			instance->_is_sending = true; // Keep awaiting response
		else
			instance->_is_sending = false;
	}
}

void WRF::add_message_to_queue(char * msg)
{
	instance->_queue->push(msg);
}

void WRF::registerChar(char byte)
{
	_receive_buffer.data[_receive_buffer.length++] = byte;

	if (byte == ETX_CHAR && _receive_buffer.length >= 2) 
	{
		if (_receive_buffer.data[_receive_buffer.length - 2] == STX_CHAR
			&& _power_up_cb)
			_power_up_cb();

	}
	else if (byte == (char)WRF_EOT) 
	{
		_receive_buffer.data[_receive_buffer.length] = 0x0;
		wrf_handle_response(_receive_buffer.data);
		_receive_buffer.length = 0;
		_receive_buffer.data[_receive_buffer.length] = 0x0;
	}
}

void WRF::registerString(char * str)
{
	int len = strlen(str);
	for (int i = 0; i < len; i++)
		registerChar(str[i]);
}

void WRF::handleSendQueue()
{
	if (!_queue->empty() > 0 && !instance->_is_sending)
	{
		_uart_writer(_queue->peek());
		instance->_is_sending = true;
	}
}

int WRF::getQueueCount()
{
	return _queue->count();
}

void WRF::clearQueue()
{
	_queue->clear();
}

#pragma endregion

#pragma region  Wraper Methods

void WRF::send_config(wrf_config &config)
{
	wrf_send_config(&config);
}


void WRF::connect()
{
	wrf_connect(false);
}

void WRF::poll()
{
	wrf_send_message((char* const)"");
}

void WRF::checkPendingUpgrades()
{
	wrf_check_upgrade();
}

void WRF::startWrfUpgrade()
{
	ota_params params;
	params.module = OTA_WRF01;
	params.delay = 0;
	params.file_no = 0;
	params.pin_toggle = "";
	params.protocol = PROTOCOL_RAW;
	wrf_get_upgrade(&params);
}

void WRF::startClientUpgrade(ota_params & params)
{
	wrf_get_upgrade(&params);
}

void WRF::send(char * raw_string)
{
	wrf_send_message(raw_string);
}

void WRF::sendCommand(wrf_command cmd, wrf_param * params, int num_params)
{
	wrf_send_command(cmd, params, num_params);
}

void WRF::sendIntrospect(char * introspect)
{
	wrf_send_introspect(introspect);
}

void WRF::setVisibility(int seconds)
{
	wrf_set_visible(seconds);
}

void WRF::setVisibility(int seconds, bool trigger_connect_cb)
{
	char s[10];
	sprintf(s, "%d", seconds);
	char t[10];
	sprintf(t, "%d", !trigger_connect_cb);
	wrf_param params[] = {
		{ WRF_SETUP_SILENT_CONNECT_STR, t },
		{ WRF_SETUP_VISIBILITY_STR, s }
	};
	sendCommand(WRF_COMMAND_SETUP, params, 2);
}

void WRF::reboot()
{
	wrf_reboot();
}

void WRF::deepSleep(int duration)
{
	wrf_deep_sleep( duration);
}

void WRF::clear()
{
	wrf_clear();
}

void WRF::factoryReset()
{
	wrf_factory_reset();
}

void WRF::requestStatus()
{
	wrf_ask_status();
}

#pragma endregion

#pragma region Register Callbacks


void WRF::onError(WrfErrorCallback * error_cb)
{
	_error_cb = error_cb;
}

void WRF::onConnected(WrfConnectCallback * connection_cb)
{
	_connect_cb = *connection_cb;
}

void WRF::onMessageSent(WrfCallback * message_sent_cb)
{
	_message_sent_cb = message_sent_cb;
}

void WRF::onPowerUp(WrfCallback * power_up)
{
	_power_up_cb = power_up;
}

void WRF::onMessageReceived(WrfMessageReceivedCallback * message_received_cb)
{
	_message_received_cb = message_received_cb;
}

void WRF::onPendingUpgrades(WrfUpgradeCallback * pending_upgrades_cb)
{
	_pending_upgrades_cb = pending_upgrades_cb;
}

void WRF::onNotConnected(WrfCallback * not_connected_cb)
{
	_not_connected_cb = not_connected_cb;
}

void WRF::onStatusReceived(WrfStatusReceivedCallback * status_received_cb)
{
	_status_received_cb = status_received_cb;
}

#pragma endregion

#pragma endregion