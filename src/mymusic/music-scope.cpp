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
#include <algorithm>
#include <gio/gio.h>

#include <mediascanner/MediaFile.hh>
#include <mediascanner/Album.hh>
#include <mediascanner/Filter.hh>
#include <core/net/http/response.h>
#include <core/net/http/request.h>
#include <core/net/uri.h>
#include <json/json.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/VariantBuilder.h>

#include "music-scope.h"
#include "../utils/i18n.h"

#define MAX_RESULTS 100
#define MAX_GENRES 100

static const char THUMBNAILER_SCHEMA[] = "com.canonical.Unity.Thumbnailer";
static const char THUMBNAILER_API_KEY[] = "dash-ubuntu-com-key";

static const char MISSING_ALBUM_ART[] = "album_missing.png";
static const char SONGS_CATEGORY_ICON[] = "/usr/share/icons/unity-icon-theme/places/svg/group-songs.svg";

static const char GET_STARTED_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "card-layout" : "vertical",
    "collapsed-rows" : 0,
    "non-interactive": "true"
  },
  "components": {
    "title": "title",
    "art": {
        "field": "art",
        "conciergeMode": true
    },
    "summary" : "summary"
  }
}
)";

static const char SONGS_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "card-layout" : "horizontal",
    "quick-preview-type" : "audio"
  },
  "components": {
    "title": "title",
    "art": {
      "field": "art",
      "fallback": "@FALLBACK@"
    },
    "subtitle": "artist",
    "quick-preview-data": {
        "field": "audio-data"
    }
  }
}
)";
static const char ALBUMS_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "small"
  },
  "components": {
    "title": "title",
    "art": {
      "field": "art",
      "fallback": "@FALLBACK@"
    },
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
    "art": {
      "field": "art",
      "fallback": "@FALLBACK@"
    }
  }
}
)";

static const char ARTIST_BIO_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "vertical-journal",
    "card-size": "large",
    "collapsed-rows": 0,
    "non-interactive": "true"
  },
  "components": {
    "title": "title",
    "summary": "summary",
    "art":  {
        "field": "art",
        "aspect-ratio": 1.5,
        "fallback": "@FALLBACK@"
    }
  }
}
)";

static const char AGGREGATED_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "collapsed-rows": 3,
    "card-layout": "horizontal",
    "quick-preview-type" : "audio"
  },
  "components": {
    "title": "title",
    "art": {
      "field": "art",
      "fallback": "@FALLBACK@"
    },
    "subtitle": "artist",
    "quick-preview-data": {
        "field": "audio-data"
    }
  }
}
)";

// Category renderer to use when presenting search results
static const char SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-layout" : "horizontal",
    "card-size": "large"
  },
  "components": {
    "title": "title",
    "art": {
      "field": "art",
      "fallback": "@FALLBACK@"
    },
    "subtitle": "artist"
  }
}
)";

static const char SEARCH_SONGS_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-layout" : "horizontal",
    "card-size": "large"
  },
  "components": {
    "title": "title",
    "art":  {
      "field": "art",
      "fallback": "@FALLBACK@"
    },
    "subtitle": "artist"
  }
}
)";

using namespace mediascanner;
using namespace unity::scopes;
using namespace core::net;
namespace json = Json;

void MusicScope::start(std::string const&) {
    init_gettext(*this);
    store.reset(new MediaStore(MS_READ_ONLY));
    client = http::make_client();
    set_api_key();
}

void MusicScope::set_api_key()
{
    // the API key is not expected to change, so don't monitor it
    GSettingsSchemaSource *src = g_settings_schema_source_get_default();
    GSettingsSchema *schema = g_settings_schema_source_lookup(src, THUMBNAILER_SCHEMA, true);

    if (schema)
    {
        bool status = false;
        g_settings_schema_unref(schema);
        GSettings *settings = g_settings_new(THUMBNAILER_SCHEMA);
        if (settings) {
            gchar *akey = g_settings_get_string(settings, THUMBNAILER_API_KEY);
            if (akey) {
                api_key = std::string(akey);
                status = true;
                g_free(akey);
            }
            g_object_unref(settings);
        }
        if (!status) {
            std::cerr << "Failed to get API key" << std::endl;
        }
    } else {
        std::cerr << "The schema " << THUMBNAILER_SCHEMA << " is missing" << std::endl;
    }
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

std::string MusicScope::make_artist_art_uri(const std::string &artist, const std::string &album) const {
    auto const uri = core::net::make_uri(
            "image://artistart", {}, {{"artist", artist}, {"album", album}});
    return client->uri_to_string(uri);
}

MusicQuery::MusicQuery(MusicScope &scope, CannedQuery const& query, SearchMetadata const& hints)
    : SearchQueryBase(query, hints),
      scope(scope),
      query_cancelled(false) {
}

void MusicQuery::cancelled() {
    query_cancelled = true;
}

void MusicQuery::run(SearchReplyProxy const&reply) {
    const bool empty_search_query = query().query_string().empty();
    const bool is_aggregated = search_metadata().is_aggregated();

    if (is_aggregated)
    {
        if (empty_search_query) // surfacing
        {
            const CategoryRenderer renderer = make_renderer(AGGREGATED_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
            auto cat = reply->register_category(
                "mymusic", _("My Music"), "",
                CannedQuery(query().scope_id(), query().query_string(), ""),
                renderer);
            query_songs(reply, cat, true);
        }
        else // non-empty search in albums and songs
        {
            const CategoryRenderer renderer = make_renderer(SEARCH_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
            auto cat = reply->register_category(
                "mymusic", _("My Music"), "",
                CannedQuery(query().scope_id(), query().query_string(), ""),
                renderer);
            query_artists(reply, cat);
            query_albums(reply, cat);
            query_songs(reply, cat);
        }
        return;
    }

    if (!scope.store->hasMedia(AudioMedia))
    {
        const CategoryRenderer renderer(GET_STARTED_CATEGORY_DEFINITION);
        auto cat = reply->register_category("mymusic-getstarted", "", "", renderer);
        CategorisedResult res(cat);
        res.set_uri(query().to_uri());
        res.set_title(_("Get started!"));
        res["summary"] = _("Drag and drop items from another devices. Alternatively, load your files onto a SD card.");
        res.set_art(scope.scope_directory() + "/" + "getstarted.svg");
        reply->push(res);
        return;
    }

    populate_departments(reply);

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
        query_genres(reply);
    }
    else if (current_department.find("genre:") == 0)
    {
        const int index = current_department.find(":");
        auto const genre = current_department.substr(index + 1);
        query_albums_by_genre(reply, genre);
    }
    else if (query().has_user_data() && query().user_data().get_string() == "albums_of_artist")
    {
        const std::string artist = query().query_string();
        query_albums_by_artist(reply, artist);
        query_songs_by_artist(reply, artist);
    }
    else // empty department id - default view
    {
        if (empty_search_query) // surfacing
        {
            query_artists(reply);
        }
        else // non-empty search in albums and songs
        {
            query_artists(reply);
            query_albums(reply);
            query_songs(reply);
        }
    }
}

CategoryRenderer MusicQuery::make_renderer(std::string json_text, std::string const& fallback) const {
    static std::string const placeholder("@FALLBACK@");
    size_t pos = json_text.find(placeholder);
    if (pos != std::string::npos)
    {
        json_text.replace(pos, placeholder.size(), scope.scope_directory() + "/" + fallback);
    }
    return CategoryRenderer(json_text);
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
            if (!genre.empty())
            {
                genres->add_subdepartment(unity::scopes::Department::create("genre:" + genre, query(), genre));
            }
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

void MusicQuery::query_genres(unity::scopes::SearchReplyProxy const&reply) const
{
    const CategoryRenderer renderer = make_renderer(ALBUMS_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
    mediascanner::Filter filter;

    auto const genres = scope.store->listGenres(filter);
    auto const genre_limit = std::min(static_cast<int>(genres.size()), 10);
    int limit = MAX_RESULTS;

    for (int i = 0; i < genre_limit; i++)
    {
        auto cat = reply->register_category("genre:" + genres[i], genres[i], "", renderer); //FIXME: how to make genre i18n-friendly?

        filter.setGenre(genres[i]);
        filter.setLimit(limit);
        for (const auto &album: scope.store->listAlbums(filter))
        {
            limit--;
            if (!reply->push(create_album_result(cat, album)))
                return;
        }
        if (limit <= 0)
        {
            break;
        }
    }
}

void MusicQuery::query_artists(unity::scopes::SearchReplyProxy const& reply, Category::SCPtr const& override_category) const
{
    const bool show_title = !query().query_string().empty();

    auto cat = override_category;
    if (!cat) {
        CategoryRenderer renderer = make_renderer(query().query_string() == "" ? ARTISTS_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
        cat = reply->register_category("artists", show_title ? _("Artists") : "", SONGS_CATEGORY_ICON, renderer); //FIXME: icon
    }

    CannedQuery artist_search(query());
    artist_search.set_department_id("");
    artist_search.set_query_string("");

    mediascanner::Filter filter;
    filter.setLimit(MAX_RESULTS);
    for (const auto &artist: scope.store->queryArtists(query().query_string(), filter))
    {
        artist_search.set_query_string(artist);
        artist_search.set_user_data(Variant("albums_of_artist"));

        CategorisedResult res(cat);
        res.set_uri(artist_search.to_uri());
        res.set_title(artist);

        // find first non-empty album of this artist, needed to get artist-art
        {
            std::string album_name;
            mediascanner::Filter filter;
            filter.setArtist(artist);
            for (auto const& album: scope.store->listAlbums(filter))
            {
                album_name = album.getTitle();
                if (!album_name.empty())
                {
                    break;
                }
            }
            res.set_art(scope.make_artist_art_uri(artist, album_name));
        }

        if(!reply->push(res))
        {
            return;
        }
    }
}

void MusicQuery::query_songs(unity::scopes::SearchReplyProxy const&reply, Category::SCPtr const& override_category, bool sortByMtime) const {
    const bool surfacing = query().query_string().empty();
    auto cat = override_category;
    if (!cat)
    {
        CategoryRenderer renderer = make_renderer(surfacing ? SONGS_CATEGORY_DEFINITION : SEARCH_SONGS_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
        cat = reply->register_category("songs", surfacing ? "" : _("Tracks"), SONGS_CATEGORY_ICON, renderer);
    }
    mediascanner::Filter filter;
    filter.setLimit(MAX_RESULTS);
    if (sortByMtime) {
        filter.setOrder(MediaOrder::Modified);
        filter.setReverse(true);
    }

    auto const songs = scope.store->query(query().query_string(), AudioMedia, filter);
    static const std::vector<mediascanner::MediaFile> empty_playlist;

    for (const auto &media : songs) {
        // Inline playback should only be used in surfacing mode.
        // Attach the playlist with all songs to every card (same playlist for every card).
        if(!reply->push(create_song_result(cat, media, surfacing, surfacing ? songs : empty_playlist)))
        {
            return;
        }
    }

}

void MusicQuery::query_songs_by_artist(unity::scopes::SearchReplyProxy const &reply, const std::string& artist) const
{
    CategoryRenderer renderer = make_renderer(query().query_string() == "" ? SONGS_CATEGORY_DEFINITION : SEARCH_SONGS_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
    auto cat = reply->register_category("songs", _("Tracks"), SONGS_CATEGORY_ICON, renderer);

    mediascanner::Filter filter;
    filter.setArtist(artist);
    filter.setLimit(MAX_RESULTS);

    for (const auto &media : scope.store->listSongs(filter)) {
        if(!reply->push(create_song_result(cat, media)))
        {
            return;
        }
    }
}

unity::scopes::CategorisedResult MusicQuery::create_album_result(unity::scopes::Category::SCPtr const& category, mediascanner::Album const& album) const
{
    CategorisedResult res(category);
    res.set_uri("album:///" + scope.client->url_escape(album.getArtist()) + "/" + scope.client->url_escape(album.getTitle()));
    res.set_title(album.getTitle());
    res.set_art(album.getArtUri());
    res["artist"] = album.getArtist();
    res["album"] = album.getTitle();
    res["isalbum"] = true;
    return res;
}

unity::scopes::CategorisedResult MusicQuery::create_song_result(unity::scopes::Category::SCPtr const& category, mediascanner::MediaFile const& media,
        bool audio_data, std::vector<mediascanner::MediaFile> const& album_songs) const
{
    std::string uri = media.getUri();
    CategorisedResult res(category);
    res.set_uri(uri);
    res.set_dnd_uri(uri);
    res.set_title(media.getTitle());
    res.set_art(media.getArtUri());

    res["duration"] = media.getDuration();
    res["album"] = media.getAlbum();
    res["artist"] = media.getAuthor();
    res["track-number"] = media.getTrackNumber();

    if (audio_data)
    {
        VariantMap data;
        data["uri"] = uri;
        data["duration"] = media.getDuration();
        if (album_songs.size() > 0)
        {
            VariantArray songsva;
            for (auto const& song: album_songs)
            {
                songsva.push_back(Variant(song.getUri()));
            }
            data["playlist"] = songsva;
        }
        res["audio-data"] = data;
    }

    return res;
}

void MusicQuery::query_albums_by_genre(unity::scopes::SearchReplyProxy const&reply, const std::string& genre) const
{
    CategoryRenderer renderer = make_renderer(ALBUMS_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
    auto cat = reply->register_category("albums", "", SONGS_CATEGORY_ICON, renderer);

    mediascanner::Filter filter;
    filter.setGenre(genre);
    filter.setLimit(MAX_RESULTS);
    for (const auto &album: scope.store->listAlbums(filter))
    {
        if (!reply->push(create_album_result(cat, album)))
        {
            return;
        }
    }
}

std::string MusicQuery::fetch_biography_sync(const std::string& artist, const std::string &album) const
{
    std::string bio_text;
    http::Request::Configuration config;
    auto uri = core::net::make_uri(
            "https://dash.ubuntu.com",
            {"musicproxy", "v1", "artist-bio"},
            {{"artist", artist}, {"album", album}, {"key", scope.api_key}});
    config.uri = scope.client->uri_to_string(uri);
    auto request = scope.client->get(config);
    http::Request::Handler handler;
    try
    {
        auto response = request->execute([this](const http::Request::Progress&) -> http::Request::Progress::Next {
                return query_cancelled ?  http::Request::Progress::Next::abort_operation : http::Request::Progress::Next::continue_operation;
                });
        json::Value root;
        json::Reader reader;
        if (reader.parse(response.body, root))
        {
            if (root.isObject() && root.isMember("biography"))
            {
                json::Value data = root["biography"];
                if (data.isString())
                {
                    bio_text = data.asString();
                }
            }
            if (bio_text.empty())
            {
                std::cerr << "Artist info is empty for " << artist << ", " << album << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to parse artist-bio response: " << response.body << std::endl;
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Failed to get artist info: " << e.what() << std::endl;
    }

    return bio_text;
}

void MusicQuery::query_albums_by_artist(unity::scopes::SearchReplyProxy const &reply, const std::string& artist) const
{
    CategoryRenderer bio_renderer = make_renderer(ARTIST_BIO_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
    CategoryRenderer renderer = make_renderer(ALBUMS_CATEGORY_DEFINITION, MISSING_ALBUM_ART);

    auto biocat = reply->register_category("bio", "", "", bio_renderer);
    auto albumcat = reply->register_category("albums", _("Albums"), SONGS_CATEGORY_ICON, renderer);

    bool show_bio = true;
    std::string bio_text;

    mediascanner::Filter filter;
    filter.setArtist(artist);
    filter.setLimit(MAX_RESULTS);
    auto const albums = scope.store->listAlbums(filter);

    for (const auto &album: albums)
    {
        if (show_bio && !album.getTitle().empty())
        {
            if (search_metadata().internet_connectivity() != QueryMetadata::ConnectivityStatus::Disconnected)
            {
                //
                // biography has to be the first result to display and we have all the other results ready
                // so it's ok to fetch biography synchronously.
                bio_text = fetch_biography_sync(artist, album.getTitle());
            }

            CannedQuery artist_search(query());
            artist_search.set_department_id("");
            artist_search.set_query_string(artist);
            artist_search.set_user_data(Variant("albums_of_artist"));

            CategorisedResult artist_info(biocat);
            artist_info.set_uri(artist_search.to_uri());
            artist_info.set_title(artist);
            artist_info["summary"] = bio_text;
            artist_info["art"] = scope.make_artist_art_uri(artist, album.getTitle());
            reply->push(artist_info);
            show_bio = false;
        }
        if (!reply->push(create_album_result(albumcat, album)))
        {
            return;
        }
    }
}

void MusicQuery::query_albums(unity::scopes::SearchReplyProxy const&reply, Category::SCPtr const& override_category) const {
    const bool show_title = !query().query_string().empty();

    auto cat = override_category;
    if (!cat)
    {
        CategoryRenderer renderer = make_renderer(query().query_string() == "" ? ALBUMS_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION, MISSING_ALBUM_ART);
        cat = reply->register_category("albums", show_title ? _("Albums") : "", SONGS_CATEGORY_ICON, renderer);
    }

    mediascanner::Filter filter;
    filter.setLimit(MAX_RESULTS);
    for (const auto &album : scope.store->queryAlbums(query().query_string(), filter)) {
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
    artwork.add_attribute_mapping("source", "art");
    artwork.add_attribute_value("fallback", Variant(
            scope.scope_directory() + "/" + MISSING_ALBUM_ART));

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
        std::string uri = res.uri();
        if (uri.find("file://") == 0)
        {
            uri = "music://" + uri.substr(7); // replace file:// with music://
        }

        VariantBuilder builder;
        builder.add_tuple({
                {"id", Variant("play")},
                {"uri", Variant(uri)},
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
    artwork.add_attribute_mapping("source", "art");
    artwork.add_attribute_value("fallback", Variant(
            scope.scope_directory() + "/" + MISSING_ALBUM_ART));

    PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "artist");

    PreviewWidget actions("actions", "actions");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"id", Variant("play")},
                {"uri", Variant(res.uri())},
                {"label", Variant(_("Play in music app"))}
            });
        actions.add_attribute_value("actions", builder.end());
    }

    PreviewWidget tracks("tracks", "audio");
    VariantBuilder builder;
    std::string artist = res["artist"].get_string();
    std::string album_name = res["title"].get_string();
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
