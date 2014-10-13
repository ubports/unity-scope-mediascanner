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
 *             Pawel Stolowski <pawel.stolowski@canonical.com>
 *
 */

#include "resultforwarder.h"
#include <unity/scopes/SearchReply.h>

using namespace unity::scopes;

void ResultForwarder::push(Category::SCPtr const& category) {
    upstream->register_category(category);
}

void ResultForwarder::push(CategorisedResult result) {
    {
        if (result_filter(result))
        {
            upstream->push(result);
        }
    }
    if (!ready_)
    {
        ready_ = notify_strategy_->is_ready(result);
        if (ready_)
        {
            notify_observers();
        }
    }
}

void ResultForwarder::finished(unity::scopes::CompletionDetails const& /*details*/) {
    if (!ready_)
    {
        ready_ = true;
        notify_observers();
    }
}

void ResultForwarder::notify_observers()
{
    for (auto o: observers_)
    {
        o->on_forwarder_ready(this);
    }
    observers_.clear();
}

void ResultForwarder::add_observer(std::shared_ptr<ResultForwarder> result_forwarder)
{
    if (result_forwarder.get() != this)
    {
        observers_.push_back(result_forwarder);
        result_forwarder->wait_for_.push_back(this);
    }
}

void ResultForwarder::on_forwarder_ready(ResultForwarder *fw)
{
    //
    // remove the forwarder that notified us from the wait_for_ list;
    wait_for_.remove_if([fw](ResultForwarder* r) -> bool { return r == fw; });
    if (wait_for_.size() == 0)
    {
        on_all_forwarders_ready();
    }
}

void ResultForwarder::on_all_forwarders_ready()
{
    // base impl does nothing
}
