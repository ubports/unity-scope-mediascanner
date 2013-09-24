#include "utils.h"

G_DEFINE_TYPE (TestResultSet, test_result_set, UNITY_TYPE_RESULT_SET);
static UnityResultSetClass *result_set_parent_class;

static void
test_result_set_finalize (GObject *object)
{
    TestResultSet *result_set = (TestResultSet *)object;

    unity_scope_result_destroy (&result_set->last_result);

    G_OBJECT_CLASS (result_set_parent_class)->finalize (object);
}

static void
test_result_set_add_result (UnityResultSet *object, UnityScopeResult *result)
{
    TestResultSet *result_set = (TestResultSet *)object;

    unity_scope_result_copy (result, &result_set->last_result);
}

static void
test_result_set_class_init (TestResultSetClass *class)
{
    result_set_parent_class = g_type_class_peek_parent (class);
    UNITY_RESULT_SET_CLASS (class)->add_result = test_result_set_add_result;
    G_OBJECT_CLASS (class)->finalize = test_result_set_finalize;
}

static void
test_result_set_init (TestResultSet *result_set)
{
}

TestResultSet *
test_result_set_new ()
{
    return g_object_new (test_result_set_get_type (), NULL);
}

static void
search_complete (UnityScopeSearchBase *search, void *user_data)
{
    GMainLoop *ml = (GMainLoop *)user_data;

    g_main_loop_quit (ml);
}


TestResultSet *
perform_search (UnityAbstractScope *scope, const char *search_query)
{
    UnitySearchMetadata *metadata = unity_search_metadata_new ();
    UnityFilterSet *filter_state = unity_abstract_scope_get_filters (scope);
    TestResultSet *result_set = test_result_set_new ();
    UnitySearchContext context = {
        .search_query = (char *)search_query,
        .search_type = UNITY_SEARCH_TYPE_DEFAULT,
        .filter_state = filter_state,
        .search_metadata = metadata,
        .result_set = UNITY_RESULT_SET (result_set)
    };

    UnityScopeSearchBase *search = unity_abstract_scope_create_search_for_query (scope, &context);

    GMainLoop *ml = g_main_loop_new (NULL, FALSE);
    unity_scope_search_base_run_async (search, search_complete, ml);
    g_main_loop_run (ml);
    g_main_loop_unref (ml);

    g_object_unref (search);
    g_object_unref (filter_state);
    g_object_unref (metadata);
    return result_set;
}


G_DEFINE_TYPE (TestSource, test_source, GRL_TYPE_SOURCE);
static GrlSourceClass *source_parent_class;

static void
test_source_finalize (GObject *object)
{
    TestSource *source = (TestSource *)object;

    if (source->media != NULL) {
        g_object_unref (source->media);
        source->media = NULL;
    }

    G_OBJECT_CLASS (source_parent_class)->finalize (object);
}

static GrlSupportedOps
test_source_supported_operations (GrlSource *source)
{
    return GRL_OP_SEARCH | GRL_OP_NOTIFY_CHANGE;
}

static const GList *
test_source_supported_keys (GrlSource *source)
{
    static GList *keys = NULL;

    if (keys == NULL) {
        keys = grl_metadata_key_list_new (
            GRL_METADATA_KEY_URL,
            GRL_METADATA_KEY_MIME,
            GRL_METADATA_KEY_THUMBNAIL,
            GRL_METADATA_KEY_TITLE,
            GRL_METADATA_KEY_ALBUM,
            GRL_METADATA_KEY_ARTIST,
            GRL_METADATA_KEY_TRACK_NUMBER,
            GRL_METADATA_KEY_DURATION,
            GRL_METADATA_KEY_HEIGHT,
            GRL_METADATA_KEY_WIDTH,
            GRL_METADATA_KEY_INVALID);
    }
    return keys;
}

static GrlCaps *
test_source_get_caps (GrlSource *source, GrlSupportedOps ops)
{
    return grl_caps_new ();
}

static void
test_source_search (GrlSource *source, GrlSourceSearchSpec *ss)
{
    TestSource *self = (TestSource *)source;

    ss->callback (ss->source, ss->operation_id, self->media, 0,
                  ss->user_data, NULL);
}

static gboolean
test_source_notify_change_start (GrlSource *source, GError **error)
{
    return TRUE;
}

static gboolean
test_source_notify_change_stop (GrlSource *source, GError **error)
{
    return TRUE;
}

static void
test_source_class_init (TestSourceClass *class)
{
    source_parent_class = g_type_class_peek_parent (class);
    G_OBJECT_CLASS (class)->finalize = test_source_finalize;
    GRL_SOURCE_CLASS (class)->supported_operations = test_source_supported_operations;
    GRL_SOURCE_CLASS (class)->supported_keys = test_source_supported_keys;
    GRL_SOURCE_CLASS (class)->get_caps = test_source_get_caps;
    GRL_SOURCE_CLASS (class)->search = test_source_search;
    GRL_SOURCE_CLASS (class)->notify_change_start = test_source_notify_change_start;
    GRL_SOURCE_CLASS (class)->notify_change_stop = test_source_notify_change_stop;
}

static void
test_source_init (TestSource *result_set)
{
}

GrlSource *
test_source_new (GrlMedia *media)
{
    TestSource *source = g_object_new (test_source_get_type(), NULL);
    source->media = g_object_ref (media);
    return GRL_SOURCE (source);
}
