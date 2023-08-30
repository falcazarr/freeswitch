#include "mod_v150.h"

static switch_status_t switch_g711u_init(switch_codec_t *codec, switch_codec_flag_t flags, const switch_codec_settings_t *codec_settings)
{
	uint32_t encoding, decoding;

	encoding = (flags & SWITCH_CODEC_FLAG_ENCODE);
	decoding = (flags & SWITCH_CODEC_FLAG_DECODE);

	if (!(encoding || decoding)) {
		return SWITCH_STATUS_FALSE;
	} else {
		return SWITCH_STATUS_SUCCESS;
	}
}

static switch_status_t switch_g711u_encode(switch_codec_t *codec,
										   switch_codec_t *other_codec,
										   void *decoded_data,
										   uint32_t decoded_data_len,
										   uint32_t decoded_rate, void *encoded_data, uint32_t *encoded_data_len, uint32_t *encoded_rate,
										   unsigned int *flag)
{
	short *dbuf;
	unsigned char *ebuf;
	uint32_t i;

	dbuf = decoded_data;
	ebuf = encoded_data;

	for (i = 0; i < decoded_data_len / sizeof(short); i++) {
		ebuf[i] = linear_to_ulaw(dbuf[i]);
	}

	*encoded_data_len = i;

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_g711u_decode(switch_codec_t *codec,
										   switch_codec_t *other_codec,
										   void *encoded_data,
										   uint32_t encoded_data_len,
										   uint32_t encoded_rate, void *decoded_data, uint32_t *decoded_data_len, uint32_t *decoded_rate,
										   unsigned int *flag)
{
	short *dbuf;
	unsigned char *ebuf;
	uint32_t i;

	dbuf = decoded_data;
	ebuf = encoded_data;

	if (*flag & SWITCH_CODEC_FLAG_SILENCE) {
		memset(dbuf, 0, codec->implementation->decoded_bytes_per_packet);
		*decoded_data_len = codec->implementation->decoded_bytes_per_packet;
	} else {
		for (i = 0; i < encoded_data_len; i++) {
			dbuf[i] = ulaw_to_linear(ebuf[i]);
		}

		*decoded_data_len = i * 2;
	}

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_g711u_destroy(switch_codec_t *codec)
{
	return SWITCH_STATUS_SUCCESS;
}


static switch_status_t switch_g711a_init(switch_codec_t *codec, switch_codec_flag_t flags, const switch_codec_settings_t *codec_settings)
{
	uint32_t encoding, decoding;

	encoding = (flags & SWITCH_CODEC_FLAG_ENCODE);
	decoding = (flags & SWITCH_CODEC_FLAG_DECODE);

	if (!(encoding || decoding)) {
		return SWITCH_STATUS_FALSE;
	} else {
		return SWITCH_STATUS_SUCCESS;
	}
}

static switch_status_t switch_g711a_encode(switch_codec_t *codec,
										   switch_codec_t *other_codec,
										   void *decoded_data,
										   uint32_t decoded_data_len,
										   uint32_t decoded_rate, void *encoded_data, uint32_t *encoded_data_len, uint32_t *encoded_rate,
										   unsigned int *flag)
{
	short *dbuf;
	unsigned char *ebuf;
	uint32_t i;

	dbuf = decoded_data;
	ebuf = encoded_data;

	for (i = 0; i < decoded_data_len / sizeof(short); i++) {
		ebuf[i] = linear_to_alaw(dbuf[i]);
	}

	*encoded_data_len = i;

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_g711a_decode(switch_codec_t *codec,
										   switch_codec_t *other_codec,
										   void *encoded_data,
										   uint32_t encoded_data_len,
										   uint32_t encoded_rate, void *decoded_data, uint32_t *decoded_data_len, uint32_t *decoded_rate,
										   unsigned int *flag)
{
	short *dbuf;
	unsigned char *ebuf;
	uint32_t i;

	dbuf = decoded_data;
	ebuf = encoded_data;

	if (*flag & SWITCH_CODEC_FLAG_SILENCE) {
		memset(dbuf, 0, codec->implementation->decoded_bytes_per_packet);
		*decoded_data_len = codec->implementation->decoded_bytes_per_packet;
	} else {
		for (i = 0; i < encoded_data_len; i++) {
			dbuf[i] = alaw_to_linear(ebuf[i]);
		}

		*decoded_data_len = i * 2;
	}

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_g711a_destroy(switch_codec_t *codec)
{
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t v150_codecs_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool)
{
  switch_codec_interface_t * codec_interface;
  int mpf, spf, bpf, ebpf, count;

    /* G711 */
	mpf = 10000, spf = 80, bpf = 160, ebpf = 80;
	SWITCH_ADD_CODEC(codec_interface, "G.711 ulaw");
	for (count = 12; count > 0; count--) {
		switch_core_codec_add_implementation(pool, codec_interface, SWITCH_CODEC_TYPE_AUDIO,	/* enumeration defining the type of the codec */
											 0,	/* the IANA code number */
											 "PCMU",	/* the IANA code name */
											 NULL,	/* default fmtp to send (can be overridden by the init function) */
											 8000,	/* samples transferred per second */
											 8000,	/* actual samples transferred per second */
											 64000,	/* bits transferred per second */
											 mpf * count,	/* number of microseconds per frame */
											 spf * count,	/* number of samples per frame */
											 bpf * count,	/* number of bytes per frame decompressed */
											 ebpf * count,	/* number of bytes per frame compressed */
											 1,	/* number of channels represented */
											 spf * count,	/* number of frames per network packet */
											 switch_g711u_init,	/* function to initialize a codec handle using this implementation */
											 switch_g711u_encode,	/* function to encode raw data into encoded data */
											 switch_g711u_decode,	/* function to decode encoded data into raw data */
											 switch_g711u_destroy);	/* deinitalize a codec handle using this implementation */
	}

	SWITCH_ADD_CODEC(codec_interface, "G.711 alaw");
	for (count = 12; count > 0; count--) {
		switch_core_codec_add_implementation(pool, codec_interface, SWITCH_CODEC_TYPE_AUDIO,	/* enumeration defining the type of the codec */
											 8,	/* the IANA code number */
											 "PCMA",	/* the IANA code name */
											 NULL,	/* default fmtp to send (can be overridden by the init function) */
											 8000,	/* samples transferred per second */
											 8000,	/* actual samples transferred per second */
											 64000,	/* bits transferred per second */
											 mpf * count,	/* number of microseconds per frame */
											 spf * count,	/* number of samples per frame */
											 bpf * count,	/* number of bytes per frame decompressed */
											 ebpf * count,	/* number of bytes per frame compressed */
											 1,	/* number of channels represented */
											 spf * count,	/* number of frames per network packet */
											 switch_g711a_init,	/* function to initialize a codec handle using this implementation */
											 switch_g711a_encode,	/* function to encode raw data into encoded data */
											 switch_g711a_decode,	/* function to decode encoded data into raw data */
											 switch_g711a_destroy);	/* deinitalize a codec handle using this implementation */
	}
    
  return SWITCH_STATUS_SUCCESS;
}