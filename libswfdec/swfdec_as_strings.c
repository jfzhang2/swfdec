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

#define SWFDEC_AS_CONSTANT_STRING(str) "\2" str "\0"
const char swfdec_as_strings[] = 
  SWFDEC_AS_CONSTANT_STRING ("")
  SWFDEC_AS_CONSTANT_STRING ("__proto__")
  SWFDEC_AS_CONSTANT_STRING ("this")
  SWFDEC_AS_CONSTANT_STRING ("code")
  SWFDEC_AS_CONSTANT_STRING ("level")
  SWFDEC_AS_CONSTANT_STRING ("description")
  SWFDEC_AS_CONSTANT_STRING ("status")
  SWFDEC_AS_CONSTANT_STRING ("NetConnection.Connect.Success")
  SWFDEC_AS_CONSTANT_STRING ("onLoad")
  SWFDEC_AS_CONSTANT_STRING ("onEnterFrame")
  SWFDEC_AS_CONSTANT_STRING ("onUnload")
  SWFDEC_AS_CONSTANT_STRING ("onMouseMove")
  SWFDEC_AS_CONSTANT_STRING ("onMouseDown")
  SWFDEC_AS_CONSTANT_STRING ("onMouseUp")
  SWFDEC_AS_CONSTANT_STRING ("onKeyUp")
  SWFDEC_AS_CONSTANT_STRING ("onKeyDown")
  SWFDEC_AS_CONSTANT_STRING ("onData")
  SWFDEC_AS_CONSTANT_STRING ("onPress")
  SWFDEC_AS_CONSTANT_STRING ("onRelease")
  SWFDEC_AS_CONSTANT_STRING ("onReleaseOutside")
  SWFDEC_AS_CONSTANT_STRING ("onRollOver")
  SWFDEC_AS_CONSTANT_STRING ("onRollOut")
  SWFDEC_AS_CONSTANT_STRING ("onDragOver")
  SWFDEC_AS_CONSTANT_STRING ("onDragOut")
  SWFDEC_AS_CONSTANT_STRING ("onConstruct")
  SWFDEC_AS_CONSTANT_STRING ("onStatus")
  SWFDEC_AS_CONSTANT_STRING ("error")
  SWFDEC_AS_CONSTANT_STRING ("NetStream.Buffer.Empty")
  SWFDEC_AS_CONSTANT_STRING ("NetStream.Buffer.Full")
  SWFDEC_AS_CONSTANT_STRING ("NetStream.Buffer.Flush")
  SWFDEC_AS_CONSTANT_STRING ("NetStream.Play.Start")
  SWFDEC_AS_CONSTANT_STRING ("NetStream.Play.Stop")
  SWFDEC_AS_CONSTANT_STRING ("NetStream.Play.StreamNotFound")
  SWFDEC_AS_CONSTANT_STRING ("undefined")
  SWFDEC_AS_CONSTANT_STRING ("null")
  SWFDEC_AS_CONSTANT_STRING ("[object Object]")
  SWFDEC_AS_CONSTANT_STRING ("true")
  SWFDEC_AS_CONSTANT_STRING ("false")
  SWFDEC_AS_CONSTANT_STRING ("_x")
  SWFDEC_AS_CONSTANT_STRING ("_y")
  SWFDEC_AS_CONSTANT_STRING ("_xscale")
  SWFDEC_AS_CONSTANT_STRING ("_yscale")
  SWFDEC_AS_CONSTANT_STRING ("_currentframe")
  SWFDEC_AS_CONSTANT_STRING ("_totalframes")
  SWFDEC_AS_CONSTANT_STRING ("_alpha")
  SWFDEC_AS_CONSTANT_STRING ("_visible")
  SWFDEC_AS_CONSTANT_STRING ("_width")
  SWFDEC_AS_CONSTANT_STRING ("_height")
  SWFDEC_AS_CONSTANT_STRING ("_rotation") 
  SWFDEC_AS_CONSTANT_STRING ("_target")
  SWFDEC_AS_CONSTANT_STRING ("_framesloaded")
  SWFDEC_AS_CONSTANT_STRING ("_name") 
  SWFDEC_AS_CONSTANT_STRING ("_droptarget")
  SWFDEC_AS_CONSTANT_STRING ("_url") 
  SWFDEC_AS_CONSTANT_STRING ("_highquality") 
  SWFDEC_AS_CONSTANT_STRING ("_focusrect") 
  SWFDEC_AS_CONSTANT_STRING ("_soundbuftime") 
  SWFDEC_AS_CONSTANT_STRING ("_quality")
  SWFDEC_AS_CONSTANT_STRING ("_xmouse") 
  SWFDEC_AS_CONSTANT_STRING ("_ymouse")
  SWFDEC_AS_CONSTANT_STRING ("_parent")
  SWFDEC_AS_CONSTANT_STRING ("_root")
  SWFDEC_AS_CONSTANT_STRING ("#ERROR#")
  SWFDEC_AS_CONSTANT_STRING ("number")
  SWFDEC_AS_CONSTANT_STRING ("boolean")
  SWFDEC_AS_CONSTANT_STRING ("string")
  SWFDEC_AS_CONSTANT_STRING ("movieclip")
  SWFDEC_AS_CONSTANT_STRING ("function")
  SWFDEC_AS_CONSTANT_STRING ("object")
  SWFDEC_AS_CONSTANT_STRING ("toString")
  SWFDEC_AS_CONSTANT_STRING ("valueOf")
  SWFDEC_AS_CONSTANT_STRING ("Function")
  SWFDEC_AS_CONSTANT_STRING ("prototype")
  SWFDEC_AS_CONSTANT_STRING ("constructor")
  SWFDEC_AS_CONSTANT_STRING ("Object")
  SWFDEC_AS_CONSTANT_STRING ("hasOwnProperty")
  SWFDEC_AS_CONSTANT_STRING ("NUMERIC")
  SWFDEC_AS_CONSTANT_STRING ("RETURNINDEXEDARRAY")
  SWFDEC_AS_CONSTANT_STRING ("UNIQUESORT")
  SWFDEC_AS_CONSTANT_STRING ("DESCENDING")
  SWFDEC_AS_CONSTANT_STRING ("CASEINSENSITIVE")
  SWFDEC_AS_CONSTANT_STRING ("Array")
  SWFDEC_AS_CONSTANT_STRING ("ASSetPropFlags")
  SWFDEC_AS_CONSTANT_STRING ("0")
  SWFDEC_AS_CONSTANT_STRING ("-Infinity")
  SWFDEC_AS_CONSTANT_STRING ("Infinity")
  SWFDEC_AS_CONSTANT_STRING ("NaN")
  SWFDEC_AS_CONSTANT_STRING ("Number")
  SWFDEC_AS_CONSTANT_STRING ("NAN")
  SWFDEC_AS_CONSTANT_STRING ("MAX_VALUE")
  SWFDEC_AS_CONSTANT_STRING ("MIN_VALUE")
  SWFDEC_AS_CONSTANT_STRING ("NEGATIVE_INFINITY")
  SWFDEC_AS_CONSTANT_STRING ("POSITIVE_INFINITY")
  SWFDEC_AS_CONSTANT_STRING ("[type Object]")
  SWFDEC_AS_CONSTANT_STRING ("startDrag")
  SWFDEC_AS_CONSTANT_STRING ("Mouse")
  SWFDEC_AS_CONSTANT_STRING ("hide")
  SWFDEC_AS_CONSTANT_STRING ("show")
  SWFDEC_AS_CONSTANT_STRING ("addListener")
  SWFDEC_AS_CONSTANT_STRING ("removeListener")
  SWFDEC_AS_CONSTANT_STRING ("MovieClip")
  SWFDEC_AS_CONSTANT_STRING ("attachMovie")
  SWFDEC_AS_CONSTANT_STRING ("duplicateMovieClip")
  SWFDEC_AS_CONSTANT_STRING ("getBytesLoaded")
  SWFDEC_AS_CONSTANT_STRING ("getBytesTotal")
  SWFDEC_AS_CONSTANT_STRING ("getDepth")
  SWFDEC_AS_CONSTANT_STRING ("getNextHighestDepth")
  SWFDEC_AS_CONSTANT_STRING ("getURL")
  SWFDEC_AS_CONSTANT_STRING ("gotoAndPlay")
  SWFDEC_AS_CONSTANT_STRING ("gotoAndStop")
  SWFDEC_AS_CONSTANT_STRING ("hitTest")
  SWFDEC_AS_CONSTANT_STRING ("nextFrame")
  SWFDEC_AS_CONSTANT_STRING ("play")
  SWFDEC_AS_CONSTANT_STRING ("prevFrame")
  SWFDEC_AS_CONSTANT_STRING ("removeMovieClip")
  SWFDEC_AS_CONSTANT_STRING ("stop")
  SWFDEC_AS_CONSTANT_STRING ("stopDrag")
  SWFDEC_AS_CONSTANT_STRING ("swapDepths")
  SWFDEC_AS_CONSTANT_STRING ("super")
  SWFDEC_AS_CONSTANT_STRING ("length")
  SWFDEC_AS_CONSTANT_STRING ("[type Function]")
  SWFDEC_AS_CONSTANT_STRING ("arguments")
  SWFDEC_AS_CONSTANT_STRING (",")
  SWFDEC_AS_CONSTANT_STRING ("registerClass")
  SWFDEC_AS_CONSTANT_STRING ("__constructor__")
  SWFDEC_AS_CONSTANT_STRING ("_global")
  SWFDEC_AS_CONSTANT_STRING ("aa")
  SWFDEC_AS_CONSTANT_STRING ("ab")
  SWFDEC_AS_CONSTANT_STRING ("ba")
  SWFDEC_AS_CONSTANT_STRING ("bb")
  SWFDEC_AS_CONSTANT_STRING ("ga")
  SWFDEC_AS_CONSTANT_STRING ("gb")
  SWFDEC_AS_CONSTANT_STRING ("ra")
  SWFDEC_AS_CONSTANT_STRING ("rb")
  SWFDEC_AS_CONSTANT_STRING ("getRGB")
  SWFDEC_AS_CONSTANT_STRING ("getTransform")
  SWFDEC_AS_CONSTANT_STRING ("setRGB")
  SWFDEC_AS_CONSTANT_STRING ("setTransform")
  SWFDEC_AS_CONSTANT_STRING ("Color")
  SWFDEC_AS_CONSTANT_STRING ("push")
  SWFDEC_AS_CONSTANT_STRING ("parseInt")
  SWFDEC_AS_CONSTANT_STRING ("Math")
  SWFDEC_AS_CONSTANT_STRING ("abs")
  SWFDEC_AS_CONSTANT_STRING ("acos")
  SWFDEC_AS_CONSTANT_STRING ("asin")
  SWFDEC_AS_CONSTANT_STRING ("atan")
  SWFDEC_AS_CONSTANT_STRING ("ceil")
  SWFDEC_AS_CONSTANT_STRING ("cos")
  SWFDEC_AS_CONSTANT_STRING ("exp")
  SWFDEC_AS_CONSTANT_STRING ("floor")
  SWFDEC_AS_CONSTANT_STRING ("log")
  SWFDEC_AS_CONSTANT_STRING ("sin")
  SWFDEC_AS_CONSTANT_STRING ("sqrt")
  SWFDEC_AS_CONSTANT_STRING ("tan")
  SWFDEC_AS_CONSTANT_STRING ("E")
  SWFDEC_AS_CONSTANT_STRING ("LN10")
  SWFDEC_AS_CONSTANT_STRING ("LN2")
  SWFDEC_AS_CONSTANT_STRING ("LOG10E")
  SWFDEC_AS_CONSTANT_STRING ("LOG2E")
  SWFDEC_AS_CONSTANT_STRING ("PI")
  SWFDEC_AS_CONSTANT_STRING ("SQRT1_2")
  SWFDEC_AS_CONSTANT_STRING ("SQRT2")
  SWFDEC_AS_CONSTANT_STRING ("atan2")
  SWFDEC_AS_CONSTANT_STRING ("min")
  SWFDEC_AS_CONSTANT_STRING ("max")
  SWFDEC_AS_CONSTANT_STRING ("pow")
  SWFDEC_AS_CONSTANT_STRING ("random")
  SWFDEC_AS_CONSTANT_STRING ("round")
  SWFDEC_AS_CONSTANT_STRING ("String")
  SWFDEC_AS_CONSTANT_STRING ("fromCharCode")
  SWFDEC_AS_CONSTANT_STRING ("substr")
  SWFDEC_AS_CONSTANT_STRING ("substring")
  SWFDEC_AS_CONSTANT_STRING ("toLowerCase")
  SWFDEC_AS_CONSTANT_STRING ("toUpperCase")
  SWFDEC_AS_CONSTANT_STRING ("isFinite")
  SWFDEC_AS_CONSTANT_STRING ("isNaN")
  SWFDEC_AS_CONSTANT_STRING ("setInterval")
  SWFDEC_AS_CONSTANT_STRING ("clearInterval")
  SWFDEC_AS_CONSTANT_STRING ("escape")
  SWFDEC_AS_CONSTANT_STRING ("unescape")
  SWFDEC_AS_CONSTANT_STRING ("charAt")
  SWFDEC_AS_CONSTANT_STRING ("charCodeAt")
  SWFDEC_AS_CONSTANT_STRING ("NetConnection")
  SWFDEC_AS_CONSTANT_STRING ("connect")
  SWFDEC_AS_CONSTANT_STRING ("createEmptyMovieClip")
  SWFDEC_AS_CONSTANT_STRING ("split")
  SWFDEC_AS_CONSTANT_STRING ("join")
  SWFDEC_AS_CONSTANT_STRING ("pop")
  SWFDEC_AS_CONSTANT_STRING ("shift")
  SWFDEC_AS_CONSTANT_STRING ("unshift")
  SWFDEC_AS_CONSTANT_STRING ("reverse")
  SWFDEC_AS_CONSTANT_STRING ("concat")
  SWFDEC_AS_CONSTANT_STRING ("slice")
  SWFDEC_AS_CONSTANT_STRING ("splice")
  SWFDEC_AS_CONSTANT_STRING ("sort")
  SWFDEC_AS_CONSTANT_STRING ("sortOn")
  SWFDEC_AS_CONSTANT_STRING ("NetStream")
  SWFDEC_AS_CONSTANT_STRING ("pause")
  SWFDEC_AS_CONSTANT_STRING ("seek")
  SWFDEC_AS_CONSTANT_STRING ("setBufferTime")
  SWFDEC_AS_CONSTANT_STRING ("load")
  SWFDEC_AS_CONSTANT_STRING ("XML")
  SWFDEC_AS_CONSTANT_STRING ("Video")
  SWFDEC_AS_CONSTANT_STRING ("attachVideo")
  SWFDEC_AS_CONSTANT_STRING ("clear")
  SWFDEC_AS_CONSTANT_STRING ("time")
  SWFDEC_AS_CONSTANT_STRING ("bytesLoaded")
  SWFDEC_AS_CONSTANT_STRING ("bytesTotal")
  SWFDEC_AS_CONSTANT_STRING ("indexOf")
  SWFDEC_AS_CONSTANT_STRING ("call")
  SWFDEC_AS_CONSTANT_STRING ("Boolean")
  SWFDEC_AS_CONSTANT_STRING ("addProperty")
  SWFDEC_AS_CONSTANT_STRING ("ASnative")
  SWFDEC_AS_CONSTANT_STRING ("_listeners")
  SWFDEC_AS_CONSTANT_STRING ("broadcastMessage")
  SWFDEC_AS_CONSTANT_STRING ("showAll")
  SWFDEC_AS_CONSTANT_STRING ("noBorder")
  SWFDEC_AS_CONSTANT_STRING ("exactFit")
  SWFDEC_AS_CONSTANT_STRING ("noScale")
  SWFDEC_AS_CONSTANT_STRING ("Stage")
  SWFDEC_AS_CONSTANT_STRING ("onResize")
  SWFDEC_AS_CONSTANT_STRING ("getBounds")
  SWFDEC_AS_CONSTANT_STRING ("xMin")
  SWFDEC_AS_CONSTANT_STRING ("xMax")
  SWFDEC_AS_CONSTANT_STRING ("yMin")
  SWFDEC_AS_CONSTANT_STRING ("yMax")
  SWFDEC_AS_CONSTANT_STRING ("close")
  SWFDEC_AS_CONSTANT_STRING ("_bytesLoaded")
  SWFDEC_AS_CONSTANT_STRING ("_bytesTotal")
  SWFDEC_AS_CONSTANT_STRING ("xmlDecl")
  SWFDEC_AS_CONSTANT_STRING ("docTypeDecl")
  SWFDEC_AS_CONSTANT_STRING ("XMLNode")
  SWFDEC_AS_CONSTANT_STRING ("namespaceURI")
  SWFDEC_AS_CONSTANT_STRING ("localName")
  SWFDEC_AS_CONSTANT_STRING ("prefix")
  SWFDEC_AS_CONSTANT_STRING ("previousSibling")
  SWFDEC_AS_CONSTANT_STRING ("parentNode")
  SWFDEC_AS_CONSTANT_STRING ("nodeName")
  SWFDEC_AS_CONSTANT_STRING ("nodeType")
  SWFDEC_AS_CONSTANT_STRING ("nodeValue")
  SWFDEC_AS_CONSTANT_STRING ("nextSibling")
  SWFDEC_AS_CONSTANT_STRING ("lastChild")
  SWFDEC_AS_CONSTANT_STRING ("firstChild")
  SWFDEC_AS_CONSTANT_STRING ("childNodes")
  SWFDEC_AS_CONSTANT_STRING ("cloneNode")
  SWFDEC_AS_CONSTANT_STRING ("removeNode")
  SWFDEC_AS_CONSTANT_STRING ("insertBefore")
  SWFDEC_AS_CONSTANT_STRING ("appendChild")
  SWFDEC_AS_CONSTANT_STRING ("getNamespaceForPrefix")
  SWFDEC_AS_CONSTANT_STRING ("getPrefixForNamespace")
  SWFDEC_AS_CONSTANT_STRING ("hasChildNodes")
  SWFDEC_AS_CONSTANT_STRING ("attributes")
  SWFDEC_AS_CONSTANT_STRING ("loaded")
  SWFDEC_AS_CONSTANT_STRING ("lastIndexOf")
  SWFDEC_AS_CONSTANT_STRING ("hasAudio")
  SWFDEC_AS_CONSTANT_STRING ("hasStreamingAudio")
  SWFDEC_AS_CONSTANT_STRING ("hasStreamingVideo")
  SWFDEC_AS_CONSTANT_STRING ("hasEmbeddedVideo")
  SWFDEC_AS_CONSTANT_STRING ("hasMP3")
  SWFDEC_AS_CONSTANT_STRING ("hasAudioEncoder")
  SWFDEC_AS_CONSTANT_STRING ("hasVideoEncoder")
  SWFDEC_AS_CONSTANT_STRING ("hasAccessibility")
  SWFDEC_AS_CONSTANT_STRING ("hasPrinting")
  SWFDEC_AS_CONSTANT_STRING ("hasScreenPlayback")
  SWFDEC_AS_CONSTANT_STRING ("hasScreenBroadcast")
  SWFDEC_AS_CONSTANT_STRING ("isDebugger")
  SWFDEC_AS_CONSTANT_STRING ("version")
  SWFDEC_AS_CONSTANT_STRING ("manufacturer")
  SWFDEC_AS_CONSTANT_STRING ("screenResolutionX")
  SWFDEC_AS_CONSTANT_STRING ("screenResolutionY")
  SWFDEC_AS_CONSTANT_STRING ("screenDPI")
  SWFDEC_AS_CONSTANT_STRING ("screenColor")
  SWFDEC_AS_CONSTANT_STRING ("pixelAspectRatio")
  SWFDEC_AS_CONSTANT_STRING ("os")
  SWFDEC_AS_CONSTANT_STRING ("language")
  SWFDEC_AS_CONSTANT_STRING ("hasIME")
  SWFDEC_AS_CONSTANT_STRING ("playerType")
  SWFDEC_AS_CONSTANT_STRING ("avHardwareDisable")
  SWFDEC_AS_CONSTANT_STRING ("localFileReadDisable")
  SWFDEC_AS_CONSTANT_STRING ("windowlessDisable")
  SWFDEC_AS_CONSTANT_STRING ("hasTLS")
  SWFDEC_AS_CONSTANT_STRING ("serverString")
  SWFDEC_AS_CONSTANT_STRING ("$version")
  SWFDEC_AS_CONSTANT_STRING ("contentType")
  SWFDEC_AS_CONSTANT_STRING ("application/x-www-form-urlencoded")
  SWFDEC_AS_CONSTANT_STRING ("ignoreWhite")
  SWFDEC_AS_CONSTANT_STRING ("ASconstructor")
  SWFDEC_AS_CONSTANT_STRING ("Date")
  SWFDEC_AS_CONSTANT_STRING ("UTC")
  SWFDEC_AS_CONSTANT_STRING ("getTime")
  SWFDEC_AS_CONSTANT_STRING ("getTimezoneOffset")
  SWFDEC_AS_CONSTANT_STRING ("getMilliseconds")
  SWFDEC_AS_CONSTANT_STRING ("getUTCMilliseconds")
  SWFDEC_AS_CONSTANT_STRING ("getSeconds")
  SWFDEC_AS_CONSTANT_STRING ("getUTCSeconds")
  SWFDEC_AS_CONSTANT_STRING ("getMinutes")
  SWFDEC_AS_CONSTANT_STRING ("getUTCMinutes")
  SWFDEC_AS_CONSTANT_STRING ("getHours")
  SWFDEC_AS_CONSTANT_STRING ("getUTCHours")
  SWFDEC_AS_CONSTANT_STRING ("getDay")
  SWFDEC_AS_CONSTANT_STRING ("getUTCDay")
  SWFDEC_AS_CONSTANT_STRING ("getDate")
  SWFDEC_AS_CONSTANT_STRING ("getUTCDate")
  SWFDEC_AS_CONSTANT_STRING ("getMonth")
  SWFDEC_AS_CONSTANT_STRING ("getUTCMonth")
  SWFDEC_AS_CONSTANT_STRING ("getYear")
  SWFDEC_AS_CONSTANT_STRING ("getUTCYear")
  SWFDEC_AS_CONSTANT_STRING ("getFullYear")
  SWFDEC_AS_CONSTANT_STRING ("getUTCFullYear")
  SWFDEC_AS_CONSTANT_STRING ("setTime")
  SWFDEC_AS_CONSTANT_STRING ("setMilliseconds")
  SWFDEC_AS_CONSTANT_STRING ("setUTCMilliseconds")
  SWFDEC_AS_CONSTANT_STRING ("setSeconds")
  SWFDEC_AS_CONSTANT_STRING ("setUTCSeconds")
  SWFDEC_AS_CONSTANT_STRING ("setMinutes")
  SWFDEC_AS_CONSTANT_STRING ("setUTCMinutes")
  SWFDEC_AS_CONSTANT_STRING ("setHours")
  SWFDEC_AS_CONSTANT_STRING ("setUTCHours")
  SWFDEC_AS_CONSTANT_STRING ("setDate")
  SWFDEC_AS_CONSTANT_STRING ("setUTCDate")
  SWFDEC_AS_CONSTANT_STRING ("setMonth")
  SWFDEC_AS_CONSTANT_STRING ("setUTCMonth")
  SWFDEC_AS_CONSTANT_STRING ("setYear")
  SWFDEC_AS_CONSTANT_STRING ("setUTCYear")
  SWFDEC_AS_CONSTANT_STRING ("setFullYear")
  SWFDEC_AS_CONSTANT_STRING ("setUTCFullYear")
  SWFDEC_AS_CONSTANT_STRING ("target")
  SWFDEC_AS_CONSTANT_STRING ("isPropertyEnumerable")
  SWFDEC_AS_CONSTANT_STRING ("watch")
  SWFDEC_AS_CONSTANT_STRING ("unwatch")
  SWFDEC_AS_CONSTANT_STRING ("apply")
  SWFDEC_AS_CONSTANT_STRING ("isPrototypeOf")
  SWFDEC_AS_CONSTANT_STRING ("_typewriter")
  SWFDEC_AS_CONSTANT_STRING ("_sans")
  SWFDEC_AS_CONSTANT_STRING ("_serif")
  SWFDEC_AS_CONSTANT_STRING ("align")
  SWFDEC_AS_CONSTANT_STRING ("left")
  SWFDEC_AS_CONSTANT_STRING ("right")
  SWFDEC_AS_CONSTANT_STRING ("center")
  SWFDEC_AS_CONSTANT_STRING ("justify")
  /* add more here */
;

#ifdef COMPUTE_SWFDEC_AS_STRINGS_H
/* This code creates the offset */
#include <stdio.h>
#include <string.h>

int
main (int argc, char **argv)
{
  const char *cur = swfdec_as_strings;
  char *name, *s;

  printf ("/* This is a generated file. See swfdec_as_strings.c for details. */\n");
  printf ("#ifndef _SWFDEC_AS_STRINGS_H_\n");
  printf ("#define _SWFDEC_AS_STRINGS_H_\n");
  printf ("\n");
  printf ("extern const char swfdec_as_strings[];\n");
  printf ("\n");
  while (*cur != 0) {
    cur++;
    if (!strcmp (cur, "")) {
      name = strdup ("EMPTY");
    } else if (!strcmp (cur, ",")) {
      name = strdup ("COMMA");
    } else {
      s = name = strdup (cur);
      while (*s) {
	if (!isalnum (*s))
	  *s = '_';
	s++;
      }
    }
    printf ("#define SWFDEC_AS_STR_%s &swfdec_as_strings[%u]\n", name, 
	(unsigned int) (cur - swfdec_as_strings));
    free (name);
    cur += strlen (cur) + 1;
  }
  printf ("\n");
  printf ("#endif /* _SWFDEC_AS_STRINGS_H_ */\n");
  return 0;
}
#endif
