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
 *             Pawel Stolowski <pawel.stolowski@canonical.com>
 *
 */

#include <config.h>
#include "musicaggregatorquery.h"
#include "musicaggregatorscope.h"
#include "../utils/resultforwarder.h"
#include "onlinemusicresultforwarder.h"
#include "../utils/notify-strategy.h"
#include "../utils/i18n.h"
#include <memory>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/Location.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchMetadata.h>

using namespace unity::scopes;

static const char MYMUSIC_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "horizontal-list",
    "card-size": "small"
  },
  "components": {
    "title": "title",
    "art":  "art",
    "subtitle": "artist"
  }
}
)";

static const char MYMUSIC_SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-layout" : "horizontal",
    "card-size": "large"
  },
  "components": {
    "title": "title",
    "art":  "art",
    "subtitle": "artist"
  }
}
)";

static const char GROOVESHARK_CATEGORY_DEFINITION[] = R"(
{
    "schema-version": 1,
    "components": {
        "subtitle": "artist",
        "art": "art",
        "title": "title"
    },
    "template": {
        "category-layout": "grid",
        "card-size": "large",
        "card-layout" : "horizontal"
    }
}
)";

static const char GROOVESHARK_SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-layout" : "horizontal",
    "card-size": "large"
  },
  "components": {
    "title": "title",
    "art":  "art",
    "subtitle": "artist"
  }
}
)";

static const char SEVENDIGITAL_CATEGORY_DEFINITION[] = R"(
{
    "schema-version": 1,
    "components": {
        "subtitle": "subtitle",
        "attributes": "attributes",
        "art": "art",
        "title": "title"
    },
    "template":
    {
        "category-layout": "grid",
        "card-size": "medium"
    }
}
)";

static const char SEVENDIGITAL_SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-layout" : "horizontal",
    "card-size": "large"
  },
  "components": {
        "subtitle": "subtitle",
        "attributes": "attributes",
        "art": "art",
        "title": "title"
  }
}
)";

static const char SONGKICK_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "card-layout": "horizontal"
  },
  "components": {
    "title": "title",
    "art": "art",
    "subtitle": "subtitle"
  }
}
)";

static const char SONGKICK_SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "card-layout": "horizontal"
  },
  "components": {
    "title": "title",
    "mascot":  "art",
    "subtitle": "address",
    "attributes": "attributes"
  }
}
)";

const std::string MusicAggregatorQuery::grooveshark_songs_category_id = "cat_0";

MusicAggregatorQuery::MusicAggregatorQuery(CannedQuery const& query, SearchMetadata const& hints,
        ScopeProxy local_scope,
        ScopeProxy const& grooveshark_scope,
        ScopeProxy const& soundcloud_scope,
        ScopeProxy const& sevendigital_scope,
        ScopeProxy const& songkick_scope
        ) :
    SearchQueryBase(query, hints),
    local_scope(local_scope),
    grooveshark_scope(grooveshark_scope),
    soundcloud_scope(soundcloud_scope),
    sevendigital_scope(sevendigital_scope),
    songkick_scope(songkick_scope)
{
}

MusicAggregatorQuery::~MusicAggregatorQuery() {
}

void MusicAggregatorQuery::cancelled() {
}

void MusicAggregatorQuery::run(unity::scopes::SearchReplyProxy const& parent_reply)
{
    std::vector<std::shared_ptr<ResultForwarder>> replies;
    std::vector<unity::scopes::ScopeProxy> scopes({local_scope});

    const CannedQuery mymusic_query(MusicAggregatorScope::LOCALSCOPE, query().query_string(), "");
    const CannedQuery sevendigital_query(MusicAggregatorScope::SEVENDIGITAL, query().query_string(), "newreleases");
    const CannedQuery songkick_query(MusicAggregatorScope::SONGKICK, query().query_string(), "");
    const CannedQuery grooveshark_query(MusicAggregatorScope::GROOVESHARKSCOPE, query().query_string(), "");

    const bool empty_search = query().query_string().empty();

    //
    // register categories
    auto mymusic_cat = empty_search ? parent_reply->register_category("mymusic", _("My Music"), "",
            mymusic_query, CategoryRenderer(MYMUSIC_CATEGORY_DEFINITION))
        : parent_reply->register_category("mymusic", _("My Music"), "", CategoryRenderer(MYMUSIC_SEARCH_CATEGORY_DEFINITION));
    auto sevendigital_cat = empty_search ? parent_reply->register_category("7digital", _("New albums from 7digital"), "",
            sevendigital_query, CategoryRenderer(SEVENDIGITAL_CATEGORY_DEFINITION))
        : parent_reply->register_category("7digital", _("7digital"), "", CategoryRenderer(SEVENDIGITAL_SEARCH_CATEGORY_DEFINITION));
    auto songkick_cat = empty_search ? parent_reply->register_category("songkick", _("Nearby Events on Songkick"), "",
            songkick_query, CategoryRenderer(SONGKICK_CATEGORY_DEFINITION))
        : parent_reply->register_category("songkick", _("Songkick"), "", CategoryRenderer(SONGKICK_SEARCH_CATEGORY_DEFINITION));
    auto grooveshark_cat = empty_search ? parent_reply->register_category("grooveshark", _("Popular tracks on Grooveshark"), "",
            grooveshark_query, CategoryRenderer(GROOVESHARK_CATEGORY_DEFINITION))
        : parent_reply->register_category("grooveshark", _("Grooveshark"), "", CategoryRenderer(GROOVESHARK_SEARCH_CATEGORY_DEFINITION));

    {
        auto local_reply = std::make_shared<ResultForwarder>(parent_reply, [this, mymusic_cat](CategorisedResult& res) -> bool {
                res.set_category(mymusic_cat);
                return true;
                });
        replies.push_back(local_reply);
    }

    if (sevendigital_scope)
    {
        scopes.push_back(sevendigital_scope);
        auto reply = std::make_shared<OnlineMusicResultForwarder>(parent_reply, [this, sevendigital_cat](CategorisedResult& res) -> bool {
                res.set_category(sevendigital_cat);
                return true;
                });
        replies.push_back(reply);
    }
    if (songkick_scope)
    {
        scopes.push_back(songkick_scope);
        auto reply = std::make_shared<OnlineMusicResultForwarder>(parent_reply, [this, songkick_cat](CategorisedResult& res) -> bool {
                if (res.category()->id() == "noloc") {
                    return false;
                }
                res.set_category(songkick_cat);
                return true;
            });
        replies.push_back(reply);
    }
    if (grooveshark_scope)
    {
        scopes.push_back(grooveshark_scope);

        auto reply = std::make_shared<OnlineMusicResultForwarder>(parent_reply, [this, grooveshark_cat](CategorisedResult& res) -> bool {
                    if (res.category()->id() == grooveshark_songs_category_id)
                    {
                        res.set_category(grooveshark_cat);
                        return true;
                    }
                    return false;
                });

        replies.push_back(reply);
    }
    if (soundcloud_scope)
    {
        // TODO when available
    }

    // create and chain result forwarders to enforce proper order of categories
    for (unsigned int i = 1; i < scopes.size(); ++i)
    {
        for (unsigned int j = 0; j<i; j++)
        {
            replies[j]->add_observer(replies[i]);
        }
    }

    // dispatch search to subscopes
    for (unsigned int i = 0; i < replies.size(); ++i)
    {
        std::string dept;
        SearchMetadata metadata(search_metadata());
        if (scopes[i] == sevendigital_scope)
        {
            if (empty_search)
            {
                dept = "newreleases";
            }
            metadata.set_cardinality(2);
        }
        else if (scopes[i] == local_scope)
        {
            dept = ""; // artists
        }
        else if (scopes[i] == grooveshark_scope)
        {
            if (empty_search)
            {
                metadata.set_cardinality(3);
            }
        }
        else if (scopes[i] == songkick_scope)
        {
            if (empty_search)
            {
                metadata.set_cardinality(2);
            }
        }

        // Don't send location data to scopes that don't need it.
        if (scopes[i] != songkick_scope)
        {
            metadata.set_location(Location(0, 0));
        }

        subsearch(scopes[i], query().query_string(), dept, FilterState(), metadata, replies[i]);
    }
}
