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

#include <mediascanner/MediaFile.hh>
#include <scopes/Category.h>
#include <scopes/CategorisedResult.h>

#include "video-scope.h"

using namespace unity::api::scopes;

int VideoScope::start(std::string const&, RegistryProxy const&) {
    store.reset(new MediaStore("", MS_READ_ONLY));
    return VERSION;
}

void VideoScope::stop() {
    store.reset();
}

QueryBase::UPtr VideoScope::create_query(std::string const &q,
                                         VariantMap const& hints) {
    QueryBase::UPtr query(new VideoQuery(*this, q));
    return query;
}

VideoQuery::VideoQuery(VideoScope &scope, std::string const& query)
    : scope(scope), query(query) {
}

void VideoQuery::cancelled() {
}

void VideoQuery::run(ReplyProxy const&reply) {

    auto cat = reply->register_category("local", "My Videos", "/usr/share/icons/unity-icon-theme/places/svg/group-videos.svg");
    for (const auto &media : scope.store->query(query, VideoMedia)) {
        CategorisedResult res(cat);
        res.set_uri(media.getUri());
        res.set_dnd_uri(media.getUri());
        res.set_title(media.getTitle());

        res["duration"] =media.getDuration();
        // res["width"] = media.getWidth();
        // res["height"] = media.getHeight();

        reply->push(res);
    }
}

extern "C" ScopeBase * UNITY_API_SCOPE_CREATE_FUNCTION() {
    return new VideoScope;
}

extern "C" void UNITY_API_SCOPE_DESTROY_FUNCTION(ScopeBase *scope) {
    delete scope;
}

#if 0
UnityAbstractPreview *
video_preview (UnityResultPreviewer *previewer, void *user_data)
{
    const char *title = previewer->result.title;
    const char *subtitle = "";
    const char *comment = previewer->result.comment;
    int duration = 0, width = 0, height = 0;

    if (previewer->result.metadata != NULL) {
        GVariant *variant;

        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("width")));
        if (variant) {
            width = g_variant_get_int32 (variant);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("height")));
        if (variant) {
            height = g_variant_get_int32 (variant);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("duration")));
        if (variant) {
            duration = g_variant_get_int32 (variant);
        }
    }

    UnityMoviePreview *preview = unity_movie_preview_new (
        title, subtitle, comment, NULL);
    unity_movie_preview_set_rating (preview, -1.0f, 0);
    unity_preview_set_image_source_uri (
        UNITY_PREVIEW (preview), previewer->result.uri);

    UnityPreviewAction *play_action = unity_preview_action_new (
        "play", _("Play"), NULL);
    unity_preview_add_action (UNITY_PREVIEW (preview), play_action);

    if (width > 0 && height > 0) {
        char *dimensions = g_strdup_printf ("%d*%d", width, height);
        UnityInfoHint *hint = unity_info_hint_new (
            "dimensions", _("Dimensions"), NULL, dimensions);
        g_free (dimensions);
        unity_preview_add_info (UNITY_PREVIEW (preview), hint);
    }

    if (duration > 0) {
        /* add info hint */
    }

    return UNITY_ABSTRACT_PREVIEW (preview);
}
#endif
