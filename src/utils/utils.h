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

#ifndef MEDIASCANNER_SCOPE_UTILS_H
#define MEDIASCANNER_SCOPE_UTILS_H

#include <unity/scopes/ChildScope.h>
#include <unity/scopes/Registry.h>
#include <vector>
#include <string>
#include <set>
#include <mediascanner/MediaStore.hh>

unity::scopes::ChildScopeList find_child_scopes_by_keywords(
        std::string const& aggregator_scope_id,
        unity::scopes::RegistryProxy const& registry,
        std::vector<std::string> const& predefined_scopes,
        std::string const& keyword);

bool is_database_empty(std::unique_ptr<mediascanner::MediaStore> const& db, mediascanner::MediaType media_type);

#endif
