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
 * Authored by Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 */

#include "videoaggregatorquery.h"
#include "resultforwarder.h"
#include "bufferedresultforwarder.h"
#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/SearchReply.h>

using namespace unity::scopes;

class VideoResultForwarder : public BufferedResultForwarder {
public:
    VideoResultForwarder(SearchReplyProxy const &upstream, Category::SCPtr const& category) :
        BufferedResultForwarder(upstream), category(category) {
        ResultForwarder::push(category);
    }

    virtual void push(Category::SCPtr const &cat) override {
        /* nothing: we replace the subscope categories */
    }

    virtual void push(CategorisedResult result) override {
        result.set_category(category);
        BufferedResultForwarder::push(std::move(result));
    }

private:
    Category::SCPtr category;
};

VideoAggregatorQuery::VideoAggregatorQuery(CannedQuery const& query, SearchMetadata const& hints, std::vector<ScopeMetadata> subscopes) :
    SearchQueryBase(query, hints),
    subscopes(subscopes) {
}

VideoAggregatorQuery::~VideoAggregatorQuery() {
}

void VideoAggregatorQuery::cancelled() {
}

void VideoAggregatorQuery::run(unity::scopes::SearchReplyProxy const& parent_reply) {
    auto first_reply = std::make_shared<ResultForwarder>(parent_reply);
    // Create forwarders for the other sub-scopes
    for (unsigned int i = 1; i < subscopes.size(); i++) {
        const auto &metadata = subscopes[i];
        auto category = parent_reply->register_category(
            metadata.scope_id(), metadata.display_name(), "" /* icon */,
            CategoryRenderer());
        auto subscope_reply = std::make_shared<VideoResultForwarder>(parent_reply, category);
        first_reply->add_observer(subscope_reply);
        subsearch(metadata.proxy(), query().query_string(), subscope_reply);
    }
    subsearch(subscopes[0].proxy(), query().query_string(), first_reply);
}
