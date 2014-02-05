/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by James Henstridge <james.henstridge@canonical.com>
 *
 */
#include <config.h>
#include "scope.h"
#include <mediascanner/MediaFile.hh>

#define MAX_RESULTS 100

using namespace mediascanner;

typedef struct {
    ScopeSearchData *scope_data;
    UnityScopeSearchBase *search;
    UnityScopeSearchBaseCallback cb;
    void *cb_target;
    guint operation_id;
} SearchData;

#if 0
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

    // XXX: The media scanner source seems to continue invoking the
    // search callback after cancellation.
#if 0
    /* Has this search been cancelled? */
    if (unity_cancellable_is_cancelled (context->cancellable)) {
        grl_operation_cancel (data->operation_id);
        complete_search (data);
        return;
    }
#endif

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
#endif

static void
search_sync (UnityScopeSearchBase *search, void *user_data)
{
    UnitySearchContext *context = search->search_context;
    ScopeSearchData *search_data = (ScopeSearchData *)user_data;
    MediaStore *store = search_data->store.get();

    // FIXME: handle filters
    // FIXME: enforce result limits
    for (const auto &media : store->query(context->search_query,
                                          search_data->media_type,
                                          MAX_RESULTS)) {
        search_data->add_result (context->result_set, media);
    }
}

#if 0
void
content_changed (GrlSource *source, GPtrArray *changed_medias,
                 GrlSourceChangeType change_type, gboolean location_unknown,
                 void *user_data)
{
    ScopeSearchData *data = (ScopeSearchData *)user_data;
    int i;
    gboolean matched_type = FALSE;

    for (i = 0; i < changed_medias->len; i++) {
        GrlMedia *media = g_ptr_array_index(changed_medias, i);

        /* Does the changed media match the type filter for this scope? */
        if (((data->media_type & GRL_TYPE_FILTER_AUDIO) != 0 &&
             GRL_IS_MEDIA_AUDIO (media)) ||
            ((data->media_type & GRL_TYPE_FILTER_VIDEO) != 0 &&
             GRL_IS_MEDIA_VIDEO (media)) ||
            ((data->media_type & GRL_TYPE_FILTER_IMAGE) != 0 &&
             GRL_IS_MEDIA_IMAGE (media))) {
            matched_type = TRUE;
            break;
        }
    }

    if (matched_type) {
        unity_abstract_scope_results_invalidated (
            UNITY_ABSTRACT_SCOPE (data->scope), UNITY_SEARCH_TYPE_DEFAULT);
        /* local content is displayed in home as well, invalidate that too */
        unity_abstract_scope_results_invalidated (
            UNITY_ABSTRACT_SCOPE (data->scope), UNITY_SEARCH_TYPE_GLOBAL);
    }
}
#endif

void
setup_search (UnitySimpleScope *scope, ScopeSearchData *data)
{
    data->scope = scope;
    data->content_changed_id = 0;

    unity_simple_scope_set_search_func (
        scope, search_sync, data, (GDestroyNotify)NULL);

#if 0
    if ((grl_source_supported_operations (data->source) &
         GRL_OP_NOTIFY_CHANGE) != 0) {
        data->content_changed_id = g_signal_connect (
            data->source, "content-changed",
            G_CALLBACK (content_changed), data);

        GError *error = NULL;
        if (!grl_source_notify_change_start (data->source, &error)) {
            g_warning("Could not configure change notification: %s",
                      error->message);
            g_signal_handler_disconnect (
                data->source, data->content_changed_id);
            data->content_changed_id = 0;
        }
        g_clear_error (&error);
    }
#endif
}
