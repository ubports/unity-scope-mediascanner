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
        ScopeProxy local_scope,
        unity::scopes::ScopeProxy const& grooveshark_scope,
        unity::scopes::ScopeProxy const& soundcloud_scope,
        unity::scopes::ScopeProxy const& sevendigital_scope
        ) :
    SearchQueryBase(query, hints),
    local_scope(local_scope),
    grooveshark_scope(grooveshark_scope),
    soundcloud_scope(soundcloud_scope),
    sevendigital_scope(sevendigital_scope)
{
}

MusicAggregatorQuery::~MusicAggregatorQuery() {
}

void MusicAggregatorQuery::cancelled() {
}

void MusicAggregatorQuery::run(unity::scopes::SearchReplyProxy const& parent_reply) {
    std::shared_ptr<ResultForwarder> local_reply(new ResultForwarder(parent_reply));

    std::vector<unity::scopes::ScopeProxy> scopes({local_scope});
    if (grooveshark_scope)
        scopes.push_back(grooveshark_scope);
    if (soundcloud_scope)
        scopes.push_back(soundcloud_scope);
    if (sevendigital_scope)
        scopes.push_back(sevendigital_scope);

    std::vector<std::shared_ptr<ResultForwarder>> replies({local_reply});

    // create and chain buffered result forwarders to enforce proper order of categories
    for (unsigned int i = 0; i < scopes.size(); ++i)
    {
        auto reply = std::make_shared<OnlineMusicResultForwarder>(parent_reply);
        replies.push_back(reply);
        replies[i-1]->add_observer(reply);
    }

    // dispatch search to subscopes
    for (unsigned int i = 0; i < replies.size(); ++i)
    {
        std::string dept;
        if (scopes[i] == sevendigital_scope)
        {
            dept = "newreleases";
        }
        subsearch(scopes[i], query().query_string(), dept, FilterState(), replies[i]);
    }
}
