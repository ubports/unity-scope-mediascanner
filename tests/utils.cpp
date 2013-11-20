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
test_result_set_class_init (TestResultSetClass *klass)
{
    result_set_parent_class = static_cast<UnityResultSetClass*>(g_type_class_peek_parent (klass));
    UNITY_RESULT_SET_CLASS (klass)->add_result = test_result_set_add_result;
    G_OBJECT_CLASS (klass)->finalize = test_result_set_finalize;
}

static void
test_result_set_init (TestResultSet *result_set)
{
}

TestResultSet *
test_result_set_new ()
{
    return static_cast<TestResultSet*>(g_object_new (test_result_set_get_type (), NULL));
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
