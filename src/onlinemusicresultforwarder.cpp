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

#include "onlinemusicresultforwarder.h"
#include <unity/scopes/SearchReply.h>

OnlineMusicResultForwarder::OnlineMusicResultForwarder(unity::scopes::SearchReplyProxy const& upstream,
        std::function<bool(unity::scopes::CategorisedResult&)> const& result_filter)
    : BufferedResultForwarder(upstream, result_filter)
{
}

void OnlineMusicResultForwarder::push(unity::scopes::CategorisedResult result)
{
    BufferedResultForwarder::push(std::move(result));
}

void OnlineMusicResultForwarder::push(unity::scopes::Category::SCPtr const& category)
{
    // do nothing
}
