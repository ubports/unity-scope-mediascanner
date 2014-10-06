/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by Pawel Stolowski <pawel.stolowski@canonical.com>
 *
 */

#include <gtest/gtest.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/Category.h>
#include "../src/utils/bufferedresultforwarder.h"
#include "../src/utils/resultforwarder.h"
#include "../src/utils/notify-strategy.h"

using namespace unity::scopes;
using ::testing::Truly;
using ::testing::InSequence;
using ::testing::Matcher;

struct HasCategory
{
    HasCategory(const std::string &id) : id_(id) {}

    bool operator()(const CategorisedResult& res) const
    {
        return res.category()->id() == id_;
    }

    std::string id_;
};

class ResultForwarderTest: public ::testing::Test
{
protected:
    virtual void SetUp() {
        online_category =  std::make_shared<unity::scopes::testing::Category>("online", "Online", "icon", CategoryRenderer());
        songs_category = std::make_shared<unity::scopes::testing::Category>("songs", "Songs", "icon", CategoryRenderer());
        albums_category = std::make_shared<unity::scopes::testing::Category>("albums", "Albums", "icon", CategoryRenderer());
        unknown_category = std::make_shared<unity::scopes::testing::Category>("unknown", "Foo", "icon", CategoryRenderer());

        parent_reply_proxy = SearchReplyProxy(&parent_reply, [](SearchReply*){});

        auto const result_filter = [](CategorisedResult&) -> bool { return true; };

        unity::scopes::testing::MockSearchReply reply;
        local_reply.reset(new ResultForwarder(parent_reply_proxy, result_filter, std::shared_ptr<WaitForAllCategories>(new WaitForAllCategories({"songs", "albums"}))));
        online_reply.reset(new BufferedResultForwarder(parent_reply_proxy));
        local_reply->add_observer(online_reply);
    }

    virtual void TearDown() {
    }

    Category::SCPtr online_category;
    Category::SCPtr songs_category;
    Category::SCPtr albums_category;
    Category::SCPtr unknown_category;
    unity::scopes::testing::MockSearchReply parent_reply;
    SearchReplyProxy parent_reply_proxy;
    std::shared_ptr<ResultForwarder> local_reply;
    std::shared_ptr<ResultForwarder> online_reply;
};

TEST_F(ResultForwarderTest, OnlineFirst) {
    InSequence s; // enforce strict order of push calls

    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("songs")))));
    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("albums")))));
    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("online")))));

    {
        CategorisedResult res0(online_category);
        CategorisedResult res1(songs_category);
        CategorisedResult res2(albums_category);

        online_reply->push(res0);
        local_reply->push(res1);
        local_reply->push(res2);
    }
}

TEST_F(ResultForwarderTest, OnlineSecond) {
    InSequence s; // enforce strict order of push calls

    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("songs")))));
    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("albums")))));
    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("online")))));

    {
        CategorisedResult res0(online_category);
        CategorisedResult res1(songs_category);
        CategorisedResult res2(albums_category);

        local_reply->push(res1);
        online_reply->push(res0);
        local_reply->push(res2);
    }
}

//
// push an 'unknown' category after 'songs' (which is expected by the forwarder)
// to verify that 'online' result still has to wait for 'albums' result.
TEST_F(ResultForwarderTest, UnknownCategory) {
    InSequence s; // enforce strict order of push calls

    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("songs")))));
    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("unknown")))));
    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("albums")))));
    EXPECT_CALL(parent_reply, push(Matcher<CategorisedResult const&>(Truly(HasCategory("online")))));

    {
        CategorisedResult res0(online_category);
        CategorisedResult res1(songs_category);
        CategorisedResult res2(albums_category);
        CategorisedResult res3(unknown_category);

        local_reply->push(res1);
        local_reply->push(res3);
        online_reply->push(res0);
        local_reply->push(res2);
    }
}



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
