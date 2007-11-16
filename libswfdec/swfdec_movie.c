/* Swfdec
 * Copyright (C) 2006-2007 Benjamin Otte <otte@gnome.org>
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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "swfdec_movie.h"
#include "swfdec_as_context.h"
#include "swfdec_as_strings.h"
#include "swfdec_button_movie.h"
#include "swfdec_debug.h"
#include "swfdec_draw.h"
#include "swfdec_event.h"
#include "swfdec_graphic.h"
#include "swfdec_image.h"
#include "swfdec_loader_internal.h"
#include "swfdec_player_internal.h"
#include "swfdec_sprite.h"
#include "swfdec_sprite_movie.h"
#include "swfdec_resource.h"
#include "swfdec_system.h"
#include "swfdec_utils.h"
#include "swfdec_load_object.h"
#include "swfdec_as_internal.h"

/*** MOVIE ***/

enum {
  PROP_0,
  PROP_DEPTH
};

G_DEFINE_ABSTRACT_TYPE (SwfdecMovie, swfdec_movie, SWFDEC_TYPE_AS_OBJECT)

static void
swfdec_movie_init (SwfdecMovie * movie)
{
  movie->xscale = 100;
  movie->yscale = 100;
  cairo_matrix_init_identity (&movie->original_transform);
  cairo_matrix_init_identity (&movie->matrix);
  cairo_matrix_init_identity (&movie->inverse_matrix);

  swfdec_color_transform_init_identity (&movie->color_transform);
  swfdec_color_transform_init_identity (&movie->original_ctrans);

  movie->visible = TRUE;
  movie->cache_state = SWFDEC_MOVIE_INVALID_CONTENTS;

  swfdec_rect_init_empty (&movie->extents);
}

/**
 * swfdec_movie_invalidate:
 * @movie: movie to invalidate
 *
 * Invalidates the area currently occupied by movie. If the area this movie
 * occupies has changed, call swfdec_movie_queue_update () instead.
 **/
void
swfdec_movie_invalidate (SwfdecMovie *movie)
{
  SwfdecRect rect = movie->extents;

  SWFDEC_LOG ("%s invalidating %g %g  %g %g", movie->name, 
      rect.x0, rect.y0, rect.x1, rect.y1);
  if (swfdec_rect_is_empty (&rect))
    return;
  while (movie->parent) {
    movie = movie->parent;
    if (movie->cache_state > SWFDEC_MOVIE_INVALID_EXTENTS)
      return;
    swfdec_rect_transform (&rect, &rect, &movie->matrix);
  }
  swfdec_player_invalidate (SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context), &rect);
}

/**
 * swfdec_movie_queue_update:
 * @movie: a #SwfdecMovie
 * @state: how much needs to be updated
 *
 * Queues an update of all cached values inside @movie and invalidates it.
 **/
void
swfdec_movie_queue_update (SwfdecMovie *movie, SwfdecMovieCacheState state)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));

  if (movie->cache_state < SWFDEC_MOVIE_INVALID_EXTENTS &&
      state >= SWFDEC_MOVIE_INVALID_EXTENTS)
    swfdec_movie_invalidate (movie);
  while (movie && movie->cache_state < state) {
    movie->cache_state = state;
    movie = movie->parent;
    state = SWFDEC_MOVIE_INVALID_CHILDREN;
  }
}

static void
swfdec_movie_update_extents (SwfdecMovie *movie)
{
  SwfdecMovieClass *klass;
  GList *walk;
  SwfdecRect *rect = &movie->original_extents;
  SwfdecRect *extents = &movie->extents;

  *rect = movie->draw_extents;
  if (movie->image) {
    SwfdecRect image_extents = { 0, 0, 
      movie->image->width * SWFDEC_TWIPS_SCALE_FACTOR,
      movie->image->height * SWFDEC_TWIPS_SCALE_FACTOR };
    swfdec_rect_union (rect, rect, &image_extents);
  }
  for (walk = movie->list; walk; walk = walk->next) {
    swfdec_rect_union (rect, rect, &SWFDEC_MOVIE (walk->data)->extents);
  }
  klass = SWFDEC_MOVIE_GET_CLASS (movie);
  if (klass->update_extents)
    klass->update_extents (movie, rect);
  if (swfdec_rect_is_empty (rect)) {
    *extents = *rect;
    return;
  }
  swfdec_rect_transform (extents, rect, &movie->matrix);
  if (movie->parent && movie->parent->cache_state < SWFDEC_MOVIE_INVALID_EXTENTS) {
    /* no need to invalidate here */
    movie->parent->cache_state = SWFDEC_MOVIE_INVALID_EXTENTS;
  }
}

static void
swfdec_movie_update_matrix (SwfdecMovie *movie)
{
  double d, e;

  /* we operate on x0 and y0 when setting movie._x and movie._y */
  if (movie->modified) {
    movie->matrix.xx = movie->original_transform.xx;
    movie->matrix.yx = movie->original_transform.yx;
    movie->matrix.xy = movie->original_transform.xy;
    movie->matrix.yy = movie->original_transform.yy;
  } else {
    movie->matrix = movie->original_transform;
  }

  d = movie->xscale / swfdec_matrix_get_xscale (&movie->original_transform);
  e = movie->yscale / swfdec_matrix_get_yscale (&movie->original_transform);
  cairo_matrix_scale (&movie->matrix, d, e);
  if (isfinite (movie->rotation)) {
    d = movie->rotation - swfdec_matrix_get_rotation (&movie->original_transform);
    cairo_matrix_rotate (&movie->matrix, d * G_PI / 180);
  }
  swfdec_matrix_ensure_invertible (&movie->matrix, &movie->inverse_matrix);
}

static void
swfdec_movie_do_update (SwfdecMovie *movie)
{
  GList *walk;

  for (walk = movie->list; walk; walk = walk->next) {
    SwfdecMovie *child = walk->data;

    if (child->cache_state != SWFDEC_MOVIE_UP_TO_DATE)
      swfdec_movie_do_update (child);
  }

  switch (movie->cache_state) {
    case SWFDEC_MOVIE_INVALID_MATRIX:
      swfdec_movie_update_matrix (movie);
      /* fall through */
    case SWFDEC_MOVIE_INVALID_CONTENTS:
      swfdec_movie_update_extents (movie);
      swfdec_movie_invalidate (movie);
      break;
    case SWFDEC_MOVIE_INVALID_EXTENTS:
      swfdec_movie_update_extents (movie);
      break;
    case SWFDEC_MOVIE_INVALID_CHILDREN:
      break;
    case SWFDEC_MOVIE_UP_TO_DATE:
    default:
      g_assert_not_reached ();
  }
  movie->cache_state = SWFDEC_MOVIE_UP_TO_DATE;
}

/**
 * swfdec_movie_update:
 * @movie: a #SwfdecMovie
 *
 * Brings the cached values of @movie up-to-date if they are not. This includes
 * transformation matrices and extents. It needs to be called before accessing
 * the relevant values.
 **/
void
swfdec_movie_update (SwfdecMovie *movie)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));

  if (movie->cache_state == SWFDEC_MOVIE_UP_TO_DATE)
    return;

  if (movie->parent && movie->parent->cache_state != SWFDEC_MOVIE_UP_TO_DATE) {
    swfdec_movie_update (movie->parent);
  } else {
    swfdec_movie_do_update (movie);
  }
}

SwfdecMovie *
swfdec_movie_find (SwfdecMovie *movie, int depth)
{
  GList *walk;

  g_return_val_if_fail (SWFDEC_IS_MOVIE (movie), NULL);

  for (walk = movie->list; walk; walk = walk->next) {
    SwfdecMovie *cur= walk->data;

    if (cur->depth < depth)
      continue;
    if (cur->depth == depth)
      return cur;
    break;
  }
  return NULL;
}

static gboolean
swfdec_movie_do_remove (SwfdecMovie *movie)
{
  SwfdecPlayer *player;

  SWFDEC_LOG ("removing %s %s", G_OBJECT_TYPE_NAME (movie), movie->name);

  player = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context);
  movie->will_be_removed = TRUE;
  while (movie->list) {
    GList *walk = movie->list;
    while (walk && SWFDEC_MOVIE (walk->data)->will_be_removed)
      walk = walk->next;
    if (walk == NULL)
      break;
    swfdec_movie_remove (walk->data);
  }
  /* FIXME: all of this here or in destroy callback? */
  if (player->mouse_grab == movie)
    player->mouse_grab = NULL;
  if (player->mouse_drag == movie)
    player->mouse_drag = NULL;
  swfdec_movie_invalidate (movie);
  swfdec_movie_set_depth (movie, -32769 - movie->depth); /* don't ask me why... */

  return !swfdec_movie_queue_script (movie, SWFDEC_EVENT_UNLOAD);
}

/**
 * swfdec_movie_remove:
 * @movie: #SwfdecMovie to remove
 *
 * Removes this movie from its parent. In contrast to swfdec_movie_destroy (),
 * it will definitely cause a removal from the display list, but depending on
 * movie, it might still be possible to reference it from Actionscript.
 **/
void
swfdec_movie_remove (SwfdecMovie *movie)
{
  gboolean result;

  g_return_if_fail (SWFDEC_IS_MOVIE (movie));

  if (movie->state > SWFDEC_MOVIE_STATE_RUNNING)
    return;
  result = swfdec_movie_do_remove (movie);
  movie->state = SWFDEC_MOVIE_STATE_REMOVED;
  if (result)
    swfdec_movie_destroy (movie);
}

/**
 * swfdec_movie_destroy:
 * @movie: #SwfdecMovie to destroy
 *
 * Removes this movie from its parent. After this it will no longer be present,
 * neither visually nor via ActionScript. This function will not cause an 
 * unload event. Compare with swfdec_movie_destroy ().
 **/
void
swfdec_movie_destroy (SwfdecMovie *movie)
{
  SwfdecMovieClass *klass = SWFDEC_MOVIE_GET_CLASS (movie);
  SwfdecPlayer *player = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context);

  g_assert (movie->state < SWFDEC_MOVIE_STATE_DESTROYED);
  if (movie->state < SWFDEC_MOVIE_STATE_REMOVED) {
    swfdec_movie_do_remove (movie);
  }
  SWFDEC_LOG ("destroying movie %s", movie->name);
  while (movie->list) {
    swfdec_movie_destroy (movie->list->data);
  }
  if (movie->parent) {
    movie->parent->list = g_list_remove (movie->parent->list, movie);
  } else {
    player->roots = g_list_remove (player->roots, movie);
  }
  /* FIXME: figure out how to handle destruction pre-init/construct.
   * This is just a stop-gap measure to avoid dead movies in those queues */
  swfdec_player_remove_all_actions (player, movie);
  if (klass->finish_movie)
    klass->finish_movie (movie);
  player->movies = g_list_remove (player->movies, movie);
  movie->state = SWFDEC_MOVIE_STATE_DESTROYED;
  /* unset prototype here, so we don't work in AS anymore */
  SWFDEC_AS_OBJECT (movie)->prototype = NULL;
  g_object_unref (movie);
}

static void
swfdec_movie_set_constructor (SwfdecSpriteMovie *movie)
{
  SwfdecMovie *mov = SWFDEC_MOVIE (movie);
  SwfdecAsContext *context = SWFDEC_AS_OBJECT (movie)->context;
  SwfdecAsObject *constructor = NULL;

  g_assert (mov->resource != NULL);

  if (movie->sprite) {
    const char *name;

    name = swfdec_resource_get_export_name (mov->resource,
	SWFDEC_CHARACTER (movie->sprite));
    if (name != NULL) {
      name = swfdec_as_context_get_string (context, name);
      constructor = swfdec_player_get_export_class (SWFDEC_PLAYER (context),
	  name);
    }
  }
  if (constructor == NULL)
    constructor = SWFDEC_PLAYER (context)->MovieClip;

  swfdec_as_object_set_constructor (SWFDEC_AS_OBJECT (movie), constructor);
}

/**
 * swfdec_movie_resolve:
 * @movie: movie to resolve
 *
 * Resolves a movie clip to its real version. Since movie clips can be 
 * explicitly destroyed, they have problems with references to them. In the
 * case of destruction, these references will remain as "dangling pointers".
 * However, if a movie with the same name is later created again, the reference
 * will point to that movie. This function does this resolving.
 *
 * Returns: The movie clip @movie resolves to or %NULL if none.
 **/
SwfdecMovie *
swfdec_movie_resolve (SwfdecMovie *movie)
{
  g_return_val_if_fail (SWFDEC_IS_MOVIE (movie), NULL);

  if (movie->state != SWFDEC_MOVIE_STATE_DESTROYED)
    return movie;
  if (movie->parent == NULL) {
    SWFDEC_FIXME ("figure out how to resolve root movies");
    return NULL;
  }
  /* FIXME: include unnamed ones? */
  return swfdec_movie_get_by_name (movie->parent, movie->original_name, FALSE);
}

guint
swfdec_movie_get_version (SwfdecMovie *movie)
{
  return movie->resource->version;
}

void
swfdec_movie_execute (SwfdecMovie *movie, SwfdecEventType condition)
{
  const char *name;

  g_return_if_fail (SWFDEC_IS_MOVIE (movie));

  /* special cases */
  if (condition == SWFDEC_EVENT_CONSTRUCT) {
    if (swfdec_movie_get_version (movie) <= 5)
      return;
    swfdec_movie_set_constructor (SWFDEC_SPRITE_MOVIE (movie));
  } else if (condition == SWFDEC_EVENT_ENTER) {
    if (movie->will_be_removed)
      return;
  }

  if (movie->events) {
    swfdec_event_list_execute (movie->events, SWFDEC_AS_OBJECT (movie), 
	SWFDEC_SECURITY (movie->resource), condition, 0);
  }
  /* FIXME: how do we compute the version correctly here? */
  if (swfdec_movie_get_version (movie) <= 5)
    return;
  name = swfdec_event_type_get_name (condition);
  if (name != NULL) {
    swfdec_as_object_call_with_security (SWFDEC_AS_OBJECT (movie), 
	SWFDEC_SECURITY (movie->resource), name, 0, NULL, NULL);
  }
  if (condition == SWFDEC_EVENT_CONSTRUCT)
    swfdec_as_object_call (SWFDEC_AS_OBJECT (movie), SWFDEC_AS_STR_constructor, 0, NULL, NULL);
}

/**
 * swfdec_movie_queue_script:
 * @movie: a #SwfdecMovie
 * @condition: the event that should happen
 *
 * Queues execution of all scripts associated with the given event.
 *
 * Returns: TRUE if there were any such events
 **/
gboolean
swfdec_movie_queue_script (SwfdecMovie *movie, SwfdecEventType condition)
{
  SwfdecPlayer *player;
  gboolean ret = FALSE;
  guint importance;
  
  g_return_val_if_fail (SWFDEC_IS_MOVIE (movie), FALSE);

  if (!SWFDEC_IS_SPRITE_MOVIE (movie))
    return FALSE;

  switch (condition) {
    case SWFDEC_EVENT_INITIALIZE:
      importance = 0;
      break;
    case SWFDEC_EVENT_CONSTRUCT:
      importance = 1;
      break;
    case SWFDEC_EVENT_LOAD:
    case SWFDEC_EVENT_ENTER:
    case SWFDEC_EVENT_UNLOAD:
    case SWFDEC_EVENT_MOUSE_MOVE:
    case SWFDEC_EVENT_MOUSE_DOWN:
    case SWFDEC_EVENT_MOUSE_UP:
    case SWFDEC_EVENT_KEY_UP:
    case SWFDEC_EVENT_KEY_DOWN:
    case SWFDEC_EVENT_DATA:
    case SWFDEC_EVENT_PRESS:
    case SWFDEC_EVENT_RELEASE:
    case SWFDEC_EVENT_RELEASE_OUTSIDE:
    case SWFDEC_EVENT_ROLL_OVER:
    case SWFDEC_EVENT_ROLL_OUT:
    case SWFDEC_EVENT_DRAG_OVER:
    case SWFDEC_EVENT_DRAG_OUT:
    case SWFDEC_EVENT_KEY_PRESS:
      importance = 2;
      break;
    default:
      g_return_val_if_reached (FALSE);
  }

  if (movie->events &&
      swfdec_event_list_has_conditions (movie->events, 
	  SWFDEC_AS_OBJECT (movie), condition, 0)) {
      ret = TRUE;
  } else {
    const char *name = swfdec_event_type_get_name (condition);
    if (name != NULL &&
	swfdec_as_object_has_function (SWFDEC_AS_OBJECT (movie), name))
      ret = TRUE;
  }

  player = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context);
  swfdec_player_add_action (player, movie, condition, importance);
  return ret;
}

/* NB: coordinates are in movie's coordiante system. Use swfdec_movie_get_mouse
 * if you have global coordinates */
gboolean
swfdec_movie_mouse_in (SwfdecMovie *movie, double x, double y)
{
  SwfdecMovieClass *klass;
  GList *walk;

  klass = SWFDEC_MOVIE_GET_CLASS (movie);
  if (klass->mouse_in != NULL &&
      klass->mouse_in (movie, x, y))
    return TRUE;

  for (walk = movie->list; walk; walk = walk->next) {
    double tmp_x = x;
    double tmp_y = y;
    SwfdecMovie *cur = walk->data;
    cairo_matrix_transform_point (&cur->inverse_matrix, &tmp_x, &tmp_y);
    if (swfdec_movie_mouse_in (cur, tmp_x, tmp_y))
      return TRUE;
  }
  return FALSE;
}

void
swfdec_movie_local_to_global (SwfdecMovie *movie, double *x, double *y)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (x != NULL);
  g_return_if_fail (y != NULL);

  do {
    cairo_matrix_transform_point (&movie->matrix, x, y);
  } while ((movie = movie->parent));
}

void
swfdec_movie_rect_local_to_global (SwfdecMovie *movie, SwfdecRect *rect)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (rect != NULL);

  swfdec_movie_local_to_global (movie, &rect->x0, &rect->y0);
  swfdec_movie_local_to_global (movie, &rect->x1, &rect->y1);
  if (rect->x0 > rect->x1) {
    double tmp = rect->x1;
    rect->x1 = rect->x0;
    rect->x0 = tmp;
  }
  if (rect->y0 > rect->y1) {
    double tmp = rect->y1;
    rect->y1 = rect->y0;
    rect->y0 = tmp;
  }
}

void
swfdec_movie_global_to_local (SwfdecMovie *movie, double *x, double *y)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (x != NULL);
  g_return_if_fail (y != NULL);

  if (movie->parent) {
    swfdec_movie_global_to_local (movie->parent, x, y);
  }
  if (movie->cache_state >= SWFDEC_MOVIE_INVALID_MATRIX)
    swfdec_movie_update (movie);
  cairo_matrix_transform_point (&movie->inverse_matrix, x, y);
}

void
swfdec_movie_rect_global_to_local (SwfdecMovie *movie, SwfdecRect *rect)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (rect != NULL);

  swfdec_movie_global_to_local (movie, &rect->x0, &rect->y0);
  swfdec_movie_global_to_local (movie, &rect->x1, &rect->y1);
  if (rect->x0 > rect->x1) {
    double tmp = rect->x1;
    rect->x1 = rect->x0;
    rect->x0 = tmp;
  }
  if (rect->y0 > rect->y1) {
    double tmp = rect->y1;
    rect->y1 = rect->y0;
    rect->y0 = tmp;
  }
}

/**
 * swfdec_movie_get_mouse:
 * @movie: a #SwfdecMovie
 * @x: pointer to hold result of X coordinate
 * @y: pointer to hold result of y coordinate
 *
 * Gets the mouse coordinates in the coordinate space of @movie.
 **/
void
swfdec_movie_get_mouse (SwfdecMovie *movie, double *x, double *y)
{
  SwfdecPlayer *player;

  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (x != NULL);
  g_return_if_fail (y != NULL);

  player = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context);
  *x = player->mouse_x;
  *y = player->mouse_y;
  swfdec_player_stage_to_global (player, x, y);
  swfdec_movie_global_to_local (movie, x, y);
}

void
swfdec_movie_send_mouse_change (SwfdecMovie *movie, gboolean release)
{
  double x, y;
  gboolean mouse_in;
  int button;
  SwfdecMovieClass *klass;

  swfdec_movie_get_mouse (movie, &x, &y);
  if (release) {
    mouse_in = FALSE;
    button = 0;
  } else {
    mouse_in = swfdec_movie_mouse_in (movie, x, y);
    button = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context)->mouse_button;
  }
  klass = SWFDEC_MOVIE_GET_CLASS (movie);
  g_assert (klass->mouse_change != NULL);
  klass->mouse_change (movie, x, y, mouse_in, button);
}

/**
 * swfdec_movie_get_movie_at:
 * @movie: a #SwfdecMovie
 * @x: x coordinate in parent's coordinate space
 * @y: y coordinate in the parent's coordinate space
 *
 * Gets the child at the given coordinates. The coordinates are in the 
 * coordinate system of @movie's parent (or the global coordinate system for
 * root movies).
 *
 * Returns: the child of @movie at the given coordinates or %NULL if none
 **/
SwfdecMovie *
swfdec_movie_get_movie_at (SwfdecMovie *movie, double x, double y)
{
  GList *walk, *clip_walk;
  int clip_depth = 0;
  SwfdecMovie *ret;
  SwfdecMovieClass *klass;

  SWFDEC_LOG ("%s %p getting mouse at: %g %g", G_OBJECT_TYPE_NAME (movie), movie, x, y);
  if (!swfdec_rect_contains (&movie->extents, x, y)) {
    return NULL;
  }
  cairo_matrix_transform_point (&movie->inverse_matrix, &x, &y);

  /* first check if the movie can handle mouse events, and if it can,
   * ignore its children.
   * Dunno if that's correct */
  klass = SWFDEC_MOVIE_GET_CLASS (movie);
  if (klass->mouse_change) {
    if (swfdec_movie_mouse_in (movie, x, y))
      return movie;
    else
      return NULL;
  }
  for (walk = clip_walk = g_list_last (movie->list); walk; walk = walk->prev) {
    SwfdecMovie *child = walk->data;
    if (walk == clip_walk) {
      clip_depth = 0;
      for (clip_walk = clip_walk->prev; clip_walk; clip_walk = clip_walk->prev) {
	SwfdecMovie *clip = walk->data;
	if (clip->clip_depth) {
	  double tmpx = x, tmpy = y;
	  cairo_matrix_transform_point (&clip->inverse_matrix, &tmpx, &tmpy);
	  if (!swfdec_movie_mouse_in (clip, tmpx, tmpy)) {
	    SWFDEC_LOG ("skipping depth %d to %d due to clipping", clip->depth, clip->clip_depth);
	    clip_depth = child->clip_depth;
	  }
	  break;
	}
      }
    }
    if (child->clip_depth) {
      SWFDEC_LOG ("resetting clip depth");
      clip_depth = 0;
      continue;
    }
    if (child->depth <= clip_depth && clip_depth) {
      SWFDEC_DEBUG ("ignoring depth=%d, it's clipped (clip_depth %d)", child->depth, clip_depth);
      continue;
    }
    if (!child->visible) {
      SWFDEC_LOG ("child %s %s (depth %d) is invisible, ignoring", G_OBJECT_TYPE_NAME (movie), movie->name, movie->depth);
      continue;
    }

    ret = swfdec_movie_get_movie_at (child, x, y);
    if (ret)
      return ret;
  }
  return NULL;
}

static gboolean
swfdec_movie_needs_group (SwfdecMovie *movie)
{
  return (movie->blend_mode > 1);
}

static cairo_operator_t
swfdec_movie_get_operator_for_blend_mode (guint blend_mode)
{
  switch (blend_mode) {
    case 0:
    case 1:
      SWFDEC_ERROR ("shouldn't need to get operator without blend mode?!");
    case 2:
      return CAIRO_OPERATOR_OVER;
    case 8:
      return CAIRO_OPERATOR_ADD;
    case 11:
      return CAIRO_OPERATOR_DEST_IN;
    case 12:
      return CAIRO_OPERATOR_DEST_OUT;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 9:
    case 10:
    case 13:
    case 14:
      SWFDEC_WARNING ("blend mode %u unimplemented in cairo", blend_mode);
      return CAIRO_OPERATOR_OVER;
    default:
      SWFDEC_WARNING ("invalid blend mode %u", blend_mode);
      return CAIRO_OPERATOR_OVER;
  }
}

/* NB: Since there is no way to union paths in cairo, we use masks 
 * instead. To create the mask, we force black rendering using the color 
 * transform and then do the usual rendering.
 * Using a mask will of course cause artifacts on non pixel-aligned 
 * boundaries, but without the help of cairo, there is no way to avoid 
 * this. */ 
static cairo_pattern_t *
swfdec_movie_push_clip (cairo_t *cr, SwfdecMovie *clip_movie, 
    const SwfdecRect *inval)
{
  SwfdecColorTransform black;
  cairo_pattern_t *mask;

  swfdec_color_transform_init_color (&black, SWFDEC_COLOR_COMBINE (0, 0, 0, 255));
  cairo_push_group_with_content (cr, CAIRO_CONTENT_ALPHA);
  swfdec_movie_render (clip_movie, cr, &black, inval);
  mask = cairo_pop_group (cr);
  cairo_push_group (cr);

  return mask;
}

static cairo_pattern_t *
swfdec_movie_pop_clip (cairo_t *cr, cairo_pattern_t *mask)
{
  cairo_pop_group_to_source (cr);
  cairo_mask (cr, mask);
  cairo_pattern_destroy (mask);
  return NULL;
}

void
swfdec_movie_render (SwfdecMovie *movie, cairo_t *cr,
    const SwfdecColorTransform *color_transform, const SwfdecRect *inval)
{
  SwfdecMovieClass *klass;
  GList *g;
  GSList *walk;
  int clip_depth = 0;
  SwfdecColorTransform trans;
  SwfdecRect rect;
  gboolean group;
  cairo_pattern_t *mask = NULL;

  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (cr != NULL);
  if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) {
    g_warning ("%s", cairo_status_to_string (cairo_status (cr)));
  }
  g_return_if_fail (color_transform != NULL);
  g_return_if_fail (inval != NULL);
  
  if (!swfdec_rect_intersect (NULL, &movie->extents, inval)) {
    SWFDEC_LOG ("not rendering %s %s, extents %g %g  %g %g are not in invalid area %g %g  %g %g",
	G_OBJECT_TYPE_NAME (movie), movie->name, 
	movie->extents.x0, movie->extents.y0, movie->extents.x1, movie->extents.y1,
	inval->x0, inval->y0, inval->x1, inval->y1);
    return;
  }
  if (!movie->visible) {
    SWFDEC_LOG ("not rendering %s %p, movie is invisible",
	G_OBJECT_TYPE_NAME (movie), movie->name);
    return;
  }

  cairo_save (cr);
  group = swfdec_movie_needs_group (movie);
  if (group) {
    SWFDEC_DEBUG ("pushing group for blend mode %u", movie->blend_mode);
    cairo_push_group (cr);
  }

  SWFDEC_LOG ("transforming movie, transform: %g %g  %g %g   %g %g",
      movie->matrix.xx, movie->matrix.yy,
      movie->matrix.xy, movie->matrix.yx,
      movie->matrix.x0, movie->matrix.y0);
  cairo_transform (cr, &movie->matrix);
  swfdec_rect_transform (&rect, inval, &movie->inverse_matrix);
  SWFDEC_LOG ("%sinvalid area is now: %g %g  %g %g",  movie->parent ? "  " : "",
      rect.x0, rect.y0, rect.x1, rect.y1);
  swfdec_color_transform_chain (&trans, &movie->original_ctrans, color_transform);
  swfdec_color_transform_chain (&trans, &movie->color_transform, &trans);

  /* exeute the movie's drawing commands */
  for (walk = movie->draws; walk; walk = walk->next) {
    SwfdecDraw *draw = walk->data;

    if (!swfdec_rect_intersect (NULL, &draw->extents, &rect))
      continue;
    
    swfdec_draw_paint (draw, cr, &trans);
  }

  /* if the movie loaded an image, draw it here now */
  if (movie->image) {
    cairo_surface_t *surface = swfdec_image_create_surface_transformed (movie->image,
	&trans);
    if (surface) {
      static const cairo_matrix_t matrix = { 1.0 / SWFDEC_TWIPS_SCALE_FACTOR, 0, 0, 1.0 / SWFDEC_TWIPS_SCALE_FACTOR, 0, 0 };
      cairo_pattern_t *pattern = cairo_pattern_create_for_surface (surface);
      SWFDEC_LOG ("rendering loaded image");
      cairo_pattern_set_matrix (pattern, &matrix);
      cairo_set_source (cr, pattern);
      cairo_paint (cr);
      cairo_pattern_destroy (pattern);
      cairo_surface_destroy (surface);
    }
  }

  /* draw the children movies */
  for (g = movie->list; g; g = g_list_next (g)) {
    SwfdecMovie *child = g->data;

    if (clip_depth && child->depth > clip_depth) {
      SWFDEC_INFO ("unsetting clip depth %d for depth %d", clip_depth, child->depth);
      clip_depth = 0;
      mask = swfdec_movie_pop_clip (cr, mask);
    }

    if (child->clip_depth) {
      if (clip_depth) {
	/* FIXME: is clipping additive? */
	SWFDEC_FIXME ("unsetting clip depth %d for new clip depth %d", clip_depth,
	    child->clip_depth);
	mask = swfdec_movie_pop_clip (cr, mask);
      }
      SWFDEC_INFO ("clipping up to depth %d by using %p with depth %d", child->clip_depth,
	  child, child->depth);
      clip_depth = child->clip_depth;
      mask = swfdec_movie_push_clip (cr, child, &rect);
      continue;
    }

    SWFDEC_LOG ("rendering %p with depth %d", child, child->depth);
    swfdec_movie_render (child, cr, &trans, &rect);
  }
  if (clip_depth) {
    SWFDEC_INFO ("unsetting clip depth %d after rendering", clip_depth);
    clip_depth = 0;
    mask = swfdec_movie_pop_clip (cr, mask);
  }
  g_assert (mask == NULL);
  klass = SWFDEC_MOVIE_GET_CLASS (movie);
  if (klass->render)
    klass->render (movie, cr, &trans, &rect);
#if 0
  /* code to draw a red rectangle around the area occupied by this movie clip */
  {
    double x = 1.0, y = 0.0;
    cairo_transform (cr, &movie->inverse_transform);
    cairo_user_to_device_distance (cr, &x, &y);
    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_set_line_width (cr, 1 / sqrt (x * x + y * y));
    cairo_rectangle (cr, object->extents.x0 + 10, object->extents.y0 + 10,
	object->extents.x1 - object->extents.x0 - 20,
	object->extents.y1 - object->extents.y0 - 20);
    cairo_stroke (cr);
  }
#endif
  if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) {
    g_warning ("error rendering with cairo: %s", cairo_status_to_string (cairo_status (cr)));
  }
  if (group) {
    cairo_pattern_t *pattern;

    pattern = cairo_pop_group (cr);
    cairo_set_source (cr, pattern);
    cairo_set_operator (cr, swfdec_movie_get_operator_for_blend_mode (movie->blend_mode));
    cairo_paint (cr);
    cairo_pattern_destroy (pattern);
  }
  cairo_restore (cr);
}

static void
swfdec_movie_get_property (GObject *object, guint param_id, GValue *value, 
    GParamSpec * pspec)
{
  SwfdecMovie *movie = SWFDEC_MOVIE (object);

  switch (param_id) {
    case PROP_DEPTH:
      g_value_set_int (value, movie->depth);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
swfdec_movie_set_property (GObject *object, guint param_id, const GValue *value, 
    GParamSpec * pspec)
{
  SwfdecMovie *movie = SWFDEC_MOVIE (object);

  switch (param_id) {
    case PROP_DEPTH:
      movie->depth = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
swfdec_movie_dispose (GObject *object)
{
  SwfdecMovie * movie = SWFDEC_MOVIE (object);
  GSList *iter;

  g_assert (movie->list == NULL);

  SWFDEC_LOG ("disposing movie %s (depth %d)", movie->name, movie->depth);
  if (movie->resource) {
    g_object_unref (movie->resource);
    movie->resource = NULL;
  }
  if (movie->events) {
    swfdec_event_list_free (movie->events);
    movie->events = NULL;
  }
  if (movie->graphic) {
    g_object_unref (movie->graphic);
    movie->graphic = NULL;
  }
  for (iter = movie->variable_listeners; iter != NULL; iter = iter->next) {
    g_free (iter->data);
  }
  g_slist_free (movie->variable_listeners);
  movie->variable_listeners = NULL;

  if (movie->image) {
    g_object_unref (movie->image);
    movie->image = NULL;
  }
  g_slist_foreach (movie->draws, (GFunc) g_object_unref, NULL);
  g_slist_free (movie->draws);
  movie->draws = NULL;

  G_OBJECT_CLASS (swfdec_movie_parent_class)->dispose (G_OBJECT (movie));
}

static void
swfdec_movie_mark (SwfdecAsObject *object)
{
  SwfdecMovie *movie = SWFDEC_MOVIE (object);
  GList *walk;
  GSList *iter;

  if (movie->parent)
    swfdec_as_object_mark (SWFDEC_AS_OBJECT (movie->parent));
  swfdec_as_string_mark (movie->original_name);
  swfdec_as_string_mark (movie->name);
  for (walk = movie->list; walk; walk = walk->next) {
    swfdec_as_object_mark (walk->data);
  }
  for (iter = movie->variable_listeners; iter != NULL; iter = iter->next) {
    SwfdecMovieVariableListener *listener = iter->data;
    swfdec_as_object_mark (listener->object);
    swfdec_as_string_mark (listener->name);
  }
  swfdec_resource_mark (movie->resource);

  SWFDEC_AS_OBJECT_CLASS (swfdec_movie_parent_class)->mark (object);
}

/* FIXME: This function can definitely be implemented easier */
SwfdecMovie *
swfdec_movie_get_by_name (SwfdecMovie *movie, const char *name, gboolean unnamed)
{
  GList *walk;
  int i;
  gulong l;
  guint version = SWFDEC_AS_OBJECT (movie)->context->version;
  char *end;
  SwfdecPlayer *player = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context);

  if ((version >= 7 && g_str_has_prefix (name, "_level")) ||
      (version < 7 && strncasecmp (name, "_level", 6) == 0)) {
    errno = 0;
    l = strtoul (name + 6, &end, 10);
    if (errno != 0 || *end != 0 || l > G_MAXINT)
      return NULL;
    i = l - 16384;
    for (walk = player->roots; walk; walk = walk->next) {
      SwfdecMovie *cur = walk->data;
      if (cur->depth < i)
	continue;
      if (cur->depth == i)
	return cur;
      break;
    }
  }

  for (walk = movie->list; walk; walk = walk->next) {
    SwfdecMovie *cur = walk->data;
    if (cur->original_name == SWFDEC_AS_STR_EMPTY && !unnamed)
      continue;
    if (swfdec_strcmp (version, cur->name, name) == 0)
      return cur;
  }
  return NULL;
}

SwfdecMovie *
swfdec_movie_get_root (SwfdecMovie *movie)
{
  g_return_val_if_fail (SWFDEC_IS_MOVIE (movie), NULL);

  while (movie->parent)
    movie = movie->parent;

  return movie;
}

static gboolean
swfdec_movie_get_variable (SwfdecAsObject *object, SwfdecAsObject *orig,
    const char *variable, SwfdecAsValue *val, guint *flags)
{
  SwfdecMovie *movie = SWFDEC_MOVIE (object);

  movie = swfdec_movie_resolve (movie);
  if (movie == NULL)
    return FALSE;

  if (SWFDEC_AS_OBJECT_CLASS (swfdec_movie_parent_class)->get (object, orig, variable, val, flags))
    return TRUE;

  if (swfdec_movie_get_asprop (movie, variable, val)) {
    *flags = 0;
    return TRUE;
  }

  /* FIXME: check that this is correct */
  if (object->context->version > 5 && variable == SWFDEC_AS_STR__global) {
    SWFDEC_AS_VALUE_SET_OBJECT (val, object->context->global);
    *flags = 0;
    return TRUE;
  }
  if (movie->parent == NULL && variable == SWFDEC_AS_STR__version) {
    SWFDEC_AS_VALUE_SET_STRING (val, swfdec_as_context_get_string (object->context,
	  SWFDEC_PLAYER (object->context)->system->version));
    *flags = 0;
    return TRUE;
  }
  
  movie = swfdec_movie_get_by_name (movie, variable, FALSE);
  if (movie) {
    SWFDEC_AS_VALUE_SET_OBJECT (val, SWFDEC_AS_OBJECT (movie));
    *flags = 0;
    return TRUE;
  }
  return FALSE;
}

void
swfdec_movie_add_variable_listener (SwfdecMovie *movie, SwfdecAsObject *object,
    const char *name, const SwfdecMovieVariableListenerFunction function)
{
  SwfdecMovieVariableListener *listener;
  GSList *iter;

  for (iter = movie->variable_listeners; iter != NULL; iter = iter->next) {
    listener = iter->data;

    if (listener->object == object && listener->name == name &&
	listener->function == function)
      break;
  }
  if (iter != NULL)
    return;

  listener = g_new0 (SwfdecMovieVariableListener, 1);
  listener->object = object;
  listener->name = name;
  listener->function = function;

  movie->variable_listeners = g_slist_prepend (movie->variable_listeners,
      listener);
}

void
swfdec_movie_remove_variable_listener (SwfdecMovie *movie,
    SwfdecAsObject *object, const char *name,
    const SwfdecMovieVariableListenerFunction function)
{
  GSList *iter;

  for (iter = movie->variable_listeners; iter != NULL; iter = iter->next) {
    SwfdecMovieVariableListener *listener = iter->data;

    if (listener->object == object && listener->name == name &&
	listener->function == function)
      break;
  }
  if (iter == NULL)
    return;

  g_free (iter->data);
  movie->variable_listeners =
    g_slist_remove (movie->variable_listeners, iter->data);
}

static void
swfdec_movie_call_variable_listeners (SwfdecMovie *movie, const char *name,
    const SwfdecAsValue *val)
{
  GSList *iter;

  for (iter = movie->variable_listeners; iter != NULL; iter = iter->next) {
    SwfdecMovieVariableListener *listener = iter->data;

    if (listener->name != name &&
	(SWFDEC_AS_OBJECT (movie)->context->version >= 7 ||
	 !swfdec_str_case_equal (listener->name, name)))
      continue;

    listener->function (listener->object, name, val);
  }
}

static void
swfdec_movie_set_variable (SwfdecAsObject *object, const char *variable, 
    const SwfdecAsValue *val, guint flags)
{
  SwfdecMovie *movie = SWFDEC_MOVIE (object);

  movie = swfdec_movie_resolve (movie);
  if (movie == NULL)
    return;

  if (swfdec_movie_set_asprop (movie, variable, val))
    return;

  swfdec_movie_call_variable_listeners (movie, variable, val);

  SWFDEC_AS_OBJECT_CLASS (swfdec_movie_parent_class)->set (object, variable, val, flags);
}

static char *
swfdec_movie_get_debug (SwfdecAsObject *object)
{
  SwfdecMovie *movie = SWFDEC_MOVIE (object);

  return swfdec_movie_get_path (movie, TRUE);
}

static gboolean
swfdec_movie_iterate_end (SwfdecMovie *movie)
{
  return movie->parent == NULL || 
	 movie->state < SWFDEC_MOVIE_STATE_REMOVED;
}

static void
swfdec_movie_class_init (SwfdecMovieClass * movie_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (movie_class);
  SwfdecAsObjectClass *asobject_class = SWFDEC_AS_OBJECT_CLASS (movie_class);

  object_class->dispose = swfdec_movie_dispose;
  object_class->get_property = swfdec_movie_get_property;
  object_class->set_property = swfdec_movie_set_property;

  asobject_class->mark = swfdec_movie_mark;
  asobject_class->get = swfdec_movie_get_variable;
  asobject_class->set = swfdec_movie_set_variable;
  asobject_class->debug = swfdec_movie_get_debug;

  g_object_class_install_property (object_class, PROP_DEPTH,
      g_param_spec_int ("depth", "depth", "z order inside the parent",
	  G_MININT, G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  movie_class->iterate_end = swfdec_movie_iterate_end;
}

void
swfdec_movie_initialize (SwfdecMovie *movie)
{
  SwfdecMovieClass *klass;

  g_return_if_fail (SWFDEC_IS_MOVIE (movie));

  klass = SWFDEC_MOVIE_GET_CLASS (movie);
  if (klass->init_movie)
    klass->init_movie (movie);
}

void
swfdec_movie_set_depth (SwfdecMovie *movie, int depth)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));

  if (movie->depth == depth)
    return;

  swfdec_movie_invalidate (movie);
  movie->depth = depth;
  if (movie->parent) {
    movie->parent->list = g_list_sort (movie->parent->list, swfdec_movie_compare_depths);
  } else {
    SwfdecPlayer *player = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context);
    player->roots = g_list_sort (player->roots, swfdec_movie_compare_depths);
  }
  g_object_notify (G_OBJECT (movie), "depth");
}

/**
 * swfdec_movie_new:
 * @player: a #SwfdecPlayer
 * @depth: depth of movie
 * @parent: the parent movie or %NULL to make this a root movie
 * @resource: the resource that is responsible for this movie
 * @graphic: the graphic that is displayed by this movie or %NULL to create an 
 *           empty movieclip
 * @name: a garbage-collected string to be used as the name for this movie or 
 *        %NULL for a default one.
 *
 * Creates a new movie #SwfdecMovie for the given properties. No movie may exist
 * at the given @depth. The actual type of
 * this movie depends on the @graphic parameter. The movie will be initialized 
 * with default properties. No script execution will be scheduled. After all 
 * properties are set, the new-movie signal will be emitted if @player is a 
 * debugger.
 *
 * Returns: a new #SwfdecMovie
 **/
SwfdecMovie *
swfdec_movie_new (SwfdecPlayer *player, int depth, SwfdecMovie *parent, SwfdecResource *resource,
    SwfdecGraphic *graphic, const char *name)
{
  SwfdecMovie *movie;
  gsize size;

  g_return_val_if_fail (SWFDEC_IS_PLAYER (player), NULL);
  g_return_val_if_fail (parent == NULL || SWFDEC_IS_MOVIE (parent), NULL);
  g_return_val_if_fail (SWFDEC_IS_RESOURCE (resource), NULL);
  g_return_val_if_fail (graphic == NULL || SWFDEC_IS_GRAPHIC (graphic), NULL);

  /* create the right movie */
  if (graphic == NULL) {
    movie = g_object_new (SWFDEC_TYPE_SPRITE_MOVIE, "depth", depth, NULL);
    size = sizeof (SwfdecSpriteMovie);
  } else {
    SwfdecGraphicClass *klass = SWFDEC_GRAPHIC_GET_CLASS (graphic);
    g_return_val_if_fail (klass->create_movie != NULL, NULL);
    movie = klass->create_movie (graphic, &size);
    movie->graphic = g_object_ref (graphic);
    movie->depth = depth;
  }
  /* register it to the VM */
  /* FIXME: It'd be nice if we'd not overuse memory here when calling this function from a script */
  if (!swfdec_as_context_use_mem (SWFDEC_AS_CONTEXT (player), size)) {
    size = 0;
  }
  g_object_ref (movie);
  /* set essential properties */
  movie->parent = parent;
  movie->resource = g_object_ref (resource);
  if (parent) {
    parent->list = g_list_insert_sorted (parent->list, movie, swfdec_movie_compare_depths);
    SWFDEC_DEBUG ("inserting %s %s (depth %d) into %s %p", G_OBJECT_TYPE_NAME (movie), movie->name,
	movie->depth,  G_OBJECT_TYPE_NAME (parent), parent);
    /* invalidate the parent, so it gets visible */
    swfdec_movie_queue_update (parent, SWFDEC_MOVIE_INVALID_CHILDREN);
  } else {
    player->roots = g_list_insert_sorted (player->roots, movie, swfdec_movie_compare_depths);
  }
  /* set its name */
  if (name) {
    movie->original_name = name;
    movie->name = name;
  } else {
    movie->original_name = SWFDEC_AS_STR_EMPTY;
    if (SWFDEC_IS_SPRITE_MOVIE (movie) || SWFDEC_IS_BUTTON_MOVIE (movie)) {
      movie->name = swfdec_as_context_give_string (SWFDEC_AS_CONTEXT (player), 
	  g_strdup_printf ("instance%u", ++player->unnamed_count));
    } else {
      movie->name = SWFDEC_AS_STR_EMPTY;
    }
  }
  /* add the movie to the global movies list */
  /* NB: adding to the movies list happens before setting the parent.
   * Setting the parent does a gotoAndPlay(0) for Sprites which can cause
   * new movies to be created (and added to this list)
   */
  player->movies = g_list_prepend (player->movies, movie);
  /* only add the movie here, because it needs to be setup for the debugger */
  swfdec_as_object_add (SWFDEC_AS_OBJECT (movie), SWFDEC_AS_CONTEXT (player), size);
  /* only setup here, the resource assumes it can access the player via the movie */
  if (resource->movie == NULL) {
    g_assert (SWFDEC_IS_SPRITE_MOVIE (movie));
    resource->movie = SWFDEC_SPRITE_MOVIE (movie);
  }
  return movie;
}

/* FIXME: since this is only used in PlaceObject, wouldn't it be easier to just have
 * swfdec_movie_update_static_properties (movie); that's notified when any of these change
 * and let PlaceObject modify the movie directly?
 */
void
swfdec_movie_set_static_properties (SwfdecMovie *movie, const cairo_matrix_t *transform,
    const SwfdecColorTransform *ctrans, int ratio, int clip_depth, guint blend_mode,
    SwfdecEventList *events)
{
  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (clip_depth >= -16384 || clip_depth <= 0);
  g_return_if_fail (ratio >= -1);

  if (movie->modified) {
    SWFDEC_LOG ("%s has already been modified by scripts, ignoring updates", movie->name);
    return;
  }
  if (transform) {
    movie->original_transform = *transform;
    movie->matrix.x0 = movie->original_transform.x0;
    movie->matrix.y0 = movie->original_transform.y0;
    movie->xscale = swfdec_matrix_get_xscale (&movie->original_transform);
    movie->yscale = swfdec_matrix_get_yscale (&movie->original_transform);
    movie->rotation = swfdec_matrix_get_rotation (&movie->original_transform);
    swfdec_movie_queue_update (movie, SWFDEC_MOVIE_INVALID_MATRIX);
  }
  if (ctrans) {
    movie->original_ctrans = *ctrans;
    swfdec_movie_invalidate (movie);
  }
  if (ratio >= 0 && (guint) ratio != movie->original_ratio) {
    SwfdecMovieClass *klass;
    movie->original_ratio = ratio;
    klass = SWFDEC_MOVIE_GET_CLASS (movie);
    if (klass->set_ratio)
      klass->set_ratio (movie);
  }
  if (clip_depth && clip_depth != movie->clip_depth) {
    movie->clip_depth = clip_depth;
    /* FIXME: is this correct? */
    swfdec_movie_invalidate (movie->parent ? movie->parent : movie);
  }
  if (blend_mode != movie->blend_mode) {
    movie->blend_mode = blend_mode;
    swfdec_movie_invalidate (movie);
  }
  if (events) {
    if (movie->events)
      swfdec_event_list_free (movie->events);
    movie->events = swfdec_event_list_copy (events);
  }
}

/**
 * swfdec_movie_duplicate:
 * @movie: #SwfdecMovie to copy
 * @name: garbage-collected name for the new copy
 * @depth: depth to put this movie in
 *
 * Creates a duplicate of @movie. The duplicate will not be initialized or
 * queued up for any events. You have to do this manually. In particular calling
 * swfdec_movie_initialize() on the returned movie must be done.
 *
 * Returns: a newly created movie or %NULL on error
 **/
SwfdecMovie *
swfdec_movie_duplicate (SwfdecMovie *movie, const char *name, int depth)
{
  SwfdecMovie *parent, *copy;

  g_return_val_if_fail (SWFDEC_IS_MOVIE (movie), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  parent = movie->parent;
  if (movie->parent == NULL) {
    SWFDEC_FIXME ("don't know how to duplicate root movies");
    return NULL;
  }
  copy = swfdec_movie_find (movie->parent, depth);
  if (copy) {
    SWFDEC_LOG ("depth %d already occupied while duplicating, removing old movie", depth);
    swfdec_movie_remove (copy);
  }
  copy = swfdec_movie_new (SWFDEC_PLAYER (SWFDEC_AS_OBJECT (movie)->context), depth, 
      parent, movie->resource, movie->graphic, name);
  if (copy == NULL)
    return NULL;
  swfdec_movie_set_static_properties (copy, &movie->original_transform,
      &movie->original_ctrans, movie->original_ratio, movie->clip_depth, 
      movie->blend_mode, movie->events);
  if (SWFDEC_IS_SPRITE_MOVIE (copy)) {
    swfdec_movie_queue_script (copy, SWFDEC_EVENT_INITIALIZE);
    swfdec_movie_queue_script (copy, SWFDEC_EVENT_LOAD);
    swfdec_movie_execute (copy, SWFDEC_EVENT_CONSTRUCT);
  }
  swfdec_movie_initialize (copy);
  return copy;
}

SwfdecMovie *
swfdec_movie_new_for_content (SwfdecMovie *parent, const SwfdecContent *content)
{
  SwfdecPlayer *player;
  SwfdecMovie *movie;

  g_return_val_if_fail (SWFDEC_IS_MOVIE (parent), NULL);
  g_return_val_if_fail (SWFDEC_IS_GRAPHIC (content->graphic), NULL);
  g_return_val_if_fail (swfdec_movie_find (parent, content->depth) == NULL, NULL);

  SWFDEC_DEBUG ("new movie for parent %p", parent);
  player = SWFDEC_PLAYER (SWFDEC_AS_OBJECT (parent)->context);
  movie = swfdec_movie_new (player, content->depth, parent, parent->resource, content->graphic, 
      content->name ? swfdec_as_context_get_string (SWFDEC_AS_CONTEXT (player), content->name) : NULL);

  swfdec_movie_set_static_properties (movie, content->has_transform ? &content->transform : NULL,
      content->has_color_transform ? &content->color_transform : NULL, 
      content->ratio, content->clip_depth, content->blend_mode, content->events);
  if (SWFDEC_IS_SPRITE_MOVIE (movie)) {
    swfdec_movie_queue_script (movie, SWFDEC_EVENT_INITIALIZE);
    swfdec_movie_queue_script (movie, SWFDEC_EVENT_CONSTRUCT);
    swfdec_movie_queue_script (movie, SWFDEC_EVENT_LOAD);
  }
  swfdec_movie_initialize (movie);

  return movie;
}

static void
swfdec_movie_load_variables_on_data (SwfdecAsContext *cx,
    SwfdecAsObject *object, guint argc, SwfdecAsValue *argv,
    SwfdecAsValue *ret)
{
  SwfdecAsObject *target;
  SwfdecAsValue val;

  if (argc < 1)
    return;

  if (!SWFDEC_AS_VALUE_IS_STRING (&argv[0]))
    return;

  swfdec_as_object_get_variable (object, SWFDEC_AS_STR_target, &val);
  g_return_if_fail (SWFDEC_AS_VALUE_IS_OBJECT (&val));
  target = SWFDEC_AS_VALUE_GET_OBJECT (&val);
  g_return_if_fail (SWFDEC_IS_MOVIE (target));

  swfdec_as_object_decode (target, swfdec_as_value_to_string (cx, &argv[0]));

  if (cx->version >= 6)
    swfdec_as_object_call (target, SWFDEC_AS_STR_onData, 0, NULL, NULL);
}

void
swfdec_movie_load_variables (SwfdecMovie *movie, const char *url,
    SwfdecLoaderRequest request, SwfdecBuffer *data)
{
  SwfdecAsObject *loader;
  SwfdecAsContext *context;
  SwfdecAsValue val;

  g_return_if_fail (SWFDEC_IS_MOVIE (movie));
  g_return_if_fail (url != NULL);

  if (request != SWFDEC_LOADER_REQUEST_DEFAULT) {
    SWFDEC_FIXME ("loadVariables: Different request-modes not supported");
    return;
  }

  context = SWFDEC_AS_OBJECT (movie)->context;
  loader = swfdec_as_object_new_empty (context);
  swfdec_as_object_add_function (loader, SWFDEC_AS_STR_onData, 0,
      swfdec_movie_load_variables_on_data, 0);
  SWFDEC_AS_VALUE_SET_OBJECT (&val, SWFDEC_AS_OBJECT (movie));
  swfdec_as_object_set_variable (loader, SWFDEC_AS_STR_target, &val);

  swfdec_load_object_new (loader, url, request, data);
}

char *
swfdec_movie_get_path (SwfdecMovie *movie, gboolean dot)
{
  GString *s;

  g_return_val_if_fail (SWFDEC_IS_MOVIE (movie), NULL);
  
  s = g_string_new ("");
  do {
    if (movie->parent) {
      g_string_prepend (s, movie->name);
      g_string_prepend_c (s, (dot ? '.' : '/'));
    } else {
      char *ret;
      if (dot) {
	ret = g_strdup_printf ("_level%u%s", movie->depth + 16384, s->str);
	g_string_free (s, TRUE);
      } else {
	if (s->str[0] != '/')
	  g_string_prepend_c (s, '/');
	ret = g_string_free (s, FALSE);
      }
      return ret;
    }
    movie = movie->parent;
  } while (TRUE);

  g_assert_not_reached ();

  return NULL;
}

int
swfdec_movie_compare_depths (gconstpointer a, gconstpointer b)
{
  if (SWFDEC_MOVIE (a)->depth < SWFDEC_MOVIE (b)->depth)
    return -1;
  if (SWFDEC_MOVIE (a)->depth > SWFDEC_MOVIE (b)->depth)
    return 1;
  return 0;
}

/**
 * swfdec_depth_classify:
 * @depth: the depth to classify
 *
 * Classifies a depth. This classification is mostly used when deciding if
 * certain operations are valid in ActionScript.
 *
 * Returns: the classification of the depth.
 **/
SwfdecDepthClass
swfdec_depth_classify (int depth)
{
  if (depth < -16384)
    return SWFDEC_DEPTH_CLASS_EMPTY;
  if (depth < 0)
    return SWFDEC_DEPTH_CLASS_TIMELINE;
  if (depth < 1048576)
    return SWFDEC_DEPTH_CLASS_DYNAMIC;
  if (depth < 2130690046)
    return SWFDEC_DEPTH_CLASS_RESERVED;
  return SWFDEC_DEPTH_CLASS_EMPTY;
}

/**
 * swfdec_movie_get_own_resource:
 * @movie: movie to query
 *
 * Queries the movie for his own resource. A movie only has its own resource if
 * it contains data loaded with the loadMovie() function, or if it is the root
 * movie.
 *
 * Returns: The own resource of @movie or %NULL
 **/
SwfdecResource *
swfdec_movie_get_own_resource (SwfdecMovie *movie)
{
  g_return_val_if_fail (SWFDEC_IS_MOVIE (movie), NULL);

  if (!SWFDEC_IS_SPRITE_MOVIE (movie))
    return NULL;

  if (SWFDEC_MOVIE (movie->resource->movie) != movie)
    return NULL;

  return movie->resource;
}

