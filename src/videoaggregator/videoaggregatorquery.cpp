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

#include <cstdio>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/SearchReply.h>
#include <algorithm>

#include "../utils/i18n.h"
#include "videoaggregatorquery.h"
#include "../utils/bufferedresultforwarder.h"

using namespace unity::scopes;

static char SURFACING_CATEGORY_DEFINITION[] = R"(
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

static char SEARCH_CATEGORY_DEFINITION[] = R"(
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

VideoAggregatorQuery::VideoAggregatorQuery(CannedQuery const& query, SearchMetadata const& hints, ChildScopeList const& scopes) :
    SearchQueryBase(query, hints),
    child_scopes(scopes) {
        std::reverse(child_scopes.begin(), child_scopes.end());
}

VideoAggregatorQuery::~VideoAggregatorQuery() {
}

void VideoAggregatorQuery::cancelled() {
}

void VideoAggregatorQuery::run(unity::scopes::SearchReplyProxy const& parent_reply) {
    const std::string query_string = query().query_string();
    const bool surfacing = query_string.empty();
    const std::string department_id = "aggregated:videoaggregator";
    const FilterState filter_state;

    unity::scopes::utility::BufferedResultForwarder::SPtr next_forwarder;

    // Create forwarders for the other sub-scopes
    for (auto const& child: child_scopes) {
        if (child.enabled)
        {
            Category::SCPtr category;
            CannedQuery category_query(child.id, query().query_string(), "");
            char title[500];
            if (surfacing) {
                snprintf(title, sizeof(title), _("%s Features"),
                        child.metadata.display_name().c_str());
                category = parent_reply->register_category(
                        child.id, title, "" /* icon */, category_query,
                        CategoryRenderer(SURFACING_CATEGORY_DEFINITION));
            } else {
                snprintf(title, sizeof(title), _("Results from %s"),
                        child.metadata.display_name().c_str());
                category = parent_reply->register_category(
                        child.id, title, "" /* icon */, category_query,
                        CategoryRenderer(SEARCH_CATEGORY_DEFINITION));
            }
            next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [category](CategorisedResult& res) -> bool {
                    res.set_category(category);
                    return true;
                });
            subsearch(child, query_string, department_id, filter_state, next_forwarder);
        }
    }
}
