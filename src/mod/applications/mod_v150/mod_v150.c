#include <switch.h>
#include "mod_v150.h"

struct v150_globals v150_globals = { 0 };

SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_v150_shutdown);
SWITCH_MODULE_DEFINITION(mod_v150, mod_v150_load, mod_v150_shutdown, NULL);

SWITCH_STANDARD_APP(v150_sprt_tx_function)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "SPRT TX Application executed!\n");
    send_reinvite_with_sdp_payload(session, FUNCTION_TX);
}

SWITCH_STANDARD_API(v150_sprt_tx_api)
{
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "SPRT TX API executed!\n");

    return status;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_v150_load)
{
    switch_application_interface_t *app_interface;
    switch_api_interface_t *api_interface;

    memset(&v150_globals, 0, sizeof(v150_globals));
    v150_globals.pool = pool;

    *module_interface = switch_loadable_module_create_module_interface(pool, modname);
    switch_mutex_init(&v150_globals.mutex, SWITCH_MUTEX_NESTED, pool);

    SWITCH_ADD_APP(app_interface, "sprt_tx", "V150 SPRT Application", "V150 SPRT Application", v150_sprt_tx_function, "", SAF_NONE);
    SWITCH_ADD_API(api_interface, "v150_sprt_tx", "V150 API - sprt tx", v150_sprt_tx_api, "");

    load_configuration(SWITCH_FALSE);
    v150_codecs_load(module_interface, pool);

    if ((switch_event_bind(modname, SWITCH_EVENT_RELOADXML, NULL, event_handler, NULL) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind our reloadxml handler!\n");
		/* Not such severe to prevent loading */
	}

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "%s module loaded!\n", modname);

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_v150_shutdown)
{
    switch_event_unbind_callback(event_handler);

    if(v150_globals.config_pool) {
        switch_core_destroy_memory_pool(&v150_globals.config_pool);
    }

    memset(&v150_globals, 0, sizeof(v150_globals));

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "mod_v150 unloaded!\n");

    return SWITCH_STATUS_UNLOAD;
}

switch_status_t load_configuration(switch_bool_t reload)
{
    switch_status_t status = SWITCH_STATUS_FALSE;

    switch_mutex_lock(v150_globals.mutex);

    if (v150_globals.config_pool) {
		switch_core_destroy_memory_pool(&v150_globals.config_pool);
	}

	switch_core_new_memory_pool(&v150_globals.config_pool);

    v150_globals.enable_sprt = 1;
    v150_globals.total_sessions = 0;
    v150_globals.verbose = 0;
    v150_globals.verbose_log_level = SWITCH_LOG_DEBUG;
    v150_globals.ident = "V150 SPRT Ident";
    v150_globals.header = "V150 SPRT Header";
    v150_globals.timezone = "";
    v150_globals.sprt_tx_reinvite_packet_count = 100;
    v150_globals.sprt_rx_reinvite_packet_count = 50;

    // TODO - Investigate XML config and config_pool

    /* Unlock before exitting function*/
    switch_mutex_unlock(v150_globals.mutex);

    return status;
}

static void event_handler(switch_event_t *event)
{
	load_configuration(SWITCH_FALSE);
}