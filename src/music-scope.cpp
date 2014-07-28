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
#include <iostream>

#include <mediascanner/MediaFile.hh>
#include <mediascanner/Album.hh>
#include <mediascanner/Filter.hh>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/VariantBuilder.h>

#include "music-scope.h"
#include "i18n.h"

#define MAX_RESULTS 100
#define MAX_GENRES 100

static const char MISSING_ALBUM_ART[] = "/usr/share/unity/icons/album_missing.png";
static const char SONGS_CATEGORY_ICON[] = "/usr/share/icons/unity-icon-theme/places/svg/group-songs.svg";
static const char SONGS_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "carousel",
    "overlay": true,
    "card-size": "medium"
  },
  "components": {
    "title": "title",
    "art":  "art",
    "subtitle": "artist"
  }
}
)";
static const char ALBUMS_CATEGORY_DEFINITION[] = R"(
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

static const char ARTISTS_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium"
  },
  "components": {
    "title": "title",
    "art":  "art"
  }
}
)";

// Category renderer to use when presenting search results
// FIXME: This should use list category-layout (LP #1279279)
static const char SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "small"
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

void MusicScope::start(std::string const&, RegistryProxy const&) {
    setlocale(LC_ALL, "");
    store.reset(new MediaStore(MS_READ_ONLY));
}

void MusicScope::stop() {
    store.reset();
}

SearchQueryBase::UPtr MusicScope::search(CannedQuery const &q,
                                         SearchMetadata const& hints) {
    SearchQueryBase::UPtr query(new MusicQuery(*this, q, hints));
    return query;
}

PreviewQueryBase::UPtr MusicScope::preview(Result const& result,
                                    ActionMetadata const& hints) {
    PreviewQueryBase::UPtr previewer(new MusicPreview(*this, result, hints));
    return previewer;
}

MusicQuery::MusicQuery(MusicScope &scope, CannedQuery const& query, SearchMetadata const& hints)
    : SearchQueryBase(query, hints),
      scope(scope) {
}

void MusicQuery::cancelled() {
}

void MusicQuery::run(SearchReplyProxy const&reply) {
    const bool empty_search_query = query().query_string().empty();

    if (empty_search_query)
    {
        populate_departments(reply);
    }

    auto const current_department = query().department_id();
    if (current_department == "tracks")
    {
        query_songs(reply);
    }
    else if (current_department == "albums")
    {
        query_albums(reply);
    }
    else if (current_department == "genres")
    {
        query_albums(reply); //TODO
    }
    else if (current_department.find("genre:") == 0)
    {
        const int index = current_department.find(":");
        auto const genre = current_department.substr(index + 1);
        query_albums_by_genre(reply, genre);
    }
    else if (current_department == "albums_of_artist") // fake department that's not really displayed
    {
        query_albums_by_artist(reply, query().query_string());
        query_songs_by_artist(reply, query().query_string());
    }
    else // empty department id or 'artists'
    {
        query_artists(reply);

        if (!empty_search_query)
        {
            query_albums(reply);
            query_songs(reply);
        }
    }
}

void MusicQuery::populate_departments(unity::scopes::SearchReplyProxy const &reply) const
{
    unity::scopes::Department::SPtr artists = unity::scopes::Department::create("", query(), _("Artists"));
    unity::scopes::Department::SPtr albums = unity::scopes::Department::create("albums", query(), _("Albums"));
    unity::scopes::Department::SPtr tracks = unity::scopes::Department::create("tracks", query(), _("Tracks"));
    unity::scopes::Department::SPtr genres = unity::scopes::Department::create("genres", query(), _("Genres"));

    auto const current_department = query().department_id();

    if (current_department == "genres" || current_department.find("genre:") == 0)
    {
        const mediascanner::Filter filter;
        for (const auto &genre: scope.store->listGenres(filter))
        {
            genres->add_subdepartment(unity::scopes::Department::create("genre:" + genre, query(), genre));
        }
    }
    else
    {
        genres->set_has_subdepartments(true);
    }

    artists->set_subdepartments({albums, genres, tracks});

    try
    {
        reply->register_departments(artists);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to register departments: " << e.what() << std::endl;
    }
}

void MusicQuery::query_artists(unity::scopes::SearchReplyProxy const& reply) const
{
    const bool show_title = !query().query_string().empty();

    CategoryRenderer renderer(query().query_string() == "" ? ARTISTS_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION);
    auto cat = reply->register_category("artists", show_title ? _("Artists") : "", SONGS_CATEGORY_ICON, renderer); //FIXME: icon

    CannedQuery artist_search(query());
    artist_search.set_department_id("albums_of_artist"); // virtual department

    for (const auto &artist: scope.store->queryArtists(query().query_string(), MAX_RESULTS))
    {
        artist_search.set_query_string(artist);

        CategorisedResult res(cat);
        res.set_uri(artist_search.to_uri());
        res.set_title(artist);

        if(!reply->push(res))
        {
            return;
        }
    }
}

void MusicQuery::query_songs(unity::scopes::SearchReplyProxy const&reply) const {
    const bool show_title = !query().query_string().empty();

    CategoryRenderer renderer(query().query_string() == "" ? SONGS_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION);
    auto cat = reply->register_category("songs", show_title ? _("Tracks") : "", SONGS_CATEGORY_ICON, renderer);
    for (const auto &media : scope.store->query(query().query_string(), AudioMedia, MAX_RESULTS)) {
        if(!reply->push(create_song_result(cat, media)))
        {
            return;
        }
    }

}

void MusicQuery::query_songs_by_artist(unity::scopes::SearchReplyProxy const &reply, const std::string& artist) const
{
    CategoryRenderer renderer(query().query_string() == "" ? SONGS_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION);
    auto cat = reply->register_category("songs", _("Tracks"), SONGS_CATEGORY_ICON, renderer);

    mediascanner::Filter filter;
    filter.setArtist(artist);

    for (const auto &media : scope.store->listSongs(filter)) {
        if(!reply->push(create_song_result(cat, media)))
        {
            return;
        }
    }
}

static std::string uriencode(const std::string &src) {
    const char DEC2HEX[16+1] = "0123456789ABCDEF";
    std::string result = "";
    for (const char ch : src) {
        if (isalnum(ch)) {
            result += ch;
        } else {
            result += '%';
            result += DEC2HEX[(unsigned char)ch >> 4];
            result += DEC2HEX[(unsigned char)ch & 0x0F];
        }
    }
    return result;
}

unity::scopes::CategorisedResult MusicQuery::create_album_result(unity::scopes::Category::SCPtr const& category, mediascanner::Album const& album)
{
    CategorisedResult res(category);
    res.set_uri("album:///" + uriencode(album.getArtist()) + "/" + uriencode(album.getTitle()));
    res.set_title(album.getTitle());
    res["artist"] = album.getArtist();
    res["album"] = album.getTitle();
    res["isalbum"] = true;
    return res;
}

unity::scopes::CategorisedResult MusicQuery::create_song_result(unity::scopes::Category::SCPtr const& category, mediascanner::MediaFile const& media)
{
    CategorisedResult res(category);
    res.set_uri(media.getUri());
    res.set_dnd_uri(media.getUri());
    res.set_title(media.getTitle());

    res["duration"] = media.getDuration();
    res["album"] = media.getAlbum();
    res["artist"] = media.getAuthor();
    res["track-number"] = media.getTrackNumber();

    return res;
}

void MusicQuery::query_albums_by_genre(unity::scopes::SearchReplyProxy const&reply, const std::string& genre) const
{
    CategoryRenderer renderer(ALBUMS_CATEGORY_DEFINITION);
    auto cat = reply->register_category("albums", "", SONGS_CATEGORY_ICON, renderer);

    mediascanner::Filter filter;
    filter.setGenre(genre);
    for (const auto &album: scope.store->listAlbums(filter))
    {
        if (!reply->push(create_album_result(cat, album)))
        {
            return;
        }
    }
}

void MusicQuery::query_albums_by_artist(unity::scopes::SearchReplyProxy const &reply, const std::string& artist) const
{
    CategoryRenderer renderer(ALBUMS_CATEGORY_DEFINITION);
    auto cat = reply->register_category("albums", _("Albums"), SONGS_CATEGORY_ICON, renderer);

    mediascanner::Filter filter;
    filter.setArtist(artist);
    for (const auto &album: scope.store->listAlbums(filter))
    {
        if (!reply->push(create_album_result(cat, album)))
        {
            return;
        }
    }
}

void MusicQuery::query_albums(unity::scopes::SearchReplyProxy const&reply) const {
    const bool show_title = !query().query_string().empty();

    CategoryRenderer renderer(query().query_string() == "" ? ALBUMS_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION);
    auto cat = reply->register_category("albums", show_title ? _("Albums") : "", SONGS_CATEGORY_ICON, renderer);

    for (const auto &album : scope.store->queryAlbums(query().query_string(), MAX_RESULTS)) {
        if (!reply->push(create_album_result(cat, album)))
        {
            return;
        }
    }
}

MusicPreview::MusicPreview(MusicScope &scope, Result const& result, ActionMetadata const& hints)
    : PreviewQueryBase(result, hints),
      scope(scope) {
}

void MusicPreview::cancelled() {
}

static std::string make_art_uri(const std::string &artist, const std::string &album) {
    std::string result = "image://albumart/";
    result += "artist=" + uriencode(artist);
    result += "&album=" + uriencode(album);
    return result;
}

void MusicPreview::run(PreviewReplyProxy const& reply)
{
    if(result().contains("isalbum"))
    {
        album_preview(reply);
    }
    else
    {
        song_preview(reply);
    }
}

void MusicPreview::song_preview(unity::scopes::PreviewReplyProxy const &reply) const {
    ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    layout1col.add_column({"art", "header", "actions", "tracks"});

    layout2col.add_column({"art"});
    layout2col.add_column({"header", "actions", "tracks"});

    layout3col.add_column({"art"});
    layout3col.add_column({"header", "actions", "tracks"});
    layout3col.add_column({});
    reply->register_layout({layout1col, layout2col, layout3col});

    PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "artist");

    auto const res = result();

    PreviewWidget artwork("art", "image");
    std::string artist = res["artist"].get_string();
    std::string album = res["album"].get_string();
    std::string art;
    if (artist.empty() || album.empty()) {
        art = MISSING_ALBUM_ART;
    } else {
        art = make_art_uri(artist, album);
    }
    artwork.add_attribute_value("source", Variant(art));

    PreviewWidget tracks("tracks", "audio");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"title", Variant(res.title())},
                {"source", Variant(res.uri())},
                {"length", res["duration"]}
            });
        tracks.add_attribute_value("tracks", builder.end());
    }

    PreviewWidget actions("actions", "actions");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"id", Variant("play")},
                {"label", Variant(_("Play in music app"))}
            });
        actions.add_attribute_value("actions", builder.end());
    }

    reply->push({artwork, header, actions, tracks});
}

void MusicPreview::album_preview(unity::scopes::PreviewReplyProxy const &reply) const {
    ColumnLayout layout1col(1), layout2col(2);
    layout1col.add_column({"art", "header", "actions", "tracks"});

    layout2col.add_column({"art"});
    layout2col.add_column({"header", "actions", "tracks"});

    reply->register_layout({layout1col, layout2col});

    auto const res = result();

    PreviewWidget artwork("art", "image");
    std::string artist = res["artist"].get_string();
    std::string album_name = res["title"].get_string();
    std::string art;
    if (artist.empty() || album_name.empty()) {
        art = MISSING_ALBUM_ART;
    } else {
        art = make_art_uri(artist, album_name);
    }
    artwork.add_attribute_value("source", Variant(art));

    PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "artist");

    PreviewWidget actions("actions", "actions");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"uri", Variant(res.uri())},
                {"label", Variant(_("Play in music app"))}
            });
        actions.add_attribute_value("actions", builder.end());
    }

    PreviewWidget tracks("tracks", "audio");
    VariantBuilder builder;
    Album album(album_name, artist);
    for(const auto &track : scope.store->getAlbumSongs(album)) {
        std::vector<std::pair<std::string, Variant>> tmp;
        tmp.emplace_back("title", Variant(track.getTitle()));
        tmp.emplace_back("source", Variant(track.getUri()));
        tmp.emplace_back("length", Variant(track.getDuration()));
        builder.add_tuple(tmp);
    }
    tracks.add_attribute_value("tracks", builder.end());
    reply->push({artwork, header, actions, tracks});
}

extern "C" ScopeBase * UNITY_SCOPE_CREATE_FUNCTION() {
    return new MusicScope;
}

extern "C" void UNITY_SCOPE_DESTROY_FUNCTION(ScopeBase *scope) {
    delete scope;
}
