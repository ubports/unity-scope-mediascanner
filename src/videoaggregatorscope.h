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

#ifndef VIDEOAGGREGATORSCOPE_H
#define VIDEOAGGREGATORSCOPE_H

#include <vector>

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/ScopeMetadata.h>
#include <unity/scopes/ReplyProxyFwd.h>

class VideoAggregatorScope : public unity::scopes::ScopeBase
{
public:
    virtual void start(std::string const&) override;

    virtual void stop() override;

    virtual unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&,
            const unity::scopes::ActionMetadata&) override;

    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const& q,
            unity::scopes::SearchMetadata const& hints) override;

private:
    void find_subscopes(bool warn_missing);

    std::vector<unity::scopes::ScopeMetadata> subscopes;
};

#endif
