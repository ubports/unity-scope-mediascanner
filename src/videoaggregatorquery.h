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

#ifndef VIDEOAGGREGATORQUERY_H_
#define VIDEOAGGREGATORQUERY_H_

#include <vector>

#include <unity/scopes/ScopeMetadata.h>
#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>

class VideoAggregatorQuery : public unity::scopes::SearchQueryBase
{
public:
    VideoAggregatorQuery(unity::scopes::CannedQuery const& query,
            unity::scopes::SearchMetadata const& hints,
            std::vector<unity::scopes::ScopeMetadata> subscopes);
    ~VideoAggregatorQuery();
    virtual void cancelled() override;

    virtual void run(unity::scopes::SearchReplyProxy const& reply) override;

private:
    std::vector<unity::scopes::ScopeMetadata> subscopes;
};

#endif
