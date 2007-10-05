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

#include "swfdec_edittext.h"
#include "swfdec_edittext_movie.h"
#include "swfdec_as_strings.h"
#include "swfdec_debug.h"
#include "swfdec_as_native_function.h"
#include "swfdec_as_internal.h"
#include "swfdec_as_context.h"
#include "swfdec_as_frame_internal.h"
#include "swfdec_internal.h"
#include "swfdec_player_internal.h"

static void
swfdec_edit_text_movie_get_text (SwfdecAsContext *cx, SwfdecAsObject *object,
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *ret)
{
  SwfdecEditTextMovie *text;

  if (!SWFDEC_IS_EDIT_TEXT_MOVIE (object))
    return;
  text = SWFDEC_EDIT_TEXT_MOVIE (object);

  SWFDEC_AS_VALUE_SET_STRING (ret, (text->text_display != NULL ? swfdec_as_context_get_string (cx, text->text_display) : SWFDEC_AS_STR_EMPTY));
}

static void
swfdec_edit_text_movie_do_set_text (SwfdecAsContext *cx, SwfdecAsObject *object,
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *ret)
{
  SwfdecEditTextMovie *text;
  const char *value;

  SWFDEC_AS_CHECK (SWFDEC_TYPE_EDIT_TEXT_MOVIE, (gpointer)&text, "s", &value);

  swfdec_edit_text_movie_set_text (text, value);
}

/*
 * Order of tags:
 * TEXTFORMAT / P or LI / FONT / A / B / I / U
 *
 * Order of attributes:
 * TEXTFORMAT:
 * LEFTMARGIN / RIGHTMARGIN / INDENT / LEADING / BLOCKINDENT / TABSTOPS
 * P: ALIGN
 * LI: none
 * FONT: FACE / SIZE / COLOR / LETTERSPACING / KERNING
 * A: HREF / TARGET
 * B: none
 * I: none
 * U: none
 */

typedef enum {
  CHANGE_LEVEL_NONE,
  CHANGE_LEVEL_UNDERLINE,
  CHANGE_LEVEL_ITALIC,
  CHANGE_LEVEL_BOLD,
  CHANGE_LEVEL_A,
  CHANGE_LEVEL_FONT,
  CHANGE_LEVEL_P,
  CHANGE_LEVEL_TEXTFORMAT,
} ChangeLevel;

static const char *
align_to_string (SwfdecTextAlign align)
{
  switch (align) {
    case SWFDEC_TEXT_ALIGN_LEFT:
      return "LEFT";
    case SWFDEC_TEXT_ALIGN_RIGHT:
      return "RIGHT";
    case SWFDEC_TEXT_ALIGN_CENTER:
      return "CENTER";
    case SWFDEC_TEXT_ALIGN_JUSTIFY:
      return "JUSTIFY";
    default:
      g_assert_not_reached ();
  }
}

static void
swfdec_edit_text_movie_get_htmlText (SwfdecAsContext *cx,
    SwfdecAsObject *object, guint argc, SwfdecAsValue *argv,
    SwfdecAsValue *ret)
{
  SwfdecEditTextMovie *text;
  SwfdecTextFormat *format, *format_prev, *format_font;
  GString *string;
  GSList *iter, *fonts;
  guint index_, index_prev;
  int change_level;
  gboolean bullet = FALSE;

  SWFDEC_AS_CHECK (SWFDEC_TYPE_EDIT_TEXT_MOVIE, (gpointer)&text, "");

  if (text->text->html == FALSE) {
    swfdec_edit_text_movie_get_text (cx, object, argc, argv, ret);
    return;
  }

  if (text->text_display == NULL) {
    SWFDEC_AS_VALUE_SET_STRING (ret, SWFDEC_AS_STR_EMPTY);
    return;
  }

  string = g_string_new ("");

  fonts = NULL;
  index_prev = 0;
  format_prev = NULL;
  for (iter = text->formats; iter != NULL; iter = iter->next)
  {
    index_ = ((SwfdecFormatIndex *)(iter->data))->index;
    format = ((SwfdecFormatIndex *)(iter->data))->format;

    string = g_string_append_len (string, text->text_display + index_prev,
	index_ - index_prev);

    if (format_prev == NULL) {
      if (format->bullet) {
	g_string_append (string, "<LI>");
	bullet = TRUE;
      } else {
	g_string_append_printf (string, "<P ALIGN=\"%s\">",
	    align_to_string (format->align));
	bullet = FALSE;
      }
      g_string_append_printf (string, "<FONT FACE=\"%s\" SIZE=\"%i\" COLOR=\"#%06X\" LETTERSPACING=\"%i\" KERNING=\"%i\">", format->font, format->size, format->color, (int)format->letter_spacing, (format->kerning ? 1 : 0));
      fonts = g_slist_prepend (fonts, format);
      if (format->url != SWFDEC_AS_STR_EMPTY)
	g_string_append_printf (string, "<A HREF=\"%s\" TARGET=\"%s\">",
	    format->url, format->target);
      if (format->bold)
	string = g_string_append (string, "<B>");
      if (format->italic)
	string = g_string_append (string, "<I>");
      if (format->underline)
	string = g_string_append (string, "<U>");
    } else {
      // Figure out what tags need to be rewritten
      if (format->font != format_prev->font ||
	  format->size != format_prev->size ||
	  format->color != format_prev->color ||
	  (int)format->letter_spacing != (int)format_prev->letter_spacing ||
	  format->kerning != format_prev->kerning) {
	change_level = CHANGE_LEVEL_FONT;
      } else if (format->url != format_prev->url ||
	  format->target != format_prev->target) {
	change_level = CHANGE_LEVEL_A;
      } else if (format->bold != format_prev->bold) {
	change_level = CHANGE_LEVEL_BOLD;
      } else if (format->italic != format_prev->italic) {
	change_level = CHANGE_LEVEL_ITALIC;
      } else if (format->underline != format_prev->underline) {
	change_level = CHANGE_LEVEL_UNDERLINE;
      } else {
	change_level = CHANGE_LEVEL_NONE;
      }

      // Close tags
      if (change_level >= CHANGE_LEVEL_UNDERLINE) {
	GSList *iter_font;
	for (iter_font = fonts; iter_font != NULL; iter_font = iter_font->next)
	{
	  format_font = (SwfdecTextFormat *)iter_font->data;
	  if (format->font == format_font->font &&
	    format->size == format_font->size &&
	    format->color == format_font->color &&
	    (int)format->letter_spacing == (int)format_font->letter_spacing &&
	    format->kerning == format_font->kerning) {
	    break;
	  }
	}
	if (iter_font != NULL) {
	  while (fonts != iter_font) {
	    string = g_string_append (string, "</FONT>");
	    fonts = g_slist_remove (fonts, fonts->data);
	  }
	}
      }
      if (format_prev->underline && change_level >= CHANGE_LEVEL_UNDERLINE)
	string = g_string_append (string, "</U>");
      if (format_prev->italic && change_level >= CHANGE_LEVEL_UNDERLINE)
	string = g_string_append (string, "</I>");
      if (format_prev->bold && change_level >= CHANGE_LEVEL_UNDERLINE)
	string = g_string_append (string, "</B>");
      if (format_prev->url != SWFDEC_AS_STR_EMPTY &&
	  change_level >= CHANGE_LEVEL_UNDERLINE) {
	string = g_string_append (string, "</A>");
      }

      // Open tags
      format_font = (SwfdecTextFormat *)fonts->data;
      if (change_level >= CHANGE_LEVEL_FONT &&
	  (format->font != format_font->font ||
	   format->size != format_font->size ||
	   format->color != format_font->color ||
	   (int)format->letter_spacing != (int)format_font->letter_spacing ||
	   format->kerning != format_font->kerning))
      {
	fonts = g_slist_prepend (fonts, format);

	string = g_string_append (string, "<FONT");
	if (format->font != format_font->font)
	  g_string_append_printf (string, " FONT=\"%s\"", format->font);
	if (format->size != format_font->size)
	  g_string_append_printf (string, " SIZE=\"%i\"", format->size);
	if (format->color != format_font->color)
	  g_string_append_printf (string, " COLOR=\"#%06X\"", format->color);
	if ((int)format->letter_spacing != (int)format_font->letter_spacing) {
	  g_string_append_printf (string, " LETTERSPACING=\"%i\"",
	      (int)format->letter_spacing);
	}
	if (format->kerning != format_font->kerning) {
	  g_string_append_printf (string, " KERNING=\"%i\"",
	      (format->kerning ? 1 : 0));
	}
	string = g_string_append (string, ">");
      }
      if (format->url != SWFDEC_AS_STR_EMPTY &&
	  change_level >= CHANGE_LEVEL_UNDERLINE) {
	g_string_append_printf (string, "<A HREF=\"%s\" TARGET=\"%s\">",
	    format->url, format->target);
      }
      if (format->bold && change_level >= CHANGE_LEVEL_UNDERLINE)
	string = g_string_append (string, "<B>");
      if (format->italic && change_level >= CHANGE_LEVEL_UNDERLINE)
	string = g_string_append (string, "<I>");
      if (format->underline && change_level >= CHANGE_LEVEL_UNDERLINE)
	string = g_string_append (string, "<U>");
    }

    index_prev = index_;
    format_prev = format;
  }
  string = g_string_append (string, text->text_display + index_prev);

  g_assert (format_prev != NULL);

  if (format_prev->underline)
    string = g_string_append (string, "</U>");
  if (format_prev->italic)
    string = g_string_append (string, "</I>");
  if (format_prev->bold)
    string = g_string_append (string, "</B>");
  if (format_prev->url != SWFDEC_AS_STR_EMPTY)
    string = g_string_append (string, "</A>");
  for (iter = fonts; iter != NULL; iter = iter->next)
    string = g_string_append (string, "</FONT>");
  g_slist_free (fonts);
  if (bullet) {
    string = g_string_append (string, "</LI>");
  } else {
    string = g_string_append (string, "</P>");
  }

  SWFDEC_AS_VALUE_SET_STRING (ret,
      swfdec_as_context_give_string (cx, g_string_free (string, FALSE)));
}

static void
swfdec_edit_text_movie_set_htmlText (SwfdecAsContext *cx,
    SwfdecAsObject *object, guint argc, SwfdecAsValue *argv,
    SwfdecAsValue *ret)
{
  SwfdecEditTextMovie *text;
  const char *value;

  SWFDEC_AS_CHECK (SWFDEC_TYPE_EDIT_TEXT_MOVIE, (gpointer)&text, "s", &value);

  swfdec_edit_text_movie_set_text (text, value);
}

static void
swfdec_edit_text_movie_get_html (SwfdecAsContext *cx, SwfdecAsObject *object,
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *ret)
{
  SwfdecEditTextMovie *text;

  SWFDEC_AS_CHECK (SWFDEC_TYPE_EDIT_TEXT_MOVIE, (gpointer)&text, "");

  SWFDEC_AS_VALUE_SET_BOOLEAN (ret, text->text->html);
}

static void
swfdec_edit_text_movie_set_html (SwfdecAsContext *cx, SwfdecAsObject *object,
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *ret)
{
  SwfdecEditTextMovie *text;
  gboolean value;

  SWFDEC_AS_CHECK (SWFDEC_TYPE_EDIT_TEXT_MOVIE, (gpointer)&text, "b", &value);

  text->text->html = value;
}

SWFDEC_AS_NATIVE (104, 102, swfdec_edit_text_movie_setTextFormat)
void
swfdec_edit_text_movie_setTextFormat (SwfdecAsContext *cx,
    SwfdecAsObject *object, guint argc, SwfdecAsValue *argv,
    SwfdecAsValue *ret)
{
  SwfdecEditTextMovie *text;
  int i;
  SwfdecFormatIndex *findex, *findex_new;
  SwfdecTextFormat *format;
  guint findex_end_index, start_index, end_index;
  GSList *iter, *next;

  SWFDEC_AS_CHECK (SWFDEC_TYPE_EDIT_TEXT_MOVIE, (gpointer)&text, "");

  if (argc < 1)
    return;

  i = 0;
  if (argc >= 2) {
    start_index = swfdec_as_value_to_integer (cx, &argv[i++]);
    start_index = MIN (start_index, strlen (text->text_display));
  } else {
    start_index = 0;
  }
  if (argc >= 3) {
    end_index = swfdec_as_value_to_integer (cx, &argv[i++]);
    end_index = CLAMP (end_index, start_index, strlen (text->text_display));
  } else {
    end_index = strlen (text->text_display);
  }
  if (start_index == end_index)
    return;

  if (!SWFDEC_AS_VALUE_IS_OBJECT (&argv[i]))
    return;
  if (!SWFDEC_IS_TEXT_FORMAT (SWFDEC_AS_VALUE_GET_OBJECT (&argv[i])))
    return;

  format = SWFDEC_TEXT_FORMAT (SWFDEC_AS_VALUE_GET_OBJECT (&argv[i]));

  g_assert (text->formats != NULL);
  g_assert (text->formats->data != NULL);
  g_assert (((SwfdecFormatIndex *)text->formats->data)->index == 0);
  for (iter = text->formats; iter != NULL &&
      ((SwfdecFormatIndex *)iter->data)->index < end_index;
      iter = next)
  {
    next = iter->next;
    findex = iter->data;
    if (iter->next != NULL) {
      findex_end_index =
	((SwfdecFormatIndex *)iter->next->data)->index;
    } else {
      findex_end_index = strlen (text->text_display);
    }

    if (findex_end_index < start_index)
      continue;

    if (findex_end_index > end_index) {
      findex_new = g_new (SwfdecFormatIndex, 1);
      findex_new->index = end_index;
      findex_new->format = swfdec_text_format_copy (findex->format);

      iter = g_slist_insert (iter, findex_new, 1);
    }

    if (findex->index < start_index) {
      findex_new = g_new (SwfdecFormatIndex, 1);
      findex_new->index = start_index;
      findex_new->format = swfdec_text_format_copy (findex->format);
      swfdec_text_format_add (findex_new->format, format);

      iter = g_slist_insert (iter, findex_new, 1);
    } else {
      swfdec_text_format_add (findex->format, format);
    }
  }

  swfdec_edit_text_movie_format_changed (text);
}

SWFDEC_AS_NATIVE (104, 200, swfdec_edit_text_movie_createTextField)
void
swfdec_edit_text_movie_createTextField (SwfdecAsContext *cx,
    SwfdecAsObject *object, guint argc, SwfdecAsValue *argv,
    SwfdecAsValue *rval)
{
  SwfdecMovie *movie, *parent;
  SwfdecEditText *edittext;
  int depth;
  const char *name;
  SwfdecAsFunction *fun;
  SwfdecAsValue val;

  SWFDEC_AS_CHECK (SWFDEC_TYPE_MOVIE, (gpointer)&parent, "si", &name, &depth);

  edittext = g_object_new (SWFDEC_TYPE_EDIT_TEXT, NULL);

  movie = swfdec_movie_find (parent, depth);
  if (movie)
    swfdec_movie_remove (movie);

  movie = swfdec_movie_new (SWFDEC_PLAYER (cx), depth, parent,
      SWFDEC_GRAPHIC (edittext), name);
  g_assert (SWFDEC_IS_EDIT_TEXT_MOVIE (movie));
  swfdec_movie_initialize (movie);

  swfdec_as_object_get_variable (cx->global, SWFDEC_AS_STR_TextField, &val);
  if (!SWFDEC_AS_VALUE_IS_OBJECT (&val))
    return;
  fun = (SwfdecAsFunction *) SWFDEC_AS_VALUE_GET_OBJECT (&val);
  if (!SWFDEC_IS_AS_FUNCTION (fun))
    return;

  /* set initial variables */
  if (swfdec_as_object_get_variable (SWFDEC_AS_OBJECT (fun),
	SWFDEC_AS_STR_prototype, &val)) {
    swfdec_as_object_set_variable_and_flags (SWFDEC_AS_OBJECT (movie),
	SWFDEC_AS_STR___proto__, &val,
	SWFDEC_AS_VARIABLE_HIDDEN | SWFDEC_AS_VARIABLE_PERMANENT);
  }
  SWFDEC_AS_VALUE_SET_OBJECT (&val, SWFDEC_AS_OBJECT (fun));
  if (cx->version < 7) {
    swfdec_as_object_set_variable_and_flags (SWFDEC_AS_OBJECT (movie),
	SWFDEC_AS_STR_constructor, &val, SWFDEC_AS_VARIABLE_HIDDEN);
  }
  swfdec_as_object_set_variable_and_flags (SWFDEC_AS_OBJECT (movie),
      SWFDEC_AS_STR___constructor__, &val,
      SWFDEC_AS_VARIABLE_HIDDEN | SWFDEC_AS_VARIABLE_VERSION_6_UP);

  swfdec_as_function_call (fun, SWFDEC_AS_OBJECT (movie), 0, NULL, rval);
  cx->frame->construct = TRUE;
  swfdec_as_context_run (cx);
}

static void
swfdec_edit_text_movie_add_variable (SwfdecAsObject *object,
    const char *variable, SwfdecAsNative get, SwfdecAsNative set)
{
  SwfdecAsFunction *get_func, *set_func;

  g_return_if_fail (SWFDEC_IS_AS_OBJECT (object));
  g_return_if_fail (variable != NULL);
  g_return_if_fail (get != NULL);

  get_func =
    swfdec_as_native_function_new (object->context, variable, get, 0, NULL);
  if (get_func == NULL)
    return;

  if (set != NULL) {
    set_func =
      swfdec_as_native_function_new (object->context, variable, set, 0, NULL);
  } else {
    set_func = NULL;
  }

  swfdec_as_object_add_variable (object, variable, get_func, set_func, 0);
}

SWFDEC_AS_CONSTRUCTOR (104, 0, swfdec_edit_text_movie_construct, swfdec_edit_text_movie_get_type)
void
swfdec_edit_text_movie_construct (SwfdecAsContext *cx, SwfdecAsObject *object,
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *ret)
{
  if (!cx->frame->construct) {
    SwfdecAsValue val;
    if (!swfdec_as_context_use_mem (cx, sizeof (SwfdecEditTextMovie)))
      return;
    object = g_object_new (SWFDEC_TYPE_EDIT_TEXT_MOVIE, NULL);
    swfdec_as_object_add (object, cx, sizeof (SwfdecEditTextMovie));
    swfdec_as_object_get_variable (cx->global, SWFDEC_AS_STR_TextField, &val);
    if (SWFDEC_AS_VALUE_IS_OBJECT (&val)) {
      swfdec_as_object_set_constructor (object, SWFDEC_AS_VALUE_GET_OBJECT (&val));
    } else {
      SWFDEC_INFO ("\"TextField\" is not an object");
    }
  }

  g_return_if_fail (SWFDEC_IS_EDIT_TEXT_MOVIE (object));

  if (!SWFDEC_PLAYER (cx)->edittext_movie_properties_initialized) {
    SwfdecAsValue val;
    SwfdecAsObject *proto;

    swfdec_as_object_get_variable (object, SWFDEC_AS_STR___proto__, &val);
    g_return_if_fail (SWFDEC_AS_VALUE_IS_OBJECT (&val));
    proto = SWFDEC_AS_VALUE_GET_OBJECT (&val);

    swfdec_edit_text_movie_add_variable (proto, SWFDEC_AS_STR_text,
	swfdec_edit_text_movie_get_text, swfdec_edit_text_movie_do_set_text);
    swfdec_edit_text_movie_add_variable (proto, SWFDEC_AS_STR_htmlText,
	swfdec_edit_text_movie_get_htmlText,
	swfdec_edit_text_movie_set_htmlText);
    swfdec_edit_text_movie_add_variable (proto, SWFDEC_AS_STR_html,
	swfdec_edit_text_movie_get_html, swfdec_edit_text_movie_set_html);

    SWFDEC_PLAYER (cx)->edittext_movie_properties_initialized = TRUE;
  }

  SWFDEC_AS_VALUE_SET_OBJECT (ret, object);
}