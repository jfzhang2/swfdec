#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib-object.h>
#include <swfdec.h>
#include <swfdec_button.h>
#include <swfdec_edittext.h>
#include <swfdec_font.h>
#include <swfdec_movie.h>
#include <swfdec_player_internal.h>
#include <swfdec_root_movie.h>
#include <swfdec_sprite.h>
#include <swfdec_shape.h>
#include <swfdec_shape.h>
#include <swfdec_swf_decoder.h>
#include <swfdec_text.h>

static gboolean verbose = FALSE;

static void
dump_sprite (SwfdecSprite *s)
{
  if (!verbose) {
    g_print ("  %u frames\n", s->n_frames);
  } else {
    guint i, j;

    for (i = 0; i < s->n_frames; i++) {
      SwfdecSpriteFrame * frame = &s->frames[i];
      if (frame->actions == NULL)
	continue;
      for (j = 0; j < frame->actions->len; j++) {
	SwfdecSpriteAction *action = 
	  &g_array_index (frame->actions, SwfdecSpriteAction, j);
	switch (action->type) {
	  case SWFDEC_SPRITE_ACTION_SCRIPT:
	    g_print ("   %4u script\n", i);
	  case SWFDEC_SPRITE_ACTION_UPDATE:
	  case SWFDEC_SPRITE_ACTION_REMOVE:
	    break;
	  case SWFDEC_SPRITE_ACTION_ADD:
	    {
	      SwfdecContent *content = action->data;
	      g_assert (content == content->sequence);
	      g_assert (content->start == i);
	      g_print ("   %4u -%4u %3u", i, content->end, content->depth);
	      if (content->clip_depth)
		g_print ("%4u", content->clip_depth);
	      else
		g_print ("    ");
	      if (content->graphic) {
		g_print (" %s %u", G_OBJECT_TYPE_NAME (content->graphic), 
		    SWFDEC_CHARACTER (content->graphic)->id);
	      } else {
		g_print (" ---");
	      }
	      if (content->name)
		g_print (" as %s", content->name);
	      g_print ("\n");
	    }
	    break;
	  default:
	    g_assert_not_reached ();
	}
      }
    }
  }
}

static void
dump_path (cairo_path_t *path)
{
  int i;
  cairo_path_data_t *data = path->data;
  const char *name;

  for (i = 0; i < path->num_data; i++) {
    name = NULL;
    switch (data[i].header.type) {
      case CAIRO_PATH_CURVE_TO:
	g_print ("      curve %g %g (%g %g . %g %g)\n",
	    data[i + 3].point.x, data[i + 3].point.y,
	    data[i + 1].point.x, data[i + 1].point.y,
	    data[i + 2].point.x, data[i + 2].point.y);
	i += 3;
	break;
      case CAIRO_PATH_LINE_TO:
	name = "line ";
      case CAIRO_PATH_MOVE_TO:
	if (!name)
	  name = "move ";
	i++;
	g_print ("      %s %g %g\n", name, data[i].point.x, data[i].point.y);
	break;
      case CAIRO_PATH_CLOSE_PATH:
	g_print ("      close\n");
	break;
      default:
	g_assert_not_reached ();
	break;
    }
  }
}

static void
dump_shape (SwfdecShape *shape)
{
  SwfdecShapeVec *shapevec;
  unsigned int i;

  for (i = 0; i < shape->vecs->len; i++) {
    shapevec = &g_array_index (shape->vecs, SwfdecShapeVec, i);

    g_print("   %3u: ", shapevec->last_index);
    if (shapevec->pattern == NULL) {
      g_print ("not filled\n");
    } else {
      char *str = swfdec_pattern_to_string (shapevec->pattern);
      g_print ("%s\n", str);
      g_free (str);
      if (verbose)
      g_print ("        %g %g  %g %g  %g %g\n", 
	  shapevec->pattern->transform.xx, shapevec->pattern->transform.xy,
	  shapevec->pattern->transform.yx, shapevec->pattern->transform.yy,
	  shapevec->pattern->transform.x0, shapevec->pattern->transform.y0);
    }
    if (verbose) {
      dump_path (&shapevec->path);
    }
  }
}

static void
dump_edit_text (SwfdecEditText *text)
{
  g_print ("  %s\n", text->text ? text->text : "");
  if (verbose) {
    if (text->variable)
      g_print ("  variable %s\n", text->variable);
    else
      g_print ("  no variable\n");
  }
}

static void
dump_text (SwfdecText *text)
{
  guint i;
  gunichar2 uni[text->glyphs->len];
  char *s;

  for (i = 0; i < text->glyphs->len; i++) {
    SwfdecTextGlyph *glyph = &g_array_index (text->glyphs, SwfdecTextGlyph, i);
    uni[i] = g_array_index (glyph->font->glyphs, SwfdecFontEntry, glyph->glyph).value;
    if (uni[i] == 0)
      goto fallback;
  }
  s = g_utf16_to_utf8 (uni, text->glyphs->len, NULL, NULL, NULL);
  if (s == NULL)
    goto fallback;
  g_print ("  text: %s\n", s);
  g_free (s);
  return;

fallback:
  g_print ("  %u characters\n", text->glyphs->len);
}

static void
dump_font (SwfdecFont *font)
{
  unsigned int i;
  if (font->name)
    g_print ("  %s\n", font->name);
  g_print ("  %u characters\n", font->glyphs->len);
  if (verbose) {
    for (i = 0; i < font->glyphs->len; i++) {
      gunichar2 c = g_array_index (font->glyphs, SwfdecFontEntry, i).value;
      char *s;
      if (c == 0 || (s = g_utf16_to_utf8 (&c, 1, NULL, NULL, NULL)) == NULL) {
	g_print (" ");
      } else {
	if (g_str_equal (s, "D"))
	  dump_shape (g_array_index (font->glyphs, SwfdecFontEntry, i).shape);
	g_print ("%s ", s);
	g_free (s);
      }
    }
    g_print ("\n");
  }
}

static void
dump_button (SwfdecButton *button)
{
  guint i;

  if (verbose) {
    for (i = 0; i < button->records->len; i++) {
      SwfdecButtonRecord *record = &g_array_index (button->records, SwfdecButtonRecord, i);

      g_print ("  %s %s %s %s  %s %d\n", 
	  record->states & SWFDEC_BUTTON_UP ? "U" : " ",
	  record->states & SWFDEC_BUTTON_OVER ? "O" : " ",
	  record->states & SWFDEC_BUTTON_DOWN ? "D" : " ",
	  record->states & SWFDEC_BUTTON_HIT ? "H" : " ",
	  G_OBJECT_TYPE_NAME (record->graphic), SWFDEC_CHARACTER (record->graphic)->id);
    }
  }
}

static void 
dump_objects (SwfdecSwfDecoder *s)
{
  GList *g;
  SwfdecCharacter *c;

  for (g = g_list_last (s->characters); g; g = g->prev) {
    c = g->data;
    g_print ("%d: %s\n", c->id, G_OBJECT_TYPE_NAME (c));
    if (verbose && SWFDEC_IS_GRAPHIC (c)) {
      SwfdecGraphic *graphic = SWFDEC_GRAPHIC (c);
      g_print ("  extents: %g %g  %g %g\n", graphic->extents.x0, graphic->extents.y0,
	  graphic->extents.x1, graphic->extents.y1);
    }
    if (SWFDEC_IS_SPRITE (c)) {
      dump_sprite (SWFDEC_SPRITE (c));
    }
    if (SWFDEC_IS_SHAPE(c)) {
      dump_shape(SWFDEC_SHAPE(c));
    }
    if (SWFDEC_IS_TEXT (c)) {
      dump_text (SWFDEC_TEXT (c));
    }
    if (SWFDEC_IS_EDIT_TEXT (c)) {
      dump_edit_text (SWFDEC_EDIT_TEXT (c));
    }
    if (SWFDEC_IS_FONT (c)) {
      dump_font (SWFDEC_FONT (c));
    }
    if (SWFDEC_IS_BUTTON (c)) {
      dump_button (SWFDEC_BUTTON (c));
    }
  }
}

int
main (int argc, char *argv[])
{
  char *fn = "it.swf";
  SwfdecSwfDecoder *s;
  SwfdecPlayer *player;
  GError *error = NULL;
  GOptionEntry options[] = {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "bew verbose", NULL },
    { NULL }
  };
  GOptionContext *ctx;

  ctx = g_option_context_new ("");
  g_option_context_add_main_entries (ctx, options, "options");
  g_option_context_parse (ctx, &argc, &argv, &error);
  g_option_context_free (ctx);
  if (error) {
    g_printerr ("Error parsing command line arguments: %s\n", error->message);
    g_error_free (error);
    return 1;
  }

  swfdec_init();

  if(argc>=2){
	fn = argv[1];
  }

  player = swfdec_player_new_from_file (argv[1], &error);
  if (player == NULL) {
    g_printerr ("Couldn't open file \"%s\": %s\n", argv[1], error->message);
    g_error_free (error);
    return 1;
  }
  s = (SwfdecSwfDecoder *) SWFDEC_ROOT_MOVIE (player->roots->data)->decoder;
  if (swfdec_player_get_rate (player) == 0 || 
      !SWFDEC_IS_SWF_DECODER (s)) {
    g_printerr ("File \"%s\" is not a SWF file\n", argv[1]);
    g_object_unref (player);
    player = NULL;
    return 1;
  }

  g_print ("file:\n");
  g_print ("  version: %d\n", s->version);
  g_print ("  rate   : %g fps\n",  SWFDEC_DECODER (s)->rate / 256.0);
  g_print ("  size   : %ux%u pixels\n", SWFDEC_DECODER (s)->width, SWFDEC_DECODER (s)->height);
  g_print ("objects:\n");
  dump_objects(s);

  g_print ("main sprite:\n");
  dump_sprite(s->main_sprite);
  g_object_unref (s);
  s = NULL;

  return 0;
}

