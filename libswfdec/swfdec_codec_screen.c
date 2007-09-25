/* Swfdec
 * Copyright (C) 2007 Benjamin Otte <otte@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <zlib.h>
#include <liboil/liboil.h>

#include "swfdec_codec_video.h"
#include "swfdec_bits.h"
#include "swfdec_debug.h"
#include "swfdec_internal.h"

typedef struct _SwfdecCodecScreen SwfdecCodecScreen;

struct _SwfdecCodecScreen {
  SwfdecVideoDecoder	decoder;	/* the decoder */
  gulong		width;		/* width of last image */
  gulong		height;		/* height of last image */
  SwfdecBuffer *	buffer;		/* buffer containing last decoded image */
};

static SwfdecBuffer *
swfdec_video_decoder_screen_decode (SwfdecVideoDecoder *dec, SwfdecBuffer *buffer,
    guint *width, guint *height, guint *rowstride)
{
  SwfdecCodecScreen *screen = (SwfdecCodecScreen *) dec;
  SwfdecBuffer *ret;
  SwfdecBits bits;
  gulong i, j, w, h, bw, bh, stride;

  swfdec_bits_init (&bits, buffer);
  bw = (swfdec_bits_getbits (&bits, 4) + 1) * 16;
  w = swfdec_bits_getbits (&bits, 12);
  bh = (swfdec_bits_getbits (&bits, 4) + 1) * 16;
  h = swfdec_bits_getbits (&bits, 12);
  if (screen->width == 0 || screen->height == 0) {
    if (w == 0 || h == 0) {
      SWFDEC_ERROR ("width or height is 0: %lux%lu", w, h);
      return NULL;
    }
    screen->width = w;
    screen->height = h;
  } else if (screen->width != w || screen->height != h) {
    SWFDEC_ERROR ("width or height differ from original: was %lux%lu, is %lux%lu",
	screen->width, screen->height, w, h);
    /* FIXME: this is what ffmpeg does, should we be more forgiving? */
    return NULL;
  }
  if (screen->buffer && screen->buffer->ref_count == 1) {
    g_assert (screen->buffer->length == w * h * 4);
    swfdec_buffer_ref (screen->buffer);
    ret = screen->buffer;
  } else {
    ret = swfdec_buffer_new_and_alloc (w * h * 4);
    if (screen->buffer) {
      g_assert (screen->buffer->length == w * h * 4);
      memcpy (ret->data, screen->buffer->data, screen->buffer->length);
      swfdec_buffer_unref (screen->buffer);
    }
    swfdec_buffer_ref (ret);
    screen->buffer = ret;
  }
  stride = w * 4;
  SWFDEC_LOG ("size: %lu x %lu - block size %lu x %lu\n", w, h, bw, bh);
  for (j = 0; j < h; j += bh) {
    for (i = 0; i < w; i += bw) {
      guint x, y, size;
      SwfdecBuffer *buf;
      guint8 *in, *out;
      size = swfdec_bits_get_bu16 (&bits);
      if (size == 0)
	continue;
      buf = swfdec_bits_decompress (&bits, size, bw * bh * 4);
      if (buf == NULL) {
	SWFDEC_WARNING ("error decoding block");
	continue;
      }
      /* convert format and write out data */
      out = ret->data + stride * (h - j - 1) + i * 4;
      in = buf->data;
      for (y = 0; y < MIN (bh, h - j); y++) {
	for (x = 0; x < MIN (bw, w - i); x++) {
	  out[x * 4 - y * stride + SWFDEC_COLOR_INDEX_BLUE] = *in++;
	  out[x * 4 - y * stride + SWFDEC_COLOR_INDEX_GREEN] = *in++;
	  out[x * 4 - y * stride + SWFDEC_COLOR_INDEX_RED] = *in++;
	  out[x * 4 - y * stride + SWFDEC_COLOR_INDEX_ALPHA] = 0xFF;
	}
      }
      swfdec_buffer_unref (buf);
    }
  }
  *width = screen->width;
  *height = screen->height;
  *rowstride = stride;
  return ret;
}

static void
swfdec_video_decoder_screen_free (SwfdecVideoDecoder *dec)
{
  SwfdecCodecScreen *screen = (SwfdecCodecScreen *) dec;

  if (screen->buffer)
    swfdec_buffer_unref (screen->buffer);
  g_free (screen);
}

SwfdecVideoDecoder *
swfdec_video_decoder_screen_new (SwfdecVideoCodec type)
{
  SwfdecCodecScreen *screen;
  
  if (type != SWFDEC_VIDEO_CODEC_SCREEN)
    return NULL;
  
  screen = g_new0 (SwfdecCodecScreen, 1);
  screen->decoder.decode = swfdec_video_decoder_screen_decode;
  screen->decoder.free = swfdec_video_decoder_screen_free;

  return &screen->decoder;
}

