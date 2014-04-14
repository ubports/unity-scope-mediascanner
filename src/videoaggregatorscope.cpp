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

#include "videoaggregatorscope.h"
#include "videoaggregatorquery.h"
#include <unity/scopes/Registry.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <iostream>

using namespace unity::scopes;

const char *LOCALSCOPE = "mediascanner-video";
const char *ONLINESCOPE = "com.canonical.scopes.remotevideos";

int VideoAggregatorScope::start(std::string const&, unity::scopes::RegistryProxy const& registry) {
    this->registry = registry;
    CategoryRenderer basic;
    local_scope = registry->get_metadata(LOCALSCOPE).proxy();
    try {
        online_scope = registry->get_metadata(ONLINESCOPE).proxy();
    } catch(const std::exception &e)
    {
        std::cerr << "Could not instantiate online scope:" << e.what() << std::endl;
    }
    return VERSION;
}

void VideoAggregatorScope::stop() {
}

SearchQueryBase::UPtr VideoAggregatorScope::search(CannedQuery const& q,
                                                   SearchMetadata const&) {
    // FIXME: workaround for problem with no remote scopes on first run
    // until network becomes available
    if (online_scope == nullptr)
    {
        try
        {
            online_scope = registry->get_metadata(ONLINESCOPE).proxy();
        } catch(std::exception &e)
        {
            // silently ignore
        }
    }
    SearchQueryBase::UPtr query(new VideoAggregatorQuery(q, local_scope, online_scope));
    return query;
}

PreviewQueryBase::UPtr VideoAggregatorScope::preview(Result const& /*result*/, ActionMetadata const& /*hints*/) {
    return nullptr;
}

#define EXPORT __attribute__ ((visibility ("default")))

extern "C"
{

    EXPORT
    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new VideoAggregatorScope();
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}
