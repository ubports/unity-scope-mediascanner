#include <cerrno>
#include <cstring>
#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>
#include <mediascanner/MediaStore.hh>
#include <unity/scopes/testing/Category.h>
#include <unity/scopes/testing/MockPreviewReply.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/Result.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

#include "../src/music-scope.h"

using namespace mediascanner;
using namespace unity::scopes;
using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Matcher;
using ::testing::Property;
using ::testing::Return;
using ::testing::Truly;

class MusicScopeTest : public unity::scopes::testing::TypedScopeFixture<MusicScope> {
protected:
    virtual void SetUp() {
        cachedir = "/tmp/mediastore.XXXXXX";
        // mkdtemp edits the string in place without changing its length
        if (mkdtemp(const_cast<char*>(cachedir.c_str())) == nullptr) {
            throw std::runtime_error(strerror(errno));
        }
        ASSERT_EQ(0, setenv("MEDIASCANNER_CACHEDIR", cachedir.c_str(), 1));
        store.reset(new MediaStore(MS_READ_WRITE));

        unity::scopes::testing::TypedScopeFixture<MusicScope>::SetUp();
    }

    virtual void TearDown() {
        unity::scopes::testing::TypedScopeFixture<MusicScope>::TearDown();

        store.reset();
        if (!cachedir.empty()) {
            std::string cmd = "rm -rf " + cachedir;
            ASSERT_EQ(0, system(cmd.c_str()));
        }
    }

    void populateStore() {
        MediaStore store(MS_READ_WRITE);
        {
            MediaFileBuilder builder("/path/foo1.ogg");
            builder.setType(AudioMedia);
            builder.setGenre("Rock");
            builder.setTitle("Straight Through The Sun");
            builder.setAuthor("Spiderbait");
            builder.setAlbum("Spiderbait");
            builder.setDate("2013-11-15");
            builder.setTrackNumber(1);
            builder.setDuration(235);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/foo2.ogg");
            builder.setType(AudioMedia);
            builder.setGenre("Rock");
            builder.setTitle("It's Beautiful");
            builder.setAuthor("Spiderbait");
            builder.setAlbum("Spiderbait");
            builder.setDate("2013-11-15");
            builder.setTrackNumber(2);
            builder.setDuration(220);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/foo3.ogg");
            builder.setType(AudioMedia);
            builder.setGenre("Rock");
            builder.setTitle("Buy Me a Pony");
            builder.setAuthor("Spiderbait");
            builder.setAlbum("Ivy and the Big Apples");
            builder.setDate("1996-10-04");
            builder.setTrackNumber(3);
            builder.setDuration(104);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/foo4.ogg");
            builder.setType(AudioMedia);
            builder.setGenre("Folk");
            builder.setTitle("Peaches & Cream");
            builder.setAuthor("The John Butler Trio");
            builder.setAlbum("Sunrise Over Sea");
            builder.setDate("2004-03-08");
            builder.setTrackNumber(2);
            builder.setDuration(407);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/foo5.ogg");
            builder.setType(AudioMedia);
            builder.setGenre("Folk");
            builder.setTitle("Zebra");
            builder.setAuthor("The John Butler Trio");
            builder.setAlbum("Sunrise Over Sea");
            builder.setDate("2004-03-08");
            builder.setTrackNumber(10);
            builder.setDuration(237);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/foo6.ogg");
            builder.setType(AudioMedia);
            builder.setTitle("Revolution");
            builder.setAuthor("The John Butler Trio");
            builder.setAlbum("April Uprising");
            builder.setDate("2010-01-01");
            builder.setTrackNumber(1);
            builder.setDuration(305);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/foo7.ogg");
            builder.setType(AudioMedia);
            builder.setGenre("Metal");
            builder.setTitle("One Way Road");
            builder.setAuthor("The John Butler Trio");
            builder.setAlbum("April Uprising");
            builder.setDate("2010-01-01");
            builder.setTrackNumber(2);
            builder.setDuration(185);
            store.insert(builder.build());
        }
    }

    std::string cachedir;
    std::unique_ptr<MediaStore> store;
};

MATCHER_P2(ResultProp, prop, value, "") {
    if (arg.contains(prop)) {
        *result_listener << "result[" << prop << "] is " << arg[prop].serialize_json();
    } else {
        *result_listener << "result[" << prop << "] is not set";
    }
    return arg.contains(prop) && arg[prop] == unity::scopes::Variant(value);
}

MATCHER_P(ResultUriMatchesCannedQuery, q, "") {
    *result_listener << "result.uri is " << arg.uri();
    auto const query = unity::scopes::CannedQuery::from_uri(arg.uri());
    return query.scope_id() == q.scope_id()
        && query.query_string() == q.query_string()
        && query.department_id() == q.department_id();
}

TEST_F(MusicScopeTest, QueryResult) {
    populateStore();

    CannedQuery q("mediascanner-music", "road", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr artists_category = std::make_shared<unity::scopes::testing::Category>(
        "artists", "Artists", "icon", CategoryRenderer());
    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "Albums", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_category("artists", _, _, _))
        .WillOnce(Return(artists_category));
    EXPECT_CALL(reply, register_category("songs", _, _, _))
        .WillOnce(Return(songs_category));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .WillOnce(Return(albums_category));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(CannedQuery("mediascanner-music", "The John Butler Trio", "albums_of_artist")),
            ResultProp("title", "The John Butler Trio")))))
        .WillOnce(Return(true));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("uri", "file:///path/foo7.ogg"),
            ResultProp("dnd_uri", "file:///path/foo7.ogg"),
            ResultProp("title", "One Way Road"),
            ResultProp("duration", 185),
            ResultProp("album", "April Uprising"),
            ResultProp("artist", "The John Butler Trio"),
            ResultProp("track-number", 2)))))
        .WillOnce(Return(true));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("uri", "album:///The%20John%20Butler%20Trio/April%20Uprising"),
            ResultProp("title", "April Uprising"),
            ResultProp("album", "April Uprising"),
            ResultProp("artist", "The John Butler Trio"),
            ResultProp("isalbum", true)))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

/* Check that we get some results for a short query */
TEST_F(MusicScopeTest, ShortQuery) {
    populateStore();

    CannedQuery q("mediascanner-music", "r", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr artists_category = std::make_shared<unity::scopes::testing::Category>(
        "artists", "Artists", "icon", CategoryRenderer());
    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Songs", "icon", CategoryRenderer());
    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "Albums", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_category("artists", _, _, _))
        .WillOnce(Return(artists_category));
    EXPECT_CALL(reply, register_category("songs", _, _, _))
        .WillOnce(Return(songs_category));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .WillOnce(Return(albums_category));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(CannedQuery("mediascanner-music", "The John Butler Trio", "albums_of_artist")),
            ResultProp("title", "The John Butler Trio")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "One Way Road"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Revolution"))))
        .WillOnce(Return(true));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("title", "April Uprising"),
            ResultProp("isalbum", true)))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(MusicScopeTest, SurfacingQuery) {
    populateStore();

    CannedQuery q("mediascanner-music", "", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "Artists", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;

    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("artists", _, _, _))
        .WillOnce(Return(albums_category));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
                        ResultUriMatchesCannedQuery(CannedQuery("mediascanner-music", "Spiderbait", "albums_of_artist")),
                        ResultProp("title", "Spiderbait")
                        ))))
        .WillOnce(Return(true));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
                        ResultUriMatchesCannedQuery(CannedQuery("mediascanner-music", "The John Butler Trio", "albums_of_artist")),
                        ResultProp("title", "The John Butler Trio"))
            )))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(MusicScopeTest, TracksDepartmentSurfacing) {
    populateStore();

    CannedQuery q("mediascanner-music", "", "tracks");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Tracks", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("songs", _, _, _))
        .WillOnce(Return(songs_category));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Straight Through The Sun"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "It's Beautiful"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Buy Me a Pony"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Peaches & Cream"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Zebra"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Revolution"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "One Way Road"))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(MusicScopeTest, GenresDepartmentSurfacing) {
    populateStore();

    CannedQuery q("mediascanner-music", "", "genre:Rock");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;

    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .WillOnce(Return(albums_category));

    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("title", "Spiderbait"),
            ResultProp("isalbum", true)))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("title", "Ivy and the Big Apples"),
            ResultProp("isalbum", true)))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(MusicScopeTest, PreviewSong) {
    unity::scopes::testing::Result result;
    result.set_uri("file:///xyz");
    result.set_title("Song title");
    result["artist"] = "Artist name";
    result["album"] = "Album name";
    result["duration"] = 42;
    result["track-number"] = 5;

    ActionMetadata hints("en_AU", "phone");
    auto previewer = scope->preview(result, hints);

    unity::scopes::testing::MockPreviewReply reply;
    EXPECT_CALL(reply, register_layout(_))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<PreviewWidgetList const&>(ElementsAre(
        AllOf(
            Property(&PreviewWidget::id, "art"),
            Property(&PreviewWidget::widget_type, "image"),
            Truly([](const PreviewWidget &w) -> bool {
                    return w.attribute_mappings().at("source") == "art";
                })
            ),
        AllOf(
            Property(&PreviewWidget::id, "header"),
            Property(&PreviewWidget::widget_type, "header"),
            Truly([](const PreviewWidget &w) -> bool {
                    return
                        w.attribute_mappings().at("title") == "title" &&
                        w.attribute_mappings().at("subtitle") == "artist";
                })
            ),
        AllOf(
            Property(&PreviewWidget::id, "actions"),
            Property(&PreviewWidget::widget_type, "actions"),
            Truly([](const PreviewWidget &w) -> bool {
                    const auto actions = w.attribute_values().at("actions").get_array();
                    if (actions.size() != 1) {
                        return false;
                    }
                    const auto play = actions[0].get_dict();
                    return
                        play.at("id").get_string() == "play" &&
                        play.at("uri").get_string() == "music:///xyz";
                })
            ),
        AllOf(
            Property(&PreviewWidget::id, "tracks"),
            Property(&PreviewWidget::widget_type, "audio"),
            Truly([](const PreviewWidget &w) -> bool {
                    const auto tracks = w.attribute_values().at("tracks").get_array();
                    if (tracks.size() != 1) {
                        return false;
                    }
                    const auto track = tracks[0].get_dict();
                    return
                        track.at("title").get_string() == "Song title" &&
                        track.at("source").get_string() == "file:///xyz" &&
                        track.at("length").get_int() == 42;
                })
            )))))
        .WillOnce(Return(true));

    PreviewReplyProxy proxy(&reply, [](PreviewReply*){});
    previewer->run(proxy);
}

TEST_F(MusicScopeTest, PreviewAlbum) {
    populateStore();

    unity::scopes::testing::Result result;
    result.set_uri("album:///The%20John%20Butler%20Trio/April%20Uprising");
    result.set_title("April Uprising");
    result["artist"] = "The John Butler Trio";
    result["album"] = "April Uprising";
    result["isalbum"] = true;

    ActionMetadata hints("en_AU", "phone");
    auto previewer = scope->preview(result, hints);

    unity::scopes::testing::MockPreviewReply reply;
    EXPECT_CALL(reply, register_layout(_));
    EXPECT_CALL(reply, push(Matcher<PreviewWidgetList const&>(ElementsAre(
        AllOf(
            Property(&PreviewWidget::id, "art"),
            Property(&PreviewWidget::widget_type, "image"),
            Truly([](const PreviewWidget &w) -> bool {
                    return w.attribute_mappings().at("source") == "art";
                })
            ),
        AllOf(
            Property(&PreviewWidget::id, "header"),
            Property(&PreviewWidget::widget_type, "header"),
            Truly([](const PreviewWidget &w) -> bool {
                    return
                        w.attribute_mappings().at("title") == "title" &&
                        w.attribute_mappings().at("subtitle") == "artist";
                })
            ),
        AllOf(
            Property(&PreviewWidget::id, "actions"),
            Property(&PreviewWidget::widget_type, "actions"),
            Truly([](const PreviewWidget &w) -> bool {
                    const auto actions = w.attribute_values().at("actions").get_array();
                    if (actions.size() != 1) {
                        return false;
                    }
                    const auto play = actions[0].get_dict();
                    return
                        play.at("id").get_string() == "play" &&
                        play.at("uri").get_string() == "album:///The%20John%20Butler%20Trio/April%20Uprising";
                })
            ),
        AllOf(
            Property(&PreviewWidget::id, "tracks"),
            Property(&PreviewWidget::widget_type, "audio"),
            Truly([](const PreviewWidget &w) -> bool {
                    const auto tracks = w.attribute_values().at("tracks").get_array();
                    if (tracks.size() != 2) {
                        return false;
                    }
                    const auto track1 = tracks[0].get_dict();
                    const auto track2 = tracks[1].get_dict();
                    return
                        track1.at("title").get_string() == "Revolution" &&
                        track1.at("source").get_string() == "file:///path/foo6.ogg" &&
                        track1.at("length").get_int() == 305 &&
                        track2.at("title").get_string() == "One Way Road" &&
                        track2.at("source").get_string() == "file:///path/foo7.ogg" &&
                        track2.at("length").get_int() == 185;
                })
            )))));

    PreviewReplyProxy proxy(&reply, [](PreviewReply*){});
    previewer->run(proxy);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
