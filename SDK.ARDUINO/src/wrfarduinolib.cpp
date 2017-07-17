/*	Copyright 2016 DeviceDrive AS
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

#include "wrfarduinolib.h"

unsigned char* _data_to_send;

#pragma region Handle Uart

uint32_t WRFArduino::write_serial(unsigned char* str, int length) {
	Serial1.write(str, length);
	return 0;
}

void WRFArduino::read_serial() {
	while (Serial1.available() > 0)
		registerChar(Serial1.read());
}

#pragma endregion

void WRFArduino::handle_poll(long now_ms)
{
	if (is_polling && !awaiting_poll  && now_ms > (last_poll + poll_intervall)) {
		poll();
		last_poll = now_ms;
		awaiting_poll = true;
	}
	else if (is_polling && awaiting_poll && now_ms > (last_poll + (poll_intervall * 3))) {
		awaiting_poll = false;
	}
}

void WRFArduino::handle_sendfile_cb( int length)
{
	WRF* wrf = WRF::getInstance();
	wrf->sendFilePacket(_data_to_send, length);	
}

WRFArduino::WRFArduino() :
WRF() {
	is_polling = false;
	awaiting_poll = false;
	last_poll = 0;
	poll_intervall = 10000;
	WRF::init_instance(
		(wrf_write_string)write_serial, 
		DEFAULT_WRF_RECEIVE_BUFFER_SIZE, 
		DEFAULT_WRF_QUEUE_SIZE);
	set_handle_response_override(handle_response);
	WRF::setInstance(this);
}

WRFArduino::WRFArduino(int receive_buffer, int queue_size) :
WRF() {
	is_polling = false;
	awaiting_poll = false;
	last_poll = 0;
	poll_intervall = 10000;
	WRF::init_instance((wrf_write_string)write_serial, receive_buffer, queue_size);
	WRF::setInstance(this);
}

bool WRFArduino::handle_response(wrf_result_code code, void * object)
{
	switch (code)
	{
	case WRF_EMPTY:
	case WRF_MESSAGE:
		((WRFArduino*)instance)->awaiting_poll = false;
		break;
	case WRF_LOCAL_ERROR:
	case WRF_REMOTE_ERROR:
		if (((wrf_error*)object)->code != WRF_ERROR_SYSTEM_BUSY)
			((WRFArduino*)instance)->awaiting_poll = false;
		break;
	default:
		break;
	}
	return false;
}

void WRFArduino::setup()
{
	Serial1.begin(115200);
}

void WRFArduino::init(wrf_config & config)
{
	send_config(config);
}

void WRFArduino::handle()
{
	read_serial();
	handleSendQueue();
	handle_poll(millis());
}

void WRFArduino::startPoll(int ms_intervall)
{
	is_polling = true;
	awaiting_poll = false;
	last_poll = 0;
	poll_intervall = ms_intervall;
}

void WRFArduino::stopPoll()
{
	is_polling = false;
	awaiting_poll = false;
}

void WRFArduino::registerString(String str)
{
	char *p = const_cast<char*>(str.c_str());
	instance->registerString(p);
}

void WRFArduino::send(String raw_string)
{
	char *p = const_cast<char*>(raw_string.c_str());
	instance->send(p);
}

void WRFArduino::sendFile(String file_name, int file_size, unsigned char* file) 
{
	char *name = const_cast<char*>(file_name.c_str());
	_data_to_send = file;
	instance->sendFile(name, file_size, handle_sendfile_cb);
}


void WRFArduino::sendIntrospect(String introspect)
{
	char *p = const_cast<char*>(introspect.c_str());
	instance->sendIntrospect(p);
}

void WRFArduino::startClientUpgrade(int delay)
{
	ota_params params;
	params.delay = delay;
	params.file_no = 0;
	params.module = OTA_CLIENT;
	params.pin_toggle = "010";
	params.protocol = PROTOCOL_ARDUINO;
	instance->startClientUpgrade(params);
}

WRFArduino& WRFArduino::getInstance(){
	if (!instance)
		new WRFArduino();
	return*((WRFArduino*)instance);
}

WRFArduino & WRFArduino::getInstance(int receive_buffer, int queue_size){
	if (!instance)
		new WRFArduino(receive_buffer, queue_size);
	return*((WRFArduino*)instance);
}
