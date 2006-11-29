#include <gtk/gtk.h>
#include <math.h>
#include <libswfdec/swfdec.h>
#include "swfdec_widget.h"
#ifdef CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>

static void
save_to_svg (GtkWidget *button, SwfdecPlayer *player)
{
  GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save current frame as",
      GTK_WINDOW (gtk_widget_get_toplevel (button)), GTK_FILE_CHOOSER_ACTION_SAVE, 
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
    int w, h;
    cairo_t *cr;
    cairo_surface_t *surface;

    swfdec_player_get_image_size (player, &w, &h);
    if (w == 0 || h == 0) {
      return;
    }
    surface = cairo_svg_surface_create (
	gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)),
	w, h);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);
    swfdec_player_render (player, cr, NULL);
    cairo_show_page (cr);
    cairo_destroy (cr);
  }
  gtk_widget_destroy (dialog);
}
#endif /* CAIRO_HAS_SVG_SURFACE */

static void
iterate (gpointer dec)
{
  swfdec_player_iterate (dec);
}

static void
view_swf (SwfdecPlayer *dec, double scale, gboolean use_image)
{
  GtkWidget *window, *widget, *vbox;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  vbox = gtk_vbox_new (FALSE, 3);
  widget = swfdec_widget_new (dec);
  swfdec_widget_set_scale (SWFDEC_WIDGET (widget), scale);
  swfdec_widget_set_use_image (SWFDEC_WIDGET (widget), use_image);
  gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

  widget = gtk_button_new_with_mnemonic ("_Iterate");
  g_signal_connect_swapped (widget, "clicked", G_CALLBACK (iterate), dec);
  gtk_box_pack_end (GTK_BOX (vbox), widget, FALSE, TRUE, 0);

#ifdef CAIRO_HAS_SVG_SURFACE
  widget = gtk_button_new_with_mnemonic ("_Save current frame");
  g_signal_connect (widget, "clicked", G_CALLBACK (save_to_svg), dec);
  gtk_box_pack_end (GTK_BOX (vbox), widget, FALSE, TRUE, 0);
#endif

  gtk_container_add (GTK_CONTAINER (window), vbox);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_widget_show_all (window);

  gtk_main ();
}

int 
main (int argc, char *argv[])
{
  int ret = 100;
  double scale;
  SwfdecPlayer *player;
  GError *error = NULL;
  gboolean use_image = FALSE;

  GOptionEntry options[] = {
    { "scale", 's', 0, G_OPTION_ARG_INT, &ret, "scale factor", "PERCENT" },
    { "image", 'i', 0, G_OPTION_ARG_NONE, &use_image, "use an intermediate image surface for drawing", NULL },
    { NULL }
  };
  GOptionContext *ctx;

  ctx = g_option_context_new ("");
  g_option_context_add_main_entries (ctx, options, "options");
  g_option_context_add_group (ctx, gtk_get_option_group (TRUE));
  g_option_context_parse (ctx, &argc, &argv, &error);
  g_option_context_free (ctx);

  if (error) {
    g_printerr ("Error parsing command line arguments: %s\n", error->message);
    g_error_free (error);
    return 1;
  }

  scale = ret / 100.;
  swfdec_init ();

  if (argc < 2) {
    g_printerr ("Usage: %s [OPTIONS] filename\n", argv[0]);
    return 1;
  }

  player = swfdec_player_new_from_file (argv[1], &error);
  if (player == NULL) {
    g_printerr ("Couldn't open file \"%s\": %s\n", argv[1], error->message);
    g_error_free (error);
    return 1;
  }
  if (swfdec_player_get_rate (player) == 0) {
    g_printerr ("File \"%s\" is not a SWF file\n", argv[1]);
    g_object_unref (player);
    player = NULL;
    return 1;
  }

  view_swf (player, scale, use_image);

  player = NULL;
  return 0;
}

