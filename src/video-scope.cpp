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
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/VariantBuilder.h>

#include "video-scope.h"

#define MAX_RESULTS 100

using namespace mediascanner;
using namespace unity::scopes;

int VideoScope::start(std::string const&, RegistryProxy const&) {
    store.reset(new MediaStore(MS_READ_ONLY));
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

QueryBase::UPtr VideoScope::preview(Result const& result,
                                    VariantMap const& hints) {
    QueryBase::UPtr previewer(new VideoPreview(*this, result));
    return previewer;
}

VideoQuery::VideoQuery(VideoScope &scope, std::string const& query)
    : scope(scope), query(query) {
}

void VideoQuery::cancelled() {
}

void VideoQuery::run(SearchReplyProxy const&reply) {

    auto cat = reply->register_category("local", "My Videos", "/usr/share/icons/unity-icon-theme/places/svg/group-videos.svg");
    for (const auto &media : scope.store->query(query, VideoMedia, MAX_RESULTS)) {
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

VideoPreview::VideoPreview(VideoScope &scope, Result const& result)
    : scope(scope), result(result) {
}

void VideoPreview::cancelled() {
}

void VideoPreview::run(PreviewReplyProxy const& reply)
{
    ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    layout1col.add_column({"header", "video", "actions"});

    layout2col.add_column({"header", "video"});
    layout2col.add_column({"actions"});

    layout3col.add_column({"header", "video"});
    layout3col.add_column({"actions"});
    layout3col.add_column({});
    reply->register_layout({layout1col, layout2col, layout3col});

    PreviewWidget header("header", "header");
    header.add_component("title", "title");

    PreviewWidget video("video", "video");
    video.add_component("source", "uri");

    PreviewWidget actions("actions", "actions");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"id", Variant("play")},
                {"label", Variant("Play")}
            });
        actions.add_attribute("actions", builder.end());
    }

    reply->push({header, video, actions});
}

extern "C" ScopeBase * UNITY_SCOPE_CREATE_FUNCTION() {
    return new VideoScope;
}

extern "C" void UNITY_SCOPE_DESTROY_FUNCTION(ScopeBase *scope) {
    delete scope;
}
