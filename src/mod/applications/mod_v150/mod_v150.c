#include <switch.h>

SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_v150_shutdown);
SWITCH_MODULE_DEFINITION(mod_v150, mod_v150_load, mod_v150_shutdown, NULL);

SWITCH_STANDARD_APP(v150_function);

// Function executed when the module is loaded
SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load)
{
    switch_application_interface_t *app_interface;
    *module_interface = switch_loadable_module_create_module_interface(pool, modname);
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

// The V150 application that gets registered with FreeSWITCH
SWITCH_STANDARD_APP(v150_function)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Simple app executed!\n");
}

