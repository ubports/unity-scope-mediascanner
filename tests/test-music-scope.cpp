#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>
#include <unity/scopes/testing/Category.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

#include "../src/music-scope.h"

using namespace mediascanner;
using namespace unity::scopes;
using ::testing::_;
using ::testing::AllOf;
using ::testing::Return;

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

        store.insert(MediaFile("/path/foo1.ogg", "audio/ogg", "etag",
                               "Straight Through The Sun", "2013-11-15", "Spiderbait",
                               "Spiderbait", "Spiderbait", 1, 235, AudioMedia));
        store.insert(MediaFile("/path/foo2.ogg", "audio/ogg", "etag",
                               "It's Beautiful", "2013-11-15", "Spiderbait",
                               "Spiderbait", "Spiderbait", 2, 220, AudioMedia));

        store.insert(MediaFile("/path/foo3.ogg", "audio/ogg", "etag",
                               "Buy Me a Pony", "1996-10-04", "Spiderbait",
                               "Ivy and the Big Apples", "Spiderbait", 3, 104, AudioMedia));

        store.insert(MediaFile("/path/foo4.ogg", "audio/ogg", "etag",
                               "Peaches & Cream", "2004-03-08", "The John Butler Trio",
                               "Sunrise Over Sea", "The John Butler Trio", 2, 407, AudioMedia));
        store.insert(MediaFile("/path/foo5.ogg", "audio/ogg", "etag",
                               "Zebra", "2004-03-08", "The John Butler Trio",
                               "Sunrise Over Sea", "The John Butler Trio", 10, 237, AudioMedia));

        store.insert(MediaFile("/path/foo6.ogg", "audio/ogg", "etag",
                               "Revolution", "2010-01-01", "The John Butler Trio",
                               "April Uprising", "The John Butler Trio", 1, 305, AudioMedia));
        store.insert(MediaFile("/path/foo7.ogg", "audio/ogg", "etag",
                               "One Way Road", "2010-01-01", "The John Butler Trio",
                               "April Uprising", "The John Butler Trio", 2, 185, AudioMedia));
    }

    std::string cachedir;
    std::unique_ptr<MediaStore> store;
};

// To make errors more readable
std::ostream& operator <<(std::ostream& os, const Variant& v) {
    os << "Variant(" << v.serialize_json() << ")";
    return os;
}

MATCHER_P2(ResultProp, prop, value, "") {
    if (arg.contains(prop)) {
        *result_listener << "result[" << prop << "] is " << arg[prop].serialize_json();
    } else {
        *result_listener << "result[" << prop << "] is not set";
    }
    return arg.contains(prop) && arg[prop] == value;
}

TEST_F(MusicScopeTest, QueryResult) {
    populateStore();

    CannedQuery q("mediascanner-music", "road", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Songs", "icon", CategoryRenderer());
    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "Albums", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_category("songs", _, _, _))
        .Times(1)
        .WillOnce(Return(songs_category));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .Times(1)
        .WillOnce(Return(albums_category));
    EXPECT_CALL(reply, push(AllOf(
            ResultProp("uri", Variant("file:///path/foo7.ogg")),
            ResultProp("dnd_uri", Variant("file:///path/foo7.ogg")),
            ResultProp("title", Variant("One Way Road")),
            ResultProp("duration", Variant(185)),
            ResultProp("album", Variant("April Uprising")),
            ResultProp("artist", Variant("The John Butler Trio")),
            ResultProp("track-number", Variant(2)))))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(reply, push(AllOf(
            ResultProp("uri", Variant("album:///The%20John%20Butler%20Trio/April%20Uprising")),
            ResultProp("title", Variant("April Uprising")),
            ResultProp("album", Variant("April Uprising")),
            ResultProp("artist", Variant("The John Butler Trio")),
            ResultProp("isalbum", Variant(true)))))
        .Times(1)
        .WillOnce(Return(true));

    SearchReplyProxy proxy{&reply, [](unity::scopes::SearchReply*){}};
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
        .Times(1)
        .WillOnce(Return(songs_category));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .Times(1)
        .WillOnce(Return(albums_category));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("One Way Road"))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Revolution"))))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(reply, push(AllOf(
            ResultProp("title", Variant("April Uprising")),
            ResultProp("isalbum", Variant(true)))))
        .Times(1)
        .WillOnce(Return(true));

    SearchReplyProxy proxy{&reply, [](unity::scopes::SearchReply*){}};
    query->run(proxy);
}

TEST_F(MusicScopeTest, SurfacingQuery) {
    populateStore();

    CannedQuery q("mediascanner-music", "", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr songs_category = std::make_shared<unity::scopes::testing::Category>(
        "songs", "Songs", "icon", CategoryRenderer());
    Category::SCPtr albums_category = std::make_shared<unity::scopes::testing::Category>(
        "albums", "Albums", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_category("songs", _, _, _))
        .Times(1)
        .WillOnce(Return(songs_category));
    EXPECT_CALL(reply, register_category("albums", _, _, _))
        .Times(1)
        .WillOnce(Return(albums_category));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Straight Through The Sun"))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("It's Beautiful"))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Buy Me a Pony"))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Peaches & Cream"))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Zebra"))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Revolution"))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("One Way Road"))))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(reply, push(AllOf(
            ResultProp("title", Variant("Spiderbait")),
            ResultProp("isalbum", Variant(true)))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(AllOf(
            ResultProp("title", Variant("Ivy and the Big Apples")),
            ResultProp("isalbum", Variant(true)))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(AllOf(
            ResultProp("title", Variant("Sunrise Over Sea")),
            ResultProp("isalbum", Variant(true)))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(AllOf(
            ResultProp("title", Variant("April Uprising")),
            ResultProp("isalbum", Variant(true)))))
        .Times(1)
        .WillOnce(Return(true));


    SearchReplyProxy proxy{&reply, [](unity::scopes::SearchReply*){}};
    query->run(proxy);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
