#include <switch.h>

/* The global stuff */
struct sprt_globals {
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

extern struct sprt_globals sprt_globals;


typedef enum {
	FUNCTION_TX,
	FUNCTION_RX,
	FUNCTION_GW
} mod_sprt_application_mode_t;

struct mod_sprt_log_data {
	switch_log_level_t verbose_log_level;
	switch_core_session_t *session;
	FILE *trace_file;
};
typedef struct mod_sprt_log_data mod_sprt_log_data_t;

/* Functions - below */

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet:
 */