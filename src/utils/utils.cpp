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

#include "utils.h"
#include <algorithm>
#include <unity/scopes/ScopeMetadata.h>

unity::scopes::ChildScopeList find_child_scopes_by_keywords(
        std::string const& aggregator_scope_id,
        unity::scopes::RegistryProxy const& registry,
        std::vector<std::string> const& predefined_scopes,
        std::string const& keyword)
{
    auto music_scopes = registry->list_if([keyword, aggregator_scope_id, predefined_scopes](unity::scopes::ScopeMetadata const& item)
    {
        if (item.scope_id() == aggregator_scope_id)
        {
            return false;
        }
        auto keywords = item.keywords();
        return keywords.find(keyword) != keywords.end() || std::find(predefined_scopes.begin(), predefined_scopes.end(), item.scope_id()) != predefined_scopes.end();
    });

    unity::scopes::ChildScopeList list;

    // ensure predefined scopes are first in the resulting child scopes list
    for (auto const& scope_id: predefined_scopes)
    {
        auto it = music_scopes.find(scope_id);
        if (it != music_scopes.end())
        {
            list.emplace_back(unity::scopes::ChildScope{it->first, it->second, true, {keyword}});
            music_scopes.erase(it);
        }
    }

    // append any remaining music scopes
    for (auto const& scope : music_scopes)
    {
        list.emplace_back(unity::scopes::ChildScope{scope.first, scope.second, true, {keyword}});
    }
    return list;
}
