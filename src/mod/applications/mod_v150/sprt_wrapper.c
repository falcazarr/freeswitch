#include "mod_v150.h"
#include "udptl.h"

typedef enum {
	AUDIO_MODE,
	SPRT_MODE,
	VBD_MODE,
} transport_mode_t;

typedef enum {
	SPRT_MODE_UNKNOWN = 0,
	SPRT_MODE_NEGOTIATED = 1,
	SPRT_MODE_REQUESTED = 2,
	SPRT_MODE_REFUSED = -1,
} sprt_mode_t;

const char * get_sprt_status(sprt_mode_t mode) {
	const char *str = "off";
	switch(mode) {
	case SPRT_MODE_NEGOTIATED:
		str = "negotiated";
		break;
	case SPRT_MODE_REQUESTED:
		str = "requested";
		break;
	case SPRT_MODE_REFUSED:
		str = "refused";
		break;
	default:
		break;
	}
	return str;
}

struct pvt_s {
	switch_core_session_t *session;
	mod_v150_application_mode_t app_mode;
	switch_mutex_t *mutex;
	udptl_state_t *udptl_state;
	char *ident;
	char *header;
	char *timezone;
	int verbose;
	switch_log_level_t verbose_log_level;
	int caller;
	int done;
	sprt_mode_t sprt_mode;

	struct pvt_s *next;
};

typedef struct pvt_s pvt_t;

static void launch_timer_thread(void);

static struct {
	pvt_t *head;
	switch_mutex_t *mutex;
	switch_thread_t *thread;
	int thread_running;
} sprt_state_list;

static int add_pvt(pvt_t *pvt)
{
	int r = 0;

	if (sprt_state_list.thread_running == 1) {
		switch_mutex_lock(sprt_state_list.mutex);
		pvt->next = sprt_state_list.head;
		sprt_state_list.head = pvt;
		switch_mutex_unlock(sprt_state_list.mutex);
		r = 1;
		//wake_thread(0);
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Error launching thread\n");
	}

	return r;
}

static int del_pvt(pvt_t *del_pvt)
{
	pvt_t *p, *l = NULL;
	int r = 0;


	switch_mutex_lock(sprt_state_list.mutex);

	for (p = sprt_state_list.head; p; p = p->next) {
		if (p == del_pvt) {
			if (l) {
				l->next = p->next;
			} else {
				sprt_state_list.head = p->next;
			}
			switch_safe_free(del_pvt)
			p->next = NULL;
			r = 1;
			break;
		}

		l = p;
	}

	switch_mutex_unlock(sprt_state_list.mutex);

	//wake_thread(0);

	return r;
}

/**  WILL ADD BACK IN WHEN SOFTTIMER IS REQUIRED FOR IMPLEMENTATION
 
static void wake_thread(int force)
{
	if (force) {
		switch_thread_cond_signal(v150_globals.cond);
		return;
	}

	if (switch_mutex_trylock(v150_globals.cond_mutex) == SWITCH_STATUS_SUCCESS) {
		switch_thread_cond_signal(v150_globals.cond);
		switch_mutex_unlock(v150_globals.cond_mutex);
	}
}

static void launch_timer_thread(void)
{

	switch_threadattr_t *thd_attr = NULL;

	switch_threadattr_create(&thd_attr, v150_globals.pool);
	switch_threadattr_stacksize_set(thd_attr, SWITCH_THREAD_STACKSIZE);
	switch_thread_create(&sprt_state_list.thread, thd_attr, timer_thread_run, NULL, v150_globals.pool);
}

static void *SWITCH_THREAD_FUNC timer_thread_run(switch_thread_t *thread, void *obj)
{
	switch_timer_t timer = { 0 };
	pvt_t *pvt;
	int samples = 160;
	int ms = 20;

	if (switch_core_timer_init(&timer, "soft", ms, samples, NULL) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "timer init failed.\n");
		t38_state_list.thread_running = -1;
		goto end;
	}

	sprt_state_list.thread_running = 1;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "SPRT timer thread started.\n");

	switch_mutex_lock(v150_globals.cond_mutex);

	while(sprt_state_list.thread_running == 1) {

		switch_mutex_lock(sprt_state_list.mutex);

		if (!sprt_state_list.head) {
			switch_mutex_unlock(sprt_state_list.mutex);
			switch_thread_cond_wait(v150_globals.cond, v150_globals.cond_mutex);
			switch_core_timer_sync(&timer);
			continue;
		}

		for (pvt = sprt_state_list.head; pvt; pvt = pvt->next) {
			if (pvt->udptl_state && pvt->session && switch_channel_ready(switch_core_session_get_channel(pvt->session))) {
				switch_mutex_lock(pvt->mutex);
				// TODO: Add handling for sprt_state_list.head->sprt_mode - every 20ms
				switch_mutex_unlock(pvt->mutex);
			}
		}

		switch_mutex_unlock(sprt_state_list.mutex);

		switch_core_timer_next(&timer);
	}
}

**/

void mod_v150_process_sprt(switch_core_session_t *session, mod_v150_application_mode_t app_mode)
{
	pvt_t *pvt;
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_codec_t read_codec = { 0 };
	switch_codec_t write_codec = { 0 };
	switch_frame_t *read_frame = { 0 };
	switch_frame_t write_frame = { 0 };
	switch_codec_implementation_t read_impl = { 0 };
	int16_t *buf = NULL;
	uint32_t req_counter = 0;

	switch_core_session_get_read_impl(session, &read_impl);

	session_counter_increment();

	pvt = pvt_init(session, app_mode);
	switch_channel_set_private(channel, "_sprt_pvt", pvt);

	buf = switch_core_session_alloc(session, SWITCH_RECOMMENDED_BUFFER_SIZE);

	while (switch_channel_ready(channel)) {
		int tx = 0
		switch_status_t status;

		switch_ivr_parse_all_events(session);

		status = switch_core_session_read_frame(session, &read_frame, SWITCH_IO_FLAG_NONE, 0);

		if (!SWITCH_READ_ACCEPTABLE(status) || pvt->done) {
			goto done;
		}

		switch_ivr_sleep(session, 250, SWITCH_TRUE, NULL); // Required after codec initialization - TODO still need to add

		if (pvt->app_mode == FUNCTION_TX) {
	  		req_counter = v150_globals.sprt_tx_reinvite_packet_count;
	  } else {
	  		req_counter = v150_globals.sprt_rx_reinvite_packet_count;
		}


		switch (pvt->sprt_mode) {
			case SPRT_MODE_REQUESTED:
				{
				    if (switch_channel_test_app_flag_key("SPRT", channel, CF_APP_SPRT_FAIL)) {
				    	pvt->sprt_mode = SPRT_MODE_REFUSED;
				    	continue;
				    } else if (switch_channel_test_app_flag_key("SPRT", channel, CF_APP_SPRT)) {
				    	switch_core_session_message_t msg = { 0 };
				    	pvt->sprt_mode = SPRT_MODE_NEGOTIATED;
				    	switch_channel_set_app_flag_key("SPRT", channel, CF_APP_SPRT_NEGOTIATED);
				    	if (sprt_init(pvt, SPRT_MODE) == SWITCH_STATUS_SUCCESS) {
				    		configure_sprt(pvt);
				    		/* add to timer thread processing */
				    		if (!add_pvt(pvt)) {
				    			switch_channel_hangup(channel, SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER);
				    		}
				    	} else {
				    		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Cannot initialize SPRT - sprt_init (SPRT_REQ)");
				    		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "Cannot initialize SPRT - sprt_init (SPRT_REQ)");
				    		goto done;
				    	}

				    	/* This will change the rtp stack to udptl mode */
				    	msg.from = __FILE__;
				    	msg.message_id = SWITCH_MESSAGE_INDICATE_UDPTL_MODE;
				    	switch_core_session_receive_message(session, &msg);
				    }
				    continue;
				}
				break;

			case SPRT_MODE_UNKNOWN:
		   {
		   		if (req_counter) {
		   			if (!--req_counter) {
		   				request_sprt(pvt);
		   			}
		   		}

		   	if (switch_channel_test_app_flag_key("SPRT", channel, CF_APP_SPRT)) {
		   		if (negotiate_sprt(pvt) == SPRT_MODE_NEGOTIATED) {

		   			if (sprt_init(pvt, SPRT_MODE) == SWITCH_STATUS_SUCCESS) {
		   				/* add to timer thread processing */
		   				if (!add_pvt(pvt)) {
		   					switch_channel_hangup(channel, SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER);
		   				}
		   			} else {
		   				switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Cannot initialize SPRT - sprt_init (SPRT_UNKNOWN)\n");
		   				switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "Cannot initialize SPRT - sprt_init (SPRT_UNKNOWN)");
		   				goto done;
		   			}
		   			continue;
		   		}
		   	}
		   }
				break;
			case SPRT_MODE_NEGOTIATED:
				break;
			default:
				break;
		}
	}

	done: 

	// destroy sprt structures - TODO

	switch_channel_clear_app_flag_key("SPRT", channel, CF_APP_SPRT_POSSIBLE);

	switch_core_session_set_read_codec(session, NULL);

	if (switch_core_codec_ready(&read_codec)) {
		switch_core_codec_destroy(&read_codec)
	}

	if (switch_core_codec_ready(&write_codec)) {
		switch_core_codec_destroy(&write_codec)
	}

}

static switch_status_t sprt_init (pvt_t *pvt, transport_mode_t trans_mode) 
{
	switch_core_session_t *session;
	switch_channel_t *channel;
	const char *tmp;

	session = (switch_core_session_t *) pvt->session;
	switch_assert(session);

	channel = switch_core_session_get_channel(session);
	switch_assert(channel);

	switch (trans_mode) {

	case AUDIO_MODE:
		break;

	case SPRT_MODE:
		switch_core_session_message_t msg = { 0 };

		msg.from = __FILE__;
		msg.message_id = SWITCH_MESSAGE_INDICATE_UDPTL_MODE;
		switch_core_session_receive_message(pvt->session, &msg);

		break;

	case VBD_MODE:
		break;

	default:
		assert(0);
		break;

	}							/* Switch trans mode */

	return SWITCH_STATUS_SUCCESS;
}

static sprt_mode_t configure_sprt(pvt_t *pvt) 
{
	switch_core_session_t *session;
	switch_channel_t *channel;
	switch_sprt_options_t *sprt_options;
	int method = 2;

	switch_assert(pvt && pvt->session);
	session = pvt->session;
	channel = switch_core_session_get_channel(session);
	sprt_options = switch_channel_get_private(channel, "sprt_options");

	if (!sprt_options) {
		pvt->sprt_mode = SPRT_MODE_REFUSED;
		return pvt->sprt_mode;
	}

	return pvt->sprt_mode;
}

static sprt_mode_t negotiate_sprt(pvt_t *pvt) 
{
	switch_core_session_t *session = pvt->session;
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_core_session_message_t msg = { 0 };
	switch_sprt_options_t *sprt_options = switch_channel_get_private(channel, "sprt_options");
	int enabled = 0, insist = 0;
	const char *v;

	pvt->sprt_mode = SPRT_MODE_REFUSED;

	if (pvt->app_mode == FUNCTION_GW) {
		enabled = 1;
	} else {
		enabled = v150_globals.enable_sprt;
	}

	if (!(enabled && sprt_options)) {
		/* if there is no sprt_options the endpoint will refuse the transition */
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_WARNING, "%s NO SPRT options detected.\n", switch_channel_get_name(channel));
		switch_channel_set_private(channel, "sprt_options", NULL);
	} else {
		pvt->sprt_mode = SPRT_MODE_NEGOTIATED;
		switch_channel_set_app_flag_key("SPRT", channel, CF_APP_SPRT_NEGOTIATED);

		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO,
			"SPRT SDP Origin = %s\n"
			"SPRT ModemRelayType = %d\n"
			"SPRT MediaGatewayType = %d\n"
			"SPRT ModemRelayModulations = %d\n"
			"SPRT CallDiscriminationSelect = %d\n"
			"SPRT JMDelay = %d\n"
			"ip = '%s'\n"
			"port = %d\n",
			sprt_options->sdp_o_line,
			sprt_options->modem_relay_type,
			sprt_options->media_gateway_type,
			sprt_options->mr_mods,
			sprt_options->cdsc_select,
			sprt_options->jm_delay,
			sprt_options->remote_ip ? sprt_options->remote_ip : "Not specified",
			sprt_options->remote_port);

		// Keep defaults for now - Negotiation happens here 
	}

	if ((v = switch_channel_get_variable(channel, "enable_sprt_insist"))) {
		insist = switch_true(v);
	} else {
		insist = v150_globals.enable_sprt_insist;
	}

	/* This will send the options back in a response */
	msg.from = __FILE__;
	msg.message_id = SWITCH_MESSAGE_INDICATE_SPRT_DESCRIPTION;
	msg.numeric_arg = insist;
	switch_core_session_receive_message(session, &msg);

	return pvt->sprt_mode;
}

static sprt_mode_t request_sprt(pvt_t *pvt) 
{
	switch_core_session_t *session = pvt->session;
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_core_session_message_t msg = { 0 };
	switch_sprt_options_t *sprt_options = NULL;
	int enabled = 0, insist = 0;
	const char *v;

	pvt->sprt_mode = SPRT_MODE_UNKNOWN;
	enabled = 1; // Set the sprt_options unless they already exist

	if (switch_channel_get_private(channel, "sprt_options")) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_WARNING,
				"%s already has SPRT data\n", switch_channel_get_name(channel));
		enabled = 0;
	}

	if (enabled) {
		if (!(sprt_options = switch_channel_get_private(channel, "_preconfigured_sprt_options"))) {
			sprt_options = switch_core_session_alloc(session, sizeof(*sprt_options));
			sprt_options->modem_relay_type = 0;
			sprt_options->media_gateway_type = 0;
			sprt_options->mr_mods = 0;
			sprt_options->cdsc_select = 0;
			sprt_options->jm_delay = SWITCH_FALSE;
		}

		switch_channel_set_private(channel, "sprt_options", sprt_options);
		switch_channel_set_private(channel, "_preconfigured_sprt_options", NULL);

		pvt->sprt_mode = SPRT_MODE_REQUESTED;
		switch_channel_set_app_flag_key("SPRT", channel, CF_APP_SPRT_REQ;

		/* This will send a request for sprt mode */
		msg.from = __FILE__;
		msg.message_id = SWITCH_MESSAGE_INDICATE_REQUEST_SPRT_MEDIA;
		msg.numeric_arg = insist;
		switch_core_session_receive_message(session, &msg);
	}

	return pvt->sprt_mode;
}

static void session_counter_increment(void)
{
	switch_mutex_lock(v150_globals.mutex);
	v150_globals.total_sessions++;
	switch_mutex_unlock(v150_globals.mutex);
}

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

static pvt_t *pvt_init(switch_core_session_t *session, mod_v150_application_mode_t app_mode)
{
	switch_channel_t *channel;
	pvt_t *pvt = NULL;
	const char *tmp;

	channel = switch_core_session_get_channel(session);
	switch_assert(channel != NULL);

	if (!switch_channel_media_ready(channel)) {
		switch_channel_answer(channel);
	}

	pvt = switch_core_session_alloc(session, sizeof(pvt_t));
	pvt->session = session;
	pvt->app_mode = app_mode;
	pvt->sprt_mode = SPRT_MODE_UNKNOWN;

	switch(pvt->app_mode) {

		case FUNCTION_TX:
			pvt->caller = 1;
			break;
		case FUNCTION_RX:
			pvt->caller = 0;
			break;
		case FUNCTION_GW:
			break;
	}

	switch_mutex_init(&pvt->mutex, SWITCH_MUTEX_NESTED, switch_core_session_get_pool(session));

	return pvt;
}