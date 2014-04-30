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
#include <unity/scopes/testing/MockPreviewReply.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/Result.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

#include "../src/video-scope.h"

using namespace mediascanner;
using namespace unity::scopes;
using ::testing::_;
using ::testing::AllOf;
using ::testing::Return;

class VideoScopeTest : public unity::scopes::testing::TypedScopeFixture<VideoScope> {
protected:
    virtual void SetUp() {
        cachedir = "/tmp/mediastore.XXXXXX";
        // mkdtemp edits the string in place without changing its length
        if (mkdtemp(const_cast<char*>(cachedir.c_str())) == NULL) {
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

        store.insert(MediaFile("/path/elephantsdream.ogv", "video/ogg", "etag",
                               "Elephant's Dream", "2006-03-24", "",
                               "", "", 0, 654, VideoMedia));
        store.insert(MediaFile("/path/bigbuckbunny.ogv", "video/ogg", "etag",
                               "Big Buck Bunny", "2008-04-10", "",
                               "", "", 0, 596, VideoMedia));
        store.insert(MediaFile("/path/sintel.ogv", "video/ogg", "etag",
                               "Sintel", "2010-09-27", "",
                               "", "", 0, 888, VideoMedia));
        store.insert(MediaFile("/path/tearsofsteel.ogv", "video/ogg", "etag",
                               "Tears of Steel", "2012-09-26", "",
                               "", "", 0, 734, VideoMedia));
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

TEST_F(VideoScopeTest, QueryResult) {
    populateStore();

    CannedQuery q("mediascanner-video", "bunny", "");
    SearchMetadata hints("en_AU", "phone");
    auto query = scope->search(q, hints);

    Category::SCPtr category = std::make_shared<unity::scopes::testing::Category>(
        "local", "My Videos", "icon", CategoryRenderer());
    unity::scopes::testing::MockSearchReply reply;
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(AllOf(
            ResultProp("uri", Variant("file:///path/bigbuckbunny.ogv")),
            ResultProp("dnd_uri", Variant("file:///path/bigbuckbunny.ogv")),
            ResultProp("title", Variant("Big Buck Bunny")),
            ResultProp("duration", Variant(596)))))
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
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Sintel"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Tears of Steel"))))
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
    EXPECT_CALL(reply, register_category("local", _, _, _))
        .WillOnce(Return(category));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Elephant's Dream"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Big Buck Bunny"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Sintel"))))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push(ResultProp("title", Variant("Tears of Steel"))))
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
