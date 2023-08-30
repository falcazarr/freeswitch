#include <switch.h>

/* The global stuff */
struct v150_globals {
	switch_memory_pool_t *pool;
	switch_memory_pool_t *config_pool;
	switch_mutex_t *mutex;

	uint32_t total_sessions;

	short int verbose;
	switch_log_level_t verbose_log_level;
	short int enable_sprt;
	short int enable_sprt_request;
	short int enable_sprt_insist;
	char *ident;
	char *header;
	char *timezone;
	char *prepend_string;
	char *spool;
	switch_thread_cond_t *cond;
	switch_mutex_t *cond_mutex;
	int modem_count;
	int modem_verbose;
	char *modem_context;
	char *modem_dialplan;
	char *modem_directory;
  int sprt_tx_reinvite_packet_count;
  int sprt_rx_reinvite_packet_count;
};

extern struct v150_globals v150_globals;

typedef enum {
	FUNCTION_TX,
	FUNCTION_RX,
	FUNCTION_GW
} mod_v150_application_mode_t;

typedef enum {
	SPRT_MODE_AUDIO,
	SPRT_MODE_VBD,
  SPRT_MODE_MR	
} mod_sprt_mode_t;

struct mod_v150_log_data {
	switch_log_level_t verbose_log_level;
	switch_core_session_t *session;
	FILE *trace_file;
};

typedef struct mod_v150_log_data mod_v150_log_data_t;

void mod_v150_log_message(void *session, int level, const char *msg);
switch_status_t load_configuration(switch_bool_t reload);
switch_status_t v150_codecs_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool);