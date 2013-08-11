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
