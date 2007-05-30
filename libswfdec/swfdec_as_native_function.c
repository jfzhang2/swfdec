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

#include "swfdec_as_native_function.h"
#include "swfdec_as_context.h"
#include "swfdec_as_frame.h"
#include "swfdec_as_stack.h"
#include "swfdec_debug.h"

G_DEFINE_TYPE (SwfdecAsNativeFunction, swfdec_as_native_function, SWFDEC_TYPE_AS_FUNCTION)

static SwfdecAsFrame *
swfdec_as_native_function_call (SwfdecAsFunction *function)
{
  SwfdecAsNativeFunction *native = SWFDEC_AS_NATIVE_FUNCTION (function);
  SwfdecAsFrame *frame;

  frame = swfdec_as_frame_new_native (SWFDEC_AS_OBJECT (function)->context);
  g_assert (native->name);
  frame->function_name = native->name;
  frame->function = function;
  return frame;
}

static void
swfdec_as_native_function_dispose (GObject *object)
{
  SwfdecAsNativeFunction *function = SWFDEC_AS_NATIVE_FUNCTION (object);

  g_free (function->name);
  function->name = NULL;

  G_OBJECT_CLASS (swfdec_as_native_function_parent_class)->dispose (object);
}

static void
swfdec_as_native_function_class_init (SwfdecAsNativeFunctionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  SwfdecAsFunctionClass *function_class = SWFDEC_AS_FUNCTION_CLASS (klass);

  object_class->dispose = swfdec_as_native_function_dispose;

  function_class->call = swfdec_as_native_function_call;
}

static void
swfdec_as_native_function_init (SwfdecAsNativeFunction *function)
{
}

SwfdecAsFunction *
swfdec_as_native_function_new (SwfdecAsContext *context, const char *name,
    SwfdecAsNative native, guint min_args)
{
  SwfdecAsNativeFunction *nfun;
  SwfdecAsFunction *fun;

  g_return_val_if_fail (SWFDEC_IS_AS_CONTEXT (context), NULL);
  g_return_val_if_fail (native != NULL, NULL);

  fun = swfdec_as_function_create (context, SWFDEC_TYPE_AS_NATIVE_FUNCTION,
      sizeof (SwfdecAsNativeFunction));
  if (fun == NULL)
    return NULL;
  nfun = SWFDEC_AS_NATIVE_FUNCTION (fun);
  nfun->native = native;
  nfun->min_args = min_args;
  nfun->name = g_strdup (name);

  return fun;
}

/**
 * swfdec_as_native_function_set_object_type:
 * @function: a #SwfdecAsNativeFunction
 * @type: required #GType for the this object
 *
 * Sets the required type for the this object to @type. If the this object 
 * isn't of the required type, the function will not be called and its
 * return value will be undefined.
 **/
void
swfdec_as_native_function_set_object_type (SwfdecAsNativeFunction *function, GType type)
{
  GTypeQuery query;

  g_return_if_fail (SWFDEC_IS_AS_NATIVE_FUNCTION (function));
  g_return_if_fail (g_type_is_a (type, SWFDEC_TYPE_AS_OBJECT));

  g_type_query (type, &query);
  function->type = type;
}

/**
 * swfdec_as_native_function_set_construct_type:
 * @function: a #SwfdecAsNativeFunction
 * @type: #GType used when constructing an object with @function
 *
 * Sets the @type to be used when using @function as a constructor. If this is
 * not set, using @function as a constructor will create a #SwfdecAsObject.
 **/
void
swfdec_as_native_function_set_construct_type (SwfdecAsNativeFunction *function, GType type)
{
  GTypeQuery query;

  g_return_if_fail (SWFDEC_IS_AS_NATIVE_FUNCTION (function));
  g_return_if_fail (g_type_is_a (type, SWFDEC_TYPE_AS_OBJECT));

  g_type_query (type, &query);
  function->construct_type = type;
  function->construct_size = query.instance_size;
}

