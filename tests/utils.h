#include <glib-object.h>
#include <grilo.h>
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

/* A GrlSource that generates a single result */

typedef struct _TestSource TestSource;
typedef struct _TestSourceClass TestSourceClass;

struct _TestSource {
    GrlSource parent;

    GrlMedia *media;
};

struct _TestSourceClass {
    GrlSourceClass parent_class;
};

GrlSource *test_source_new (GrlMedia *result);
