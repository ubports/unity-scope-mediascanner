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

#include "music-scope.h"

#define MAX_RESULTS 100

static const char SONGS_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium"
  },
  "components": {
    "title": "title",
    "art":  "art",
    "subtitle": "artist"
  }
}
)";

using namespace mediascanner;
using namespace unity::scopes;

int MusicScope::start(std::string const&, RegistryProxy const&) {
    store.reset(new MediaStore(MS_READ_ONLY));
    return VERSION;
}

void MusicScope::stop() {
    store.reset();
}

QueryBase::UPtr MusicScope::create_query(std::string const &q,
                                         VariantMap const& hints) {
    QueryBase::UPtr query(new MusicQuery(*this, q));
    return query;
}

QueryBase::UPtr MusicScope::preview(Result const& result,
                                    VariantMap const& hints) {
    QueryBase::UPtr previewer(new MusicPreview(*this, result));
    return previewer;
}

MusicQuery::MusicQuery(MusicScope &scope, std::string const& query)
    : scope(scope), query(query) {
}

void MusicQuery::cancelled() {
}

void MusicQuery::run(SearchReplyProxy const&reply) {
    auto cat = reply->register_category("songs", "Songs", "/usr/share/icons/unity-icon-theme/places/svg/group-songs.svg", CategoryRenderer(SONGS_CATEGORY_DEFINITION));
    for (const auto &media : scope.store->query(query, AudioMedia, MAX_RESULTS)) {
        CategorisedResult res(cat);
        res.set_uri(media.getUri());
        res.set_dnd_uri(media.getUri());
        res.set_title(media.getTitle());

        res["mimetype"] = media.getContentType();
        res["duration"] = media.getDuration();
        res["album"] = media.getAlbum();
        res["artist"] = media.getAuthor();
        res["track-number"] = media.getTrackNumber();

        reply->push(res);
    }
}

MusicPreview::MusicPreview(MusicScope &scope, Result const& result)
    : scope(scope), result(result) {
}

void MusicPreview::cancelled() {
}

void MusicPreview::run(PreviewReplyProxy const& reply)
{
    ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    layout1col.add_column({"header", "art", "tracks", "actions"});

    layout2col.add_column({"header", "art"});
    layout2col.add_column({"tracks", "actions"});

    layout3col.add_column({"header", "art"});
    layout3col.add_column({"tracks", "actions"});
    layout3col.add_column({});
    reply->register_layout({layout1col, layout2col, layout3col});

    PreviewWidget header("header", "header");
    header.add_attribute("title", Variant(result.title()));
    header.add_attribute("subtitle", result["artist"]);

    PreviewWidget artwork("art", "image");
    // XXX: album art?
    artwork.add_attribute("source", Variant(""));

    PreviewWidget tracks("tracks", "audio");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"title", Variant(result.title())},
                {"source", Variant(result.uri())},
                {"length", result["duration"]}
            });
        tracks.add_attribute("tracks", builder.end());
    }

    PreviewWidget actions("actions", "actions");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"id", Variant("play")},
                {"label", Variant("Play")}
            });
        actions.add_attribute("actions", builder.end());
    }

    reply->push({header, artwork, tracks, actions});
}

extern "C" ScopeBase * UNITY_SCOPE_CREATE_FUNCTION() {
    return new MusicScope;
}

extern "C" void UNITY_SCOPE_DESTROY_FUNCTION(ScopeBase *scope) {
    delete scope;
}
