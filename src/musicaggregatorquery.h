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

class MusicAggregatorQuery : public unity::scopes::SearchQueryBase
{
public:
    MusicAggregatorQuery(unity::scopes::CannedQuery const& query,
            unity::scopes::ScopeProxy local_scope,
            unity::scopes::ScopeProxy online_scope);
    ~MusicAggregatorQuery();
    virtual void cancelled() override;

    virtual void run(unity::scopes::SearchReplyProxy const& reply) override;

private:
    unity::scopes::CannedQuery query;
    unity::scopes::ScopeProxy local_scope;
    unity::scopes::ScopeProxy online_scope;
};

#endif
