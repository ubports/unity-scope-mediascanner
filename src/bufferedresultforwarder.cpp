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
#include <iostream>

BufferedResultForwarder::BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream)
    : ResultForwarder(upstream),
      buffer_(true)
{
}

void BufferedResultForwarder::push(unity::scopes::CategorisedResult result)
{
    if (buffer_)
    {
        std::cout << "to buffer: " << result.uri() << std::endl;
        result_buffer_.push_back(std::move(result));
    }
    else
    {
        std::cout << "direct push: " << result.uri() << std::endl;
        ResultForwarder::push(std::move(result));
    }
}

void BufferedResultForwarder::flush()
{
    std::cout << "flushing" << std::endl;
    for (auto const& r: result_buffer_)
    {
        std::cout << "\tpush " << r.uri() << std::endl;
        ResultForwarder::push(r);
    }
    result_buffer_.clear();
}

void BufferedResultForwarder::on_forwarder_ready(ResultForwarder*)
{
    std::cout << "on_forwarder_ready" << std::endl;
    buffer_ = false;
    flush();
}
