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
#include "../utils/i18n.h"
#include "../utils/bufferedresultforwarder.h"
#include <memory>
#include <map>
#include <mutex>
#include <algorithm>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/Location.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/ChildScope.h>

using namespace unity::scopes;

// FIXME: once child scopes are updated to handle is_aggregated flag, they should provide
// own renderer for aggregator and these definitions should be removed
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

static const char SOUNDCLOUD_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "overlay": true,
    "card-background": "color:///#000000"
  },
  "components": {
    "title": "title",
    "art" : {
      "field": "waveform",
      "aspect-ratio": 4.0
    },
    "subtitle": "username",
    "mascot": "art",
    "attributes": {
      "field": "attributes",
      "max-count": 3
    }
  }
}
)";
// unconfuse emacs: "

static const char SOUNDCLOUD_SEARCH_CATEGORY_DEFINITION[] = R"(
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

static char YOUTUBE_SURFACING_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium"
  },
  "components": {
    "title": "title",
    "subtitle": "subtitle",
    "art":  {
      "field": "art",
      "aspect-ratio": 1.5
    }
  }
}
)";

static char YOUTUBE_SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium",
    "card-layout": "horizontal"
  },
  "components": {
    "title": "title",
    "subtitle": "subtitle",
    "art":  {
      "field": "art",
      "aspect-ratio": 1.5
    }
  }
}
)";

const std::string MusicAggregatorQuery::grooveshark_songs_category_id = "cat_0";

MusicAggregatorQuery::MusicAggregatorQuery(CannedQuery const& query, SearchMetadata const& hints,
        ChildScopeList const& scopes
        ) :
    SearchQueryBase(query, hints),
    child_scopes(scopes)
{
    std::reverse(child_scopes.begin(), child_scopes.end());
}

MusicAggregatorQuery::~MusicAggregatorQuery() {
}

void MusicAggregatorQuery::cancelled() {
}

void MusicAggregatorQuery::run(unity::scopes::SearchReplyProxy const& parent_reply)
{
    std::vector<unity::scopes::utility::BufferedResultForwarder::SPtr> replies;
    ChildScopeList scopes;
    const std::string department_id = "aggregated:musicaggregator";

    const CannedQuery sevendigital_query(MusicAggregatorScope::SEVENDIGITAL, query().query_string(), "newreleases");
    const CannedQuery soundcloud_query(MusicAggregatorScope::SOUNDCLOUD, query().query_string(), "");
    const CannedQuery songkick_query(MusicAggregatorScope::SONGKICK, query().query_string(), "");
    const CannedQuery grooveshark_query(MusicAggregatorScope::GROOVESHARKSCOPE, query().query_string(), "");
    const CannedQuery youtube_query(MusicAggregatorScope::YOUTUBE, query().query_string(), department_id);

    const bool empty_search = query().query_string().empty();

    //
    // register categories
    auto sevendigital_cat = empty_search ? parent_reply->register_category("7digital", _("New albums from 7digital"), "",
            sevendigital_query, CategoryRenderer(SEVENDIGITAL_CATEGORY_DEFINITION))
        : parent_reply->register_category("7digital", _("7digital"), "", CategoryRenderer(SEVENDIGITAL_SEARCH_CATEGORY_DEFINITION));
    auto soundcloud_cat = empty_search ? parent_reply->register_category("soundcloud", _("Popular tracks on SoundCloud"), "",
            soundcloud_query, CategoryRenderer(SOUNDCLOUD_CATEGORY_DEFINITION))
        : parent_reply->register_category("soundcloud", _("SoundCloud"), "", CategoryRenderer(SOUNDCLOUD_SEARCH_CATEGORY_DEFINITION));
    auto songkick_cat = empty_search ? parent_reply->register_category("songkick", _("Nearby Events on Songkick"), "",
            songkick_query, CategoryRenderer(SONGKICK_CATEGORY_DEFINITION))
        : parent_reply->register_category("songkick", _("Songkick"), "", CategoryRenderer(SONGKICK_SEARCH_CATEGORY_DEFINITION));
    auto grooveshark_cat = empty_search ? parent_reply->register_category("grooveshark", _("Popular tracks on Grooveshark"), "",
            grooveshark_query, CategoryRenderer(GROOVESHARK_CATEGORY_DEFINITION))
        : parent_reply->register_category("grooveshark", _("Grooveshark"), "", CategoryRenderer(GROOVESHARK_SEARCH_CATEGORY_DEFINITION));
    auto youtube_cat = empty_search ? parent_reply->register_category("youtube", _("Popular tracks on Youtube"), "",
                youtube_query, CategoryRenderer(YOUTUBE_SURFACING_CATEGORY_DEFINITION))
            : parent_reply->register_category("youtube", _("Youtube"), "", youtube_query, CategoryRenderer(YOUTUBE_SEARCH_CATEGORY_DEFINITION));

    unity::scopes::utility::BufferedResultForwarder::SPtr next_forwarder;

    //
    // maps scope id to category id of first received result from that scope.
    // this is used to ignore results from different categories (i.e. child scope is
    // misbehaving when aggregated).
    std::map<std::string, std::string> child_id_to_category_id;
    std::mutex child_id_map_mutex;

    for (auto const& child: child_scopes)
    {
        if (child.enabled)
        {
            scopes.push_back(child);

            if (child.id == MusicAggregatorScope::LOCALSCOPE)
            {
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder);
                replies.push_back(next_forwarder);
            }
            else if (child.id == MusicAggregatorScope::SEVENDIGITAL)
            {
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [sevendigital_cat](CategorisedResult& res) -> bool {
                        res.set_category(sevendigital_cat);
                        return true;
                    });
                replies.push_back(next_forwarder);
            }
            else if (child.id == MusicAggregatorScope::SOUNDCLOUD)
            {
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [soundcloud_cat](CategorisedResult& res) -> bool {
                        if (res.category()->id() == "soundcloud_login_nag") {
                            return false;
                        }
                        res.set_category(soundcloud_cat);
                        return true;
                    });
                replies.push_back(next_forwarder);
            }
            else if (child.id == MusicAggregatorScope::SONGKICK)
            {
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [songkick_cat](CategorisedResult& res) -> bool {
                        if (res.category()->id() == "noloc") {
                            return false;
                        }
                        res.set_category(songkick_cat);
                        return true;
                    });
                replies.push_back(next_forwarder);
            }
            else if (child.id == MusicAggregatorScope::GROOVESHARKSCOPE)
            {
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [this, grooveshark_cat](CategorisedResult& res) -> bool {
                        if (res.category()->id() == grooveshark_songs_category_id)
                        {
                            res.set_category(grooveshark_cat);
                            return true;
                        }
                        return false;
                    });

                replies.push_back(next_forwarder);
            }
            else if (child.id == MusicAggregatorScope::YOUTUBE)
            {
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [youtube_cat](CategorisedResult& res) -> bool {
                        res.set_category(youtube_cat);
                        return !res["musicaggregation"].is_null();
                    });
                replies.push_back(next_forwarder);
            }
            else // dynamically added scope (from keywords)
            {
                auto const child_id = child.id;
                auto const child_name = child.metadata.display_name();
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [this, child_id, child_name, empty_search,
                        parent_reply, &child_id_to_category_id, &child_id_map_mutex](CategorisedResult& res) -> bool {
                    // register a single category for aggregated results of this child scope and update incoming results with this category;
                    // the new category has custom id and title, but reuses the renderer of first incoming result
                    Category::SCPtr category = parent_reply->lookup_category(child_id);
                    if (!category) {
                        CannedQuery category_query(child_id, query().query_string(), "");
                        auto const renderer = res.category()->renderer_template();
                        char title[500];
                        if (empty_search) {
                            snprintf(title, sizeof(title), _("%s Features"), child_name.c_str());
                        } else {
                            snprintf(title, sizeof(title), _("Results from %s"), child_name.c_str());
                        }
                        category = parent_reply->register_category(child_id, title, "" /* icon */, category_query, renderer);

                        // remember the first encountered category for this child
                        {
                            std::lock_guard<std::mutex> lock(child_id_map_mutex);
                            child_id_to_category_id[child_id] = res.category()->id();
                        }
                    }

                    {
                        std::lock_guard<std::mutex> lock(child_id_map_mutex);
                        if (child_id_to_category_id[child_id] == res.category()->id()) {
                            res.set_category(category);
                            return true;
                        }
                    }
                    return false; // filter out results from other categories
                });
                replies.push_back(next_forwarder);
            }
        }
    }

    // dispatch search to subscopes
    for (unsigned int i = 0; i < replies.size(); ++i)
    {
        std::string dept;
        SearchMetadata metadata(search_metadata());
        if (scopes[i].id == MusicAggregatorScope::SEVENDIGITAL)
        {
            if (empty_search)
            {
                dept = "newreleases";
            }
            metadata.set_cardinality(2);
        }
        else if (scopes[i].id == MusicAggregatorScope::LOCALSCOPE)
        {
            if (empty_search)
            {
                metadata.set_cardinality(4);
            }
        }
        else if (scopes[i].id == MusicAggregatorScope::GROOVESHARKSCOPE)
        {
            if (empty_search)
            {
                metadata.set_cardinality(3);
            }
        }
        else if (scopes[i].id == MusicAggregatorScope::SOUNDCLOUD)
        {
            if (empty_search)
            {
                metadata.set_cardinality(3);
            }
        }
        else if (scopes[i].id == MusicAggregatorScope::SONGKICK)
        {
            if (empty_search)
            {
                metadata.set_cardinality(2);
            }
        }
        else if (scopes[i].id == MusicAggregatorScope::YOUTUBE)
        {
            if (empty_search)
            {
                metadata.set_cardinality(2);
            }
            dept = department_id;
        }

        // Don't send location data to scopes that don't need it.
        if (scopes[i].id != MusicAggregatorScope::SONGKICK || !scopes[i].metadata.location_data_needed())
        {
            metadata.set_location(Location(0, 0));
        }

        subsearch(scopes[i], query().query_string(), dept, FilterState(), metadata, replies[i]);
    }
}
