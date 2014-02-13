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
#include <mediascanner/MediaFile.hh>

using namespace mediascanner;

void
video_add_result (UnityResultSet *result_set, const MediaFile &media)
{
    UnityScopeResult result = { 0, };

    const std::string uri = media.getUri();
    result.uri = const_cast<char*>(uri.c_str());
    result.icon_hint = const_cast<char*>("");
    result.category = 0;
    result.result_type = UNITY_RESULT_TYPE_PERSONAL;
    result.mimetype = const_cast<char*>(media.getContentType().c_str());
    result.title = const_cast<char*>(media.getTitle().c_str());
    result.comment = const_cast<char*>("");
    result.dnd_uri = result.uri;
    result.metadata = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);

    int duration = media.getDuration();
    GVariant *variant = g_variant_new_int32 (duration);
    g_hash_table_insert (result.metadata, const_cast<char*>("duration"), g_variant_ref_sink (variant));

    int width = 0; //grl_media_video_get_width (GRL_MEDIA_VIDEO (media));
    if (width > 0) {
        variant = g_variant_new_int32 (width);
        g_hash_table_insert (result.metadata, const_cast<char*>("width"), g_variant_ref_sink (variant));
    }

    int height = 0; //grl_media_video_get_height (GRL_MEDIA_VIDEO (media));
    if (height > 0) {
        variant = g_variant_new_int32 (height);
        g_hash_table_insert (result.metadata, const_cast<char*>("height"), g_variant_ref_sink (variant));
    }

    unity_result_set_add_result (result_set, &result);
    g_hash_table_unref (result.metadata);
}


UnityAbstractPreview *
video_preview (UnityResultPreviewer *previewer, void *user_data)
{
    const char *title = previewer->result.title;
    const char *subtitle = "";
    const char *comment = previewer->result.comment;
    int duration = 0, width = 0, height = 0;

    if (previewer->result.metadata != NULL) {
        GVariant *variant;

        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("width")));
        if (variant) {
            width = g_variant_get_int32 (variant);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("height")));
        if (variant) {
            height = g_variant_get_int32 (variant);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("duration")));
        if (variant) {
            duration = g_variant_get_int32 (variant);
        }
    }

    UnityMoviePreview *preview = unity_movie_preview_new (
        title, subtitle, comment, NULL);
    unity_movie_preview_set_rating (preview, -1.0f, 0);
    unity_preview_set_image_source_uri (
        UNITY_PREVIEW (preview), previewer->result.uri);

    UnityPreviewAction *play_action = unity_preview_action_new (
        "play", _("Play"), NULL);
    unity_preview_add_action (UNITY_PREVIEW (preview), play_action);

    if (width > 0 && height > 0) {
        char *dimensions = g_strdup_printf ("%d*%d", width, height);
        UnityInfoHint *hint = unity_info_hint_new (
            "dimensions", _("Dimensions"), NULL, dimensions);
        g_free (dimensions);
        unity_preview_add_info (UNITY_PREVIEW (preview), hint);
    }

    if (duration > 0) {
        /* add info hint */
    }

    return UNITY_ABSTRACT_PREVIEW (preview);
}


UnityAbstractScope *
video_scope_new (std::shared_ptr<MediaStore> store)
{
    UnitySimpleScope *scope = unity_simple_scope_new ();

    unity_simple_scope_set_group_name (scope, DBUS_NAME);
    unity_simple_scope_set_unique_name (scope, DBUS_VIDEO_PATH);

    /* Set up schema */
    UnitySchema *schema = unity_schema_new ();
    unity_schema_add_field (schema, "duration", "i", UNITY_SCHEMA_FIELD_TYPE_REQUIRED);
    unity_schema_add_field (schema, "width", "i", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_schema_add_field (schema, "height", "i", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_simple_scope_set_schema (scope, schema);

    /* Set up categories */
    UnityCategorySet *categories = unity_category_set_new ();
    GFile *icon_dir = g_file_new_for_path ("/usr/share/icons/unity-icon-theme/places/svg");

    GFile *icon_file = g_file_get_child (icon_dir, "group-videos.svg");
    GIcon *icon = g_file_icon_new (icon_file);
    unity_category_set_add (categories,
                            unity_category_new ("local", _("My Videos"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);
    g_object_unref (icon_file);

    g_object_unref (icon_dir);
    unity_simple_scope_set_category_set (scope, categories);

    /* Set up search */
    ScopeSearchData *search_data = g_new0(ScopeSearchData, 1);
    search_data->store = store;
    search_data->media_type = VideoMedia;
    search_data->add_result = video_add_result;
    setup_search (scope, search_data);
    // XXX: handle cleanup of search_data

    unity_simple_scope_set_preview_func (
        scope, video_preview, NULL, (GDestroyNotify)NULL);

    return UNITY_ABSTRACT_SCOPE (scope);
}