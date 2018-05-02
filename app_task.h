/*
* app_task.h
*
* Created: 2017/8/23 9:44:46
*  Author: user
*/


#ifndef APP_TASK_H_
#define APP_TASK_H_
#include "asf.h"

#define TASK_TIMER				(TCE0)
//IO input filter timer clock freq = 24000000Hz
#define TASK_TIMER_CLKSRC		(TC_CLKSEL_DIV1_gc)
//IO input filter timer period = 1/(24000000/1024)s = 42.7us
#define IO_FILTER_TIMER_PERIOD_CNT  (1024)
#define TASK_TIMER_PERIOD_CNT_2MS  (48000UL)
#define TASK_TIMER_PERIOD_CNT_4MS  ( 96000UL)

#define TASK_NUM                    (70)
#define TASK_TC_COV_TIME_IN_US(x)		(x * 24UL)

#define IO_INPUT0_FILTER_TIMER_US_TO_STEP		(24)
#define IO_INPUT0_FILTER_TIMER_PRE_STEP_IN_US	(1)

typedef struct _global_task_queue
{
	uint8_t task_index;
	void * p_task;
}g_task_queue;



typedef struct _task_type
{
	volatile uint32_t base;
	volatile uint32_t current;
	uint32_t total_count;
	volatile uint32_t task_elapse;
	uint8_t priority;
	volatile bool is_ready_to_begin;
	volatile bool task_is_end;
	volatile bool auto_resume;
    volatile uint32_t poll_cnt;
    //volatile uint32_t error_handle;
    //uint32_t count_times;
	void (*call_func)(void);
}task_t;

void task_init(void);

task_t* task_create(void (*fun)(void),uint32_t wait_count,bool auto_resume,uint8_t priority);
void task_resume(task_t * task,volatile uint32_t wait_count);
void task_destory(task_t* task);
void task_end(task_t *task);
int8_t task_add_task_to_queue(task_t * task);
void task_run_time_handler(void);
bool is_task_running(void);
bool is_task_end(task_t * task);

//internal function
void task_poll(task_t * task);
void det_timer_clear(void);
void set_timer_clear(void);
#endif /* APP_TASK_H_ */