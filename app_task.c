/*
* Task.c
*
* Created: 2017/8/23 9:44:27
*  Author: user
*/
#include "app_task.h"
#include "app_debug.h"

#include <string.h>

//static uint8_t tmp_read_tc = 0;
volatile uint8_t irq;
uint32_t count_times;
uint64_t my_cnt = 0;
task_t *task_queue[TASK_NUM];
static volatile uint8_t current_task_num = 0;

static  volatile bool is_task_timer_clear = true;
static volatile bool call_back = false;
static void my_io_callback_timeout(void);

void task_init(void)
{
    //set IO input filter timer
    tc_enable(&TASK_TIMER);
    tc_set_wgm(&TASK_TIMER, TC_WG_NORMAL);
    tc_set_overflow_interrupt_callback(&TASK_TIMER, my_io_callback_timeout);

    //begin IO input filter timer
    tc_write_period(&TASK_TIMER,  TASK_TIMER_PERIOD_CNT_2MS);
    tc_set_overflow_interrupt_level(&TASK_TIMER, TC_INT_LVL_HI);
    tc_write_clock_source(&TASK_TIMER, TASK_TIMER_CLKSRC);
}



task_t* task_create(void (*fun)(void),uint64_t wait_count,bool auto_resume,uint8_t priority)
{
    task_t *task = NULL;
    task = (task_t *)malloc(sizeof(task_t));
    task->current = tc_read_count(&TASK_TIMER);
    task->call_func = fun;
    task->task_is_end = false;
	
    if (wait_count > TASK_TIMER_PERIOD_CNT_2MS - task->current)
    {
        task->time_t1 = TASK_TIMER_PERIOD_CNT_2MS - task->current;
        task->time_tn = (wait_count - task->time_t1) / TASK_TIMER_PERIOD_CNT_2MS;
        task->time_t2 =  (wait_count - task->time_t1) % TASK_TIMER_PERIOD_CNT_2MS;
    }
    else
    {
        task->time_t1 = 0;
        task->time_tn = 0;
        task->time_t2 = (uint32_t)wait_count + task->current;
    }
    
    task->total_count = wait_count;
    task->auto_resume  = auto_resume;
    task->priority = priority;
    task->is_running_sleep = true;
	
    if (wait_count == 0)
    {
        task->is_ready_to_begin = true;
        task->is_running_sleep = false;
    }
    else
    {
        task->is_ready_to_begin = false;
    }
    return task;
}

void task_resume(task_t * task,volatile uint64_t wait_count)
{
    if (task == NULL)
    {        
        return;
    }
    
    task->current =  tc_read_count(&TASK_TIMER);
	
    task->task_is_end = false;
    task->is_running_sleep = true;
	
    if (wait_count == 0)
    {
        task->is_ready_to_begin = true;
        task->is_running_sleep = false;
        return;
    }
    else if (wait_count > TASK_TIMER_PERIOD_CNT_2MS - task->current)
    {
        task->time_t1 = TASK_TIMER_PERIOD_CNT_2MS - task->current;
        task->time_tn = (wait_count - task->time_t1) / TASK_TIMER_PERIOD_CNT_2MS;
        task->time_t2 = (wait_count - task->time_t1) % TASK_TIMER_PERIOD_CNT_2MS;
    }
    else
    {
        task->time_t1 = 0;
        task->time_tn = 0;
        task->time_t2 = (uint32_t)wait_count + task->current;
    }
}

void task_poll(task_t *task)
{
    if (task == NULL)
    {
        return;
    }
    task->current =  tc_read_count(&TASK_TIMER);
    
    if (is_task_timer_clear)
    {
        if (task->time_t1 > 0)
        {
            task->time_t1 = 0;

            return;
        }
        else
        {
            if(task->time_tn > 0)
            {              
                task->time_tn --;               
                if (task->time_tn == 0 && task->time_t2 == 0)
                {
					task->is_ready_to_begin =true;
                }
				else
				{
					return;
					
				}
            }
            else
            {
                return;
            }
        }
    }
    else
    {
        if (task->time_t1 == 0)
        {
            if (task->time_tn == 0)
            {
                if ( task->time_t2 <= task->current)
                {

                    task->time_t2  = 0;
                    task->is_ready_to_begin =true;
                  
                }
				else
				{
					return;
				}
            }
        }
		else
		{
			return;
		}
    }

    if (task->is_ready_to_begin)
    {
		task->call_func();
        if (task->auto_resume)
        {            
            task->is_running_sleep = false;
            task->is_ready_to_begin = false;
            task->task_is_end = true;
            task_resume(task,task->total_count);
        }
        else
        {
            task->task_is_end = true;
            task->is_running_sleep = false;
            task->is_ready_to_begin = false;
        }
        
    }
    
}

void task_end(task_t *task)
{
    task->task_is_end = true;
}

void task_destory(task_t* task)
{
    if (task == NULL)
    {
        return;
    }
    free(task);
}


static void my_io_callback_timeout(void)
{
    //tc_write_count(&TASK_TIMER,0x0);	
    call_back = true;
}


void det_timer_clear(void)
{
    if (call_back)
    {
        is_task_timer_clear = true;
        call_back = false;
    }
}

void set_timer_clear(void)
{
    if (is_task_timer_clear)
    {
        is_task_timer_clear = false;
    }
}


int8_t task_add_task_to_queue(task_t * task)
{
    if (current_task_num == (TASK_NUM -1))
    {
        return -1;
    }
    task_queue[current_task_num] = task;
    current_task_num ++;
    return 0;
}

void task_run_time_handler(void)
{
    uint8_t i = 0;

    det_timer_clear();
    for (i = 0; i < current_task_num; i++)
    {
        if (task_queue[i])
        {
            if(task_queue[i]->task_is_end == true)
            {
               continue;
            }
            task_poll(task_queue[i]);
        }
    }
    set_timer_clear();
    
}


bool is_task_running(void)
{
    if (current_task_num != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool is_task_end(task_t * task)
{
    return task->task_is_end;
}
