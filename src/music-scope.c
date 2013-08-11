#include <config.h>
#include "scope.h"

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

void
music_add_result (UnityResultSet *result_set, GrlMedia *media)
{
    UnityScopeResult result = { 0, };

    result.uri = (char *)grl_media_get_url (media);
    // XXX: can we get thumbnails?
    result.icon_hint = (char *)grl_media_get_thumbnail (media);
    if (result.icon_hint == NULL) {
        result.icon_hint = "/usr/share/unity/icons/album_missing.png";
    }
    result.category = 0;
    result.result_type = UNITY_RESULT_TYPE_PERSONAL;
    result.mimetype = (char *)grl_media_get_mime (media);
    result.title = (char *)grl_media_get_title (media);
    result.comment = "";
    result.dnd_uri = result.uri;
    result.metadata = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);

    int duration = grl_media_get_duration (media);
    GVariant *variant = g_variant_new_int32 (duration);
    g_hash_table_insert (result.metadata, "duration", g_variant_ref_sink (variant));

    const char *artist = grl_media_audio_get_artist (GRL_MEDIA_AUDIO (media));
    if (artist) {
        variant = g_variant_new_string (artist);
        g_hash_table_insert (result.metadata, "artist", g_variant_ref_sink (variant));
    }

    const char *album = grl_media_audio_get_album (GRL_MEDIA_AUDIO (media));
    if (album) {
        variant = g_variant_new_string (album);
        g_hash_table_insert (result.metadata, "album", g_variant_ref_sink (variant));
    }

    int track_number = grl_media_audio_get_track_number (GRL_MEDIA_AUDIO (media));
    if (track_number > 0) {
        variant = g_variant_new_int32 (track_number);
        g_hash_table_insert (result.metadata, "track-number", g_variant_ref_sink (variant));
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
    //const char *album = "";
    int duration = 0;
    int track_number = 0;

    if (previewer->result.metadata != NULL) {
        GVariant *variant;

        variant = g_hash_table_lookup (previewer->result.metadata, "artist");
        if (variant) {
            artist = g_variant_get_string (variant, NULL);
        }
        //variant = g_hash_table_lookup (previewer->result.metadata, "album");
        //if (variant) {
        //    album = g_variant_get_string (variant, NULL);
        //}
        variant = g_hash_table_lookup (previewer->result.metadata, "duration");
        if (variant) {
            duration = g_variant_get_int32 (variant);
        }
        variant = g_hash_table_lookup (previewer->result.metadata, "track-number");
        if (variant) {
            track_number = g_variant_get_int32 (variant);
        }
    }

    // XXX: icon hint?
    UnityMusicPreview *preview = unity_music_preview_new (
        title, artist, NULL);

    UnityTrackMetadata *track = unity_track_metadata_new ();
    unity_track_metadata_set_uri (track, uri);
    unity_track_metadata_set_track_no (track, track_number);
    unity_track_metadata_set_title (track, title);
    unity_track_metadata_set_length (track, duration);
    unity_music_preview_add_track (preview, track);

    UnityPreviewAction *play_action = unity_preview_action_new (
        "play", _("Play"), NULL);
    unity_preview_add_action (UNITY_PREVIEW (preview), play_action);

    return UNITY_ABSTRACT_PREVIEW (preview);
}


UnityAbstractScope *
music_scope_new (GrlSource *source)
{
    g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);
    g_return_val_if_fail ((grl_source_supported_operations (source) & GRL_OP_SEARCH) != 0, NULL);

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
    unity_category_set_add (categories,
                            unity_category_new ("global", _("Music"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);
    g_object_unref (icon_file);

    g_object_unref (icon_dir);
    unity_simple_scope_set_category_set (scope, categories);

    /* Set up filters */
    unity_simple_scope_set_filter_set (scope, music_get_filters ());

    /* Set up search */
    ScopeSearchData *search_data = g_new0(ScopeSearchData, 1);
    search_data->source = g_object_ref (source);
    search_data->media_type = GRL_TYPE_FILTER_AUDIO;
    search_data->metadata_keys = grl_metadata_key_list_new (
        GRL_METADATA_KEY_URL,
        GRL_METADATA_KEY_MIME,
        GRL_METADATA_KEY_THUMBNAIL,
        GRL_METADATA_KEY_TITLE,
        GRL_METADATA_KEY_ALBUM,
        GRL_METADATA_KEY_ARTIST,
        GRL_METADATA_KEY_TRACK_NUMBER,
        GRL_METADATA_KEY_DURATION,
        GRL_METADATA_KEY_INVALID);
    search_data->add_result = music_add_result;
    search_data->apply_filters = music_apply_filters;
    setup_search (scope, search_data);
    // XXX: handle cleanup of search_data

    unity_simple_scope_set_preview_func (
        scope, music_preview, NULL, (GDestroyNotify)NULL);

    return UNITY_ABSTRACT_SCOPE (scope);
}
