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

UnityFilterSet *
music_get_filters (void)
{
    UnityFilterSet *filters = unity_filter_set_new ();
    UnityMultiRangeFilter *filter = unity_multi_range_filter_new ("decade", _("Decade"), NULL, FALSE);
    unity_options_filter_add_option (
        UNITY_OPTIONS_FILTER (filter), "0", _("Old"), NULL);
    unity_options_filter_add_option (
        UNITY_OPTIONS_FILTER (filter), "1960", _("60s"), NULL);
    unity_options_filter_add_option (
        UNITY_OPTIONS_FILTER (filter), "1970", _("70s"), NULL);
    unity_options_filter_add_option (
        UNITY_OPTIONS_FILTER (filter), "1980", _("80s"), NULL);
    unity_options_filter_add_option (
        UNITY_OPTIONS_FILTER (filter), "1990", _("90s"), NULL);
    unity_options_filter_add_option (
        UNITY_OPTIONS_FILTER (filter), "2000", _("00s"), NULL);
    unity_options_filter_add_option (
        UNITY_OPTIONS_FILTER (filter), "2010", _("10s"), NULL);
    unity_filter_set_add (filters, UNITY_FILTER (filter));

    return filters;
}

#if 0
static gboolean
get_decade_filter (UnityFilterSet *filter_state, int *min_year, int *max_year)
{
    UnityFilter *filter = unity_filter_set_get_filter_by_id (
        filter_state, "decade");

    if (filter == NULL || !unity_filter_get_filtering (filter))
        return FALSE;

    UnityFilterOption *option = unity_multi_range_filter_get_first_active (
        UNITY_MULTI_RANGE_FILTER (filter));
    if (option == NULL)
        return FALSE;
    *min_year = g_strtod (unity_filter_option_get_id (option), 0);
    g_object_unref (option);

    option = unity_multi_range_filter_get_last_active (
        UNITY_MULTI_RANGE_FILTER (filter));
    if (option == NULL)
        return FALSE;
    *max_year = g_strtod (unity_filter_option_get_id (option), 0) + 9;
    g_object_unref (option);

    return TRUE;
}

void
music_apply_filters (UnityFilterSet *filter_state, GrlOperationOptions *options)
{
    int min_year, max_year;
    if (get_decade_filter (filter_state, &min_year, &max_year)) {
        GValue min_val = G_VALUE_INIT;
        GValue max_val = G_VALUE_INIT;
        GDateTime *min_date = g_date_time_new_utc (min_year, 1, 1, 0, 0, 0);
        GDateTime *max_date = g_date_time_new_utc (max_year, 12, 31, 23, 59, 59);

        g_value_init (&min_val, G_TYPE_DATE_TIME);
        g_value_take_boxed (&min_val, min_date);
        g_value_init (&max_val, G_TYPE_DATE_TIME);
        g_value_take_boxed (&max_val, max_date);

        grl_operation_options_set_key_range_filter_value (
            options, GRL_METADATA_KEY_CREATION_DATE,
            &min_val, &max_val);
        g_value_unset (&min_val);
        g_value_unset (&max_val);
    }
}
#endif

void
music_add_result (UnityResultSet *result_set, const MediaFile &media)
{
    UnityScopeResult result = { 0, };

    const std::string uri = media.getUri();
    result.uri = const_cast<char*>(uri.c_str());
    // XXX: can we get thumbnails?
    result.icon_hint = const_cast<char*>("");
    result.category = 1;
    result.result_type = UNITY_RESULT_TYPE_PERSONAL;
    result.mimetype = const_cast<char*>(media.getContentType().c_str());
    result.title = const_cast<char*>(media.getTitle().c_str());
    result.comment = const_cast<char*>("");
    result.dnd_uri = result.uri;
    result.metadata = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);

    GVariant *variant = g_variant_new_int32 (media.getDuration());
    g_hash_table_insert (result.metadata, const_cast<char*>("duration"), g_variant_ref_sink (variant));

    const std::string artist = media.getAuthor();
    if (!artist.empty()) {
        variant = g_variant_new_string (artist.c_str());
        g_hash_table_insert (result.metadata, const_cast<char*>("artist"), g_variant_ref_sink (variant));
        result.comment = const_cast<char*>(artist.c_str());
    }

    const std::string album = media.getAlbum();
    if (!album.empty()) {
        variant = g_variant_new_string (album.c_str());
        g_hash_table_insert (result.metadata, const_cast<char*>("album"), g_variant_ref_sink (variant));
    }

    int track_number = media.getTrackNumber();
    if (track_number > 0) {
        variant = g_variant_new_int32 (track_number);
        g_hash_table_insert (result.metadata, const_cast<char*>("track-number"), g_variant_ref_sink (variant));
    }

    unity_result_set_add_result (result_set, &result);
    g_hash_table_unref (result.metadata);
}


UnityAbstractPreview *
music_preview (UnityResultPreviewer *previewer, void *user_data)
{
    const char *uri = previewer->result.uri;
    const char *title = previewer->result.title;
    const char *artist = "";
    const char *album = "";
    int duration = 0;
    int track_number = 0;

    if (previewer->result.metadata != NULL) {
        GVariant *variant;

        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char *>("artist")));
        if (variant) {
            artist = g_variant_get_string (variant, NULL);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("album")));
        if (variant) {
            album = g_variant_get_string (variant, NULL);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("duration")));
        if (variant) {
            duration = g_variant_get_int32 (variant);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("track-number")));
        if (variant) {
            track_number = g_variant_get_int32 (variant);
        }
    }

    GIcon *image = g_icon_new_for_string (previewer->result.icon_hint, NULL);
    UnityMusicPreview *preview = unity_music_preview_new (
        title, artist, image);
    g_object_unref (image);

    UnityTrackMetadata *track = unity_track_metadata_new ();
    unity_track_metadata_set_uri (track, uri);
    unity_track_metadata_set_track_no (track, track_number);
    unity_track_metadata_set_title (track, title);
    unity_track_metadata_set_artist (track, artist);
    unity_track_metadata_set_album (track, album);
    unity_track_metadata_set_length (track, duration);
    unity_music_preview_add_track (preview, track);

    UnityPreviewAction *play_action = unity_preview_action_new (
        "play", _("Play"), NULL);
    unity_preview_add_action (UNITY_PREVIEW (preview), play_action);

    return UNITY_ABSTRACT_PREVIEW (preview);
}


UnityAbstractScope *
music_scope_new (std::shared_ptr<MediaStore> store)
{
    UnitySimpleScope *scope = unity_simple_scope_new ();

    unity_simple_scope_set_group_name (scope, DBUS_NAME);
    unity_simple_scope_set_unique_name (scope, DBUS_MUSIC_PATH);

    /* Set up schema */
    UnitySchema *schema = unity_schema_new ();
    unity_schema_add_field (schema, "duration", "i", UNITY_SCHEMA_FIELD_TYPE_REQUIRED);
    unity_schema_add_field (schema, "artist", "s", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_schema_add_field (schema, "album", "s", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_schema_add_field (schema, "track-number", "i", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_simple_scope_set_schema (scope, schema);

    /* Set up categories */
    UnityCategorySet *categories = unity_category_set_new ();
    GFile *icon_dir = g_file_new_for_path ("/usr/share/icons/unity-icon-theme/places/svg");

    GFile *icon_file = g_file_get_child (icon_dir, "group-songs.svg");
    GIcon *icon = g_file_icon_new (icon_file);
    g_object_unref (icon_file);
    unity_category_set_add (categories,
                            unity_category_new ("global", _("Music"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    unity_category_set_add (categories,
                            unity_category_new ("songs", _("Songs"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);

    icon_file = g_file_get_child (icon_dir, "group-albums.svg");
    icon = g_file_icon_new (icon_file);
    g_object_unref (icon_file);
    unity_category_set_add (categories,
                            unity_category_new ("albums", _("Albums"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);

    icon_file = g_file_get_child (icon_dir, "group-treat-yourself.svg");
    icon = g_file_icon_new (icon_file);
    g_object_unref (icon_file);
    unity_category_set_add (categories,
                            unity_category_new ("more", _("More suggestions"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);

    g_object_unref (icon_dir);
    unity_simple_scope_set_category_set (scope, categories);

    /* Set up filters */
    unity_simple_scope_set_filter_set (scope, music_get_filters ());

    /* Set up search */
    ScopeSearchData *search_data = g_new0(ScopeSearchData, 1);
    search_data->store = store;
    search_data->media_type = AudioMedia;
    search_data->add_result = music_add_result;
    //search_data->apply_filters = music_apply_filters;
    setup_search (scope, search_data);
    // XXX: handle cleanup of search_data

    unity_simple_scope_set_preview_func (
        scope, music_preview, NULL, (GDestroyNotify)NULL);

    return UNITY_ABSTRACT_SCOPE (scope);
}
