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

#include <stdbool.h>
#include <string.h>
#include "app_button.h"
#include "boards.h"
#include "app_gpiote.h"
#include "app_timer.h"
#include "app_uart.h"
#include "wrf_sdk.h"
#include "json.h"

//TODO: Please get your product key at https://devicedrive.com/subscription
#define PRODUCT_KEY "<Your product key here>"

//Please refer to the serial specification for details on introspection syntax
//In this example we define one interface with two properties, status and power.
//  *  "status" is defined as read only
//  *  "power" is defined as read/write property
//TODO: Please update with your company name in the interface
#define INTROSPECTION_INTERFACES "\"interfaces\":[[\"com.devicedrive.light\",\
\"@status>s\",\
\"@version>s\",\
\"@power=b\"]]"
	
#define LIGHT_VERSION "1.0.NRF"

#define APP_TIMER_PRESCALER         0		/**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS        2		/**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE     3		/**< Size of timer operation queues. */
#define BUTTON_DEBOUNCE_DELAY		50		/**< Delay from a GPIOTE event until a button is reported as pushed. */ 
#define APP_GPIOTE_MAX_USERS        1		/**< Maximum number of users of the GPIOTE handler. */
#define APP_NUM_BUTTONS				1		/**< Number of buttons to registrate in this example */
#define UART_TX_BUF_SIZE			1024    /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE			1024    /**< UART RX buffer size. */
#define WRF_RECEIVE_BUFFER_SIZE		1024	/**< Size of buffer the WRF SDK is allowed to allocate */
#define WRF_SEND_QUEUE_SIZE			10		/**< Size of send queue */
#define POLL_DELAY_MS				3000	/**< Poll intervall in milliseconds */

#define APP_LED					LED_1		/**< The pin for our led,	  @note when using NRF52 Dev board, check that pins are not used by WRF01 Shield */
#define APP_BUTTON				BUTTON_3	/**< The pin for our button,  @note when using NRF52 Dev board, check that pins are not used by WRF01 Shield */
	
#define STATUS_OFF_STR				"Off"	/**< String to populat the status field in introspect*/
#define STATUS_ON_STR				"On"	/**< String to populat the status field in introspect*/

WRF *wrf;									/**< Pointer to our wrf instance, see @ref wrf_init */				
wrf_config config;							/**< Our Wrf config, see @ref wrf_init */
uint32_t uart_write_string(char* string);	/**< Forward declaring the function for writing to uart connected to the WRF01 */

uint32_t power = 0;							/**< Indicating whether our light is on or off. */
APP_TIMER_DEF(poll_timer_id);				/**< Creating a timer id for out polling event  */


static void button_handler(uint8_t pin_no, uint8_t button_action);	/**< Forward declaring function for handeling buttons */

				 
#pragma region Uart

void uart_error_handle(app_uart_evt_t * p_event)
{
	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
	{
		// TODO: Handle error
		APP_ERROR_HANDLER(p_event->data.error_communication);
	}
	else if (p_event->evt_type == APP_UART_FIFO_ERROR)
	{
		// TODO: Handle error
		APP_ERROR_HANDLER(p_event->data.error_code);
	}
}

void init_uart()
{
	uint32_t err_code;
	const app_uart_comm_params_t comm_params =
	{
		ARDUINO_0_PIN,
		ARDUINO_1_PIN,
		0,
		0,
		APP_UART_FLOW_CONTROL_DISABLED,
		false,
		UART_BAUDRATE_BAUDRATE_Baud115200
	};
	APP_UART_FIFO_INIT(&comm_params,
		UART_RX_BUF_SIZE,
		UART_TX_BUF_SIZE,
		uart_error_handle,
		APP_IRQ_PRIORITY_LOW,
		err_code);

	APP_ERROR_CHECK(err_code);
}

uint32_t uart_write_string(char* s)
{	
	uint32_t err_code;
	uint8_t len = strlen((char *) s);
	for (uint8_t i = 0; i < len; i++)
	{
		err_code = app_uart_put(s[i]);
		APP_ERROR_CHECK(err_code);
	}
	return 0;
}
#pragma endregion	

#pragma region Initialization functions
				  
void init_clock()
{
	NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_LFCLKSTART    = 1;
	while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
		; // Wait for clock to start
}	

void init_leds()
{
	nrf_gpio_cfg_output(LED_1);
	nrf_gpio_pin_set(LED_1);
}

void init_buttons()
{
	uint32_t err_code;

	// Button configuration structure.
	static app_button_cfg_t p_button[] = { { BUTTON_3, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler } };

	// Initializing the buttons.
	err_code = app_button_init(p_button, APP_NUM_BUTTONS, BUTTON_DEBOUNCE_DELAY);
	APP_ERROR_CHECK(err_code);
                                            
	// Enabling the buttons.										
	err_code = app_button_enable();
	APP_ERROR_CHECK(err_code);
}

#pragma endregion

#pragma region Light functionality

/** @brief	Function for sending the status of our light.*/
void send_status()
{
	char* msg = (char*)malloc(100);
	sprintf(msg, "{\"com.devicedrive.light\":{\"status\":\"%s\",\"power\":%lu,\"version\" :\"%s\"}}",
			power?STATUS_ON_STR:STATUS_OFF_STR,
			power,
			LIGHT_VERSION);
	wrf->send(msg);
	free(msg);
}

/** @brief	Function for toggel led*/
void set_led(bool on)
{
	// Set the LED and power state
	nrf_gpio_pin_write(APP_LED, !on);
	power = on;
	
	// Now we want to tell the cloud that we have changed state
	send_status();
}

/** @brief	Function for handleing messages from the cloud */ 
void handle_message(char* msg)
{
	json_char* json;
	json_value* value;

	// Remowing the EOT char from the JSON
	int len = strlen(msg);    
	if (msg[len - 1] == WRF_EOT)
		len--;
	
	json = (json_char*)msg;
	value = json_parse(json, len);
	uint32_t new_power = power;

	// Check if message conntains a value for our power.
	if (value && value->type == json_object) {
		if (strcmp(value->u.object.values[0].name, "com.devicedrive.light") == 0) {
			value = value->u.object.values[0].value;
			int length = value->u.object.length;
			for (int x = 0; x < length; x++) {
				if (strcmp(value->u.object.values[x].name, "power") == 0)
					new_power = atoi(value->u.object.values[x].value->u.string.ptr);
			}
		}
	}
	
	// Now we set the LED to the new value
	set_led(new_power);
}

/** @brief	Function for handeling poll timer callbacks 
*	
*	@note	This is a very simple way to do polling, and may cause problems.
*			This implementation simply adds a poll message to the queue every
*			@ref POLL_DELAY_MS, and therefore it may flood the message queue. 
*			For a more robust implementation, only do a poll if there are no awaiting polls. 
*/
static void poll_timeout_handler(void * p_context)
{
	wrf->poll();
}

/** @brief Function for handeling button events */
static void button_handler(uint8_t pin_no, uint8_t button_action)
{
	if (button_action == APP_BUTTON_PUSH && pin_no == APP_BUTTON)
		set_led(!power);
}

#pragma endregion

#pragma region  WRF
			
void onWrfStart()
{
	wrf->send_config(config);
}

void onWrfError(wrf_error *error)
{
	// TODO Handle errors
}

void onWrfNotConnected()
{
	// If we loose connection we want to stopp polling and make WRF01 visible for Linkup
	app_timer_stop(poll_timer_id);
	wrf->setVisibility(-1);
}

void onWrfConnected(wrf_device_state* state)
{
	// When we connect we want to tell the world about what we can do!
	wrf->sendIntrospect(INTROSPECTION_INTERFACES);
	// Then we tell them how we are
	send_status();
	// And we start asking if we shold do anything
	app_timer_start(poll_timer_id, APP_TIMER_TICKS(POLL_DELAY_MS, APP_TIMER_PRESCALER), NULL);
}

void onMessageReceived(char* msg)
{	
	// If we get an message form the world, we check if we are able to understand what they ask of us, then we do it!
	handle_message(msg);
	// We also ask if we should change ourself for the better!
	wrf->checkPendingUpgrades();
}

void onPendingUpgrades(wrf_module_list* list)
{
	// We check if someone wants our WRF01 to become better, then we upgrade it.
	// Here you can check if there is a clinet upgrade for the NRF also. 
	for (int i = 0; i < list->size; i++) {
		if (list->modules[i] == OTA_WRF01) {
			wrf->startWrfUpgrade();
		}
	}
}

void init_wrf()
{
	// Need to set these pins on nrf52 devboard with WRF01 Sheild
	nrf_gpio_pin_write(ARDUINO_9_PIN, 0);	
	nrf_gpio_pin_write(ARDUINO_8_PIN, 0);
	nrf_gpio_pin_write(ARDUINO_5_PIN, 1);
	
	// First we need to get our WRF instance
	wrf = WRF::getInstance(uart_write_string, WRF_RECEIVE_BUFFER_SIZE, WRF_SEND_QUEUE_SIZE);
	
	// Then we set up our wanted configurations
	DEFAULT_WRF_CONFIG(config);
	config.debug_mode = WRF_MODE_ALL;
	config.silent_connect = false;
	config.product_key = PRODUCT_KEY;
	config.version = LIGHT_VERSION;
	
	// At last we registrate the callbacks we want to handle.
	wrf->onError(onWrfError);
	wrf->onPowerUp(onWrfStart);
	wrf->onConnected(onWrfConnected);
	wrf->onNotConnected(onWrfNotConnected);
	wrf->onMessageReceived(onMessageReceived);
	wrf->onPendingUpgrades(onPendingUpgrades);
}
	  
#pragma endregion

int main(void)
{	
	// Macro for initializing the application timer module.
	// It will handle dimensioning and allocation of the memory buffer required by the timer, 
	// making sure that the buffer is correctly aligned. It will also connect the timer module to the scheduler (if specified).
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
 
	// Macro for initializing the GPIOTE module.
	// It will handle dimensioning and allocation of the memory buffer required by the module, making sure that the buffer is correctly aligned.
	APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
	
	init_leds();
	init_clock();
	init_buttons();
	init_uart();
	init_wrf();

	// When we start we want to tell the WRF01 how to behave!
	wrf->send_config(config);

	// Create the poll timer! It is started in @ref onWrfConnected and stopped in @ref onWrfNotConnected
	app_timer_create(&poll_timer_id, APP_TIMER_MODE_REPEATED, poll_timeout_handler);
	
	while (true)
	{
		// We read our uart and asks wrf to handle it.
		char byte = '\x0';
		int res = app_uart_get((uint8_t*)&byte);
		if (res == NRF_SUCCESS)
		{
			wrf->registerChar(byte);
		}
		
		// We need to give the WRF SDK the possibility to send messages to the WRF. 
		wrf->handleSendQueue();
		__SEV();
		__WFE();
		__WFE();
	}
}