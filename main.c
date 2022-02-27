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

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo includes. */
#include "supporting_functions.h"

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xffffff )

/* The task functions. */
void vPrintTask(void* pvParameters);
uint32_t fibonacci(uint32_t num);
void vTerribleFib(void* pvParameters);

/*-----------------------------------------------------------*/

int main( void )
{
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
        vPrintStringAndNumber("next fib: ", nextNum);
        vPrintStringAndNumber("maxStack: ", uxTaskGetStackHighWaterMark(NULL));
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

        //block this task until 1000ms have passed
        vTaskDelayUntil(&xLastWakeTime, xDelay1000ms); 
    }
}
