/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by James Henstridge <james.henstridge@canonical.com>
 *
 */
#include <config.h>
#include "scope.h"

extern "C" int
unity_scope_module_get_version ()
{
    return UNITY_SCOPE_API_VERSION;
}

extern "C" GList *
unity_scope_module_load_scopes (GError **error)
{
    auto store = std::make_shared<MediaStore>(MS_READ_ONLY);
    UnityAbstractScope *music = music_scope_new (store);
    UnityAbstractScope *video = video_scope_new (store);

    return g_list_append (g_list_append(NULL, music), video);
}
