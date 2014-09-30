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

#include "../src/video-scope.h"

using namespace mediascanner;
using namespace unity::scopes;
using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Matcher;
using ::testing::Property;
using ::testing::Return;
using ::testing::Truly;

class VideoScopeTest : public unity::scopes::testing::TypedScopeFixture<VideoScope> {
protected:
    virtual void SetUp() {
        cachedir = "/tmp/mediastore.XXXXXX";
        // mkdtemp edits the string in place without changing its length
        if (mkdtemp(const_cast<char*>(cachedir.c_str())) == nullptr) {
            throw std::runtime_error(strerror(errno));
        }
        ASSERT_EQ(0, setenv("MEDIASCANNER_CACHEDIR", cachedir.c_str(), 1));
        store.reset(new MediaStore(MS_READ_WRITE));

        unity::scopes::testing::TypedScopeFixture<VideoScope>::SetUp();
    }

    virtual void TearDown() {
        unity::scopes::testing::TypedScopeFixture<VideoScope>::TearDown();

        store.reset();
        if (!cachedir.empty()) {
            std::string cmd = "rm -rf " + cachedir;
            ASSERT_EQ(0, system(cmd.c_str()));
        }
    }

    void populateStore() {
        MediaStore store(MS_READ_WRITE);
        {
            MediaFileBuilder builder("/path/elephantsdream.ogv");
            builder.setType(VideoMedia);
            builder.setTitle("Elephant's Dream");
            builder.setDate("2006-03-24");
            builder.setDuration(654);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/bigbuckbunny.ogv");
            builder.setType(VideoMedia);
            builder.setTitle("Big Buck Bunny");
            builder.setDate("2008-04-10");
            builder.setDuration(596);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/sintel.ogv");
            builder.setType(VideoMedia);
            builder.setTitle("Sintel");
            builder.setDate("2010-09-27");
            builder.setDuration(888);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/path/tearsofsteel.ogv");
            builder.setType(VideoMedia);
            builder.setTitle("Tears of Steel");
            builder.setDate("2012-09-26");
            builder.setDuration(734);
            store.insert(builder.build());
        }
        {
            MediaFileBuilder builder("/home/phablet/Videos/video20140702_0001.mp4");
            builder.setType(VideoMedia);
            builder.setTitle("From camera");
            builder.setDuration(100);
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

TEST_F(VideoScopeTest, QueryResult) {
    populateStore();

    CannedQuery q("mediascanner-video", "bunny", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr category = std::make_shared<unity::scopes::testing::Category>(
        "local", "My Videos", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(AllOf(
            ResultProp("uri", "file:///path/bigbuckbunny.ogv"),
            ResultProp("dnd_uri", "file:///path/bigbuckbunny.ogv"),
            ResultProp("title", "Big Buck Bunny"),
            ResultProp("duration", 596)))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

/* Check that we get some results for a short query */
TEST_F(VideoScopeTest, ShortQuery) {
    populateStore();

    CannedQuery q("mediascanner-music", "s", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr category = std::make_shared<unity::scopes::testing::Category>(
        "local", "My Videos", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Sintel"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", "Tears of Steel"))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(VideoScopeTest, SurfacingQuery) {
    populateStore();

    CannedQuery q("mediascanner-music", "", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr category = std::make_shared<unity::scopes::testing::Category>(
        "local", "My Videos", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Elephant's Dream")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Big Buck Bunny")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Sintel")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Tears of Steel")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("From camera")))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(VideoScopeTest, CameraDepartmentQuery) {
    populateStore();

    CannedQuery q("mediascanner-music", "", "camera");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr category = std::make_shared<unity::scopes::testing::Category>(
        "local", "My Videos", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("From camera")))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(VideoScopeTest, DownloadsDepartmentQuery) {
    populateStore();

    CannedQuery q("mediascanner-music", "", "downloads");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr category = std::make_shared<unity::scopes::testing::Category>(
        "local", "My Videos", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_departments(_));
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Elephant's Dream")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Big Buck Bunny")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Sintel")))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<CategorisedResult const&>(ResultProp("title", Variant("Tears of Steel")))))
        .WillOnce(Return(true));

    SearchReplyProxy proxy(&reply, [](SearchReply*){});
    query->run(proxy);
}

TEST_F(VideoScopeTest, PreviewVideo) {
    unity::scopes::testing::Result result;
    result.set_uri("file:///xyz");
    result.set_title("Video title");
    result["duration"] = 42;

    ActionMetadata hints("en_AU", "phone");
    auto previewer = scope->preview(result, hints);

    unity::scopes::testing::MockPreviewReply reply;
    EXPECT_CALL(reply, register_layout(_))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<PreviewWidgetList const&>(ElementsAre(
        AllOf(
            Property(&PreviewWidget::id, "video"),
            Property(&PreviewWidget::widget_type, "video"),
            Truly([](const PreviewWidget &w) -> bool {
                    return
                        w.attribute_values().at("source").get_string() == "video:///xyz" &&
                        w.attribute_mappings().at("screenshot") == "art";
                })
            ),
        AllOf(
            Property(&PreviewWidget::id, "header"),
            Property(&PreviewWidget::widget_type, "header"),
            Truly([](const PreviewWidget &w) -> bool {
                    return w.attribute_mappings().at("title") == "title";
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
                        play.at("uri").get_string() == "video:///xyz";
                })
            )))))
        .WillOnce(Return(true));

    PreviewReplyProxy proxy(&reply, [](PreviewReply*){});
    previewer->run(proxy);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
