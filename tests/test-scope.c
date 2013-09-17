#include <config.h>
#include <glib.h>
#include "../src/scope.h"
#include "utils.h"

static void
test_music_add_result ()
{
    GrlMedia *media = grl_media_audio_new ();
    TestResultSet *result_set = test_result_set_new ();

    grl_media_set_id (media, "test-id");
    grl_media_set_url (media, "http://example.com/foo.ogg");
    grl_media_set_mime (media, "audio/ogg");
    grl_media_set_title (media, "Title");
    grl_media_set_thumbnail (media, "http://example.com/thumbnail.jpg");
    grl_media_set_duration (media, 60);
    grl_media_audio_set_artist (GRL_MEDIA_AUDIO (media), "Artist");
    grl_media_audio_set_album (GRL_MEDIA_AUDIO (media), "Album");
    grl_media_audio_set_track_number (GRL_MEDIA_AUDIO (media), 42);

    music_add_result (UNITY_RESULT_SET (result_set), media);

    UnityScopeResult *result = &result_set->last_result;
    g_assert_cmpstr (result->uri, ==, "http://example.com/foo.ogg");
    g_assert_cmpstr (result->icon_hint, ==, "http://example.com/thumbnail.jpg");
    g_assert_cmpint (result->result_type, ==, UNITY_RESULT_TYPE_PERSONAL);
    g_assert_cmpstr (result->mimetype, ==, "audio/ogg");
    g_assert_cmpstr (result->title, ==, "Title");
    g_assert_cmpstr (result->comment, ==, "Artist");
    g_assert_cmpstr (result->dnd_uri, ==, "http://example.com/foo.ogg");

    GVariant *variant;
    variant = g_hash_table_lookup (result->metadata, "duration");
    g_assert_cmpint (g_variant_get_int32 (variant), ==, 60);

    variant = g_hash_table_lookup (result->metadata, "artist");
    g_assert_cmpstr (g_variant_get_string (variant, NULL), ==, "Artist");

    variant = g_hash_table_lookup (result->metadata, "album");
    g_assert_cmpstr (g_variant_get_string (variant, NULL), ==, "Album");

    variant = g_hash_table_lookup (result->metadata, "track-number");
    g_assert_cmpint (g_variant_get_int32 (variant), ==, 42);

    g_object_unref (result_set);
    g_object_unref (media);
}

static void
test_music_apply_filters_none ()
{
    UnityFilterSet *filter_state = music_get_filters ();
    GrlOperationOptions *options = grl_operation_options_new (NULL);

    music_apply_filters (filter_state, options);

    GValue *min_value = NULL, *max_value = NULL;
    grl_operation_options_get_key_range_filter (
        options, GRL_METADATA_KEY_CREATION_DATE, &min_value, &max_value);
    g_assert (min_value == NULL);
    g_assert (max_value == NULL);

    g_object_unref (options);
    g_object_unref (filter_state);
}

static void
test_music_apply_filters_decade ()
{
    UnityFilterSet *filter_state = music_get_filters ();
    GrlOperationOptions *options = grl_operation_options_new (NULL);

    UnityFilter *filter = unity_filter_set_get_filter_by_id (filter_state, "decade");
    g_assert (filter != NULL);
    unity_filter_set_filtering (filter, TRUE);
    UnityFilterOption *option = unity_options_filter_get_option (
        UNITY_OPTIONS_FILTER (filter), "1980");
    unity_filter_option_set_active (option, TRUE);
    g_object_unref (option);

    option = unity_options_filter_get_option (
        UNITY_OPTIONS_FILTER (filter), "1990");
    unity_filter_option_set_active (option, TRUE);
    g_object_unref (option);

    music_apply_filters (filter_state, options);

    GValue *min_value = NULL, *max_value = NULL;
    grl_operation_options_get_key_range_filter (
        options, GRL_METADATA_KEY_CREATION_DATE, &min_value, &max_value);
    g_assert (min_value != NULL);
    g_assert (G_VALUE_HOLDS (min_value, G_TYPE_DATE_TIME));
    g_assert (max_value != NULL);
    g_assert (G_VALUE_HOLDS (max_value, G_TYPE_DATE_TIME));

    GDateTime *min_date = g_value_get_boxed (min_value);
    g_assert_cmpint (g_date_time_get_year (min_date), ==, 1980);
    g_assert_cmpint (g_date_time_get_month (min_date), ==, 1);
    g_assert_cmpint (g_date_time_get_day_of_month (min_date), ==, 1);

    GDateTime *max_date = g_value_get_boxed (max_value);
    g_assert_cmpint (g_date_time_get_year (max_date), ==, 1999);
    g_assert_cmpint (g_date_time_get_month (max_date), ==, 12);
    g_assert_cmpint (g_date_time_get_day_of_month (max_date), ==, 31);

    g_object_unref (options);
    g_object_unref (filter_state);
}

static void
test_music_preview ()
{
    UnityScopeResult result = { 0, };

    result.uri = "http://example.com/foo.ogg";
    result.icon_hint = "http://example.com/thumbnail.jpg";
    result.result_type = UNITY_RESULT_TYPE_PERSONAL;
    result.mimetype = "audio/ogg";
    result.title = "Title";
    result.comment = "";
    result.dnd_uri = result.uri;
    result.metadata = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);

    g_hash_table_insert (result.metadata, "duration", g_variant_new_int32 (180));
    g_hash_table_insert (result.metadata, "artist", g_variant_new_string ("Artist"));
    g_hash_table_insert (result.metadata, "album", g_variant_new_string ("Album"));
    g_hash_table_insert (result.metadata, "track-number", g_variant_new_int32 (42));

    UnitySimpleScope *scope = unity_simple_scope_new ();
    UnitySearchMetadata *metadata= unity_search_metadata_new ();
    UnityResultPreviewer *previewer = unity_abstract_scope_create_previewer (
        UNITY_ABSTRACT_SCOPE (scope), &result, metadata);

    UnityAbstractPreview *preview = music_preview (previewer, NULL);
    g_assert (UNITY_IS_MUSIC_PREVIEW (preview));

    g_object_unref (previewer);
    g_object_unref (metadata);
    g_object_unref (scope);
    g_hash_table_unref (result.metadata);

    g_assert_cmpstr (
        unity_preview_get_title (UNITY_PREVIEW (preview)), ==, "Title");
    g_assert_cmpstr (
        unity_preview_get_subtitle (UNITY_PREVIEW (preview)), ==, "Artist");
    g_assert_cmpstr (
        unity_preview_get_image_source_uri (UNITY_PREVIEW (preview)), ==,
        "http://example.com/thumbnail.jpg");

    g_object_unref (preview);
}

static void
test_music_search ()
{
    GrlMedia *media = grl_media_audio_new ();
    grl_media_set_id (media, "test-id");
    grl_media_set_url (media, "http://example.com/foo.ogg");
    grl_media_set_mime (media, "audio/ogg");
    grl_media_set_title (media, "Title");
    grl_media_set_thumbnail (media, "http://example.com/thumbnail.jpg");
    grl_media_set_duration (media, 60);
    grl_media_audio_set_artist (GRL_MEDIA_AUDIO (media), "Artist");
    grl_media_audio_set_album (GRL_MEDIA_AUDIO (media), "Album");
    grl_media_audio_set_track_number (GRL_MEDIA_AUDIO (media), 42);

    GrlSource *source = test_source_new (media);

    UnityAbstractScope *scope = music_scope_new (source);

    TestResultSet *result_set = perform_search (scope, "query");

    UnityScopeResult *result = &result_set->last_result;
    g_assert_cmpstr (result->uri, ==, "http://example.com/foo.ogg");

    g_object_unref (result_set);
    g_object_unref (scope);
    g_object_unref (source);
    g_object_unref (media);
}

static void
test_video_add_result ()
{
    GrlMedia *media = grl_media_video_new ();
    TestResultSet *result_set = test_result_set_new ();

    grl_media_set_id (media, "test-id");
    grl_media_set_url (media, "http://example.com/foo.mp4");
    grl_media_set_mime (media, "video/mp4");
    grl_media_set_title (media, "Title");
    grl_media_set_duration (media, 60);

    video_add_result (UNITY_RESULT_SET (result_set), media);

    UnityScopeResult *result = &result_set->last_result;
    g_assert_cmpstr (result->uri, ==, "http://example.com/foo.mp4");
    g_assert_cmpint (result->result_type, ==, UNITY_RESULT_TYPE_PERSONAL);
    g_assert_cmpstr (result->mimetype, ==, "video/mp4");
    g_assert_cmpstr (result->title, ==, "Title");
    g_assert_cmpstr (result->dnd_uri, ==, "http://example.com/foo.mp4");

    GVariant *variant;
    variant = g_hash_table_lookup (result->metadata, "duration");
    g_assert_cmpint (g_variant_get_int32 (variant), ==, 60);

    g_object_unref (result_set);
    g_object_unref (media);
}

static void
test_video_preview ()
{
    UnityScopeResult result = { 0, };

    result.uri = "http://example.com/foo.mp4";
    result.result_type = UNITY_RESULT_TYPE_PERSONAL;
    result.mimetype = "video/mp4";
    result.title = "Title";
    result.comment = "";
    result.dnd_uri = result.uri;
    result.metadata = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);

    g_hash_table_insert (result.metadata, "duration", g_variant_new_int32 (180));
    g_hash_table_insert (result.metadata, "height", g_variant_new_int32 (1920));
    g_hash_table_insert (result.metadata, "width", g_variant_new_int32 (1080));

    UnitySimpleScope *scope = unity_simple_scope_new ();
    UnitySearchMetadata *metadata= unity_search_metadata_new ();
    UnityResultPreviewer *previewer = unity_abstract_scope_create_previewer (
        UNITY_ABSTRACT_SCOPE (scope), &result, metadata);

    UnityAbstractPreview *preview = video_preview (previewer, NULL);
    g_assert (UNITY_IS_MOVIE_PREVIEW (preview));

    g_object_unref (previewer);
    g_object_unref (metadata);
    g_object_unref (scope);
    g_hash_table_unref (result.metadata);

    g_assert_cmpstr (
        unity_preview_get_title (UNITY_PREVIEW (preview)), ==, "Title");
    g_assert_cmpstr (
        unity_preview_get_subtitle (UNITY_PREVIEW (preview)), ==, "");

    g_object_unref (preview);
}

static void
test_video_search ()
{
    GrlMedia *media = grl_media_video_new ();
    grl_media_set_id (media, "test-id");
    grl_media_set_url (media, "http://example.com/foo.mp4");
    grl_media_set_mime (media, "video/mp4");
    grl_media_set_title (media, "Title");
    grl_media_set_duration (media, 60);

    GrlSource *source = test_source_new (media);

    UnityAbstractScope *scope = video_scope_new (source);

    TestResultSet *result_set = perform_search (scope, "query");

    UnityScopeResult *result = &result_set->last_result;
    g_assert_cmpstr (result->uri, ==, "http://example.com/foo.mp4");

    g_object_unref (result_set);
    g_object_unref (scope);
    g_object_unref (source);
    g_object_unref (media);
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    grl_init (&argc, &argv);

    g_test_add_func ("/Music/AddResult", test_music_add_result);
    g_test_add_func ("/Music/ApplyFilters/None", test_music_apply_filters_none);
    g_test_add_func ("/Music/ApplyFilters/Decade", test_music_apply_filters_decade);
    g_test_add_func ("/Music/Preview", test_music_preview);
    g_test_add_func ("/Music/Search", test_music_search);
    g_test_add_func ("/Video/AddResult", test_video_add_result);
    g_test_add_func ("/Video/Preview", test_video_preview);
    g_test_add_func ("/Video/Search", test_video_search);

    g_test_run ();
    return 0;
}
