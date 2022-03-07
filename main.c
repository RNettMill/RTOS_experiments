/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#include <time.h>

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

/* Demo includes. */
#include "supporting_functions.h"

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xffffff )

//define sync bits for the syncing tasks
#define syncBit0 1UL
#define syncBit1 (1UL<<1)
#define syncBit2 (1UL<<2)


/* The task functions. */
void vPrintTask(void* pvParameters);
uint32_t fibonacci(uint32_t num);
void vTerribleFib(void* pvParameters);
void vQueueSender(void* pvParameters);
void vQueueReceiver(void* pvParameters);
void vTimerFunction(void);
static void vSynchTask(void* pvParameters);

//event grouping bitfields
EventGroupHandle_t xEventGroup;

/* Pseudo random number generation functions - implemented in this file as the
MSVC rand() function has unexpected consequences. */
static uint32_t prvRand(void);
static void prvSRand(uint32_t ulSeed);

//queues to use
QueueHandle_t xNumQueue;

//timers
TimerHandle_t xSimpleTimer;

/* Use by the pseudo random number generator. */
static uint32_t ulNextRand;

//fake data for queues
typedef enum {
data0 = 100,
data1 = 200
} Fake_data_t;

//data struct format
typedef struct
{
    uint8_t ucValue;
    Fake_data_t eDataSource;
} Data_t;

uint32_t numberParameters[] = { 0, 1 };

/*-----------------------------------------------------------*/

int main( void )
{
    xNumQueue = xQueueCreate(5, sizeof(uint32_t));

	/* Create one of the two tasks. */
    xTaskCreate(vPrintTask, //pointer to function
        "Print1", //text name for task
        100, //stack depth
        "Task 1\r\n", //task parameter note used
        2, //priority 2
        NULL); //task handle not used

	/* Create the other task in exactly the same way. */
	xTaskCreate( vPrintTask, "Task 2", 1000, "Task 2\r\n", 2, NULL );
    xTaskCreate( vTerribleFib, "FibTask", 1000, NULL, 1, NULL);

    //create a timer
    const char* timerName = "simpTime";
    const TickType_t xDelay100ms = pdMS_TO_TICKS(10UL); //timeout time
    xSimpleTimer = xTimerCreate(timerName, //timer name
        xDelay100ms, //period
        pdFALSE, //auto reload 
        0, //timer id
        vTimerFunction); //timer callback function

    //create two tasks to feed the queue and one to read from it
    xTaskCreate(vQueueSender, "Queue0", 100, numberParameters[0], 2, NULL);
    xTaskCreate(vQueueSender, "Queue1", 100, numberParameters[1], 2, NULL);
    xTaskCreate(vQueueReceiver, "Rec1", 100, NULL, 3, NULL);

    //create the syncing events and their event bits
    xEventGroup = xEventGroupCreate();
    xTaskCreate(vSynchTask, "Sync0", 100, syncBit0, 4, NULL);
    xTaskCreate(vSynchTask, "Sync1", 100, syncBit1, 4, NULL);
    xTaskCreate(vSynchTask, "Sync2", 100, syncBit2, 4, NULL);

	/* Start the scheduler to start the tasks executing. */
	vTaskStartScheduler();	

	/* The following line should never be reached because vTaskStartScheduler() 
	will only return if there was not enough FreeRTOS heap memory available to
	create the Idle and (if configured) Timer tasks.  Heap management, and
	techniques for trapping heap exhaustion, are described in the book text. */
	for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/

void vTimerFunction(void) {
    vPrintString("timer \r\n");
}

uint32_t fibonacci(uint32_t num) {
    if (num < 2) return num;
    else {
        num = fibonacci(num - 1) + fibonacci(num - 2);
        return num;
    }
}
/*
* Calculates the fibonnaci sequence in the worst way possible, lowest priority
*/
void vTerribleFib(void* pvParameters) {

    uint32_t number = 0;
    for (;;) {
        uint32_t nextNum = fibonacci(number);
        vPrintStringAndNumber("fib: ", nextNum);
        //vPrintStringAndNumber("maxStack: ", uxTaskGetStackHighWaterMark(NULL));
        number++;
    }
}
void vPrintTask(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xDelay1000ms = pdMS_TO_TICKS(1000UL); //timeout time
    volatile uint32_t ul; //vol ensures ul is not optimized away

    char* printText = (char*)pvParameters;

    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        //print name of task
        vPrintString(printText);

        //one of the tasks should start a timer
        if (printText[5] == '1') xTimerStart(xSimpleTimer, 0);

        //block this task until 1000ms have passed
        vTaskDelayUntil(&xLastWakeTime, xDelay1000ms); 
    }
}

void vQueueSender(void* pvParameters)
{
    TickType_t xLastWakeTime;
    BaseType_t xQueueStatus;
    const TickType_t xDelay1000ms = pdMS_TO_TICKS(2000UL); //timeout time
    //how long to wait for queue to empty
    const TickType_t xQueueWaitTicks = pdMS_TO_TICKS(100); 
    volatile uint32_t ul; //vol ensures ul is not optimized away

    uint32_t lDataNum = (uint32_t)pvParameters;
    //uint32_t lDataNum = 1;

    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        //slap the data into the queue
        /*Data_t data2Send;
        data2Send.eDataSource = (int)pvParameters;
        data2Send.ucValue = Fake_Data_t*/
        xQueueStatus = xQueueSendToBack(xNumQueue, &lDataNum, xQueueWaitTicks);
        if (xQueueStatus != pdPASS)
        {
            /* The send operation could not complete, even after waiting for 100ms.
            This must be an error as the receiving task should make space in the
            queue as soon as both sending tasks are in the Blocked state. */
            vPrintString("Could not send to the queue.\r\n");
        }
        

        //block this task until 1000ms have passed
        vTaskDelayUntil(&xLastWakeTime, xDelay1000ms);
    }
}

void vQueueReceiver(void* pvParameters) {
    //how long to wait for queue to empty
    const TickType_t xQueueWaitTicks = pdMS_TO_TICKS(3000);
    BaseType_t xQueueStatus;
    uint32_t lReceivedValue;

    for (;;)
    {
        if (uxQueueMessagesWaiting(xNumQueue) != 0) {
            vPrintString("queue should be empty but isn't");
        }


        xQueueStatus = xQueueReceive(xNumQueue, &lReceivedValue, xQueueWaitTicks);
        if (xQueueStatus == pdPASS)
        {
            /* Data was successfully received from the queue, print out the received
            value and the source of the value. */
            if (lReceivedValue == 0)
            {
                vPrintString("From Sender 0\r\n");
            }
            else
            {
                vPrintString("From Sender 1\r\n");
            }
        }
        else
        {
            vPrintString("couldn't receive\r\n");
        }
    }
}

static void vSynchTask(void* pvParameters) {
    const TickType_t xMaxDelay = pdMS_TO_TICKS(5000UL);
    const TickType_t xMinDelay = pdMS_TO_TICKS(200UL);
    TickType_t xDelayTime;
    EventBits_t uxThisTaskSyncBit;
    const EventBits_t uxAllSyncBits =  syncBit0 | syncBit1 | syncBit2;

    uxThisTaskSyncBit = (EventBits_t)pvParameters;

    for (;;) {
        //create a fake timeout to simulate asynchronous stuff
        xDelayTime = (prvRand() % xMaxDelay) + xMinDelay;
        vTaskDelay(xDelayTime);

        //sync point is here
        vPrintTwoStrings(pcTaskGetTaskName(NULL), "synching");

        //this will block until all event bits are set
        xEventGroupSync(xEventGroup, uxThisTaskSyncBit, uxAllSyncBits, portMAX_DELAY);

        vPrintTwoStrings(pcTaskGetTaskName(NULL), "synched");

    }


}

static uint32_t prvRand(void)
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;
    uint32_t ulReturn;

    /* Utility function to generate a pseudo random number as the MSVC rand()
    function has unexpected consequences. */
    taskENTER_CRITICAL();
    ulNextRand = (ulMultiplier * ulNextRand) + ulIncrement;
    ulReturn = (ulNextRand >> 16UL) & 0x7fffUL;
    taskEXIT_CRITICAL();
    return ulReturn;
}
/*-----------------------------------------------------------*/

static void prvSRand(uint32_t ulSeed)
{
    /* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/





