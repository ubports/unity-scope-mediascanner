#include <cerrno>
#include <cstring>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>

#include "../src/music-scope.h"

using namespace mediascanner;
using namespace unity::scopes;

class MusicScopeTest : public ::testing::Test
{
protected:
    virtual void SetUp() {
        cachedir = "/tmp/mediastore.XXXXXX";
        // mkdtemp edits the string in place without changing its length
        if (mkdtemp(const_cast<char*>(cachedir.c_str())) == NULL) {
            throw std::runtime_error(strerror(errno));
        }
        ASSERT_EQ(0, setenv("MEDIASCANNER_CACHEDIR", cachedir.c_str(), 1));
        store.reset(new MediaStore(MS_READ_WRITE));
    }

    virtual void TearDown() {
        store.reset();
        if (!cachedir.empty()) {
            std::string cmd = "rm -rf " + cachedir;
            ASSERT_EQ(0, system(cmd.c_str()));
        }
    }

    std::string cachedir;
    std::unique_ptr<MediaStore> store;
};

TEST_F(MusicScopeTest, Construct) {
    MusicScope scope;
}

TEST_F(MusicScopeTest, StartStop) {
    MusicScope scope;
    auto expected = ScopeBase::VERSION;
    ASSERT_EQ(expected, scope.start("mediascanner-music", nullptr));
    scope.stop();
}

TEST_F(MusicScopeTest, CreateQuery) {
    MusicScope scope;
    auto expected = ScopeBase::VERSION;
    ASSERT_EQ(expected, scope.start("mediascanner-music", nullptr));
    VariantMap hints;
    auto query = scope.create_query("query", hints);
    ASSERT_NE(nullptr, dynamic_cast<MusicQuery*>(query.get()));
    scope.stop();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
