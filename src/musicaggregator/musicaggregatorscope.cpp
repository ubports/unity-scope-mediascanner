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
#include "musicaggregatorscope.h"
#include "musicaggregatorquery.h"
#include <unity/scopes/Registry.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <vector>
#include <iostream>
#include "../utils/i18n.h"

using namespace unity::scopes;

#ifdef CLICK_MODE
const std::string MusicAggregatorScope::LOCALSCOPE = "com.ubuntu.scopes.mymusic_mymusic";
#else
const std::string MusicAggregatorScope::LOCALSCOPE = "mediascanner-music";
#endif
const std::string MusicAggregatorScope::GROOVESHARKSCOPE = "com.canonical.scopes.grooveshark";
const std::string MusicAggregatorScope::SEVENDIGITAL = "com.canonical.scopes.sevendigital";
const std::string MusicAggregatorScope::SOUNDCLOUD = "com.canonical.scopes.soundcloud";
const std::string MusicAggregatorScope::SONGKICK = "com.canonical.scopes.songkick_songkick";

void MusicAggregatorScope::start(std::string const&) {
    init_gettext(*this);
    CategoryRenderer basic;
    local_scope = registry()->get_metadata(LOCALSCOPE).proxy();
}

void MusicAggregatorScope::stop() {
}

SearchQueryBase::UPtr MusicAggregatorScope::search(CannedQuery const& q,
                                                   SearchMetadata const& hints) {
    // FIXME: workaround for problem with no remote scopes on first run
    // until network becomes available
    init_scope_proxies();
    SearchQueryBase::UPtr query(new MusicAggregatorQuery(q, hints, local_scope, grooveshark_scope, soundcloud_scope, sevendigital_scope, songkick_scope));
    return query;
}

void MusicAggregatorScope::init_scope_proxy(std::string const& scope, unity::scopes::ScopeProxy& proxy, unity::scopes::VariantMap const& config)
{
    try
    {
        if (!config.at(scope).get_bool())
        {
            proxy = nullptr;
            return;
        }
    }
    catch (const std::exception &e)
    {
        // setting is missing, consider enabled
    }

    if (proxy == nullptr)
    {
        try
        {
            proxy = registry()->get_metadata(scope).proxy();
        } catch(std::exception &e)
        {
            std::cerr << "Failed to get proxy for scope " << scope << std::endl;
        }
    }
}

void MusicAggregatorScope::init_scope_proxies()
{
    const auto config = settings();
    init_scope_proxy(GROOVESHARKSCOPE, grooveshark_scope, config);
    init_scope_proxy(SEVENDIGITAL, sevendigital_scope, config);
    //init_scope_proxy(SOUNDCLOUD, soundcloud_scope, config);
    init_scope_proxy(SONGKICK, songkick_scope, config);
}

PreviewQueryBase::UPtr MusicAggregatorScope::preview(Result const& /*result*/, ActionMetadata const& /*hints*/) {
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
        return new MusicAggregatorScope();
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}
