/*
 *  FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
 *  All rights reserved
 *
 *  VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.
 *
 *  This file is part of the FreeRTOS distribution.
 *
 *  FreeRTOS is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by the
 *  Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.
 *
 ***************************************************************************
 *  >>!   NOTE: The modification to the GPL is included to allow you to     !<<
 *  >>!   distribute a combined work that includes FreeRTOS without being   !<<
 *  >>!   obliged to provide the source code for proprietary components     !<<
 *  >>!   outside of the FreeRTOS kernel.                                   !<<
 ***************************************************************************
 *
 *  FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  Full license text is available on the following
 *  link: http://www.freertos.org/a00114.html
 *
 ***************************************************************************
 *                                                                       *
 *    FreeRTOS provides completely free yet professionally developed,    *
 *    robust, strictly quality controlled, supported, and cross          *
 *    platform software that is more than just the market leader, it     *
 *    is the industry's de facto standard.                               *
 *                                                                       *
 *    Help yourself get started quickly while simultaneously helping     *
 *    to support the FreeRTOS project by purchasing a FreeRTOS           *
 *    tutorial book, reference manual, or both:                          *
 *    http://www.FreeRTOS.org/Documentation                              *
 *                                                                       *
 ***************************************************************************
 *
 *  http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
 *  the FAQ page "My application does not run, what could be wrong?".  Have you
 *  defined configASSERT()?
 *
 *  http://www.FreeRTOS.org/support - In return for receiving this top quality
 *  embedded software for free we request you assist our global community by
 *  participating in the support forum.
 *
 *  http://www.FreeRTOS.org/training - Investing in training allows your team to
 *  be as productive as possible as early as possible.  Now you can receive
 *  FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
 *  Ltd, and the world's leading authority on the world's leading RTOS.
 *
 *  http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
 *  including FreeRTOS+Trace - an indispensable productivity tool, a DOS
 *  compatible FAT file system, and our tiny thread aware UDP/IP stack.
 *
 *  http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
 *  Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.
 *
 *  http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
 *  Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
 *  licenses offer ticketed support, indemnification and commercial middleware.
 *
 *  http://www.SafeRTOS.com - High Integrity Systems also provide a safety
 *  engineered and independently SIL3 certified version for use in safety and
 *  mission critical applications that require provable dependability.
 *
 *  1 tab == 4 spaces!
 */

/*
 * Modified to run a demo of a Modbus application with end-to-end capabilities.
 *
 * Michael Dodson, 2021
 */

/* Standard includes. */
#include <stdio.h>
#include <time.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

/* Demo application includes. */
#include "ModbusServer.h"
/*#include "demo_logging.h" */

#ifdef __CHERI_PURE_CAPABILITY__
    #include <cheri/cheri-utility.h>
#endif

/* Modbus TCP  server task parameters.  The standard Modbus port is used. */
#define mainMODBUS_SERVER_TASK_PRIORITY               ( tskIDLE_PRIORITY )
#define mainMODBUS_SERVER_PORT_NUMBER                 ( 502UL )

/* Dimensions the buffer used to send UDP print and debug messages. */
#define cmdPRINTF_BUFFER_SIZE                         512

/* Define a name that will be used for LLMNR and NBNS searches. */
#define mainHOST_NAME                                 "RTOSDemo"
#define mainDEVICE_NICK_NAME                          "windows_demo"

/* Set to 0 to run the STDIO examples once only, or 1 to create multiple tasks
 * that run the tests continuously. */
#define mainRUN_STDIO_TESTS_IN_MULTIPLE_TASK          0

/* Set the following constants to 1 or 0 to define which tasks to include and
 * exclude:
 *
 * mainCREATE_MODBUS_SERVER_TASKS:  When set to 1 a Modbus server is created that
 * uses a TCP port for input and output.  The port is set by the constant
 * mainMODBUS_SERVER_PORT_NUMBER constant above.  The server is based on libmodbus
 * and a libmodbus client can be used to connect.
 * libmodbus: https://github.com/stephane/libmodbus
 */

#define mainCREATE_MODBUS_SERVER_TASKS                1

/* Set the following constant to pdTRUE to log using the method indicated by the
 * name of the constant, or pdFALSE to not log using the method indicated by the
 * name of the constant.  Options include to standard out (mainLOG_TO_STDOUT), to a
 * disk file (mainLOG_TO_DISK_FILE), and to a UDP port (mainLOG_TO_UDP).  If
 * mainLOG_TO_UDP is set to pdTRUE then UDP messages are sent to the IP address
 * configured as the echo server address (see the configECHO_SERVER_ADDR0
 * definitions in FreeRTOSConfig.h) and the port number set by configPRINT_PORT in
 * FreeRTOSConfig.h. */
#define mainLOG_TO_STDOUT                             pdTRUE
#define mainLOG_TO_DISK_FILE                          pdFALSE
#define mainLOG_TO_UDP                                pdFALSE

/*-----------------------------------------------------------*/

/*
 * A software timer is created that periodically checks that some of the TCP/IP
 * demo tasks are still functioning as expected.  This is the timer's callback
 * function.
 */
static void prvCheckTimerCallback( TimerHandle_t xTimer );

/*
 * Just seeds the simple pseudo random number generator.
 */
static void prvSRand( UBaseType_t ulSeed );

/*
 * Miscellaneous initialisation including preparing the logging and seeding the
 * random number generator.
 */
static void prvMiscInitialisation( void );

/*
 * Functions used to create and then test files on a disk.
 */
extern void vCreateAndVerifyExampleFiles( const char * pcMountPath );
extern void vStdioWithCWDTest( const char * pcMountPath );
extern void vMultiTaskStdioWithCWDTest( const char * const pcMountPath,
                                        uint16_t usStackSizeWords );

/* The default IP and MAC address used by the demo.  The address configuration
 * defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
 * 1 but a DHCP server could not be contacted.  See the online documentation for
 * more information. */
static const uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 };
static const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 };
static const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 };
static const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 };

/* Default MAC address configuration.  The demo creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition for information on how to configure
 * the real network connection to use. */
const uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };

/* The UDP address to which print messages are sent. */
static struct freertos_sockaddr xPrintUDPAddress;

/* Use by the pseudo random number generator. */
/*static UBaseType_t ulNextRand; */

/* Handle of the task that runs the FTP and HTTP servers. */
static TaskHandle_t xServerWorkTaskHandle = NULL;

/*-----------------------------------------------------------*/

#if mainCONFIG_USE_DYNAMIC_LOADER == 0
    static UBaseType_t cheri_exception_handler( uintptr_t * exception_frame )
    {
        #ifdef __CHERI_PURE_CAPABILITY__
            size_t cause = 0;
            size_t epc = 0;
            size_t cheri_cause;

            asm volatile ( "csrr %0, mcause" : "=r" ( cause )::);
            asm volatile ( "csrr %0, mepc" : "=r" ( epc )::);

            size_t ccsr = 0;
            asm volatile ( "csrr %0, mccsr" : "=r" ( ccsr )::);

            uint8_t reg_num = ( uint8_t ) ( ( ccsr >> 10 ) & 0x1f );
            int is_scr = ( ( ccsr >> 15 ) & 0x1 );
            cheri_cause = ( unsigned ) ( ( ccsr >> 5 ) & 0x1f );

            for( int i = 0; i < 35; i++ )
            {
                printf( "x%i ", i );
                cheri_print_cap( *( exception_frame + i ) );
            }

            printf( "mepc = 0x%lx\n", epc );
            printf( "TRAP: CCSR = 0x%lx (cause: %x reg: %u : scr: %u)\n",
                    ccsr,
                    cheri_cause,
                    reg_num, is_scr );
        #endif /* ifdef __CHERI_PURE_CAPABILITY__ */

        while( 1 )
        {
        }
    }

    static UBaseType_t default_exception_handler( uintptr_t * exception_frame )
    {
        size_t cause = 0;
        size_t epc = 0;

        asm volatile ( "csrr %0, mcause" : "=r" ( cause )::);
        asm volatile ( "csrr %0, mepc" : "=r" ( epc )::);
        printf( "mcause = %u\n", cause );
        printf( "mepc = %llx\n", epc );

        while( 1 )
        {
        }
    }
#endif /* if mainCONFIG_USE_DYNAMIC_LOADER == 0 */

int main_modbus( void )
{
    const uint32_t ulLongTime_ms = 250UL, ulCheckTimerPeriod_ms = 15000UL;
    TimerHandle_t xCheckTimer;

    /* Miscellaneous initialisation including preparing the logging and seeding
     * the random number generator. */
    prvMiscInitialisation();

    /* Initialise the network interface.
     *
     ***NOTE*** Tasks that use the network are created in the network event hook
     * when the network is connected and ready for use (see the definition of
     * vApplicationIPNetworkEventHook() below).  The address values passed in here
     * are used if ipconfigUSE_DHCP is set to 0, or if ipconfigUSE_DHCP is set to 1
     * but a DHCP server cannot be	contacted. */
    FreeRTOS_debug_printf( ( "FreeRTOS_IPInit\n" ) );
    FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );

    /* A timer is used to periodically check the example tasks are functioning
     * as expected.  First create the software timer ... */
    xCheckTimer = xTimerCreate( "Check",                 /* Text name used for debugging only. */
                                pdMS_TO_TICKS( ulCheckTimerPeriod_ms ),
                                pdTRUE,                  /* This is an auto-reload timer. */
                                NULL,                    /* Parameter not used. */
                                prvCheckTimerCallback ); /* The timer callback function. */

    /* ... assert if the timer was not created, ... */
    configASSERT( xCheckTimer );

    /* ... then start the timer. */
    xTimerStart( xCheckTimer, 0 );

    /* Start the RTOS scheduler. */
    FreeRTOS_debug_printf( ( "vTaskStartScheduler\n" ) );
}

/*-----------------------------------------------------------*/

#if 0
    void vApplicationIdleHook( void )
    {
        const uint32_t ulMSToSleep = 1;

        /* This is just a trivial example of an idle hook.  It is called on each
         * cycle of the idle task if configUSE_IDLE_HOOK is set to 1 in
         * FreeRTOSConfig.h.  It must *NOT* attempt to block.  In this case the
         * idle task just sleeps to lower the CPU usage. */
        Sleep( ulMSToSleep );
    }
#endif

/*-----------------------------------------------------------*/

#if 0
    void vAssertCalled( const char * pcFile,
                        uint32_t ulLine )
    {
        const uint32_t ulLongSleep = 1000UL;
        volatile uint32_t ulBlockVariable = 0UL;
        volatile char * pcFileName = ( volatile char * ) pcFile;
        volatile uint32_t ulLineNumber = ulLine;

        ( void ) pcFileName;
        ( void ) ulLineNumber;

        FreeRTOS_printf( ( "vAssertCalled( %s, %ld\n", pcFile, ulLine ) );

        /* Setting ulBlockVariable to a non-zero value in the debugger will allow
         * this function to be exited. */
        taskDISABLE_INTERRUPTS();
        {
            while( ulBlockVariable == 0UL )
            {
                Sleep( ulLongSleep );
            }
        }
        taskENABLE_INTERRUPTS();
    }
#endif /* if 0 */

/*-----------------------------------------------------------*/

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
 * events are only received if implemented in the MAC driver. */
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    char cBuffer[ 16 ];
    static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* If the network has just come up...*/
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the IP stack if they have not already been
         * created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /* See the comments above the definitions of these pre-processor
             * macros at the top of this file for a description of the individual
             * demo tasks. */

            #if( mainCREATE_MODBUS_SERVER_TASKS == 1 )
                {
                    /* Modbus TCP server on port specified by mainMODBUS_SERVER_PORT_NUMBER */
                    vStartModbusServerTask( configMINIMAL_STACK_SIZE, mainMODBUS_SERVER_PORT_NUMBER, mainMODBUS_SERVER_TASK_PRIORITY );
                }
            #endif /* mainCREATE_MODBUS_SERVER_TASKS */

            xTasksAlreadyCreated = pdTRUE;
        }

        /* Print out the network configuration, which may have come from a DHCP
         * server. */
        FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );
        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
        FreeRTOS_printf( ( "\r\n\r\nIP Address: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
        FreeRTOS_printf( ( "Subnet Mask: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
        FreeRTOS_printf( ( "Gateway Address: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
        FreeRTOS_printf( ( "DNS Server Address: %s\r\n\r\n\r\n", cBuffer ) );
    }
}
/*-----------------------------------------------------------*/

/* Called automatically when a reply to an outgoing ping is received. */
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus,
                                uint16_t usIdentifier )
{
    static const char * pcSuccess = "Ping reply received - ";
    static const char * pcInvalidChecksum = "Ping reply received with invalid checksum - ";
    static const char * pcInvalidData = "Ping reply received with invalid data - ";

    switch( eStatus )
    {
        case eSuccess:
            FreeRTOS_printf( ( pcSuccess ) );
            break;

        case eInvalidChecksum:
            FreeRTOS_printf( ( pcInvalidChecksum ) );
            break;

        case eInvalidData:
            FreeRTOS_printf( ( pcInvalidData ) );
            break;

        default:

            /* It is not possible to get here as all enums have their own
             * case. */
            break;
    }

    FreeRTOS_printf( ( "identifier %d\r\n", ( int ) usIdentifier ) );

    /* Prevent compiler warnings in case FreeRTOS_debug_printf() is not defined. */
    ( void ) usIdentifier;
}
/*-----------------------------------------------------------*/

#if 0
    void vApplicationMallocFailedHook( void )
    {
        /* Called if a call to pvPortMalloc() fails because there is insufficient
         * free memory available in the FreeRTOS heap.  pvPortMalloc() is called
         * internally by FreeRTOS API functions that create tasks, queues, software
         * timers, and semaphores.  The size of the FreeRTOS heap is set by the
         * configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
        vAssertCalled( __FILE__, __LINE__ );
    }
#endif

/*-----------------------------------------------------------*/

static void prvCheckTimerCallback( TimerHandle_t xTimer )
{
    static volatile uint32_t ulEchoClientErrors_Single = 0, ulEchoClientErrors_Separate = 0, ulEchoServerErrors = 0, ulUDPEchoClientErrors = 0, ulUDPSelectServerErrors = 0;

    ( void ) xTimer;

    /* Not all the demo tasks contain a check function yet - although an
     * assert() will be triggered if a task fails. */

    #if ( mainCREATE_TCP_ECHO_TASKS_SEPARATE == 1 )
        {
            if( xAreSeparateTaskTCPEchoClientsStillRunning() != pdPASS )
            {
                ulEchoClientErrors_Separate++;
            }
        }
    #endif

    #if ( mainCREATE_SIMPLE_TCP_ECHO_SERVER == 1 )
        {
            if( xAreTCPEchoServersStillRunning() != pdPASS )
            {
                ulEchoServerErrors++;
            }
        }
    #endif

    #if ( mainCREATE_SELECT_UDP_SERVER_TASKS == 1 )
        {
            if( xAreUDPSelectTasksStillRunning() != pdPASS )
            {
                ulUDPSelectServerErrors++;
            }
        }
    #endif
}

/*-----------------------------------------------------------*/

#if 0
    UBaseType_t uxRand( void )
    {
        const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

        /* Utility function to generate a pseudo random number. */

        ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
        return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
    }
#endif
/*-----------------------------------------------------------*/

static void prvSRand( UBaseType_t ulSeed )
{
    /* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/

static void prvMiscInitialisation( void )
{
    time_t xTimeNow;
    uint32_t ulLoggingIPAddress;

    /* Seed the random number generator. */
    time( &xTimeNow );
    FreeRTOS_debug_printf( ( "Seed for randomiser: %lu\n", xTimeNow ) );
    prvSRand( ( uint32_t ) xTimeNow );
    FreeRTOS_debug_printf( ( "Random numbers: %08X %08X %08X %08X\n", ipconfigRAND32(), ipconfigRAND32(), ipconfigRAND32(), ipconfigRAND32() ) );

    #ifdef __CHERI_PURE_CAPABILITY__
        /* Setup an exception handler for CHERI. This is only used for debugging when the demo is built
         * with purecap and without compartmentalization support. If ccompartmentalization is enabled,
         * the RISC-V-Generic BSP instead sets this handler to catch/handle compartment faults.
         */
        #if (DEBUG && configPORT_ALLOW_APP_EXCEPTION_HANDLERS && !configCHERI_COMPARTMENTALIZATION)
            vPortSetExceptionHandler( 0x1c, cheri_exception_handler );
        #endif
    #endif
}
/*-----------------------------------------------------------*/

struct tm * gmtime_r( const time_t * pxTime,
                      struct tm * tmStruct )
{
    /* Dummy time functions to keep the file system happy in the absence of
     * target support. */
    memcpy( tmStruct, gmtime( pxTime ), sizeof( *tmStruct ) );
    return tmStruct;
}
/*-----------------------------------------------------------*/

#if 0
    time_t FreeRTOS_time( time_t * pxTime )
    {
        time_t xReturn;

        xReturn = time( &xReturn );

        if( pxTime != NULL )
        {
            *pxTime = xReturn;
        }

        return xReturn;
    }
#endif /* if 0 */

/*-----------------------------------------------------------*/

/*
 * Callback that provides the inputs necessary to generate a randomized TCP
 * Initial Sequence Number per RFC 6528.  THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 */
extern uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort )
{
    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    return uxRand();
}
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) || ( ipconfigDHCP_REGISTER_HOSTNAME != 0 )

    const char * pcApplicationHostnameHook( void )
    {
        /* Assign the name "FreeRTOS" to this network node.  This function will
         * be called during the DHCP: the machine will be registered with an IP
         * address plus this name. */
        return mainHOST_NAME;
    }

#endif
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 )

    BaseType_t xApplicationDNSQueryHook( const char * pcName )
    {
        BaseType_t xReturn;

        /* Determine if a name lookup is for this node.  Two names are given
         * to this node: that returned by pcApplicationHostnameHook() and that set
         * by mainDEVICE_NICK_NAME. */
        if( FF_stricmp( pcName, pcApplicationHostnameHook() ) == 0 )
        {
            xReturn = pdPASS;
        }
        else if( FF_stricmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
        {
            xReturn = pdPASS;
        }
        else
        {
            xReturn = pdFAIL;
        }

        return xReturn;
    }

#endif /* if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) */
/*-----------------------------------------------------------*/
