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

#ifndef BUFFEREDRESULTFORWARDER_H_
#define BUFFEREDRESULTFORWARDER_H_

#include "resultforwarder.h"
#include <list>
#include <unity/scopes/CategorisedResult.h>

/*
   ResultForwarder that buffers results up until it gets
   notified via on_forwarder_ready() by another ResultForwarder.
*/
class BufferedResultForwarder : public ResultForwarder {

public:

    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream,
            std::function<bool(unity::scopes::CategorisedResult&)> const &result_filter = [](unity::scopes::CategorisedResult&) -> bool { return true; });
    virtual ~BufferedResultForwarder() {}

    virtual void push(unity::scopes::CategorisedResult result) override;
    void flush();

protected:
    void on_all_forwarders_ready() override;

private:
    bool buffer_;
    std::list<unity::scopes::CategorisedResult> result_buffer_;
};

#endif
