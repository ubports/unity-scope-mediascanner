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

#include<unity/scopes/SearchListenerBase.h>
#include<unity/scopes/SearchReplyProxyFwd.h>
#include<unity/scopes/ListenerBase.h>
#include<unity/scopes/CategorisedResult.h>
#include<list>
#include<memory>
#include<cassert>
#include "notify-strategy.h"

class ResultForwarder : public unity::scopes::SearchListenerBase, public std::enable_shared_from_this<ResultForwarder> {

public:

    ResultForwarder(unity::scopes::SearchReplyProxy const& upstream,
                    std::shared_ptr<NotifyStrategy> notify_strategy = std::make_shared<WaitForAnyResult>()) :
        upstream(upstream),
        notify_strategy_(notify_strategy),
        ready_(false) {
            assert(notify_strategy != nullptr);
    }
    virtual ~ResultForwarder() {}

    virtual void push(unity::scopes::Category::SCPtr const& category) override;
    virtual void push(unity::scopes::CategorisedResult result) override;
    void add_observer(std::shared_ptr<ResultForwarder> result_forwarder);

    virtual void finished(unity::scopes::ListenerBase::Reason reason,
            std::string const& error_message) override;

protected:
    void on_forwarder_ready(ResultForwarder *fw);
    virtual void on_all_forwarders_ready();
    unity::scopes::SearchReplyProxy upstream;
    void notify_observers();

private:
    std::list<std::shared_ptr<ResultForwarder>> observers_;
    std::list<std::shared_ptr<ResultForwarder>> wait_for_;
    std::shared_ptr<NotifyStrategy> notify_strategy_;
    bool ready_;
};


#endif
