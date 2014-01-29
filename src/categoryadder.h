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

#ifndef CATEGORYADDER_H_
#define CATEGORYADDER_H_

#include<unity/scopes/SearchListener.h>
#include<unity/scopes/ReplyProxyFwd.h>
#include<unity/scopes/ListenerBase.h>
#include<unity/scopes/CategorisedResult.h>

class CategoryAdder : public unity::scopes::SearchListener {

public:

    CategoryAdder(unity::scopes::Category::SCPtr category,
            unity::scopes::SearchReplyProxy const& upstream) :
        category(category),
        upstream(upstream) {
    }
    virtual ~CategoryAdder() {}

    virtual void push(unity::scopes::Category::SCPtr category) override;
    virtual void push(unity::scopes::CategorisedResult result) override;

    virtual void finished(unity::scopes::ListenerBase::Reason reason,
            std::string const& error_message) override;

private:
    unity::scopes::Category::SCPtr category;
    unity::scopes::SearchReplyProxy upstream;
};


#endif
