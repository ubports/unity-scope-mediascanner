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
    layout1col.add_column({"header", "art", "tracks"});

    layout2col.add_column({"header", "art"});
    layout2col.add_column({"tracks"});

    layout3col.add_column({"header", "art"});
    layout3col.add_column({"tracks"});
    layout3col.add_column({});
    reply->register_layout({layout1col, layout2col, layout3col});

    PreviewWidget header("header", "header");
    header.add_attribute("title", Variant(result.title()));
    header.add_attribute("subtitle", result["artist"]);

    PreviewWidget artwork("art", "image");
    // XXX: album art?
    artwork.add_attribute("source", Variant(""));

    PreviewWidget tracks("tracks", "audio");
    VariantBuilder builder;
    builder.add_tuple({
            {"title", Variant(result.title())},
            {"source", Variant(result.uri())},
            {"length", result["duration"]}
        });
    tracks.add_attribute("tracks", builder.end());

    reply->push({header, artwork, tracks});
}

extern "C" ScopeBase * UNITY_SCOPE_CREATE_FUNCTION() {
    return new MusicScope;
}

extern "C" void UNITY_SCOPE_DESTROY_FUNCTION(ScopeBase *scope) {
    delete scope;
}

#if 0
UnityAbstractPreview *
music_preview (UnityResultPreviewer *previewer, void *user_data)
{
    const char *uri = previewer->result.uri;
    const char *title = previewer->result.title;
    const char *artist = "";
    const char *album = "";
    int duration = 0;
    int track_number = 0;

    if (previewer->result.metadata != NULL) {
        GVariant *variant;

        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char *>("artist")));
        if (variant) {
            artist = g_variant_get_string (variant, NULL);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("album")));
        if (variant) {
            album = g_variant_get_string (variant, NULL);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("duration")));
        if (variant) {
            duration = g_variant_get_int32 (variant);
        }
        variant = static_cast<GVariant*>(g_hash_table_lookup (previewer->result.metadata, const_cast<char*>("track-number")));
        if (variant) {
            track_number = g_variant_get_int32 (variant);
        }
    }

    GIcon *image = g_icon_new_for_string (previewer->result.icon_hint, NULL);
    UnityMusicPreview *preview = unity_music_preview_new (
        title, artist, image);
    g_object_unref (image);

    UnityTrackMetadata *track = unity_track_metadata_new ();
    unity_track_metadata_set_uri (track, uri);
    unity_track_metadata_set_track_no (track, track_number);
    unity_track_metadata_set_title (track, title);
    unity_track_metadata_set_artist (track, artist);
    unity_track_metadata_set_album (track, album);
    unity_track_metadata_set_length (track, duration);
    unity_music_preview_add_track (preview, track);

    UnityPreviewAction *play_action = unity_preview_action_new (
        "play", _("Play"), NULL);
    unity_preview_add_action (UNITY_PREVIEW (preview), play_action);

    return UNITY_ABSTRACT_PREVIEW (preview);
}


UnityAbstractScope *
music_scope_new (std::shared_ptr<MediaStore> store)
{
    UnitySimpleScope *scope = unity_simple_scope_new ();

    unity_simple_scope_set_group_name (scope, DBUS_NAME);
    unity_simple_scope_set_unique_name (scope, DBUS_MUSIC_PATH);

    /* Set up schema */
    UnitySchema *schema = unity_schema_new ();
    unity_schema_add_field (schema, "duration", "i", UNITY_SCHEMA_FIELD_TYPE_REQUIRED);
    unity_schema_add_field (schema, "artist", "s", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_schema_add_field (schema, "album", "s", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_schema_add_field (schema, "track-number", "i", UNITY_SCHEMA_FIELD_TYPE_OPTIONAL);
    unity_simple_scope_set_schema (scope, schema);

    /* Set up categories */
    UnityCategorySet *categories = unity_category_set_new ();
    GFile *icon_dir = g_file_new_for_path ("/usr/share/icons/unity-icon-theme/places/svg");

    GFile *icon_file = g_file_get_child (icon_dir, "group-songs.svg");
    GIcon *icon = g_file_icon_new (icon_file);
    g_object_unref (icon_file);
    unity_category_set_add (categories,
                            unity_category_new ("global", _("Music"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    unity_category_set_add (categories,
                            unity_category_new ("songs", _("Songs"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);

    icon_file = g_file_get_child (icon_dir, "group-albums.svg");
    icon = g_file_icon_new (icon_file);
    g_object_unref (icon_file);
    unity_category_set_add (categories,
                            unity_category_new ("albums", _("Albums"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);

    icon_file = g_file_get_child (icon_dir, "group-treat-yourself.svg");
    icon = g_file_icon_new (icon_file);
    g_object_unref (icon_file);
    unity_category_set_add (categories,
                            unity_category_new ("more", _("More suggestions"),
                                                icon, UNITY_CATEGORY_RENDERER_DEFAULT));
    g_object_unref (icon);

    g_object_unref (icon_dir);
    unity_simple_scope_set_category_set (scope, categories);

    /* Set up filters */
    unity_simple_scope_set_filter_set (scope, music_get_filters ());

    /* Set up search */
    ScopeSearchData *search_data = g_new0(ScopeSearchData, 1);
    search_data->store = store;
    search_data->media_type = AudioMedia;
    search_data->add_result = music_add_result;
    //search_data->apply_filters = music_apply_filters;
    setup_search (scope, search_data);
    // XXX: handle cleanup of search_data

    unity_simple_scope_set_preview_func (
        scope, music_preview, NULL, (GDestroyNotify)NULL);

    return UNITY_ABSTRACT_SCOPE (scope);
}

#endif
