/* Vivified
 * Copyright (C) 2008 Benjamin Otte <otte@gnome.org>
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

#include "vivi_code_trace.h"
#include "vivi_code_printer.h"
#include "vivi_code_compiler.h"

G_DEFINE_TYPE (ViviCodeTrace, vivi_code_trace, VIVI_TYPE_CODE_STATEMENT)

static void
vivi_code_trace_dispose (GObject *object)
{
  ViviCodeTrace *trace = VIVI_CODE_TRACE (object);

  g_object_unref (trace->value);

  G_OBJECT_CLASS (vivi_code_trace_parent_class)->dispose (object);
}

static void
vivi_code_trace_print (ViviCodeToken *token, ViviCodePrinter *printer)
{
  ViviCodeTrace *trace = VIVI_CODE_TRACE (token);

  vivi_code_printer_print (printer, "trace (");
  vivi_code_printer_print_token (printer, VIVI_CODE_TOKEN (trace->value));
  vivi_code_printer_print (printer, ");");
  vivi_code_printer_new_line (printer, FALSE);
}

static void
vivi_code_trace_compile (ViviCodeToken *token, ViviCodeCompiler *compiler)
{
  ViviCodeTrace *trace = VIVI_CODE_TRACE (token);

  vivi_code_compiler_add_action (compiler, SWFDEC_AS_ACTION_TRACE);

  vivi_code_compiler_compile_value (compiler, trace->value);

  vivi_code_compiler_end_action (compiler);
}

static void
vivi_code_trace_class_init (ViviCodeTraceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ViviCodeTokenClass *token_class = VIVI_CODE_TOKEN_CLASS (klass);

  object_class->dispose = vivi_code_trace_dispose;

  token_class->print = vivi_code_trace_print;
  token_class->compile = vivi_code_trace_compile;
}

static void
vivi_code_trace_init (ViviCodeTrace *token)
{
}

ViviCodeStatement *
vivi_code_trace_new (ViviCodeValue *value)
{
  ViviCodeTrace *trace;

  g_return_val_if_fail (VIVI_IS_CODE_VALUE (value), NULL);

  trace = g_object_new (VIVI_TYPE_CODE_TRACE, NULL);
  trace->value = g_object_ref (value);

  return VIVI_CODE_STATEMENT (trace);
}

