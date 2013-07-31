#include "scope.h"

static void
video_add_result (UnityResultSet *result_set, GrlMedia *media)
{
    UnityScopeResult result = { 0, };

    result.uri = grl_media_get_url (media);
    // XXX: can we get thumbnails?
    result.icon_hint = grl_media_get_thumbnail (media);
    if (result.icon_hint == NULL) {
        result.icon_hint = "/usr/share/unity/icons/album_missing.png";
    }
    result.category = 0;
    result.result_type = UNITY_RESULT_TYPE_PERSONAL;
    result.mimetype = grl_media_get_mime (media);
    result.title = grl_media_get_title (media);
    result.comment = "";
    result.dnd_uri = result.uri;
    result.metadata = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);

    gint duration = grl_media_get_duration (media);
    g_hash_table_insert (result.metadata, "duration", g_variant_new_int32 (duration));

    unity_result_set_add_result (result_set, &result);
}


static UnityAbstractPreview *
video_preview (UnityResultPreviewer *previewer, void *user_data)
{
    return NULL;
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
        GRL_METADATA_KEY_INVALID);
    search_data->add_result = video_add_result;
    setup_search (scope, search_data);
    // XXX: handle cleanup of search_data

    unity_simple_scope_set_preview_func (
        scope, video_preview, NULL, (GDestroyNotify)NULL);

    return UNITY_ABSTRACT_SCOPE (scope);
}
