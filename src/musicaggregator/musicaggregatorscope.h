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

#ifndef MUSICAGGREGATORSCOPE_H
#define MUSICAGGREGATORSCOPE_H

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/Variant.h>

class MusicAggregatorScope : public unity::scopes::ScopeBase
{
public:
    static const std::string LOCALSCOPE;
    static const std::string GROOVESHARKSCOPE;
    static const std::string SEVENDIGITAL;
    static const std::string SOUNDCLOUD;
    static const std::string SONGKICK;
    static const std::string YOUTUBE;

    virtual void start(std::string const&) override;

    virtual void stop() override;

    virtual unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&,
            const unity::scopes::ActionMetadata&) override;

    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const& q,
            unity::scopes::SearchMetadata const& hints) override;

private:
    void init_scope_proxies();
    void init_scope_proxy(std::string const& scope, unity::scopes::ScopeProxy& proxy, unity::scopes::VariantMap const& config);
    unity::scopes::ScopeProxy local_scope;
    unity::scopes::ScopeProxy grooveshark_scope;
    unity::scopes::ScopeProxy soundcloud_scope;
    unity::scopes::ScopeProxy sevendigital_scope;
    unity::scopes::ScopeProxy songkick_scope;
    unity::scopes::ScopeProxy youtube_scope;
};

#endif
