/* SwfdecAs
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

#ifndef _SWFDEC_AS_TYPES_H_
#define _SWFDEC_AS_TYPES_H_

#include <glib.h>

G_BEGIN_DECLS

/* fundamental types */
typedef enum {
  SWFDEC_AS_TYPE_UNDEFINED = 0,
  SWFDEC_AS_TYPE_BOOLEAN,
  SWFDEC_AS_TYPE_INT, /* unimplemented, but reserved if someone wants it */
  SWFDEC_AS_TYPE_NUMBER,
  SWFDEC_AS_TYPE_STRING,
  SWFDEC_AS_TYPE_NULL,
  SWFDEC_AS_TYPE_OBJECT
} SwfdecAsType;

typedef struct _SwfdecAsArray SwfdecAsArray;
typedef struct _SwfdecAsContext SwfdecAsContext;
typedef struct _SwfdecAsFrame SwfdecAsFrame;
typedef struct _SwfdecAsFunction SwfdecAsFunction;
typedef struct _SwfdecAsObject SwfdecAsObject;
typedef struct _SwfdecAsStack SwfdecAsStack;
typedef struct _SwfdecAsValue SwfdecAsValue;
typedef void (* SwfdecAsNativeCall) (SwfdecAsContext *context, SwfdecAsObject *thisp, guint argc, SwfdecAsValue *argv, SwfdecAsValue *retval);

/* IMPORTANT: a SwfdecAsValue memset to 0 is a valid undefined value */
struct _SwfdecAsValue {
  SwfdecAsType		type;
  union {
    gboolean		boolean;
    double		number;
    const char *	string;
    SwfdecAsObject *	object;
  } value;
};

#define SWFDEC_IS_AS_VALUE(val) ((val)->type <= SWFDEC_TYPE_AS_OBJECT)

#define SWFDEC_AS_VALUE_IS_UNDEFINED(val) ((val)->type == SWFDEC_AS_TYPE_UNDEFINED)
#define SWFDEC_AS_VALUE_SET_UNDEFINED(val) (val)->type = SWFDEC_AS_TYPE_UNDEFINED

#define SWFDEC_AS_VALUE_IS_BOOLEAN(val) ((val)->type == SWFDEC_AS_TYPE_BOOLEAN)
#define SWFDEC_AS_VALUE_GET_BOOLEAN(val) ((val)->value.boolean)
#define SWFDEC_AS_VALUE_SET_BOOLEAN(val,b) G_STMT_START { \
  SwfdecAsValue *__val = (val); \
  (__val)->type = SWFDEC_AS_TYPE_BOOLEAN; \
  (__val)->value.boolean = b; \
} G_STMT_END

#define SWFDEC_AS_VALUE_IS_NUMBER(val) ((val)->type == SWFDEC_AS_TYPE_NUMBER)
#define SWFDEC_AS_VALUE_GET_NUMBER(val) ((val)->value.number)
#define SWFDEC_AS_VALUE_SET_NUMBER(val,d) G_STMT_START { \
  SwfdecAsValue *__val = (val); \
  (__val)->type = SWFDEC_AS_TYPE_NUMBER; \
  (__val)->value.number = d; \
} G_STMT_END

#define SWFDEC_AS_VALUE_IS_STRING(val) ((val)->type == SWFDEC_AS_TYPE_STRING)
#define SWFDEC_AS_VALUE_GET_STRING(val) ((val)->value.string)
#define SWFDEC_AS_VALUE_SET_STRING(val,s) G_STMT_START { \
  SwfdecAsValue *__val = (val); \
  (__val)->type = SWFDEC_AS_TYPE_STRING; \
  (__val)->value.string = s; \
} G_STMT_END

#define SWFDEC_AS_VALUE_IS_NULL(val) ((val)->type == SWFDEC_AS_TYPE_NULL)
#define SWFDEC_AS_VALUE_SET_NULL(val) (val)->type = SWFDEC_AS_TYPE_NULL

#define SWFDEC_AS_VALUE_IS_OBJECT(val) ((val)->type == SWFDEC_AS_TYPE_OBJECT)
#define SWFDEC_AS_VALUE_GET_OBJECT(val) ((val)->value.object)
#define SWFDEC_AS_VALUE_SET_OBJECT(val,o) G_STMT_START { \
  SwfdecAsValue *__val = (val); \
  g_assert (o != NULL); \
  (__val)->type = SWFDEC_AS_TYPE_OBJECT; \
  (__val)->value.object = o; \
} G_STMT_END


/* List of static strings that are required all the time */
extern const char *swfdec_as_strings[];
#define SWFDEC_AS_STR_CONSTANT(n) (&swfdec_as_strings[(n)][1])

#define SWFDEC_AS_STR_EMPTY		SWFDEC_AS_STR_CONSTANT(0)
#define SWFDEC_AS_STR_PROTO		SWFDEC_AS_STR_CONSTANT(1)
#define SWFDEC_AS_STR_THIS		SWFDEC_AS_STR_CONSTANT(2)
#define SWFDEC_AS_STR_CODE		SWFDEC_AS_STR_CONSTANT(3)
#define SWFDEC_AS_STR_LEVEL		SWFDEC_AS_STR_CONSTANT(4)
#define SWFDEC_AS_STR_DESCRIPTION	SWFDEC_AS_STR_CONSTANT(5)
#define SWFDEC_AS_STR_STATUS		SWFDEC_AS_STR_CONSTANT(6)
#define SWFDEC_AS_STR_SUCCESS		SWFDEC_AS_STR_CONSTANT(7)
#define SWFDEC_AS_STR_NET_CONNECTION_CONNECT_SUCCESS SWFDEC_AS_STR_CONSTANT(8)
#define SWFDEC_AS_STR_ON_LOAD		SWFDEC_AS_STR_CONSTANT(9)
#define SWFDEC_AS_STR_ON_ENTER_FRAME	SWFDEC_AS_STR_CONSTANT(10)
#define SWFDEC_AS_STR_ON_UNLOAD		SWFDEC_AS_STR_CONSTANT(11)
#define SWFDEC_AS_STR_ON_MOUSE_MOVE	SWFDEC_AS_STR_CONSTANT(12)
#define SWFDEC_AS_STR_ON_MOUSE_DOWN	SWFDEC_AS_STR_CONSTANT(13)
#define SWFDEC_AS_STR_ON_MOUSE_UP	SWFDEC_AS_STR_CONSTANT(14)
#define SWFDEC_AS_STR_ON_KEY_UP		SWFDEC_AS_STR_CONSTANT(15)
#define SWFDEC_AS_STR_ON_KEY_DOWN	SWFDEC_AS_STR_CONSTANT(16)
#define SWFDEC_AS_STR_ON_DATA		SWFDEC_AS_STR_CONSTANT(17)
#define SWFDEC_AS_STR_ON_PRESS		SWFDEC_AS_STR_CONSTANT(18)
#define SWFDEC_AS_STR_ON_RELEASE	SWFDEC_AS_STR_CONSTANT(19)
#define SWFDEC_AS_STR_ON_RELEASE_OUTSIDE SWFDEC_AS_STR_CONSTANT(20)
#define SWFDEC_AS_STR_ON_ROLL_OVER	SWFDEC_AS_STR_CONSTANT(21)
#define SWFDEC_AS_STR_ON_ROLL_OUT	SWFDEC_AS_STR_CONSTANT(22)
#define SWFDEC_AS_STR_ON_DRAG_OVER	SWFDEC_AS_STR_CONSTANT(23)
#define SWFDEC_AS_STR_ON_DRAG_OUT	SWFDEC_AS_STR_CONSTANT(24)
#define SWFDEC_AS_STR_ON_CONSTRUCT	SWFDEC_AS_STR_CONSTANT(25)
#define SWFDEC_AS_STR_ON_STATUS		SWFDEC_AS_STR_CONSTANT(26)
#define SWFDEC_AS_STR_ERROR		SWFDEC_AS_STR_CONSTANT(27)
#define SWFDEC_AS_STR_NETSTREAM_BUFFER_EMPTY SWFDEC_AS_STR_CONSTANT(28)
#define SWFDEC_AS_STR_NETSTREAM_BUFFER_FULL SWFDEC_AS_STR_CONSTANT(29)
#define SWFDEC_AS_STR_NETSTREAM_BUFFER_FLUSH SWFDEC_AS_STR_CONSTANT(30)
#define SWFDEC_AS_STR_NETSTREAM_PLAY_START SWFDEC_AS_STR_CONSTANT(31)
#define SWFDEC_AS_STR_NETSTREAM_PLAY_STOP SWFDEC_AS_STR_CONSTANT(32)
#define SWFDEC_AS_STR_NETSTREAM_PLAY_STREAMNOTFOUND SWFDEC_AS_STR_CONSTANT(33)
#define SWFDEC_AS_STR_UNDEFINED		SWFDEC_AS_STR_CONSTANT(34)
#define SWFDEC_AS_STR_NULL		SWFDEC_AS_STR_CONSTANT(35)
#define SWFDEC_AS_STR_OBJECT_OBJECT	SWFDEC_AS_STR_CONSTANT(36)
#define SWFDEC_AS_STR_TRUE		SWFDEC_AS_STR_CONSTANT(37)
#define SWFDEC_AS_STR_FALSE		SWFDEC_AS_STR_CONSTANT(38)
#define SWFDEC_AS_STR__X		SWFDEC_AS_STR_CONSTANT(39)
#define SWFDEC_AS_STR__Y		SWFDEC_AS_STR_CONSTANT(40)
#define SWFDEC_AS_STR__XSCALE		SWFDEC_AS_STR_CONSTANT(41)
#define SWFDEC_AS_STR__YSCALE		SWFDEC_AS_STR_CONSTANT(42)
#define SWFDEC_AS_STR__CURRENTFRAME	SWFDEC_AS_STR_CONSTANT(43)
#define SWFDEC_AS_STR__TOTALFRAMES	SWFDEC_AS_STR_CONSTANT(44)
#define SWFDEC_AS_STR__ALPHA		SWFDEC_AS_STR_CONSTANT(45)
#define SWFDEC_AS_STR__VISIBLE		SWFDEC_AS_STR_CONSTANT(46)
#define SWFDEC_AS_STR__WIDTH		SWFDEC_AS_STR_CONSTANT(47)
#define SWFDEC_AS_STR__HEIGHT		SWFDEC_AS_STR_CONSTANT(48)
#define SWFDEC_AS_STR__ROTATION		SWFDEC_AS_STR_CONSTANT(49)
#define SWFDEC_AS_STR__TARGET		SWFDEC_AS_STR_CONSTANT(50)
#define SWFDEC_AS_STR__FRAMESLOADED	SWFDEC_AS_STR_CONSTANT(51)
#define SWFDEC_AS_STR__NAME		SWFDEC_AS_STR_CONSTANT(52)
#define SWFDEC_AS_STR__DROPTARGET	SWFDEC_AS_STR_CONSTANT(53)
#define SWFDEC_AS_STR__URL		SWFDEC_AS_STR_CONSTANT(54)
#define SWFDEC_AS_STR__HIGHQUALITY	SWFDEC_AS_STR_CONSTANT(55)
#define SWFDEC_AS_STR__FOCUSRECT	SWFDEC_AS_STR_CONSTANT(56)
#define SWFDEC_AS_STR__SOUNDBUFTIME	SWFDEC_AS_STR_CONSTANT(57)
#define SWFDEC_AS_STR__QUALITY		SWFDEC_AS_STR_CONSTANT(58)
#define SWFDEC_AS_STR__XMOUSE		SWFDEC_AS_STR_CONSTANT(59)
#define SWFDEC_AS_STR__YMOUSE		SWFDEC_AS_STR_CONSTANT(60)
#define SWFDEC_AS_STR_HASH_ERROR	SWFDEC_AS_STR_CONSTANT(61)
#define SWFDEC_AS_STR_NUMBER		SWFDEC_AS_STR_CONSTANT(62)
#define SWFDEC_AS_STR_BOOLEAN		SWFDEC_AS_STR_CONSTANT(63)
#define SWFDEC_AS_STR_STRING		SWFDEC_AS_STR_CONSTANT(64)
#define SWFDEC_AS_STR_MOVIECLIP		SWFDEC_AS_STR_CONSTANT(65)
#define SWFDEC_AS_STR_FUNCTION		SWFDEC_AS_STR_CONSTANT(66)
#define SWFDEC_AS_STR_OBJECT		SWFDEC_AS_STR_CONSTANT(67)
#define SWFDEC_AS_STR_TOSTRING		SWFDEC_AS_STR_CONSTANT(68)
#define SWFDEC_AS_STR_VALUEOF		SWFDEC_AS_STR_CONSTANT(69)

/* all existing actions */
typedef enum {
  SWFDEC_AS_ACTION_NEXT_FRAME = 0x04,
  SWFDEC_AS_ACTION_PREVIOUS_FRAME = 0x05,
  SWFDEC_AS_ACTION_PLAY = 0x06,
  SWFDEC_AS_ACTION_STOP = 0x07,
  SWFDEC_AS_ACTION_TOGGLE_QUALITY = 0x08,
  SWFDEC_AS_ACTION_STOP_SOUNDS = 0x09,
  SWFDEC_AS_ACTION_ADD = 0x0A,
  SWFDEC_AS_ACTION_SUBTRACT = 0x0B,
  SWFDEC_AS_ACTION_MULTIPLY = 0x0C,
  SWFDEC_AS_ACTION_DIVIDE = 0x0D,
  SWFDEC_AS_ACTION_EQUALS = 0x0E,
  SWFDEC_AS_ACTION_LESS = 0x0F,
  SWFDEC_AS_ACTION_AND = 0x10,
  SWFDEC_AS_ACTION_OR = 0x11,
  SWFDEC_AS_ACTION_NOT = 0x12,
  SWFDEC_AS_ACTION_STRING_EQUALS = 0x13,
  SWFDEC_AS_ACTION_STRING_LENGTH = 0x14,
  SWFDEC_AS_ACTION_STRING_EXTRACT = 0x15,
  SWFDEC_AS_ACTION_POP = 0x17,
  SWFDEC_AS_ACTION_TO_INTEGER = 0x18,
  SWFDEC_AS_ACTION_GET_VARIABLE = 0x1C,
  SWFDEC_AS_ACTION_SET_VARIABLE = 0x1D,
  SWFDEC_AS_ACTION_SET_TARGET2 = 0x20,
  SWFDEC_AS_ACTION_STRING_ADD = 0x21,
  SWFDEC_AS_ACTION_GET_PROPERTY = 0x22,
  SWFDEC_AS_ACTION_SET_PROPERTY = 0x23,
  SWFDEC_AS_ACTION_CLONE_SPRITE = 0x24,
  SWFDEC_AS_ACTION_REMOVE_SPRITE = 0x25,
  SWFDEC_AS_ACTION_TRACE = 0x26,
  SWFDEC_AS_ACTION_START_DRAG = 0x27,
  SWFDEC_AS_ACTION_END_DRAG = 0x28,
  SWFDEC_AS_ACTION_STRING_LESS = 0x29,
  SWFDEC_AS_ACTION_THROW = 0x2A,
  SWFDEC_AS_ACTION_CAST = 0x2B,
  SWFDEC_AS_ACTION_IMPLEMENTS = 0x2C,
  SWFDEC_AS_ACTION_RANDOM = 0x30,
  SWFDEC_AS_ACTION_MB_STRING_LENGTH = 0x31,
  SWFDEC_AS_ACTION_CHAR_TO_ASCII = 0x32,
  SWFDEC_AS_ACTION_ASCII_TO_CHAR = 0x33,
  SWFDEC_AS_ACTION_GET_TIME = 0x34,
  SWFDEC_AS_ACTION_MB_STRING_EXTRACT = 0x35,
  SWFDEC_AS_ACTION_MB_CHAR_TO_ASCII = 0x36,
  SWFDEC_AS_ACTION_MB_ASCII_TO_CHAR = 0x37,
  SWFDEC_AS_ACTION_DELETE = 0x3A,
  SWFDEC_AS_ACTION_DELETE2 = 0x3B,
  SWFDEC_AS_ACTION_DEFINE_LOCAL = 0x3C,
  SWFDEC_AS_ACTION_CALL_FUNCTION = 0x3D,
  SWFDEC_AS_ACTION_RETURN = 0x3E,
  SWFDEC_AS_ACTION_MODULO = 0x3F,
  SWFDEC_AS_ACTION_NEW_OBJECT = 0x40,
  SWFDEC_AS_ACTION_DEFINE_LOCAL2 = 0x41,
  SWFDEC_AS_ACTION_INIT_ARRAY = 0x42,
  SWFDEC_AS_ACTION_INIT_OBJECT = 0x43,
  SWFDEC_AS_ACTION_TYPE_OF = 0x44,
  SWFDEC_AS_ACTION_TARGET_PATH = 0x45,
  SWFDEC_AS_ACTION_ENUMERATE = 0x46,
  SWFDEC_AS_ACTION_ADD2 = 0x47,
  SWFDEC_AS_ACTION_LESS2 = 0x48,
  SWFDEC_AS_ACTION_EQUALS2 = 0x49,
  SWFDEC_AS_ACTION_TO_NUMBER = 0x4A,
  SWFDEC_AS_ACTION_TO_STRING = 0x4B,
  SWFDEC_AS_ACTION_PUSH_DUPLICATE = 0x4C,
  SWFDEC_AS_ACTION_SWAP = 0x4D,
  SWFDEC_AS_ACTION_GET_MEMBER = 0x4E,
  SWFDEC_AS_ACTION_SET_MEMBER = 0x4F,
  SWFDEC_AS_ACTION_INCREMENT = 0x50,
  SWFDEC_AS_ACTION_DECREMENT = 0x51,
  SWFDEC_AS_ACTION_CALL_METHOD = 0x52,
  SWFDEC_AS_ACTION_NEW_METHOD = 0x53,
  SWFDEC_AS_ACTION_INSTANCE_OF = 0x54,
  SWFDEC_AS_ACTION_ENUMERATE2 = 0x55,
  SWFDEC_AS_ACTION_BIT_AND = 0x60,
  SWFDEC_AS_ACTION_BIT_OR = 0x61,
  SWFDEC_AS_ACTION_BIT_XOR = 0x62,
  SWFDEC_AS_ACTION_BIT_LSHIFT = 0x63,
  SWFDEC_AS_ACTION_BIT_RSHIFT = 0x64,
  SWFDEC_AS_ACTION_BIT_URSHIFT = 0x65,
  SWFDEC_AS_ACTION_STRICT_EQUALS = 0x66,
  SWFDEC_AS_ACTION_GREATER = 0x67,
  SWFDEC_AS_ACTION_STRING_GREATER = 0x68,
  SWFDEC_AS_ACTION_EXTENDS = 0x69,
  SWFDEC_AS_ACTION_GOTO_FRAME = 0x81,
  SWFDEC_AS_ACTION_GET_URL = 0x83,
  SWFDEC_AS_ACTION_STORE_REGISTER = 0x87,
  SWFDEC_AS_ACTION_CONSTANT_POOL = 0x88,
  SWFDEC_AS_ACTION_WAIT_FOR_FRAME = 0x8A,
  SWFDEC_AS_ACTION_SET_TARGET = 0x8B,
  SWFDEC_AS_ACTION_GOTO_LABEL = 0x8C,
  SWFDEC_AS_ACTION_WAIT_FOR_FRAME2 = 0x8D,
  SWFDEC_AS_ACTION_DEFINE_FUNCTION2 = 0x8E,
  SWFDEC_AS_ACTION_TRY = 0x8F,
  SWFDEC_AS_ACTION_WITH = 0x94,
  SWFDEC_AS_ACTION_PUSH = 0x96,
  SWFDEC_AS_ACTION_JUMP = 0x99,
  SWFDEC_AS_ACTION_GET_URL2 = 0x9A,
  SWFDEC_AS_ACTION_DEFINE_FUNCTION = 0x9B,
  SWFDEC_AS_ACTION_IF = 0x9D,
  SWFDEC_AS_ACTION_CALL = 0x9E,
  SWFDEC_AS_ACTION_GOTO_FRAME2 = 0x9F
} SwfdecAsAction;

gboolean	swfdec_as_value_to_boolean	(SwfdecAsContext *	context,
						 const SwfdecAsValue *	value);
int		swfdec_as_value_to_integer	(SwfdecAsContext *	context,
						 const SwfdecAsValue *	value);
double		swfdec_as_value_to_number	(SwfdecAsContext *	context,
						 const SwfdecAsValue *	value);
SwfdecAsObject *swfdec_as_value_to_object	(SwfdecAsContext *	context,
						 const SwfdecAsValue *	value);
const char *	swfdec_as_value_to_printable	(SwfdecAsContext *	context,
						 const SwfdecAsValue *	value);
const char *	swfdec_as_value_to_string	(SwfdecAsContext *	context,
						 const SwfdecAsValue *	value);


G_END_DECLS
#endif
