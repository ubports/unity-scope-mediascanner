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
#include <algorithm>
#include "../utils/i18n.h"

using namespace unity::scopes;

 #ifdef CLICK_MODE
const std::string MusicAggregatorScope::LOCALSCOPE = "com.ubuntu.scopes.mymusic_mymusic";
 #else
const std::string MusicAggregatorScope::LOCALSCOPE = "mediascanner-music";
 #endif
const std::string MusicAggregatorScope::GROOVESHARKSCOPE = "com.canonical.scopes.grooveshark";
const std::string MusicAggregatorScope::SEVENDIGITAL = "com.canonical.scopes.sevendigital";
const std::string MusicAggregatorScope::SOUNDCLOUD = "com.ubuntu.scopes.soundcloud_soundcloud";
const std::string MusicAggregatorScope::SONGKICK = "com.canonical.scopes.songkick_songkick";
const std::string MusicAggregatorScope::YOUTUBE = "com.ubuntu.scopes.youtube_youtube";

const std::vector<std::string> predefined_scopes {
    MusicAggregatorScope::LOCALSCOPE,
    MusicAggregatorScope::GROOVESHARKSCOPE,
    MusicAggregatorScope::SEVENDIGITAL,
    MusicAggregatorScope::SOUNDCLOUD,
    MusicAggregatorScope::SONGKICK,
    MusicAggregatorScope::YOUTUBE
};

void MusicAggregatorScope::start(std::string const&) {
    init_gettext(*this);
}

void MusicAggregatorScope::stop() {
}

SearchQueryBase::UPtr MusicAggregatorScope::search(CannedQuery const& q,
                                                   SearchMetadata const& hints) {
    SearchQueryBase::UPtr query(new MusicAggregatorQuery(q, hints, child_scopes()));
    return query;
}

ChildScopeList MusicAggregatorScope::find_child_scopes() const
{
    const std::string kw = "music";

    auto music_scopes = registry()->list_if([kw](ScopeMetadata const& item)
    {
        if (item.scope_id() == "musicaggregator")
        {
            return false;
        }
        auto keywords = item.keywords();
        return keywords.find(kw) != keywords.end() || std::find(predefined_scopes.begin(), predefined_scopes.end(), item.scope_id()) != predefined_scopes.end();
    });

    ChildScopeList list;

    // ensure predefined scopes are first in the resulting child scopes list
    for (auto const& scope_id: predefined_scopes)
    {
        auto it = music_scopes.find(scope_id);
        if (it != music_scopes.end())
        {
            list.emplace_back(ChildScope{it->first, it->second, true, {kw}});
            music_scopes.erase(it);
        }
    }

    // append any remaining music scopes
    for (auto const& scope : music_scopes)
    {
        list.emplace_back(ChildScope{scope.first, scope.second, true, {kw}});
    }
    return list;
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
