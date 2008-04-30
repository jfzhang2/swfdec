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

#include "vivi_code_number.h"
#include "vivi_code_printer.h"
#include "vivi_code_assembler.h"
#include "vivi_code_asm_push.h"

G_DEFINE_TYPE (ViviCodeNumber, vivi_code_number, VIVI_TYPE_CODE_CONSTANT)

static void
vivi_code_number_print (ViviCodeToken *token, ViviCodePrinter *printer)
{
  ViviCodeNumber *number = VIVI_CODE_NUMBER (token);
  char s[G_ASCII_DTOSTR_BUF_SIZE];

  g_ascii_dtostr (s, G_ASCII_DTOSTR_BUF_SIZE, number->value);
  vivi_code_printer_print (printer, s);
}

static void
vivi_code_number_compile (ViviCodeToken *token, ViviCodeAssembler *assembler)
{
  ViviCodeNumber *number = VIVI_CODE_NUMBER (token);
  ViviCodeAsm *code;

  code = vivi_code_asm_push_new ();
  vivi_code_asm_push_add_double (VIVI_CODE_ASM_PUSH (code), number->value);
  vivi_code_assembler_add_code (assembler, code);
  g_object_unref (code);
}

static void
vivi_code_number_class_init (ViviCodeNumberClass *klass)
{
  ViviCodeTokenClass *token_class = VIVI_CODE_TOKEN_CLASS (klass);

  token_class->print = vivi_code_number_print;
  token_class->compile = vivi_code_number_compile;
}

static void
vivi_code_number_init (ViviCodeNumber *number)
{
}

ViviCodeValue *
vivi_code_number_new (double value)
{
  ViviCodeNumber *number;

  number = g_object_new (VIVI_TYPE_CODE_NUMBER, NULL);
  number->value = value;

  return VIVI_CODE_VALUE (number);
}

double
vivi_code_number_get_value (ViviCodeNumber *number)
{
  g_return_val_if_fail (VIVI_IS_CODE_NUMBER (number), 0.0);

  return number->value;
}

