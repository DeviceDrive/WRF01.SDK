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

#ifndef _WRFARDUINOLIB_h
#define _WRFARDUINOLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "wrf_sdk.h"

#define DEFAULT_WRF_RECEIVE_BUFFER_SIZE 1024
#define DEFAULT_WRF_QUEUE_SIZE 10

class WRFArduino : public WRF {
private:

	static uint32_t write_serial(char *str);
	void read_serial();

	bool is_polling;
	bool awaiting_poll;
	long last_poll;
	int poll_intervall;

	void handle_poll(long now_ms);

protected:
	WRFArduino();
	WRFArduino(int receive_buffer, int queue_size);
public:

	static bool handle_response(wrf_result_code code, void * object);

	void setup();
	void init(wrf_config & config);
	void handle();


	void startPoll(int ms_intervall);
	void stopPoll();

	void registerString(String str);
	void send(String raw_string);
	void sendIntrospect(String introspect);
	void startClientUpgrade(int delay);

	static WRFArduino& getInstance();
	static WRFArduino& getInstance(int receive_buffer, int queue_size);
};
#endif

