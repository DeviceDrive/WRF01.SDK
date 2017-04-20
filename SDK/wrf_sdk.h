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

/**
*	Version		Description											Who		When
*	----------------------------------------------------------------------------------
*	1.0			Created wrf_sdk										ASB		27.01.2017
*	----------------------------------------------------------------------------------
*	1.1			added createInstance(writer, int int) and			
*				getInstance()										ASB		28.02.2017
*	----------------------------------------------------------------------------------
*	1.2			Added sendFile functionality						PSB		21.03.2017
*	----------------------------------------------------------------------------------
*/	
#pragma once
extern "C" {
#include "wrf.h"
#include "crc32.h"
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

#pragma region Send file struct

typedef struct {
	int size;
	char* data;
	int max_packet_size;
	int sent_bytes;
}send_file_file_struct;

#pragma endregion

#pragma region Emums

enum wrf_operating_mode {
	NORMAL,
	FILE_TRANSFER
};

#pragma endregion

#define ACK_CHAR ((char)0x06)
#define NAK_CHAR ((char)0x15)
#define CAN_CHAR ((char)0x18)


#pragma region Callback definitions
typedef void WrfCallback();
typedef void WrfErrorCallback(wrf_error *error);
typedef void WrfMessageReceivedCallback(char* msg);
typedef void WrfUpgradeCallback(wrf_module_list* list);
typedef void WrfConnectCallback(wrf_device_state* state);
typedef void WrfStatusReceivedCallback(wrf_status* status);
typedef void WrfSendFileCallback(wrf_send_file_status* code);
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

/*	@brief		Function signature for handeling packages during file sending.
*
*	@details	This function must be implemented to send file. 
*				Every time the WRF01 is ready to send another packet, this 
*				function wil be used to retrive the next data to send
*
*	@param		dest		where to store data to be sendt.
*	@param		max_length	Maximum length of data that can be written to @ref dest
*	@retval		length		Length of bytes added to buffer
*/
typedef int packet_handler(unsigned char* dest, int max_length);
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
	WrfSendFileCallback* _send_file_cb = NULL;
	pre_handle_response* response_handler_override = NULL;
	packet_handler* _packet_handler = NULL;
	
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

private:
	wrf_operating_mode _wrf_mode;

	unsigned char *data_packet = NULL;

	int packet_bytes_sent = 0;
	int bytes_sent_ack = 0;
	int file_size = 0;
	int max_packet_size = 0;

	void sendFilePacket(bool resend);
	void sendNextFilePacket();
	void resendFilePacket();
	void abortFileTransfer();

public :
	static WRF* createInstance(wrf_write_string writer, int receive_buffer_size, int queue_size);
	static WRF* getInstance();
	static void freeInstance();

	//void set_uart_buffer_writer(wrf_write_buffer writer);

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

	void sendFile(char* file_name, int file_size, packet_handler handler); 

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
	void onSendFileEvents(WrfSendFileCallback *send_file_cb);
};