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

#include "bufferedresultforwarder.h"

BufferedResultForwarder::BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream,
        unity::scopes::utility::BufferedResultForwarder::SPtr const& next_forwarder,
        std::function<bool(unity::scopes::CategorisedResult&)> const &result_filter)
    : unity::scopes::utility::BufferedResultForwarder(upstream, next_forwarder),
      result_filter_(result_filter)
{
}

void BufferedResultForwarder::push(unity::scopes::CategorisedResult result)
{
    if (result_filter_(result))
    {
        unity::scopes::utility::BufferedResultForwarder::push(result);
    }
}
