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

#include "../src/musicaggregator/musicaggregatorscope.h"
#include "../src/musicaggregator/musicaggregatorquery.h"

using namespace unity::scopes;
using ::testing::_;
using ::testing::Return;

TEST(TestMusicAgregator, TestSurfacingSearch) {

    CannedQuery q("mediascanner-music", "", "");
    SearchMetadata hints("en_AU", "phone");

    ChildScopeList child_scopes;

    MusicAggregatorQuery query( q, hints, child_scopes);

    unity::scopes::testing::MockSearchReply reply;

    Category::SCPtr mymusic_category = std::make_shared<unity::scopes::testing::Category>(
        "mymusic", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr sevendigital_category = std::make_shared<unity::scopes::testing::Category>(
        "7digital", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr songkick_category = std::make_shared<unity::scopes::testing::Category>(
        "songkick", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr grooveshark_category = std::make_shared<unity::scopes::testing::Category>(
        "grooveshark", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr youtube_category = std::make_shared<unity::scopes::testing::Category>(
        "youtube", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr soundcloud_category = std::make_shared<unity::scopes::testing::Category>(
        "soundcloud", "Tracks", "icon", CategoryRenderer());

    std::shared_ptr<unity::scopes::testing::MockQueryCtrl> queryctrl(new unity::scopes::testing::MockQueryCtrl());

    EXPECT_CALL(reply, register_category("mymusic", _, _, _,_))
        .WillOnce(Return(mymusic_category));
    EXPECT_CALL(reply, register_category("7digital", _, _, _,_))
        .WillOnce(Return(sevendigital_category));
    EXPECT_CALL(reply, register_category("songkick", _, _, _,_))
        .WillOnce(Return(songkick_category));
    EXPECT_CALL(reply, register_category("grooveshark", _, _, _,_))
        .WillOnce(Return(grooveshark_category));
    EXPECT_CALL(reply, register_category("youtube", _, _, _,_))
        .WillOnce(Return(youtube_category));
    EXPECT_CALL(reply, register_category("soundcloud", _, _, _,_))
        .WillOnce(Return(soundcloud_category));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query.run(proxy);
}

TEST(TestMusicAgregator, TestSpecificSearch) {

    CannedQuery q("mediascanner-music", "test", "");
    SearchMetadata hints("en_AU", "phone");

    ChildScopeList child_scopes;

    MusicAggregatorQuery query( q, hints, child_scopes);

    unity::scopes::testing::MockSearchReply reply;

    Category::SCPtr mymusic_category = std::make_shared<unity::scopes::testing::Category>(
        "mymusic", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr sevendigital_category = std::make_shared<unity::scopes::testing::Category>(
        "7digital", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr songkick_category = std::make_shared<unity::scopes::testing::Category>(
        "songkick", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr grooveshark_category = std::make_shared<unity::scopes::testing::Category>(
        "grooveshark", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr youtube_category = std::make_shared<unity::scopes::testing::Category>(
        "youtube", "Tracks", "icon", CategoryRenderer());
    Category::SCPtr soundcloud_category = std::make_shared<unity::scopes::testing::Category>(
        "soundcloud", "Tracks", "icon", CategoryRenderer());


    std::shared_ptr<unity::scopes::testing::MockQueryCtrl> queryctrl(new unity::scopes::testing::MockQueryCtrl());

    EXPECT_CALL(reply, register_category("mymusic", _, _, _))
        .WillOnce(Return(mymusic_category));
    EXPECT_CALL(reply, register_category("7digital", _, _,_))
        .WillOnce(Return(sevendigital_category));
    EXPECT_CALL(reply, register_category("songkick", _, _,_))
        .WillOnce(Return(songkick_category));
    EXPECT_CALL(reply, register_category("grooveshark", _, _,_))
        .WillOnce(Return(grooveshark_category));
    EXPECT_CALL(reply, register_category("youtube", _, _, _,_))
        .WillOnce(Return(youtube_category));
    EXPECT_CALL(reply, register_category("soundcloud", _, _,_))
        .WillOnce(Return(soundcloud_category));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query.run(proxy);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
