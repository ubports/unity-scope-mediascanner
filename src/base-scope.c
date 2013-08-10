#include <config.h>
#include "scope.h"

#define MAX_RESULTS 20

typedef struct {
    ScopeSearchData *scope_data;
    UnityScopeSearchBase *search;
    UnityScopeSearchBaseCallback cb;
    void *cb_target;
    guint operation_id;
} SearchData;

static void
complete_search (SearchData *data)
{
    data->cb (data->search, data->cb_target);
    g_free (data);
}

static void
search_cb (GrlSource *source, guint browse_id, GrlMedia *media,
           guint remaining, void *user_data, const GError *error)
{
    SearchData *data = (SearchData *)user_data;
    UnitySearchContext *context = data->search->search_context;

    /* Complete the search on error. */
    if (error) {
        // XXX: can we report the error some how?
        complete_search (data);
        return;
    }

    /* Has this search been cancelled? */
    if (unity_cancellable_is_cancelled (context->cancellable)) {
        grl_operation_cancel (data->operation_id);
        complete_search (data);
        return;
    }

    if (media) {
        data->scope_data->add_result  (context->result_set, media);
    }

    if (remaining == 0) {
        complete_search (data);
    }
}

static void
search_async (UnityScopeSearchBase *search, UnityScopeSearchBaseCallback cb, void *cb_target, void *user_data)
{
    UnitySearchContext *context = search->search_context;
    ScopeSearchData *search_data = (ScopeSearchData *)user_data;
    GrlSource *source = search_data->source;

    /* perform a search */
    GrlCaps *caps = grl_source_get_caps (source, GRL_OP_SEARCH);
    GrlOperationOptions *options = grl_operation_options_new (caps);
    grl_operation_options_set_count (options, MAX_RESULTS);
    grl_operation_options_set_flags (options, GRL_RESOLVE_IDLE_RELAY);
    grl_operation_options_set_type_filter (options, search_data->media_type);

    if (search_data->apply_filters)
        search_data->apply_filters (context->filter_state, options);

    SearchData *data = g_new0 (SearchData, 1);
    data->scope_data = search_data;
    data->search = search;
    data->cb = cb;
    data->cb_target = cb_target;
    data->operation_id = grl_source_search (
        source, context->search_query, search_data->metadata_keys, options,
        search_cb, data);
}


static void
search_sync_finished (UnityScopeSearchBase *search, void *user_data)
{
    GMainLoop *ml = (GMainLoop *)user_data;

    g_main_loop_quit (ml);
}

void
search_sync (UnityScopeSearchBase *search, void *user_data)
{
    GMainLoop *ml = g_main_loop_new (NULL, FALSE);

    unity_scope_search_base_run_async (search, search_sync_finished, ml);
    g_main_loop_run (ml);
    g_main_loop_unref (ml);
}

void
setup_search (UnitySimpleScope *scope, ScopeSearchData *data)
{
    unity_simple_scope_set_search_async_func (
        scope, search_async, data, (GDestroyNotify)NULL);
    /* Satisfy the blocking API with a wrapper that calls the async
     * version in a nested main loop */
    unity_simple_scope_set_search_func (
        scope, search_sync, NULL, (GDestroyNotify)NULL);
}
