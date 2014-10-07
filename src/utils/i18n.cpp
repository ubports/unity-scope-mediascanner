/*
 * Copyright (C) 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include "i18n.h"
#include <unity/scopes/ScopeBase.h>

using unity::scopes::ScopeBase;

void init_gettext(const ScopeBase &scope) {
    setlocale(LC_ALL, "");

#ifdef CLICK_MODE
    std::string locale_dir = scope.scope_directory() + "/../locale";
    bindtextdomain(GETTEXT_PACKAGE, locale_dir.c_str());
#else
    bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
#endif
}
