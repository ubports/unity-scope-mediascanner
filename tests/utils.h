#include <glib-object.h>
#include <unity.h>

/* A UnityResultSet implementation that saves the last added result
 * for inspection */

typedef struct _TestResultSet TestResultSet;
typedef struct _TestResultSetClass TestResultSetClass;

struct _TestResultSet {
    UnityResultSet parent;
    UnityScopeResult last_result;
};

struct _TestResultSetClass {
    UnityResultSetClass parent_class;
};

TestResultSet *test_result_set_new ();

TestResultSet *perform_search (UnityAbstractScope *scope, const char *search_query);
