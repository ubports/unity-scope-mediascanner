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
#include "videoaggregatorscope.h"
#include "../utils/bufferedresultforwarder.h"

using namespace unity::scopes;

// FIXME: once child scopes are updated to handle is_aggregated flag, they should provide
// own renderer for aggregator and these definition should be removed
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
    const std::string department_id = "aggregated:videoaggregator"; //FIXME: remove when child scopes handle is_aggregated
    const FilterState filter_state;

    unity::scopes::utility::BufferedResultForwarder::SPtr next_forwarder;

    //
    // maps scope id to category id of first received result from that scope.
    // this is used to ignore results from different categories (i.e. child scope is
    // misbehaving when aggregated).
    std::map<std::string, std::string> child_id_to_category_id;
    std::mutex child_id_map_mutex;

    // Create forwarders for the other sub-scopes
    for (auto const& child: child_scopes) {
        if (child.enabled)
        {
            bool const is_predefined_scope = (std::find(VideoAggregatorScope::predefined_scopes.begin(),
                        VideoAggregatorScope::predefined_scopes.end(),
                        child.id) != VideoAggregatorScope::predefined_scopes.end());

            auto const child_id = child.id;
            auto const child_name = child.metadata.display_name();

            if (child_id == VideoAggregatorScope::local_videos_scope)
            {
                // preserve category of local videos
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder);
            }
            else
            {
                next_forwarder = std::make_shared<BufferedResultForwarder>(parent_reply, next_forwarder, [this, parent_reply, is_predefined_scope, surfacing,
                        child_id, child_name, &child_id_to_category_id, &child_id_map_mutex](CategorisedResult& res) -> bool {
                        // register a single category for aggregated results of this child scope and update incoming results with this category;
                        // the new category has custom id and title, but reuses the renderer of first incoming result (except for predefined scopes, which
                        // for now use renderers hardcoded in the aggregator
                        Category::SCPtr category = parent_reply->lookup_category(child_id);
                        if (!category) {
                            CannedQuery category_query(child_id, query().query_string(), "");
                            auto const renderer = is_predefined_scope ? CategoryRenderer(surfacing ? SURFACING_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION)
                                                        : res.category()->renderer_template();
                            char title[500];
                            if (surfacing) {
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
            }
            subsearch(child, query_string, department_id, filter_state, next_forwarder);
        }
    }
}
