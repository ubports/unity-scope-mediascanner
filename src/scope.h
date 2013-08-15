#include <glib/gi18n-lib.h>
#include <grilo.h>
#include <unity.h>

#define DBUS_NAME "com.canonical.Unity.Scope.MediaScanner"
#define DBUS_MUSIC_PATH "/com/canonical/unity/scope/mediascanner/music"
#define DBUS_VIDEO_PATH "/com/canonical/unity/scope/mediascanner/video"


typedef struct _ScopeSearchData ScopeSearchData;
typedef void (* AddResultFunc) (UnityResultSet *result_set, GrlMedia *media);
typedef void (* ApplyFiltersFunc) (UnityFilterSet *filter_state, GrlOperationOptions *options);

struct _ScopeSearchData {
    GrlSource *source;
    GrlTypeFilter media_type;
    GList *metadata_keys;

    AddResultFunc add_result;
    ApplyFiltersFunc apply_filters;
};


void setup_search (UnitySimpleScope *scope,
                   ScopeSearchData *data) G_GNUC_INTERNAL;

UnityAbstractScope *music_scope_new (GrlSource *source) G_GNUC_INTERNAL;
UnityAbstractScope *video_scope_new (GrlSource *source) G_GNUC_INTERNAL;

UnityFilterSet *music_get_filters (void) G_GNUC_INTERNAL;
void music_apply_filters (UnityFilterSet *filter_state, GrlOperationOptions *options) G_GNUC_INTERNAL;

void music_add_result (UnityResultSet *result_set, GrlMedia *media) G_GNUC_INTERNAL;
void video_add_result (UnityResultSet *result_set, GrlMedia *media) G_GNUC_INTERNAL;

UnityAbstractPreview *music_preview (UnityResultPreviewer *previewer, void *user_data) G_GNUC_INTERNAL;
UnityAbstractPreview *video_preview (UnityResultPreviewer *previewer, void *user_data) G_GNUC_INTERNAL;
