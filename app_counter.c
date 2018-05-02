/*
* app_counter.c
*
* Created: 2017/10/17 17:56:17
*  Author: user
*/

#include "app_counter.h"
#include "app_global_parameters.h"
#include "app_debug.h"
#include "app_iso.h"
#include "event_system_driver.h"
#include "asf.h"

extern IntController g_intController;
static bool assert_count_mode(uint8_t index);
volatile static bool counter_lock= false;

static bool assert_count_mode(uint8_t index)
{
    //debug_printf("index %d,mode %d\n",index,g_intController.property[index]);
    if (g_intController.property[index] == IN_PROPERTY_COUNTER)
    {
        return true;
    }
    else
    {   
        
        return false;
    }
}

int counter_event(uint8_t index,bool is_enable,PORT_ISC_t mode)
{
    
    if (index > 7)
    {
        //debug_printf("counter_event index %d,enable %d\n",index);
        return -1;
    }
    
    if (is_enable)
    {
        
        sysclk_enable_peripheral_clock(&EVSYS);
        sysclk_enable_peripheral_clock(&EV_SYS_COUNT);
        if (!assert_count_mode(index))
        {
            //debug_printf("ret assert_count_mode error!\n");
            return -1;
        }
        if (counter_lock)
        {
            //debug_printf("counter lock:  %d!\n",counter_lock);
            return -2;
        }
        switch(index)
        {
            case 0:
            {
                //pin = ISO_IN0;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN4_gc);
                PORTA.PIN4CTRL = mode;
                break;
            }
            case 1:
            {
                //pin = ISO_IN1;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN5_gc);
                PORTA.PIN5CTRL = mode;
                break;
            }
            case 2:
            {
                //pin = ISO_IN2;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN6_gc);
                PORTA.PIN6CTRL = mode;
                break;
            }
            case 3:
            {
                //pin = ISO_IN3;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN7_gc);
                PORTA.PIN7CTRL = mode;
                break;
            }

            case 4:
            {
                //pin = ISO_IN4;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN0_gc);
                PORTA.PIN0CTRL = mode;
                break;
            }
            case 5:
            {
                //pin = ISO_IN5;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN1_gc);
                PORTA.PIN1CTRL = mode;
                break;
            }

            case 6:
            {
                // pin = ISO_IN6;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN2_gc);
                PORTA.PIN2CTRL = mode;
                break;
            }
            case 7:
            {
                //pin = ISO_IN7;
                EVSYS_SetEventSource(0,EVSYS_CHMUX_PORTA_PIN3_gc);
                PORTA.PIN3CTRL = mode;
                break;
            }
            
            default:
            break;
        }

        EVSYS_SetEventChannelFilter( 0, EVSYS_DIGFILT_1SAMPLE_gc );
        tc_write_period(&EV_SYS_COUNT,0xFFFF);
        tc_write_clock_source(&EV_SYS_COUNT,TC_CLKSEL_EVCH0_gc);
       // BIT_SET(counter_lock,index);
        counter_lock = 1;
    }
    else
    {
        //BIT_CLEAR(counter_lock,index);
        counter_lock = false;
        sysclk_disable_peripheral_clock(&EV_SYS_COUNT);
        //set PORTA   input as interrupt
        PORT_ConfigurePins( ISO_IN1_PIN_CLASS,
        ISO_IN1_PIN_MASK,
        false,
        false,
        PORT_OPC_TOTEM_gc,
        PORT_ISC_BOTHEDGES_gc );


        PORT_SetPinsAsInput( ISO_IN1_PIN_CLASS, ISO_IN1_PIN_MASK);

        PORT_ConfigureInterrupt0( &PORTA, PORT_INT0LVL_MED_gc, ISO_IN1_PIN_MASK);
    }
    //debug_printf("---index:%d enable %d,lock %d\n",index,is_enable,counter_lock);
    return 0;

}


uint16_t read_count_event(void)
{
    return tc_read_count(&EV_SYS_COUNT);
}

void clear_count_event(void)
{
    tc_write_count(&EV_SYS_COUNT,0);
}