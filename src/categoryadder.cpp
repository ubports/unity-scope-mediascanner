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

#include "categoryadder.h"
#include <unity/scopes/SearchReply.h>

using namespace unity::scopes;

void CategoryAdder::push(Category::SCPtr /*category*/) {
}

void CategoryAdder::push(CategorisedResult result) {
    result.set_category(category);
    upstream->push(result);
}

void CategoryAdder::finished(ListenerBase::Reason /*reason*/, std::string const& /*error_message*/) {
}
