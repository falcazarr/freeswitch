#include <switch.h>
#include "mod_v150.h"

struct v150_globals v150_globals = { 0 };

SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_v150_shutdown);
SWITCH_MODULE_DEFINITION(mod_v150, mod_v150_load, mod_v150_shutdown, NULL);

SWITCH_STANDARD_APP(v150_sprt_tx_function)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "SPRT TX App executed!\n");
}

SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load)
{
    switch_application_interface_t *app_interface;

    memset(&v150_globals, 0, sizeof(v150_globals));
    v150_globals.pool = pool;

    *module_interface = switch_loadable_module_create_module_interface(pool, modname);
    switch_mutex_init(&v150_globals.mutex, SWITCH_MUTEX_NESTED, pool);

    load_configuration(SWITCH_FALSE);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "%s module loaded!\n", modname);

    SWITCH_ADD_APP(app_interface, "sprt_tx", "V150 SPRT Application", "V150 SPRT Application", v150_sprt_tx_function, "", SAF_NONE);
    SWITCH_ADD_API(api_interface, "sprt_tx", "SPRT TX (V150)", v150_sprt_tx_function, "");

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_v150_shutdown)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "mod_v150 unloaded!\n");

    if(v150_globals.config_pool) {
        switch_core_destroy_memory_pool(&v150_globals.config_pool);
    }

    memset(&v150_globals, 0, sizeof(v150_globals));

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t load_configuration(switch_bool_t reload)
{
    switch_status_t status = SWITCH_STATUS_FALSE;

    switch_mutex_lock(v150_globals.mutex);
    /* Load configuration below ... */


    /* Unlock before exitting function*/
    switch_mutex_unlock(v150_globals.mutex);

    return status;
}