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

#ifndef RESULTFORWARDER_H_
#define RESULTFORWARDER_H_

#include<unity/scopes/SearchListener.h>
#include<unity/scopes/ReplyProxyFwd.h>
#include<unity/scopes/ListenerBase.h>
#include<unity/scopes/CategorisedResult.h>
#include<list>

class ResultForwarder : public unity::scopes::SearchListener {

public:

    ResultForwarder(unity::scopes::SearchReplyProxy const& upstream) :
        upstream(upstream),
        ready_(false) {
    }
    virtual ~ResultForwarder() {}

    virtual void push(unity::scopes::Category::SCPtr category) override;
    virtual void push(unity::scopes::CategorisedResult result) override;
    void add_observer(std::shared_ptr<ResultForwarder> result_forwarder);

    virtual void finished(unity::scopes::ListenerBase::Reason reason,
            std::string const& error_message) override;

protected:
    virtual void on_forwarder_ready(ResultForwarder*);
    unity::scopes::SearchReplyProxy upstream;
    void notify_observers();

private:
    std::list<std::weak_ptr<ResultForwarder>> observers_;
    bool ready_;
};


#endif
