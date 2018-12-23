#pragma once

#include "coroutine.h"

#include "zend_builtin_functions.h"
#include "zend_closures.h"
#include "zend_vm.h"

#include <stack>

typedef enum
{
    SW_CORO_CONTEXT_RUNNING,
    SW_CORO_CONTEXT_IN_DELAYED_TIMEOUT_LIST,
    SW_CORO_CONTEXT_TERM
} php_context_state;

enum sw_coro_hook_type
{
    SW_HOOK_FILE = 1u << 1,
    SW_HOOK_SLEEP = 1u << 2,
    SW_HOOK_TCP = 1u << 3,
    SW_HOOK_UDP = 1u << 4,
    SW_HOOK_UNIX = 1u << 5,
    SW_HOOK_UDG = 1u << 6,
    SW_HOOK_SSL = 1u << 7,
    SW_HOOK_TLS = 1u << 8,
    SW_HOOK_BLOCKING_FUNCTION = 1u << 9,
    SW_HOOK_ALL = 0x7fffffff,
};

struct defer_task
{
    swCallback callback;
    void *data;

    defer_task(swCallback _callback, void *_data) :
            callback(_callback), data(_data)
    {
    }
};

struct coro_task
{
    /** php switcher **/
    zval *vm_stack_top;
    zval *vm_stack_end;
    zend_vm_stack vm_stack;
    zend_execute_data *execute_data;
    zend_error_handling_t error_handling;
    zend_class_entry *exception_class;
    zend_object *exception;
    zend_output_globals *output_ptr;
    SW_DECLARE_EG_SCOPE(scope);

    /** swoole members **/
    swoole::Coroutine *co;
    coro_task *origin_task;
    std::stack<defer_task *> *defer_tasks;
};

struct php_args
{
    zend_fcall_info_cache *fci_cache;
    zval *argv;
    int argc;
    coro_task *origin_task;
};

struct coro_global
{
    zend_bool active;
    uint64_t max_coro_num;
    uint64_t peak_coro_num;
    uint32_t stack_size;
    double socket_connect_timeout;
    double socket_timeout;
    coro_task task;
    zend_object *exception;
};

// TODO: remove php context
struct php_context
{
    php_context_state state;
    zval coro_params;
    zval *current_coro_return_value_ptr;
    void *private_data;
    swTimer_node *timer;
    coro_task *current_task;
};

extern coro_global COROG;

long sw_get_current_cid();
void sw_coro_add_defer_task(swCallback cb, void *data);

void coro_init(void);
void coro_check(void);

#define sw_coro_is_in() (likely(COROG.active && coroutine_get_current()))
#define coro_use_return_value(); *(zend_uchar *) &execute_data->prev_execute_data->opline->result_type = IS_VAR;

/* output globals */
#define SWOG ((zend_output_globals *) &OG(handlers))

long sw_coro_create(zend_fcall_info_cache *fci_cache, int argc, zval *argv);
void sw_coro_yield();
void sw_coro_close();
int sw_coro_resume(php_context *sw_current_context, zval *retval, zval *coro_retval);
void sw_coro_save(zval *return_value, php_context *sw_php_context);
void sw_coro_set_stack_size(int stack_size);

bool sw_enable_coroutine_hook(int flags);
bool sw_disable_coroutine_hook();
