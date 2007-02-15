/* Swfdec
 * Copyright (C) 2006 Benjamin Otte <otte@gnome.org>
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

#ifndef _SWFDEC_LOADER_INTERNAL_H_
#define _SWFDEC_LOADER_INTERNAL_H_

#include "swfdec_loader.h"
#include "swfdec_loadertarget.h"

G_BEGIN_DECLS


SwfdecLoader *		swfdec_loader_load		(SwfdecLoader *		loader,
							 const char *		url);
void			swfdec_loader_parse		(SwfdecLoader *		loader);
void			swfdec_loader_parse_internal	(SwfdecLoader *		loader);
void			swfdec_loader_set_target	(SwfdecLoader *		loader,
							 SwfdecLoaderTarget *	target);

gboolean		swfdec_urldecode_one		(const char *		string,
							 char **		name,
							 char **		value,
							 const char **		end);
void			swfdec_string_append_urlencoded	(GString *		str,
							 char *			name,
							 char *			value);

G_END_DECLS
#endif
