#ifndef __ACC_TIMER__
#define __ACC_TIMER__

#ifdef __cplusplus
#define EXTC extern "C" 
#else
#define EXTC 
#endif


typedef struct _thread_acc_timer_ctx thread_acc_timer_ctx_t;
typedef struct _acc_timer_ctx  acc_timer_ctx_t;

/*
 * Starts timing for an id
 *
 * Arguments
 *  timer_ctx: the timer context
 *  id: The unique name of the section to time
 */
 EXTC void acc_timer_begin(acc_timer_ctx_t *timer_ctx, char *id);

/*
 * Ends timing for an id and add it to the accumulations
 *
 * Arguments
 *  timer_ctx: the timer context
 *  id: The unique name of the section to time
 */
 EXTC void acc_timer_end(acc_timer_ctx_t *timer_ctx, char *id);


/*
 * Writes the timer accumulations to file timings.out
 *
 * Arguments
 *  timer_ctx: the timer context
 */
EXTC void acc_write(acc_timer_ctx_t *timer_ctx);

EXTC acc_timer_ctx_t *initialize_acc_timer(int n_threads);

EXTC void destroy_acc_timer(acc_timer_ctx_t *timer_ctx);

#undef EXTC

#endif
