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

#include "musicaggregatorquery.h"
#include "resultforwarder.h"
#include "onlinemusicresultforwarder.h"
#include "notify-strategy.h"
#include <memory>
#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/SearchReply.h>

using namespace unity::scopes;

MusicAggregatorQuery::MusicAggregatorQuery(CannedQuery const& query, SearchMetadata const& hints,
        ScopeProxy local_scope, std::vector<ScopeProxy> const& online_scopes) :
    SearchQueryBase(query, hints),
    local_scope(local_scope), online_scopes(online_scopes) {
}

MusicAggregatorQuery::~MusicAggregatorQuery() {
}

void MusicAggregatorQuery::cancelled() {
}

void MusicAggregatorQuery::run(unity::scopes::SearchReplyProxy const& parent_reply) {
    std::shared_ptr<ResultForwarder> local_reply(new ResultForwarder(parent_reply));
    /*if(online_scope)
    {
        online_reply.reset(new OnlineMusicResultForwarder(parent_reply));
        local_reply->add_observer(online_reply);
        subsearch(online_scope, query().query_string(), online_reply);
    }
    subsearch(local_scope, query().query_string(), local_reply);*/

    ScopeProxy online_scope;
    std::vector<std::shared_ptr<ResultForwarder>> online_reply;

    if (online_scopes.size() > 0)
    {
        online_scope = online_scopes[0];
        auto reply = std::make_shared<OnlineMusicResultForwarder>(parent_reply);
        online_reply.push_back(reply);
        local_reply->add_observer(reply);

        for (unsigned int i = 1; i < online_scopes.size(); ++i)
        {
            auto reply = std::make_shared<OnlineMusicResultForwarder>(parent_reply);
            online_reply.push_back(reply);
            online_reply[i-1]->add_observer(reply);

            subsearch(online_scopes[i], query().query_string(), reply);
        }
    }
    subsearch(local_scope, query().query_string(), local_reply);
}
