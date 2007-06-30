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

#include <math.h>
#include <string.h>
#include "swfdec_as_context.h"
#include "swfdec_as_array.h"
#include "swfdec_as_frame.h"
#include "swfdec_as_function.h"
#include "swfdec_as_interpret.h"
#include "swfdec_as_math.h"
#include "swfdec_as_native_function.h"
#include "swfdec_as_number.h"
#include "swfdec_as_object.h"
#include "swfdec_as_stack.h"
#include "swfdec_as_string.h"
#include "swfdec_as_strings.h"
#include "swfdec_as_types.h"
#include "swfdec_debug.h"
#include "swfdec_script.h"

/*** GARBAGE COLLECTION DOCS ***/

#define SWFDEC_AS_GC_MARK (1 << 0)		/* only valid during GC */
#define SWFDEC_AS_GC_ROOT (1 << 1)		/* for objects: rooted, for strings: static */

/**
 * SECTION:Internals
 * @title: Internals and garbage collection
 * @short_description: understanding internals such as garbage collection
 * @see_also: #SwfdecAsContext
 *
 * This section deals with the internals of the Swfdec Actionscript engine. You
 * should probably read this first when trying to write code with it. If you're
 * just trying to use Swfdec for creating Flash content, you can probably skip
 * this section.
 *
 * First, I'd like to note that the Swfdec script engine has to be modeled very 
 * closely after the existing Flash players. So if there are some behaviours
 * that seem stupid at first sight, it might very well be that it was chosen for
 * a very particular reason. Now on to the features.
 *
 * The Swfdec script engine tries to imitate Actionscript as good as possible.
 * Actionscript is similar to Javascript, but not equal. Depending on the 
 * version of the script executed it might be more or less similar. An important
 * example is that Flash in versions up to 6 did case-insensitive variable 
 * lookups.
 *
 * The script engine starts its life when it is initialized via 
 * swfdec_as_context_startup (). At that point, the basic objects are created.
 * After this function has been called, you can start executing code. All code
 * execution happens by creating a new #SwfdecAsFrame and then calling 
 * swfdec_as_context_run() to execute it. This function is the single entry 
 * point for code execution. Convenience functions exist that make executing 
 * code easy, most notably swfdec_as_object_run() and 
 * swfdec_as_object_call().
 *
 * It is also easily possible to extend the environment by adding new objects.
 * In fact, without doing so, the environment is pretty bare as it just contains
 * the basic Math, String, Number, Array, Date and Boolean objects. This is done
 * by adding #SwfdecAsNative functions to the environment. The easy way
 * to do this is via swfdec_as_object_add_function().
 *
 * The Swfdec script engine is dynamically typed and knows about different types
 * of values. See #SwfdecAsValue for the different values. Memory management is
 * done using a mark and sweep garbage collector. You can initiate a garbage 
 * collection cycle by calling swfdec_as_context_gc() or 
 * swfdec_as_context_maybe_gc(). You should do this regularly to avoid excessive
 * memory use. The #SwfdecAsContext will then collect the objects and strings it
 * is keeping track of. If you want to use an object or string in the script 
 * engine, you'll have to add it first via swfdec_as_object_add() or
 * swfdec_as_context_get_string() and swfdec_as_context_give_string(), 
 * respectively.
 *
 * Garbage-collected strings are special in Swfdec in that they are unique. This
 * means the same string exists exactly once. Because of this you can do 
 * equality comparisons using == instead of strcmp. It also means that if you 
 * forget to add a string to the context before using it, your app will most 
 * likely not work, since the string will not compare equal to any other string.
 *
 * When a garbage collection cycle happens, all reachable objects and strings 
 * are marked and all unreachable ones are freed. This is done by calling the
 * context class's mark function which will mark all reachable objects. This is
 * usually called the "root set". For any reachable object, the object's mark
 * function is called so that the object in turn can mark all objects it can 
 * reach itself. Marking is done via functions described below.
 */

/*** GTK-DOC ***/

/**
 * SECTION:SwfdecAsContext
 * @title: SwfdecAsContext
 * @short_description: the main script engine context
 * @see_also: SwfdecPlayer
 *
 * A #SwfdecAsContext provides the main execution environment for Actionscript
 * execution. It provides the objects typically available in ECMAScript and
 * manages script execution, garbage collection etc. #SwfdecPlayer is a
 * subclass of the context that implements Flash specific objects on top of it.
 * However, it is possible to use the context for completely different functions
 * where a sandboxed scripting environment is needed. An example is the Swfdec 
 * debugger.
 * <note>The Actionscript engine is similar, but not equal to Javascript. It
 * is not very different, but it is different.</note>
 */

/**
 * SwfdecAsContextState
 * @SWFDEC_AS_CONTEXT_NEW: the context is not yet initialized, 
 *                         swfdec_as_context_startup() needs to be called.
 * @SWFDEC_AS_CONTEXT_RUNNING: the context is running normally
 * @SWFDEC_AS_CONTEXT_INTERRUPTED: the context has been interrupted by a 
 *                             debugger
 * @SWFDEC_AS_CONTEXT_ABORTED: the context has aborted execution due to a 
 *                         fatal error
 *
 * The state of the context describes what operations are possible on the context.
 * It will be in the state @SWFDEC_AS_CONTEXT_STATE_RUNNING almost all the time. If
 * it is in the state @SWFDEC_AS_CONTEXT_STATE_ABORTED, it will not work anymore and
 * every operation on it will instantly fail.
 */

/*** RUNNING STATE ***/

/**
 * swfdec_as_context_abort:
 * @context: a #SwfdecAsContext
 * @reason: a string describing why execution was aborted
 *
 * Aborts script execution in @context. Call this functon if the script engine 
 * encountered a fatal error and cannot continue. A possible reason for this is
 * an out-of-memory condition.
 **/
void
swfdec_as_context_abort (SwfdecAsContext *context, const char *reason)
{
  g_return_if_fail (context);

  SWFDEC_ERROR (reason);
  context->state = SWFDEC_AS_CONTEXT_ABORTED;
}

/*** MEMORY MANAGEMENT ***/

/**
 * swfdec_as_context_use_mem:
 * @context: a #SwfdecAsContext
 * @bytes: number of bytes to use
 *
 * Registers @bytes additional bytes as in use by the @context. This function
 * keeps track of the memory that script code consumes. If too many memory is 
 * in use, this function may decide to stop the script engine with an out of 
 * memory error.
 *
 * Returns: %TRUE if the memory could be allocated. %FALSE on OOM.
 **/
gboolean
swfdec_as_context_use_mem (SwfdecAsContext *context, gsize bytes)
{
  g_return_val_if_fail (SWFDEC_IS_AS_CONTEXT (context), FALSE);
  g_return_val_if_fail (bytes > 0, FALSE);

  if (context->state == SWFDEC_AS_CONTEXT_ABORTED)
    return FALSE;
  
  context->memory += bytes;
  context->memory_since_gc += bytes;
  /* FIXME: Don't foget to abort on OOM */
  return TRUE;
}

/**
 * swfdec_as_context_unuse_mem:
 * @context: a #SwfdecAsContext
 * @bytes: number of bytes to release
 *
 * Releases a number of bytes previously allocated using 
 * swfdec_as_context_use_mem(). See that function for details.
 **/
void
swfdec_as_context_unuse_mem (SwfdecAsContext *context, gsize bytes)
{
  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  g_return_if_fail (bytes > 0);
  g_return_if_fail (context->memory >= bytes);

  context->memory -= bytes;
}

/*** GC ***/

static gboolean
swfdec_as_context_remove_strings (gpointer key, gpointer value, gpointer data)
{
  SwfdecAsContext *context = data;
  char *string;

  string = (char *) value;
  /* it doesn't matter that rooted strings aren't destroyed, they're constant */
  if (string[0] & SWFDEC_AS_GC_ROOT) {
    SWFDEC_LOG ("rooted: %s", (char *) key);
    return FALSE;
  } else if (string[0] & SWFDEC_AS_GC_MARK) {
    SWFDEC_LOG ("marked: %s", (char *) key);
    string[0] &= ~SWFDEC_AS_GC_MARK;
    return FALSE;
  } else {
    gsize len;
    SWFDEC_LOG ("deleted: %s", (char *) key);
    len = (strlen ((char *) key) + 2);
    swfdec_as_context_unuse_mem (context, len);
    g_slice_free1 (len, value);
    return TRUE;
  }
}

static gboolean
swfdec_as_context_remove_objects (gpointer key, gpointer value, gpointer data)
{
  SwfdecAsObject *object;

  object = key;
  /* we only check for mark here, not root, since this works on destroy, too */
  if (object->flags & SWFDEC_AS_GC_MARK) {
    object->flags &= ~SWFDEC_AS_GC_MARK;
    SWFDEC_LOG ("%s: %s %p", (object->flags & SWFDEC_AS_GC_ROOT) ? "rooted" : "marked",
	G_OBJECT_TYPE_NAME (object), object);
    return FALSE;
  } else {
    SWFDEC_LOG ("deleted: %s %p", G_OBJECT_TYPE_NAME (object), object);
    swfdec_as_object_collect (object);
    return TRUE;
  }
}

static void
swfdec_as_context_collect (SwfdecAsContext *context)
{
  SWFDEC_INFO (">> collecting garbage");
  /* NB: This functions is called without GC from swfdec_as_context_dispose */
  g_hash_table_foreach_remove (context->strings, 
    swfdec_as_context_remove_strings, context);
  g_hash_table_foreach_remove (context->objects, 
    swfdec_as_context_remove_objects, context);
  SWFDEC_INFO (">> done collecting garbage");
}

/**
 * swfdec_as_object_mark:
 * @object: a #SwfdecAsObject
 *
 * Mark @object as being in use. Calling this function is only valid during
 * the marking phase of garbage collection.
 **/
void
swfdec_as_object_mark (SwfdecAsObject *object)
{
  SwfdecAsObjectClass *klass;

  if (object->flags & SWFDEC_AS_GC_MARK)
    return;
  object->flags |= SWFDEC_AS_GC_MARK;
  klass = SWFDEC_AS_OBJECT_GET_CLASS (object);
  g_assert (klass->mark);
  klass->mark (object);
}

/**
 * swfdec_as_string_mark:
 * @string: a garbage-collected string
 *
 * Mark @string as being in use. Calling this function is only valid during
 * the marking phase of garbage collection.
 **/
void
swfdec_as_string_mark (const char *string)
{
  char *str = (char *) string - 1;
  if (*str == 0)
    *str = SWFDEC_AS_GC_MARK;
}

/**
 * swfdec_as_value_mark:
 * @value: a #SwfdecAsValue
 *
 * Mark @value as being in use. This is just a convenience function that calls
 * the right marking function depending on the value's type. Calling this 
 * function is only valid during the marking phase of garbage collection.
 **/
void
swfdec_as_value_mark (SwfdecAsValue *value)
{
  g_return_if_fail (SWFDEC_IS_AS_VALUE (value));

  if (SWFDEC_AS_VALUE_IS_OBJECT (value)) {
    swfdec_as_object_mark (SWFDEC_AS_VALUE_GET_OBJECT (value));
  } else if (SWFDEC_AS_VALUE_IS_STRING (value)) {
    swfdec_as_string_mark (SWFDEC_AS_VALUE_GET_STRING (value));
  }
}

static void
swfdec_as_context_mark_roots (gpointer key, gpointer value, gpointer data)
{
  SwfdecAsObject *object = key;

  if ((object->flags & (SWFDEC_AS_GC_MARK | SWFDEC_AS_GC_ROOT)) == SWFDEC_AS_GC_ROOT)
    swfdec_as_object_mark (object);
}

static void
swfdec_as_context_do_mark (SwfdecAsContext *context)
{
  swfdec_as_object_mark (context->global);
  swfdec_as_object_mark (context->Function);
  if (context->Function_prototype)
    swfdec_as_object_mark (context->Function_prototype);
  swfdec_as_object_mark (context->Object);
  swfdec_as_object_mark (context->Object_prototype);
  swfdec_as_object_mark (context->Array);
  g_hash_table_foreach (context->objects, swfdec_as_context_mark_roots, NULL);
}

/**
 * swfdec_as_context_gc:
 * @context: a #SwfdecAsContext
 *
 * Calls the Swfdec Gargbage collector and reclaims any unused memory. You 
 * should call this function or swfdec_as_context_maybe_gc() regularly.
 * <warning>Calling the GC during execution of code or initialization is not
 *          allowed.</warning>
 **/
void
swfdec_as_context_gc (SwfdecAsContext *context)
{
  SwfdecAsContextClass *klass;

  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  g_return_if_fail (context->frame == NULL);
  g_return_if_fail (context->state != SWFDEC_AS_CONTEXT_NEW);

  if (context->state == SWFDEC_AS_CONTEXT_ABORTED)
    return;
  SWFDEC_INFO ("invoking the garbage collector");
  klass = SWFDEC_AS_CONTEXT_GET_CLASS (context);
  g_assert (klass->mark);
  klass->mark (context);
  swfdec_as_context_collect (context);
  context->memory_since_gc = 0;
}

static gboolean
swfdec_as_context_needs_gc (SwfdecAsContext *context)
{
  return context->memory_since_gc >= context->memory_until_gc;
}

/**
 * swfdec_as_context_maybe_gc:
 * @context: a #SwfdecAsContext
 *
 * Calls the garbage collector if necessary. It's a good idea to call this
 * function regularly instead of swfdec_as_context_gc() as it only does collect
 * garage as needed. For example, #SwfdecPlayer calls this function after every
 * frame advancement.
 **/
void
swfdec_as_context_maybe_gc (SwfdecAsContext *context)
{
  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  if (context->state == SWFDEC_AS_CONTEXT_ABORTED)
    return;
  g_return_if_fail (context->frame == NULL);

  if (swfdec_as_context_needs_gc (context))
    swfdec_as_context_gc (context);
}

/*** SWFDEC_AS_CONTEXT ***/

enum {
  TRACE,
  LAST_SIGNAL
};

G_DEFINE_TYPE (SwfdecAsContext, swfdec_as_context, G_TYPE_OBJECT)
static guint signals[LAST_SIGNAL] = { 0, };

static void
swfdec_as_context_dispose (GObject *object)
{
  SwfdecAsContext *context = SWFDEC_AS_CONTEXT (object);

  swfdec_as_context_collect (context);
  if (context->memory != 0) {
    g_critical ("%zu bytes of memory left over\n", context->memory);
  }
  g_assert (g_hash_table_size (context->objects) == 0);
  g_hash_table_destroy (context->objects);
  g_hash_table_destroy (context->strings);
  g_rand_free (context->rand);

  G_OBJECT_CLASS (swfdec_as_context_parent_class)->dispose (object);
}

static void
swfdec_as_context_class_init (SwfdecAsContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = swfdec_as_context_dispose;

  /**
   * SwfdecAsContext::trace:
   * @context: the #SwfdecAsContext affected
   * @text: the debugging string
   *
   * Emits a debugging string while running. The effect of calling any swfdec 
   * functions on the emitting @context is undefined.
   */
  signals[TRACE] = g_signal_new ("trace", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);

  klass->mark = swfdec_as_context_do_mark;
}

static void
swfdec_as_context_init (SwfdecAsContext *context)
{
  const char *s;

  context->memory_until_gc = 8 * 1024 * 1024; /* 8 MB before we run the GC */
  context->strings = g_hash_table_new (g_str_hash, g_str_equal);
  context->objects = g_hash_table_new (g_direct_hash, g_direct_equal);

  for (s = swfdec_as_strings; *s == '\2'; s += strlen (s) + 1) {
    g_hash_table_insert (context->strings, (char *) s + 1, (char *) s);
  }
  g_assert (*s == 0);
  context->global = swfdec_as_object_new (context);
  context->rand = g_rand_new ();
  g_get_current_time (&context->start_time);
}

/*** STRINGS ***/

static const char *
swfdec_as_context_create_string (SwfdecAsContext *context, const char *string, gsize len)
{
  char *new;
  
  if (!swfdec_as_context_use_mem (context, sizeof (char) * (2 + len)))
    return SWFDEC_AS_STR_EMPTY;

  new = g_slice_alloc (2 + len);
  memcpy (&new[1], string, len);
  new[len + 1] = 0;
  new[0] = 0; /* GC flags */
  g_hash_table_insert (context->strings, new + 1, new);

  return new + 1;
}

/**
 * swfdec_as_context_get_string:
 * @context: a #SwfdecAsContext
 * @string: a sting that is not garbage-collected
 *
 * Gets the garbage-collected version of @string. You need to call this function
 * for every not garbage-collected string that you want to use in Swfdecs script
 * interpreter.
 *
 * Returns: the garbage-collected version of @string
 **/
const char *
swfdec_as_context_get_string (SwfdecAsContext *context, const char *string)
{
  const char *ret;
  gsize len;

  g_return_val_if_fail (SWFDEC_IS_AS_CONTEXT (context), NULL);
  g_return_val_if_fail (string != NULL, NULL);

  if (g_hash_table_lookup_extended (context->strings, string, (gpointer) &ret, NULL))
    return ret;

  len = strlen (string);
  return swfdec_as_context_create_string (context, string, len);
}

/**
 * swfdec_as_context_give_string:
 * @context: a #SwfdecAsContext
 * @string: string to make refcounted
 *
 * Takes ownership of @string and returns a refcounted version of the same 
 * string. This function is the same as swfdec_as_context_get_string(), but 
 * takes ownership of @string.
 *
 * Returns: A refcounted string
 **/
const char *
swfdec_as_context_give_string (SwfdecAsContext *context, char *string)
{
  const char *ret;

  g_return_val_if_fail (SWFDEC_IS_AS_CONTEXT (context), NULL);
  g_return_val_if_fail (string != NULL, NULL);

  ret = swfdec_as_context_get_string (context, string);
  g_free (string);
  return ret;
}

/**
 * swfdec_as_context_get_time:
 * @context: a #SwfdecAsContext
 * @tv: a #GTimeVal to be set to the context's time
 *
 * This function queries the time to be used inside this context. By default,
 * this is the same as g_get_current_time(), but it may be overwriten to allow
 * things such as slower or faster playback.
 **/
void
swfdec_as_context_get_time (SwfdecAsContext *context, GTimeVal *tv)
{
  SwfdecAsContextClass *klass;

  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  g_return_if_fail (tv != NULL);

  klass = SWFDEC_AS_CONTEXT_GET_CLASS (context);
  if (klass->get_time)
    klass->get_time (context, tv);
  else
    g_get_current_time (tv);
}

/**
 * swfdec_as_context_run:
 * @context: a #SwfdecAsContext
 *
 * Continues running the script engine. Executing code in this engine works
 * in 2 steps: First, you push the frame to be executed onto the stack, then
 * you call this function to execute it. So this function is the single entry
 * point to script execution. This might be helpful when debugging your 
 * application. 
 * <note>A lot of convenience functions like swfdec_as_object_run() call this 
 * function automatically.</note>
 **/
void
swfdec_as_context_run (SwfdecAsContext *context)
{
  SwfdecAsFrame *frame, *last_frame;
  SwfdecAsStack *stack;
  SwfdecScript *script;
  const SwfdecActionSpec *spec;
  guint8 *startpc, *pc, *endpc, *nextpc;
#ifndef G_DISABLE_ASSERT
  SwfdecAsValue *check;
#endif
  guint action, len;
  guint8 *data;
  int version;
  SwfdecAsContextClass *klass;
  void (* step) (SwfdecAsContext *context);
  gboolean check_scope; /* some opcodes avoid a scope check */

  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  if (context->frame == NULL || context->state == SWFDEC_AS_CONTEXT_ABORTED)
    return;

  klass = SWFDEC_AS_CONTEXT_GET_CLASS (context);
  step = klass->step;

  last_frame = context->last_frame;
  context->last_frame = context->frame->next;
start:
  /* setup data */
  frame = context->frame;
  if (frame == context->last_frame)
    goto out;
  if (context->call_depth > 256) {
    /* we've exceeded our maximum call depth, throw an error and abort */
    swfdec_as_context_abort (context, "Stack overflow");
    return;
  }
  if (SWFDEC_IS_AS_NATIVE_FUNCTION (frame->function)) {
    SwfdecAsNativeFunction *native = SWFDEC_AS_NATIVE_FUNCTION (frame->function);
    if (frame->argc >= native->min_args && 
	(native->type == 0 || 
	 g_type_is_a (G_OBJECT_TYPE (frame->thisp), native->type))) {
      /* FIXME FIXME FIXME: no casting here please! */
      native->native (context, frame->thisp, frame->argc, 
	  (SwfdecAsValue *) frame->argv, frame->return_value);
    }
    swfdec_as_frame_return (frame);
    goto start;
  }
  g_assert (frame->script);
  g_assert (frame->target);
  script = frame->script;
  stack = frame->stack;
  version = SWFDEC_AS_EXTRACT_SCRIPT_VERSION (script->version);
  context->version = script->version;
  startpc = script->buffer->data;
  endpc = startpc + script->buffer->length;
  pc = frame->pc;
  check_scope = TRUE;

  while (context->state < SWFDEC_AS_CONTEXT_ABORTED) {
    if (pc == endpc) {
      swfdec_as_frame_return (frame);
      goto start;
    }
    if (pc < startpc || pc >= endpc) {
      SWFDEC_ERROR ("pc %p not in valid range [%p, %p) anymore", pc, startpc, endpc);
      goto error;
    }
    if (check_scope)
      swfdec_as_frame_check_scope (frame);

    /* decode next action */
    action = *pc;
    if (action == 0) {
      swfdec_as_frame_return (frame);
      goto start;
    }
    /* invoke debugger if there is one */
    if (step) {
      frame->pc = pc;
      (* step) (context);
      if (frame != context->frame || 
	  frame->pc != pc) {
	goto start;
      }
    }
    /* prepare action */
    spec = swfdec_as_actions + action;
    if (action & 0x80) {
      if (pc + 2 >= endpc) {
	SWFDEC_ERROR ("action %u length value out of range", action);
	goto error;
      }
      data = pc + 3;
      len = pc[1] | pc[2] << 8;
      if (data + len > endpc) {
	SWFDEC_ERROR ("action %u length %u out of range", action, len);
	goto error;
      }
      nextpc = pc + 3 + len;
    } else {
      data = NULL;
      len = 0;
      nextpc = pc + 1;
    }
    /* check action is valid */
    if (spec->exec[version] == NULL) {
      SWFDEC_ERROR ("cannot interpret action %3u 0x%02X %s for version %u", action,
	  action, spec->name ? spec->name : "Unknown", script->version);
      /* FIXME: figure out what flash player does here */
      goto error;
    }
    if (spec->remove > 0) {
      swfdec_as_stack_ensure_size (stack, spec->remove);
      if (spec->add > spec->remove)
	swfdec_as_stack_ensure_left (stack, spec->add - spec->remove);
    } else {
      if (spec->add > 0)
	swfdec_as_stack_ensure_left (stack, spec->add);
    }
    if (context->state != SWFDEC_AS_CONTEXT_RUNNING)
      goto error;
#ifndef G_DISABLE_ASSERT
    check = (spec->add >= 0 && spec->remove >= 0) ? stack->cur + spec->add - spec->remove : NULL;
#endif
    /* execute action */
    spec->exec[version] (context, action, data, len);
    /* adapt the pc if the action did not, otherwise, leave it alone */
    /* FIXME: do this via flag? */
    if (frame->pc == pc) {
      frame->pc = pc = nextpc;
      check_scope = TRUE;
    } else {
      pc = frame->pc;
      check_scope = FALSE;
    }
    if (frame == context->frame) {
#ifndef G_DISABLE_ASSERT
      if (check != NULL && check != stack->cur) {
	g_error ("action %s was supposed to change the stack by %d (+%d -%d), but it changed by %td",
	    spec->name, spec->add - spec->remove, spec->add, spec->remove,
	    stack->cur - check + spec->add - spec->remove);
      }
#endif
    } else {
      /* someone called/returned from a function, reread variables */
      goto start;
    }
  }

error:
  context->frame = context->last_frame;
out:
  context->last_frame = last_frame;
  return;
}

/*** EVAL ***/

char *
swfdec_as_slash_to_dot (const char *slash_str)
{
  const char *cur = slash_str;
  GString *str = g_string_new ("");

  if (*cur == '/') {
    g_string_append (str, "_root");
  } else {
    goto start;
  }
  while (cur && *cur == '/') {
    cur++;
start:
    if (str->len > 0)
      g_string_append_c (str, '.');
    if (cur[0] == '.' && cur[1] == '.') {
      g_string_append (str, "_parent");
      cur += 2;
    } else {
      char *slash = strchr (cur, '/');
      if (slash) {
	g_string_append_len (str, cur, slash - cur);
	cur = slash;
      } else {
	g_string_append (str, cur);
	cur = NULL;
      }
    }
    /* cur should now point to the slash */
  }
  if (cur) {
    if (*cur != '\0')
      goto fail;
  }
  SWFDEC_DEBUG ("parsed slash-notated string \"%s\" into dot notation \"%s\"",
      slash_str, str->str);
  return g_string_free (str, FALSE);

fail:
  SWFDEC_WARNING ("failed to parse slash-notated string \"%s\" into dot notation", slash_str);
  g_string_free (str, TRUE);
  return NULL;
}

static void
swfdec_as_context_eval_get_property (SwfdecAsContext *cx, 
    SwfdecAsObject *obj, const char *name, SwfdecAsValue *ret)
{
  if (obj) {
    swfdec_as_object_get_variable (obj, name, ret);
  } else {
    if (cx->frame) {
      obj = swfdec_as_frame_find_variable (cx->frame, name);
      if (obj) {
	swfdec_as_object_get_variable (obj, name, ret);
	return;
      }
    } else {
      SWFDEC_ERROR ("no frame in eval?");
    }
    SWFDEC_AS_VALUE_SET_UNDEFINED (ret);
  }
}

static void
swfdec_as_context_eval_set_property (SwfdecAsContext *cx, 
    SwfdecAsObject *obj, const char *name, const SwfdecAsValue *ret)
{
  if (obj == NULL) {
    if (cx->frame == NULL) {
      SWFDEC_ERROR ("no frame in eval_set?");
      return;
    }
    obj = swfdec_as_frame_find_variable (cx->frame, name);
    if (obj == NULL || obj == cx->global)
      obj = cx->frame->target;
  }
  swfdec_as_object_set_variable (obj, name, ret);
}

static void
swfdec_as_context_eval_internal (SwfdecAsContext *cx, SwfdecAsObject *obj, const char *str,
        SwfdecAsValue *val, gboolean set)
{
  SwfdecAsValue cur;
  char **varlist;
  guint i;

  SWFDEC_LOG ("eval called with \"%s\" on %p", str, obj);
  if (strchr (str, '/')) {
    char *work = swfdec_as_slash_to_dot (str);
    if (!work) {
      SWFDEC_AS_VALUE_SET_UNDEFINED (val);
      return;
    }
    varlist = g_strsplit (work, ".", -1);
    g_free (work);
  } else {
    varlist = g_strsplit (str, ".", -1);
  }
  for (i = 0; varlist[i] != NULL; i++) {
    const char *dot = swfdec_as_context_get_string (cx, varlist[i]);
    if (varlist[i+1] != NULL) {
      swfdec_as_context_eval_get_property (cx, obj, dot, &cur);
      if (!SWFDEC_AS_VALUE_IS_OBJECT (&cur)) {
	SWFDEC_AS_VALUE_SET_UNDEFINED (&cur);
	break;
      }
      obj = SWFDEC_AS_VALUE_GET_OBJECT (&cur);
    } else {
      if (set) {
	swfdec_as_context_eval_set_property (cx, obj, dot, val);
      } else {
	swfdec_as_context_eval_get_property (cx, obj, dot, &cur);
      }
      goto finish;
    }
  }
  if (obj == NULL && cx->frame) {
    swfdec_as_object_get_variable (SWFDEC_AS_OBJECT (cx->frame), SWFDEC_AS_STR_this, &cur);
  } else {
    SWFDEC_AS_VALUE_SET_OBJECT (&cur, obj);
  }

finish:
  g_strfreev (varlist);
  *val = cur;
}

/**
 * swfdec_as_context_eval:
 * @context: a #SwfdecAsContext
 * @obj: #SwfdecAsObject to use as source for evaluating or NULL for the 
 *       current frame's scope
 * @str: The string to evaluate
 * @val: location for the return value
 *
 * This function works like the Actionscript eval function used on @obj.
 * It handles both slash-style and dot-style notation. If an error occured
 * during evaluation, the return value will be the undefined value.
 **/
void
swfdec_as_context_eval (SwfdecAsContext *context, SwfdecAsObject *obj, const char *str, 
    SwfdecAsValue *val)
{
  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  g_return_if_fail (obj == NULL || SWFDEC_IS_AS_OBJECT (obj));
  g_return_if_fail (str != NULL);
  g_return_if_fail (val != NULL);

  swfdec_as_context_eval_internal (context, obj, str, val, FALSE);
}

/**
 * swfdec_as_context_eval_set:
 * @context: a #SwfdecAsContext
 * @obj: #SwfdecAsObject to use as source for evaluating or NULL for the 
 *       default object.
 * @str: The string to evaluate
 * @val: the value to set the variable to
 *
 * Sets the variable referenced by @str to @val. If @str does not reference 
 * a valid property, nothing happens.
 **/
void
swfdec_as_context_eval_set (SwfdecAsContext *context, SwfdecAsObject *obj, const char *str,
    const SwfdecAsValue *val)
{
  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  g_return_if_fail (obj == NULL || SWFDEC_IS_AS_OBJECT (obj));
  g_return_if_fail (str != NULL);
  g_return_if_fail (val != NULL);

  swfdec_as_context_eval_internal (context, obj, str, (SwfdecAsValue *) val, TRUE);
}

/*** AS CODE ***/

static gboolean
swfdec_as_context_ASSetPropFlags_foreach (SwfdecAsObject *object, const char *s, 
    SwfdecAsValue *val, guint cur_flags, gpointer data)
{
  guint *flags = data;
  guint real;

  /* shortcut if the flags already match */
  if ((cur_flags & flags[1]) == flags[0])
    return TRUE;

  /* first set all relevant flags */
  real = flags[0] & flags[1];
  swfdec_as_object_set_variable_flags (object, s, real);
  /* then unset all relevant flags */
  real = ~flags[0] & flags[1];
  swfdec_as_object_unset_variable_flags (object, s, real);
  return TRUE;
}

static void
swfdec_as_context_ASSetPropFlags (SwfdecAsContext *cx, SwfdecAsObject *object, 
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *retval)
{
  guint flags[2]; /* flags and mask - array so we can pass it as data pointer */
  SwfdecAsObject *obj;

  if (cx->version < 6) {
    SWFDEC_FIXME ("ASSetPropFlags needs some limitations for Flash 5");
  }
  if (!SWFDEC_AS_VALUE_IS_OBJECT (&argv[0]))
    return;
  obj = SWFDEC_AS_VALUE_GET_OBJECT (&argv[0]);
  flags[0] = swfdec_as_value_to_integer (cx, &argv[2]);
  /* be sure to not delete the NATIVE flag */
  flags[0] &= 7;
  flags[1] = (argc > 3) ? swfdec_as_value_to_integer (cx, &argv[3]) : -1;
  if (SWFDEC_AS_VALUE_IS_NULL (&argv[1])) {
    swfdec_as_object_foreach (obj, swfdec_as_context_ASSetPropFlags_foreach, flags);
  } else {
    SWFDEC_FIXME ("ASSetPropFlags for non-null properties not implemented yet");
  }
}

static void
swfdec_as_context_isFinite (SwfdecAsContext *cx, SwfdecAsObject *object, 
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *retval)
{
  double d = swfdec_as_value_to_number (cx, &argv[0]);
  SWFDEC_AS_VALUE_SET_BOOLEAN (retval, isfinite (d) ? TRUE : FALSE);
}

static void
swfdec_as_context_isNaN (SwfdecAsContext *cx, SwfdecAsObject *object, 
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *retval)
{
  double d = swfdec_as_value_to_number (cx, &argv[0]);
  SWFDEC_AS_VALUE_SET_BOOLEAN (retval, isnan (d) ? TRUE : FALSE);
}

static void
swfdec_as_context_parseInt (SwfdecAsContext *cx, SwfdecAsObject *object, 
    guint argc, SwfdecAsValue *argv, SwfdecAsValue *retval)
{
  int i = swfdec_as_value_to_integer (cx, &argv[0]);
  SWFDEC_AS_VALUE_SET_INT (retval, i);
}

static void
swfdec_as_context_init_global (SwfdecAsContext *context, guint version)
{
  SwfdecAsValue val;

  swfdec_as_object_add_function (context->global, SWFDEC_AS_STR_ASSetPropFlags, 0, 
      swfdec_as_context_ASSetPropFlags, 3);
  SWFDEC_AS_VALUE_SET_NUMBER (&val, NAN);
  swfdec_as_object_set_variable (context->global, SWFDEC_AS_STR_NaN, &val);
  SWFDEC_AS_VALUE_SET_NUMBER (&val, HUGE_VAL);
  swfdec_as_object_set_variable (context->global, SWFDEC_AS_STR_Infinity, &val);
  swfdec_as_object_add_function (context->global, SWFDEC_AS_STR_isFinite, 0,
      swfdec_as_context_isFinite, 1);
  swfdec_as_object_add_function (context->global, SWFDEC_AS_STR_isNaN, 0,
      swfdec_as_context_isNaN, 1);
  swfdec_as_object_add_function (context->global, SWFDEC_AS_STR_parseInt, 0,
      swfdec_as_context_parseInt, 1);
}

/**
 * swfdec_as_context_startup:
 * @context: a #SwfdecAsContext
 * @version: Flash version to use
 *
 * Starts up the context. This function must be called before any Actionscript
 * is called on @context. The version is responsible for deciding which native
 * functions and properties are available in the context.
 **/
void
swfdec_as_context_startup (SwfdecAsContext *context, guint version)
{
  g_return_if_fail (SWFDEC_IS_AS_CONTEXT (context));
  g_return_if_fail (context->state == SWFDEC_AS_CONTEXT_NEW);

  context->version = version;
  /* get the necessary objects up to define objects and functions sanely */
  swfdec_as_function_init_context (context, version);
  swfdec_as_object_init_context (context, version);
  /* define the global object and other important ones */
  swfdec_as_context_init_global (context, version);
  swfdec_as_array_init_context (context, version);
  /* define the type objects */
  swfdec_as_number_init_context (context, version);
  swfdec_as_string_init_context (context, version);
  /* define the rest */
  swfdec_as_math_init_context (context, version);

  if (context->state == SWFDEC_AS_CONTEXT_NEW)
    context->state = SWFDEC_AS_CONTEXT_RUNNING;
}

