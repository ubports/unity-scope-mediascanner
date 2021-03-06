#include <cerrno>
#include <cstring>
#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unity/scopes/testing/Category.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/MockScope.h>
#include <unity/scopes/testing/MockQueryCtrl.h>
#include <unity/scopes/ChildScope.h>
#include <unity/scopes/testing/ScopeMetadataBuilder.h>

#include "../src/musicaggregator/musicaggregatorscope.h"
#include "../src/musicaggregator/musicaggregatorquery.h"

using namespace unity::scopes;
using ::testing::_;
using ::testing::Return;

TEST(TestMusicAgregator, TestSurfacingSearch) {

    CannedQuery q("mediascanner-music", "", "");
    SearchMetadata hints("en_AU", "phone");

    std::shared_ptr<unity::scopes::testing::MockScope> soundcloud_scope(new unity::scopes::testing::MockScope("2", "2"));
    std::shared_ptr<unity::scopes::testing::MockScope> sevendigital_scope(new unity::scopes::testing::MockScope("3", "3"));
    std::shared_ptr<unity::scopes::testing::MockScope> songkick_scope(new unity::scopes::testing::MockScope("4", "4"));
    std::shared_ptr<unity::scopes::testing::MockScope> youtube_scope(new unity::scopes::testing::MockScope("5", "5"));
    std::shared_ptr<unity::scopes::testing::MockScope> local_scope(new unity::scopes::testing::MockScope("6", "6"));

    unity::scopes::ChildScopeList child_scopes {
        {"mediascanner-music", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("mediascanner-music")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(local_scope))()},
        {"com.canonical.scopes.sevendigital", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.canonical.scopes.sevendigital")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(sevendigital_scope))()},
        {"com.canonical.scopes.songkick_songkick", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.canonical.scopes.songkick_songkick")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(songkick_scope))()},
        {"com.ubuntu.scopes.youtube_youtube", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.ubuntu.scopes.youtube_youtube")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(youtube_scope))()},
        {"com.ubuntu.scopes.soundcloud_soundcloud", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.ubuntu.scopes.soundcloud_soundcloud")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(soundcloud_scope))()},
    };

    MusicAggregatorQuery query(q, hints, child_scopes);

    unity::scopes::testing::MockSearchReply reply;

    Category::SCPtr sevendigital_category = std::make_shared<unity::scopes::testing::Category>(
        "7digital", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr songkick_category = std::make_shared<unity::scopes::testing::Category>(
        "songkick", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr youtube_category = std::make_shared<unity::scopes::testing::Category>(
        "youtube", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr soundcloud_category = std::make_shared<unity::scopes::testing::Category>(
        "soundcloud", "Tracks", "icon", CategoryRenderer());

    std::shared_ptr<unity::scopes::testing::MockQueryCtrl> queryctrl(new unity::scopes::testing::MockQueryCtrl());

    EXPECT_CALL(reply, register_category("7digital", _, _, _,_))
        .WillOnce(Return(sevendigital_category));
    EXPECT_CALL(reply, register_category("songkick", _, _, _,_))
        .WillOnce(Return(songkick_category));
    EXPECT_CALL(reply, register_category("youtube", _, _, _,_))
        .WillOnce(Return(youtube_category));
    EXPECT_CALL(reply, register_category("soundcloud", _, _, _,_))
        .WillOnce(Return(soundcloud_category));

    // check that each scope calls search with the correct parameters...
    EXPECT_CALL(*local_scope.get(), search("","", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*sevendigital_scope.get(), search("","newreleases", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*songkick_scope.get(), search("","", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*youtube_scope.get(), search("","aggregated:musicaggregator", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*soundcloud_scope.get(), search("","", _, _, _)).WillOnce(Return(queryctrl));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query.run(proxy);
}

TEST(TestMusicAgregator, TestSpecificSearch) {

    CannedQuery q("mediascanner-music", "test", "");
    SearchMetadata hints("en_AU", "phone");

    std::shared_ptr<unity::scopes::testing::MockScope> soundcloud_scope(new unity::scopes::testing::MockScope("2", "2"));
    std::shared_ptr<unity::scopes::testing::MockScope> sevendigital_scope(new unity::scopes::testing::MockScope("3", "3"));
    std::shared_ptr<unity::scopes::testing::MockScope> songkick_scope(new unity::scopes::testing::MockScope("4", "4"));
    std::shared_ptr<unity::scopes::testing::MockScope> youtube_scope(new unity::scopes::testing::MockScope("5", "5"));
    std::shared_ptr<unity::scopes::testing::MockScope> local_scope(new unity::scopes::testing::MockScope("6", "6"));

    unity::scopes::ChildScopeList child_scopes {
        {"mediascanner-music", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("mediascanner-music")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(local_scope))()},
        {"com.canonical.scopes.sevendigital", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.canonical.scopes.sevendigital")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(sevendigital_scope))()},
        {"com.canonical.scopes.songkick_songkick", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.canonical.scopes.songkick_songkick")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(songkick_scope))()},
        {"com.ubuntu.scopes.youtube_youtube", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.ubuntu.scopes.youtube_youtube")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(youtube_scope))()},
        {"com.ubuntu.scopes.soundcloud_soundcloud", unity::scopes::testing::ScopeMetadataBuilder()
            .scope_id("com.ubuntu.scopes.soundcloud_soundcloud")
                .display_name(" ").description(" ")
                .author(" ")
                .proxy(unity::scopes::ScopeProxy(soundcloud_scope))()},
    };

    MusicAggregatorQuery query( q, hints, child_scopes);

    unity::scopes::testing::MockSearchReply reply;

    Category::SCPtr sevendigital_category = std::make_shared<unity::scopes::testing::Category>(
        "7digital", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr songkick_category = std::make_shared<unity::scopes::testing::Category>(
        "songkick", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr youtube_category = std::make_shared<unity::scopes::testing::Category>(
        "youtube", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr soundcloud_category = std::make_shared<unity::scopes::testing::Category>(
        "soundcloud", "Tracks", "icon", CategoryRenderer());


    std::shared_ptr<unity::scopes::testing::MockQueryCtrl> queryctrl(new unity::scopes::testing::MockQueryCtrl());

    EXPECT_CALL(reply, register_category("7digital", _, _,_))
        .WillOnce(Return(sevendigital_category));
    EXPECT_CALL(reply, register_category("songkick", _, _,_))
        .WillOnce(Return(songkick_category));
    EXPECT_CALL(reply, register_category("youtube", _, _, _,_))
        .WillOnce(Return(youtube_category));
    EXPECT_CALL(reply, register_category("soundcloud", _, _,_))
        .WillOnce(Return(soundcloud_category));

    // check that each scope calls search with the correct parameters...
    EXPECT_CALL(*sevendigital_scope.get(), search("test","", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*local_scope.get(), search("test","", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*songkick_scope.get(), search("test","", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*youtube_scope.get(), search("test","aggregated:musicaggregator", _, _, _)).WillOnce(Return(queryctrl));
    EXPECT_CALL(*soundcloud_scope.get(), search("test","", _, _, _)).WillOnce(Return(queryctrl));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query.run(proxy);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
