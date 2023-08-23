#include <switch.h>
#include "mod_v150.h"

struct v150_globals v150_globals = { 0 };

SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_v150_shutdown);
SWITCH_MODULE_DEFINITION(mod_v150, mod_v150_load, mod_v150_shutdown, NULL);

// The V150 application that gets registered with FreeSWITCH
SWITCH_STANDARD_APP(v150_function)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Simple app executed!\n");
}

// Function executed when the module is loaded
SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load)
{
    switch_application_interface_t *app_interface;

    memset(&v150_globals, 0, sizeof(v150_globals));
    v150_globals.pool = pool;

    *module_interface = switch_loadable_module_create_module_interface(pool, modname);
    switch_mutex_init(&v150_globals.mutex, SWITCH_MUTEX_NESTED, pool);

    load_configuration(false);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "%s module loaded!\n", modname);
    SWITCH_ADD_APP(app_interface, "v150_app", "V150 App", "V150 Development", v150_function, "", SAF_NONE);

    return SWITCH_STATUS_SUCCESS;
}

// Function executed when the module is unloaded
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_v150_shutdown)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "mod_v150 unloaded!\n");
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