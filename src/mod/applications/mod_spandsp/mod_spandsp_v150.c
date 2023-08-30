/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2014, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH mod_fax.
 *
 * The Initial Developer of the Original Code is
 * Massimo Cetra <devel@navynet.it>
 *
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Brian West <brian@freeswitch.org>
 * Anthony Minessale II <anthm@freeswitch.org>
 * Steve Underwood <steveu@coppice.org>
 * Antonio Gallo <agx@linux.it>
 * mod_spandsp_v150.c -- Fax applications provided by SpanDSP
 *
 */

#include "mod_spandsp.h"

#include "udptl.h"

/*****************************************************************************
	OUR DEFINES AND STRUCTS
*****************************************************************************/
struct pvt_s {
	switch_core_session_t *session;

	//mod_spandsp_fax_application_mode_t app_mode;

	switch_mutex_t *mutex;

	udptl_state_t *udptl_state;

	char *filename;
	char *ident;
	char *header;
	char *timezone;

	struct pvt_s *next;
};

typedef struct pvt_s pvt_t;

static pvt_t *pvt_init(switch_core_session_t *session)
{
	switch_channel_t *channel;
	pvt_t *pvt = NULL;
	//const char *tmp;

	/* Make sure we have a valid channel when starting the FAX application */
	channel = switch_core_session_get_channel(session);
	switch_assert(channel != NULL);

	if (!switch_channel_media_ready(channel)) {
		switch_channel_answer(channel);
	}

	/* Allocate our structs */
	pvt = switch_core_session_alloc(session, sizeof(pvt_t));
	pvt->session = session;

	switch_mutex_init(&pvt->mutex, SWITCH_MUTEX_NESTED, switch_core_session_get_pool(session));

	return pvt;
}

void mod_spandsp_v150_sprt(switch_core_session_t *session, const char *data)
{
	pvt_t *pvt;
	switch_channel_t *channel = switch_core_session_get_channel(session);

	pvt = pvt_init(session);
	switch_channel_set_private(channel, "_sprt_pvt", pvt);
}

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
