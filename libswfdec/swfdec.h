
#ifndef __SWFDEC_H__
#define __SWFDEC_H__

#include <glib.h>

G_BEGIN_DECLS

enum {
	SWF_OK = 0,
	SWF_NEEDBITS,
	SWF_WAIT,
	SWF_ERROR,
	SWF_EOF,
	SWF_IMAGE,
	SWF_CHANGE,
};

enum {
	SWF_COLORSPACE_RGB888 = 0,
	SWF_COLORSPACE_RGB565,
};

typedef struct swfdec_decoder_struct SwfdecDecoder;

SwfdecDecoder *swfdec_decoder_new(void);
int swfdec_decoder_addbits(SwfdecDecoder *s, unsigned char *bits, int len);
int swfdec_decoder_parse(SwfdecDecoder *s);
int swfdec_decoder_free(SwfdecDecoder *s);

int swfdec_decoder_get_n_frames(SwfdecDecoder *s, int *n_frames);
int swfdec_decoder_get_rate(SwfdecDecoder *s, double *rate);
int swfdec_decoder_get_image(SwfdecDecoder *s, unsigned char **image);
int swfdec_decoder_peek_image(SwfdecDecoder *s, unsigned char **image);
int swfdec_decoder_get_image_size(SwfdecDecoder *s, int *width, int *height);

int swfdec_decoder_set_colorspace(SwfdecDecoder *s, int colorspace);
int swfdec_decoder_set_debug_level(SwfdecDecoder *s, int level);

void *swfdec_decoder_get_sound_chunk(SwfdecDecoder *s, int *length);

G_END_DECLS

#endif

