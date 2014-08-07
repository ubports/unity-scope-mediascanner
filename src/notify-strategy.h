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

#ifndef NOTIFYSTRATEGY_H
#define NOTIFYSTRATEGY_H

#include <unity/scopes/CategorisedResult.h>
#include <set>

class NotifyStrategy
{
public:
    virtual ~NotifyStrategy() {}
    virtual bool is_ready(unity::scopes::CategorisedResult const& result) = 0;
};

class WaitForAnyResult : public NotifyStrategy
{
public:
    bool is_ready(unity::scopes::CategorisedResult const &result) override;
};

class WaitForAllCategories : public NotifyStrategy
{
public:
    WaitForAllCategories(std::initializer_list<std::string> category_ids);
    bool is_ready(unity::scopes::CategorisedResult const& result) override;

private:
    std::set<std::string> category_ids_;
};

#endif
