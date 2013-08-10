#include <config.h>
#include "scope.h"

void
video_add_result (UnityResultSet *result_set, GrlMedia *media)
{
    UnityScopeResult result = { 0, };

    result.uri = (char *)grl_media_get_url (media);
    // XXX: can we get thumbnails?
    result.icon_hint = (char *)grl_media_get_thumbnail (media);
    if (result.icon_hint == NULL) {
        result.icon_hint = "video";
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

    int width = grl_media_video_get_width (GRL_MEDIA_VIDEO (media));
    if (width > 0) {
        variant = g_variant_new_int32 (width);
        g_hash_table_insert (result.metadata, "width", g_variant_ref_sink (variant));
    }

    int height = grl_media_video_get_height (GRL_MEDIA_VIDEO (media));
    if (height > 0) {
        variant = g_variant_new_int32 (height);
        g_hash_table_insert (result.metadata, "height", g_variant_ref_sink (variant));
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

        variant = g_hash_table_lookup (previewer->result.metadata, "width");
        if (variant) {
            width = g_variant_get_int32 (variant);
        }
        variant = g_hash_table_lookup (previewer->result.metadata, "height");
        if (variant) {
            height = g_variant_get_int32 (variant);
        }
        variant = g_hash_table_lookup (previewer->result.metadata, "duration");
        if (variant) {
            duration = g_variant_get_int32 (variant);
        }
    }

    UnityMoviePreview *preview = unity_movie_preview_new (
        title, subtitle, comment, NULL);
    unity_movie_preview_set_rating (preview, -1.0f, 0);
    unity_preview_set_image_source_uri (
        UNITY_PREVIEW (preview), previewer->result.icon_hint);

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
video_scope_new (GrlSource *source)
{
    g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);
    g_return_val_if_fail ((grl_source_supported_operations (source) & GRL_OP_SEARCH) != 0, NULL);

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
    search_data->source = g_object_ref (source);
    search_data->media_type = GRL_TYPE_FILTER_VIDEO;
    search_data->metadata_keys = grl_metadata_key_list_new (
        GRL_METADATA_KEY_URL,
        GRL_METADATA_KEY_MIME,
        GRL_METADATA_KEY_THUMBNAIL,
        GRL_METADATA_KEY_TITLE,
        GRL_METADATA_KEY_DURATION,
        GRL_METADATA_KEY_HEIGHT,
        GRL_METADATA_KEY_WIDTH,
        GRL_METADATA_KEY_INVALID);
    search_data->add_result = video_add_result;
    setup_search (scope, search_data);
    // XXX: handle cleanup of search_data

    unity_simple_scope_set_preview_func (
        scope, video_preview, NULL, (GDestroyNotify)NULL);

    return UNITY_ABSTRACT_SCOPE (scope);
}
