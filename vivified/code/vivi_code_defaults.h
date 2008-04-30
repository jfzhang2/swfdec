/* Vivified
 * Copyright (C) 2008 Benjamin Otte <otte@gnome.org>
 *                    Pekka Lampila <pekka.lampila@iki.fi>
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

#ifndef DEFAULT_BINARY
#define DEFAULT_BINARY(CapsName, underscore_name, operator_name, bytecode, precedence)
#endif

DEFAULT_BINARY (Add,		add,		"add",	SWFDEC_AS_ACTION_ADD,		VIVI_PRECEDENCE_ADD)
DEFAULT_BINARY (Subtract,	subtract,	"-",	SWFDEC_AS_ACTION_SUBTRACT,	VIVI_PRECEDENCE_ADD)
DEFAULT_BINARY (Multiply,	multiply,	"*",	SWFDEC_AS_ACTION_MULTIPLY,	VIVI_PRECEDENCE_MULTIPLY)
DEFAULT_BINARY (Divide,		divide,		"/",	SWFDEC_AS_ACTION_DIVIDE,	VIVI_PRECEDENCE_MULTIPLY)
DEFAULT_BINARY (Modulo,		modulo,		"%",	SWFDEC_AS_ACTION_MODULO,	VIVI_PRECEDENCE_MULTIPLY)
DEFAULT_BINARY (Equals,		equals,		"==",	SWFDEC_AS_ACTION_EQUALS,	VIVI_PRECEDENCE_EQUALITY)
DEFAULT_BINARY (Less,		less,		"<",	SWFDEC_AS_ACTION_LESS,		VIVI_PRECEDENCE_RELATIONAL)
//DEFAULT_BINARY (LogicalAnd,	logical_and,	"and",	SWFDEC_AS_ACTION_AND,		VIVI_PRECEDENCE_AND)
//DEFAULT_BINARY (LogicalOr,	logical_or,	"or",	SWFDEC_AS_ACTION_OR,		VIVI_PRECEDENCE_OR)
DEFAULT_BINARY (StringEquals,	string_equals,	"eq",	SWFDEC_AS_ACTION_STRING_EQUALS, VIVI_PRECEDENCE_EQUALITY)
DEFAULT_BINARY (StringLess,	string_less,	"lt",	SWFDEC_AS_ACTION_STRING_LESS,	VIVI_PRECEDENCE_RELATIONAL)
DEFAULT_BINARY (Add2,		add2,		"+",	SWFDEC_AS_ACTION_ADD2,		VIVI_PRECEDENCE_ADD)
DEFAULT_BINARY (Less2,		less2,		"<",	SWFDEC_AS_ACTION_LESS2,		VIVI_PRECEDENCE_RELATIONAL)
DEFAULT_BINARY (Equals2,	equals2,	"==",	SWFDEC_AS_ACTION_EQUALS2,	VIVI_PRECEDENCE_EQUALITY)
//DEFAULT_BINARY (BitwiseAnd,	bitwise_and,	"&",	SWFDEC_AS_ACTION_BIT_AND,	VIVI_PRECEDENCE_BINARY_AND)
//DEFAULT_BINARY (BitwiseOr,	bitwise_or,	"|",	SWFDEC_AS_ACTION_BIT_OR,	VIVI_PRECEDENCE_BINARY_OR)
//DEFAULT_BINARY (BitwiseXor,	bitwise_xor,	"^",	SWFDEC_AS_ACTION_BIT_XOR,	VIVI_PRECEDENCE_BINARY_XOR)
//DEFAULT_BINARY (LeftShift,	left_shift,	"<<",	SWFDEC_AS_ACTION_BIT_LSHIFT,	VIVI_PRECEDENCE_SHIFT)
//DEFAULT_BINARY (RightShift,	right_shift,	">>",	SWFDEC_AS_ACTION_BIT_RSHIFT,	VIVI_PRECEDENCE_SHIFT)
//DEFAULT_BINARY (UnsignedRightShift,unsigned_right_shift,">>>",SWFDEC_AS_ACTION_BIT_URSHIFT,VIVI_PRECEDENCE_SHIFT)
DEFAULT_BINARY (StrictEquals,	strict_equals,	"===",	SWFDEC_AS_ACTION_STRICT_EQUALS, VIVI_PRECEDENCE_EQUALITY)
DEFAULT_BINARY (Greater,	greater,	">",	SWFDEC_AS_ACTION_GREATER,	VIVI_PRECEDENCE_RELATIONAL)
DEFAULT_BINARY (StringGreater,	string_greater, "gt",	SWFDEC_AS_ACTION_STRING_GREATER,VIVI_PRECEDENCE_RELATIONAL)
/* other special ones:
DEFAULT_BINARY (And,		and,		"&&",	???,				VIVI_PRECEDENCE_AND)
DEFAULT_BINARY (Or,		or,		"||",	???,				VIVI_PRECEDENCE_AND)
*/

#undef DEFAULT_BINARY

#ifndef DEFAULT_BUILTIN_STATEMENT
#define DEFAULT_BUILTIN_STATEMENT(CapsName, underscore_name, function_name)
#endif

DEFAULT_BUILTIN_STATEMENT (NextFrame,		next_frame,	"nextFrame")
DEFAULT_BUILTIN_STATEMENT (Play,		play,		"play")
DEFAULT_BUILTIN_STATEMENT (PreviousFrame,	previous_frame,	"prevFrame")
DEFAULT_BUILTIN_STATEMENT (Stop,		stop,		"stop")
DEFAULT_BUILTIN_STATEMENT (EndDrag,		end_drag,	"stopDrag")
DEFAULT_BUILTIN_STATEMENT (StopSounds,		stop_sounds,	"stopSounds")
DEFAULT_BUILTIN_STATEMENT (ToggleQuality,	toggle_quality,	"toggleQuality")

#undef DEFAULT_BUILTIN_STATEMENT

#ifndef DEFAULT_BUILTIN_VALUE_STATEMENT
#define DEFAULT_BUILTIN_VALUE_STATEMENT(CapsName, underscore_name, function_name)
#endif

//DEFAULT_BUILTIN_VALUE_STATEMENT (CallFrame,	call_frame,	"callFrame",		SWFDEC_AS_ACTION_CALL)
//DEFAULT_BUILTIN_VALUE_STATEMENT (GotoAndPlay,	goto_and_play,	"gotoAndPlay",		???)
//DEFAULT_BUILTIN_VALUE_STATEMENT (GotoAndStop,	goto_and_stop,	"gotoAndStop",		???)
DEFAULT_BUILTIN_VALUE_STATEMENT (RemoveSprite,	remove_sprite,	"removeMovieClip")
DEFAULT_BUILTIN_VALUE_STATEMENT (SetTarget2,	set_target2,	"setTarget")
DEFAULT_BUILTIN_VALUE_STATEMENT (Trace,		trace,		"trace")

#undef DEFAULT_BUILTIN_VALUE_STATEMENT

#ifndef DEFAULT_BUILTIN_VALUE_CALL
#define DEFAULT_BUILTIN_VALUE_CALL(CapsName, underscore_name, function_name)
#endif

DEFAULT_BUILTIN_VALUE_CALL (AsciiToChar,	ascii_to_char,	"chr")
DEFAULT_BUILTIN_VALUE_CALL (GetVariable,	get_variable,	"eval")
DEFAULT_BUILTIN_VALUE_CALL (ToInteger,		to_integer,	"int")
DEFAULT_BUILTIN_VALUE_CALL (StringLength,	string_length,	"length")
DEFAULT_BUILTIN_VALUE_CALL (ToNumber,		to_number,	"Number")
DEFAULT_BUILTIN_VALUE_CALL (CharToAscii,	char_to_ascii,	"ord")
DEFAULT_BUILTIN_VALUE_CALL (Random,		random,		"random")
DEFAULT_BUILTIN_VALUE_CALL (TargetPath,		target_path,	"targetPath")
DEFAULT_BUILTIN_VALUE_CALL (TypeOf,		type_of,	"typeOf")

#undef DEFAULT_BUILTIN_VALUE_CALL

#ifndef DEFAULT_ASM
#define DEFAULT_ASM(CapsName, underscore_name, bytecode)
#endif

DEFAULT_ASM (End, end, SWFDEC_AS_ACTION_END)
DEFAULT_ASM (NextFrame, next_frame, SWFDEC_AS_ACTION_NEXT_FRAME)
DEFAULT_ASM (PreviousFrame, previous_frame, SWFDEC_AS_ACTION_PREVIOUS_FRAME)
DEFAULT_ASM (Play, play, SWFDEC_AS_ACTION_PLAY)
DEFAULT_ASM (Stop, stop, SWFDEC_AS_ACTION_STOP)
DEFAULT_ASM (ToggleQuality, toggle_quality, SWFDEC_AS_ACTION_TOGGLE_QUALITY)
DEFAULT_ASM (StopSounds, stop_sounds, SWFDEC_AS_ACTION_STOP_SOUNDS)
DEFAULT_ASM (Add, add, SWFDEC_AS_ACTION_ADD)
DEFAULT_ASM (Subtract, subtract, SWFDEC_AS_ACTION_SUBTRACT)
DEFAULT_ASM (Multiply, multiply, SWFDEC_AS_ACTION_MULTIPLY)
DEFAULT_ASM (Divide, divide, SWFDEC_AS_ACTION_DIVIDE)
DEFAULT_ASM (Equals, equals, SWFDEC_AS_ACTION_EQUALS)
DEFAULT_ASM (Less, less, SWFDEC_AS_ACTION_LESS)
DEFAULT_ASM (And, and, SWFDEC_AS_ACTION_AND)
DEFAULT_ASM (Or, or, SWFDEC_AS_ACTION_OR)
DEFAULT_ASM (Not, not, SWFDEC_AS_ACTION_NOT)
DEFAULT_ASM (StringEquals, string_equals, SWFDEC_AS_ACTION_STRING_EQUALS)
DEFAULT_ASM (StringLength, string_length, SWFDEC_AS_ACTION_STRING_LENGTH)
DEFAULT_ASM (StringExtract, string_extract, SWFDEC_AS_ACTION_STRING_EXTRACT)
DEFAULT_ASM (Pop, pop, SWFDEC_AS_ACTION_POP)
DEFAULT_ASM (ToInteger, to_integer, SWFDEC_AS_ACTION_TO_INTEGER)
DEFAULT_ASM (GetVariable, get_variable, SWFDEC_AS_ACTION_GET_VARIABLE)
DEFAULT_ASM (SetVariable, set_variable, SWFDEC_AS_ACTION_SET_VARIABLE)
DEFAULT_ASM (SetTarget2, set_target2, SWFDEC_AS_ACTION_SET_TARGET2)
DEFAULT_ASM (StringAdd, string_add, SWFDEC_AS_ACTION_STRING_ADD)
DEFAULT_ASM (GetProperty, get_property, SWFDEC_AS_ACTION_GET_PROPERTY)
DEFAULT_ASM (SetProperty, set_property, SWFDEC_AS_ACTION_SET_PROPERTY)
DEFAULT_ASM (CloneSprite, clone_sprite, SWFDEC_AS_ACTION_CLONE_SPRITE)
DEFAULT_ASM (RemoveSprite, remove_sprite, SWFDEC_AS_ACTION_REMOVE_SPRITE)
DEFAULT_ASM (Trace, trace, SWFDEC_AS_ACTION_TRACE)
DEFAULT_ASM (StartDrag, start_drag, SWFDEC_AS_ACTION_START_DRAG)
DEFAULT_ASM (EndDrag, end_drag, SWFDEC_AS_ACTION_END_DRAG)
DEFAULT_ASM (StringLess, string_less, SWFDEC_AS_ACTION_STRING_LESS)
DEFAULT_ASM (Throw, throw, SWFDEC_AS_ACTION_THROW)
DEFAULT_ASM (Cast, cast, SWFDEC_AS_ACTION_CAST)
DEFAULT_ASM (Implements, implements, SWFDEC_AS_ACTION_IMPLEMENTS)
DEFAULT_ASM (Random, random, SWFDEC_AS_ACTION_RANDOM)
DEFAULT_ASM (MbStringLength, mb_string_length, SWFDEC_AS_ACTION_MB_STRING_LENGTH)
DEFAULT_ASM (CharToAscii, char_to_ascii, SWFDEC_AS_ACTION_CHAR_TO_ASCII)
DEFAULT_ASM (AsciiToChar, ascii_to_char, SWFDEC_AS_ACTION_ASCII_TO_CHAR)
DEFAULT_ASM (GetTime, get_time, SWFDEC_AS_ACTION_GET_TIME)
DEFAULT_ASM (MbStringExtract, mb_string_extract, SWFDEC_AS_ACTION_MB_STRING_EXTRACT)
DEFAULT_ASM (MbCharToAscii, mb_char_to_ascii, SWFDEC_AS_ACTION_MB_CHAR_TO_ASCII)
DEFAULT_ASM (MbAsciiToChar, mb_ascii_to_char, SWFDEC_AS_ACTION_MB_ASCII_TO_CHAR)
DEFAULT_ASM (Delete, delete, SWFDEC_AS_ACTION_DELETE)
DEFAULT_ASM (Delete2, delete2, SWFDEC_AS_ACTION_DELETE2)
DEFAULT_ASM (DefineLocal, define_local, SWFDEC_AS_ACTION_DEFINE_LOCAL)
DEFAULT_ASM (CallFunction, call_function, SWFDEC_AS_ACTION_CALL_FUNCTION)
DEFAULT_ASM (Return, return, SWFDEC_AS_ACTION_RETURN)
DEFAULT_ASM (Modulo, modulo, SWFDEC_AS_ACTION_MODULO)
DEFAULT_ASM (NewObject, new_object, SWFDEC_AS_ACTION_NEW_OBJECT)
DEFAULT_ASM (DefineLocal2, define_local2, SWFDEC_AS_ACTION_DEFINE_LOCAL2)
DEFAULT_ASM (InitArray, init_array, SWFDEC_AS_ACTION_INIT_ARRAY)
DEFAULT_ASM (InitObject, init_object, SWFDEC_AS_ACTION_INIT_OBJECT)
DEFAULT_ASM (TypeOf, type_of, SWFDEC_AS_ACTION_TYPE_OF)
DEFAULT_ASM (TargetPath, target_path, SWFDEC_AS_ACTION_TARGET_PATH)
DEFAULT_ASM (Enumerate, enumerate, SWFDEC_AS_ACTION_ENUMERATE)
DEFAULT_ASM (Add2, add2, SWFDEC_AS_ACTION_ADD2)
DEFAULT_ASM (Less2, less2, SWFDEC_AS_ACTION_LESS2)
DEFAULT_ASM (Equals2, equals2, SWFDEC_AS_ACTION_EQUALS2)
DEFAULT_ASM (ToNumber, to_number, SWFDEC_AS_ACTION_TO_NUMBER)
DEFAULT_ASM (ToString, to_string, SWFDEC_AS_ACTION_TO_STRING)
DEFAULT_ASM (PushDuplicate, push_duplicate, SWFDEC_AS_ACTION_PUSH_DUPLICATE)
DEFAULT_ASM (Swap, swap, SWFDEC_AS_ACTION_SWAP)
DEFAULT_ASM (GetMember, get_member, SWFDEC_AS_ACTION_GET_MEMBER)
DEFAULT_ASM (SetMember, set_member, SWFDEC_AS_ACTION_SET_MEMBER)
DEFAULT_ASM (Increment, increment, SWFDEC_AS_ACTION_INCREMENT)
DEFAULT_ASM (Decrement, decrement, SWFDEC_AS_ACTION_DECREMENT)
DEFAULT_ASM (CallMethod, call_method, SWFDEC_AS_ACTION_CALL_METHOD)
DEFAULT_ASM (NewMethod, new_method, SWFDEC_AS_ACTION_NEW_METHOD)
DEFAULT_ASM (InstanceOf, instance_of, SWFDEC_AS_ACTION_INSTANCE_OF)
DEFAULT_ASM (Enumerate2, enumerate2, SWFDEC_AS_ACTION_ENUMERATE2)
DEFAULT_ASM (Breakpoint, breakpoint, SWFDEC_AS_ACTION_BREAKPOINT)
DEFAULT_ASM (BitAnd, bit_and, SWFDEC_AS_ACTION_BIT_AND)
DEFAULT_ASM (BitOr, bit_or, SWFDEC_AS_ACTION_BIT_OR)
DEFAULT_ASM (BitXor, bit_xor, SWFDEC_AS_ACTION_BIT_XOR)
DEFAULT_ASM (BitLshift, bit_lshift, SWFDEC_AS_ACTION_BIT_LSHIFT)
DEFAULT_ASM (BitRshift, bit_rshift, SWFDEC_AS_ACTION_BIT_RSHIFT)
DEFAULT_ASM (BitUrshift, bit_urshift, SWFDEC_AS_ACTION_BIT_URSHIFT)
DEFAULT_ASM (StrictEquals, strict_equals, SWFDEC_AS_ACTION_STRICT_EQUALS)
DEFAULT_ASM (Greater, greater, SWFDEC_AS_ACTION_GREATER)
DEFAULT_ASM (StringGreater, string_greater, SWFDEC_AS_ACTION_STRING_GREATER)
DEFAULT_ASM (Extends, extends, SWFDEC_AS_ACTION_EXTENDS)
#if 0
DEFAULT_ASM (GotoFrame, goto_frame, SWFDEC_AS_ACTION_GOTO_FRAME)
DEFAULT_ASM (GetUrl, get_url, SWFDEC_AS_ACTION_GET_URL)
DEFAULT_ASM (StoreRegister, store_register, SWFDEC_AS_ACTION_STORE_REGISTER)
DEFAULT_ASM (ConstantPool, constant_pool, SWFDEC_AS_ACTION_CONSTANT_POOL)
DEFAULT_ASM (StrictMode, strict_mode, SWFDEC_AS_ACTION_STRICT_MODE)
DEFAULT_ASM (WaitForFrame, wait_for_frame, SWFDEC_AS_ACTION_WAIT_FOR_FRAME)
DEFAULT_ASM (SetTarget, set_target, SWFDEC_AS_ACTION_SET_TARGET)
DEFAULT_ASM (GotoLabel, goto_label, SWFDEC_AS_ACTION_GOTO_LABEL)
DEFAULT_ASM (WaitForFrame2, wait_for_frame2, SWFDEC_AS_ACTION_WAIT_FOR_FRAME2)
DEFAULT_ASM (DefineFunction2, define_function2, SWFDEC_AS_ACTION_DEFINE_FUNCTION2)
DEFAULT_ASM (Try, try, SWFDEC_AS_ACTION_TRY)
DEFAULT_ASM (With, with, SWFDEC_AS_ACTION_WITH)
DEFAULT_ASM (Push, push, SWFDEC_AS_ACTION_PUSH)
DEFAULT_ASM (Jump, jump, SWFDEC_AS_ACTION_JUMP)
DEFAULT_ASM (GetUrl2, get_url2, SWFDEC_AS_ACTION_GET_URL2)
DEFAULT_ASM (DefineFunction, define_function, SWFDEC_AS_ACTION_DEFINE_FUNCTION)
DEFAULT_ASM (If, if, SWFDEC_AS_ACTION_IF)
DEFAULT_ASM (Call, call, SWFDEC_AS_ACTION_CALL)
DEFAULT_ASM (GotoFrame2, goto_frame2, SWFDEC_AS_ACTION_GOTO_FRAME2)
#endif

#undef DEFAULT_ASM
