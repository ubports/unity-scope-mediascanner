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

#ifndef ONLINEMUSICRESULTFORWARDER_H 
#define ONLINEMUSICRESULTFORWARDER_H

#include "../utils/bufferedresultforwarder.h"

/*
   ResultForwarder that buffers results up until it gets
   notified via on_forwarder_ready() by another ResultForwarder.
*/
class OnlineMusicResultForwarder: public BufferedResultForwarder {

public:
    OnlineMusicResultForwarder(unity::scopes::SearchReplyProxy const& upstream,
            std::function<bool(unity::scopes::CategorisedResult&)> const &result_filter);
    virtual void push(unity::scopes::CategorisedResult result) override;
    virtual void push(unity::scopes::Category::SCPtr const& category) override;

    static const std::string songs_category_id;
};

#endif
