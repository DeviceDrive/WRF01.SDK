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
*	@brief	WRF01 cpp module and a Queue
*	
*	@details This WRF01 cpp module wraps the C parser (@ref wrf.h) into a singelton object
*			 which is easy to use. It handles the WRF01 and queues messages. 
*			 We strongly recommend tha user of this module to take a look at
*			 WRF01 serial specification which can be found at
*			 https://devicedrive.com/downloads/
*/
#pragma once
extern "C" {
#include "wrf.h"
#include "stdlib.h"
#include "string.h"
}

#pragma region Message Queue

class Queue
{
private:
	int _count;
	int _size;
	int _first, _last;
	char** _data;
public:
	Queue(int max_queue_size);
	~Queue();

	bool push(char* str);
	char* peek();
	void pop();
	void clear();
	bool empty();
	int count();
};
#pragma endregion

#pragma region Buffer

typedef struct {
	size_t allocated;
	size_t length;
	char* data;
}buffer;
#pragma endregion

#pragma region Callback definitions
typedef void WrfCallback();
typedef void WrfErrorCallback(wrf_error *error);
typedef void WrfMessageReceivedCallback(char* msg);
typedef void WrfUpgradeCallback(wrf_module_list* list);
typedef void WrfConnectCallback(wrf_device_state* state);
typedef void WrfStatusReceivedCallback(wrf_status* status);
#pragma endregion

/*	@brief		Function signature for handeling response
*
*	@details	@ref wrf.h should handle responses, but if something needs to be handled
*				differently this method should be implemented.
*
*	@note		Set this method in @ref set_handle_response_override.
*				
*	@retval		false	if base object should handle the object.
*	@retval		true	if not.
*/
typedef bool pre_handle_response(wrf_result_code code, void * object);

class WRF {

protected:
	WrfCallback* _message_sent_cb = NULL;
	WrfCallback* _not_connected_cb = NULL;
	WrfCallback* _power_up_cb = NULL;
	WrfMessageReceivedCallback* _message_received_cb = NULL;
	WrfErrorCallback* _error_cb = NULL;
	WrfConnectCallback* _connect_cb = NULL;
	WrfStatusReceivedCallback* _status_received_cb = NULL;
	WrfUpgradeCallback* _pending_upgrades_cb = NULL;
	pre_handle_response* response_handler_override = NULL;

	wrf_write_string _uart_writer;
	wrf_write_string _uart_log;
	buffer _receive_buffer;
	Queue* _queue;
	bool _is_sending;
	static WRF *instance;

	WRF();
	WRF(wrf_write_string writer, int receive_buffer_size, int queue_size);
	~WRF();

	void init_instance(wrf_write_string writer, int receive_buffer_size, int queue_size);
	void set_handle_response_override(pre_handle_response handler);

	static void setInstance(WRF* instance);
	static void handle_response(wrf_result_code code, void* object);
	static void add_message_to_queue(char* msg);

public :
	static WRF* getInstance(wrf_write_string writer, int receive_buffer_size, int queue_size);
	static void freeInstance();

	void registerChar(char byte);
	void registerString(char* str);
	void handleSendQueue();
	int getQueueCount();
	void clearQueue();

	void send_config(wrf_config &config);

	void connect();
	void poll();

	void checkPendingUpgrades();
	void startWrfUpgrade();
	void startClientUpgrade(ota_params &params);

	void send(char* raw_string);
	void sendCommand(wrf_command cmd, wrf_param* params, int num_params);

	void sendIntrospect(char* introspect);
	void setVisibility(int seconds);
	void setVisibility(int seconds, bool trigger_connect_cb);
	void reboot();
	void deepSleep(int duration);
	void clear();
	void factoryReset();
	void requestStatus();

	void onError(WrfErrorCallback *error_cb);
	void onConnected(WrfConnectCallback *connection_cb);
	void onMessageSent(WrfCallback *message_sent_cb);
	
	void onPowerUp(WrfCallback *power_up_cb); 
	void onMessageReceived(WrfMessageReceivedCallback *message_received_cb);
	void onPendingUpgrades(WrfUpgradeCallback *pending_upgrades_cb);
	void onNotConnected(WrfCallback *not_connected_cb);
	void onStatusReceived(WrfStatusReceivedCallback *status_received_cb);
};