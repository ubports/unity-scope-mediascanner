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

#ifndef MUSICAGGREGATORQUERY_H_
#define MUSICAGGREGATORQUERY_H_

#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/ReplyProxyFwd.h>

class ResultForwarder;

class MusicAggregatorQuery : public unity::scopes::SearchQueryBase
{
public:
    MusicAggregatorQuery(unity::scopes::CannedQuery const& query,
            unity::scopes::SearchMetadata const& hints,
            unity::scopes::ScopeProxy local_scope,
            unity::scopes::ScopeProxy const& grooveshark_scope,
            unity::scopes::ScopeProxy const& soundcloud_scope,
            unity::scopes::ScopeProxy const& sevendigital_scope,
            unity::scopes::ScopeProxy const& songkick_scope,
            unity::scopes::ScopeProxy const& youtube_scope);
    ~MusicAggregatorQuery();
    virtual void cancelled() override;

    virtual void run(unity::scopes::SearchReplyProxy const& reply) override;

private:
    unity::scopes::ScopeProxy local_scope;
    unity::scopes::ScopeProxy grooveshark_scope;
    unity::scopes::ScopeProxy soundcloud_scope;
    unity::scopes::ScopeProxy sevendigital_scope;
    unity::scopes::ScopeProxy songkick_scope;
    unity::scopes::ScopeProxy youtube_scope;
    static const std::string grooveshark_songs_category_id;
};

#endif
