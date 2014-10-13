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

#include "notify-strategy.h"

bool WaitForAnyResult::is_ready(unity::scopes::CategorisedResult const&)
{
    return true;
}

WaitForAllCategories::WaitForAllCategories(std::initializer_list<std::string> category_ids)
    : category_ids_(category_ids)
{
}

bool WaitForAllCategories::is_ready(unity::scopes::CategorisedResult const& result)
{
    auto it = category_ids_.find(result.category()->id());
    if (it != category_ids_.end())
    {
        category_ids_.erase(it);
    }
    return category_ids_.size() == 0;
}
