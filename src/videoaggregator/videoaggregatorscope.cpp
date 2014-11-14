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

#include <config.h>
#include "videoaggregatorscope.h"
#include "videoaggregatorquery.h"
#include <unity/scopes/Registry.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <iostream>
#include "../utils/i18n.h"

using namespace unity::scopes;

const std::vector<const char*> SUBSCOPE_NAMES{
#ifdef CLICK_MODE
    "com.ubuntu.scopes.myvideos_myvideos",
#else
    "mediascanner-video",
#endif
    "com.ubuntu.scopes.youtube_youtube",
    "com.ubuntu.scopes.vimeo_vimeo",
};

void VideoAggregatorScope::start(std::string const&) {
    init_gettext(*this);
    find_subscopes(true);
}

void VideoAggregatorScope::find_subscopes(bool warn_missing) {
    subscopes.clear();
    for (const auto &scope_name : SUBSCOPE_NAMES) {
        try {
            subscopes.emplace_back(registry()->get_metadata(scope_name));
        } catch (const std::exception &e) {
            if (warn_missing) {
                std::cerr << "Could not find scope '" << scope_name << "': " << e.what() << std::endl;
            }
        }
    }
}

void VideoAggregatorScope::stop() {
}

SearchQueryBase::UPtr VideoAggregatorScope::search(CannedQuery const& q,
                                                   SearchMetadata const& hints) {
    // FIXME: workaround for problem with no remote scopes on first run
    // until network becomes available
    if (subscopes.size() != SUBSCOPE_NAMES.size()) {
        find_subscopes(false);
    }
    SearchQueryBase::UPtr query(new VideoAggregatorQuery(q, hints, subscopes));
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
