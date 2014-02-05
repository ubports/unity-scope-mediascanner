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

using namespace mediascanner;

extern "C" int
unity_scope_module_get_version ()
{
    return UNITY_SCOPE_API_VERSION;
}

static void
on_mediascanner_invalidation_cb (GDBusConnection *connection,
                                 const gchar *sender_name,
                                 const gchar *object_path,
                                 const gchar *interface_name,
                                 const gchar *signal_name,
                                 GVariant *parameters,
                                 gpointer user_data)
{
    UnityAbstractScope **scopes = (UnityAbstractScope**) user_data;
    UnityAbstractScope *target = NULL;

    gchar *invalidation_name = NULL;
    g_variant_get (parameters, "(s)", &invalidation_name);

    target = g_str_has_suffix (invalidation_name, "music") ? scopes[0] : scopes[1];

    g_free (invalidation_name);

    if (target)
    {
        unity_abstract_scope_results_invalidated (target,
                                                  UNITY_SEARCH_TYPE_DEFAULT);
        unity_abstract_scope_results_invalidated (target,
                                                  UNITY_SEARCH_TYPE_GLOBAL);
    }
}

extern "C" GList *
unity_scope_module_load_scopes (GError **error)
{
    auto store = std::make_shared<MediaStore>(MS_READ_ONLY);
    UnityAbstractScope *music = music_scope_new (store);
    UnityAbstractScope *video = video_scope_new (store);

    UnityAbstractScope **scopes = g_new (UnityAbstractScope*, 2);
    scopes[0] = music;
    scopes[1] = video;
    GDBusConnection *conn = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
    g_dbus_connection_signal_subscribe (conn,
                                        NULL,
                                        "com.canonical.unity.scopes",
                                        "InvalidateResults",
                                        "/com/canonical/unity/scopes",
                                        NULL,
                                        (GDBusSignalFlags) 0,
                                        on_mediascanner_invalidation_cb,
                                        scopes,
                                        g_free);

    return g_list_append (g_list_append(NULL, music), video);
}
