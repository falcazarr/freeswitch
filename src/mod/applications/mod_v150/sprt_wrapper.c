#include "mod_v150.h"
#include <spandsp/version.h>


/* Logging handler for SpanDSP library - might need for V150 debugging */
void mod_v150_log_message(void *user_data, int level, const char *msg)
{
	int fs_log_level;
	mod_v150_log_data_t *log_data = (mod_v150_log_data_t *)user_data;
	switch_core_session_t *session = log_data ? log_data->session : NULL;
	switch_log_level_t verbose_log_level = log_data ? log_data->verbose_log_level : v150_globals.verbose_log_level;

	switch (level) {
	case SPAN_LOG_NONE:
		return;
	case SPAN_LOG_ERROR:
	case SPAN_LOG_PROTOCOL_ERROR:
		fs_log_level = SWITCH_LOG_ERROR;
		break;
	case SPAN_LOG_WARNING:
	case SPAN_LOG_PROTOCOL_WARNING:
		fs_log_level = SWITCH_LOG_WARNING;
		break;
	case SPAN_LOG_FLOW:
	case SPAN_LOG_FLOW_2:
	case SPAN_LOG_FLOW_3:
	default:					/* SPAN_LOG_DEBUG, SPAN_LOG_DEBUG_2, SPAN_LOG_DEBUG_3 */
		fs_log_level = verbose_log_level;
		break;
	}

	if (!zstr(msg)) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), fs_log_level, "%s", msg);
		if (log_data && log_data->trace_file) {
			fwrite(msg, strlen(msg) * sizeof(const char), 1, log_data->trace_file);
		}
	}
}