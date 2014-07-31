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
using ::testing::Return;
using ::testing::Matcher;

class MusicScopeTest : public unity::scopes::testing::TypedScopeFixture<MusicScope> {
protected:
    virtual void SetUp() {
        cachedir = "/tmp/mediastore.XXXXXX";
        // mkdtemp edits the string in place without changing its length
        if (mkdtemp(const_cast<char*>(cachedir.c_str())) == NULL) {
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

TEST_F(MusicScopeTest, QueryResult) {
    populateStore();

    CannedQuery q("mediascanner-music", "road", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "Albums", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_category("songs", _, _, _))
        .WillOnce(Return(songs_category));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .WillOnce(Return(albums_category));
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

    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Songs", "icon", CategoryRenderer());
    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "Albums", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_category("songs", _, _, _))
        .WillOnce(Return(songs_category));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .WillOnce(Return(albums_category));
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
        "albums", "Albums", "icon", CategoryRenderer());
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
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("title", "Sunrise Over Sea"),
            ResultProp("isalbum", true)))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("title", "April Uprising"),
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

    // MockPreviewReply is currently broken and can't be instantiated.
#if 0
    unity::scopes::testing::MockPreviewReply reply;
    PreviewReplyProxy proxy(&reply, [](PreviewReply*){});
    previewer->run(proxy);
#endif
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
