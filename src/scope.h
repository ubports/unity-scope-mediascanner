#include <glib/gi18n-lib.h>
#include <grilo.h>
#include <unity.h>

#define DBUS_NAME "com.canonical.Unity.Scope.Hollywood"
#define DBUS_MUSIC_PATH "/com/canonical/unity/scope/hollywood/music"
#define DBUS_VIDEO_PATH "/com/canonical/unity/scope/hollywood/video"


typedef struct _ScopeSearchData ScopeSearchData;
typedef void (* AddResultFunc) (UnityResultSet *result_set, GrlMedia *media);

struct _ScopeSearchData {
    GrlSource *source;
    GrlTypeFilter media_type;
    GList *metadata_keys;

    AddResultFunc add_result;
};


void setup_search (UnitySimpleScope *scope,
                   ScopeSearchData *data) G_GNUC_INTERNAL;

UnityAbstractScope *make_music_scope (GrlSource *source) G_GNUC_INTERNAL;

