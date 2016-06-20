/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Authored by James Henstridge <james.henstridge@canonical.com>
 *
 */
#include <config.h>

#include <stdio.h>

#include <boost/regex.hpp>
#include <mediascanner/Filter.hh>
#include <mediascanner/MediaFile.hh>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/VariantBuilder.h>

#include "video-scope.h"
#include "../utils/i18n.h"

#define MAX_RESULTS 100

using namespace mediascanner;
using namespace unity::scopes;

static const char MISSING_VIDEO_ART[] = "video_missing.png";

static const char GET_STARTED_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "card-layout" : "vertical",
    "collapsed-rows" : 0,
    "non-interactive": "true"
  },
  "components": {
    "title": "title",
    "art": {
        "field": "art",
        "conciergeMode": true
    },
    "summary" : "summary"
  }
}
)";

static const char GET_STARTED_AGG_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
      "category-layout": "grid",
      "card-size": "large",
      "card-layout" : "horizontal"
  },
  "components": {
    "title": "title",
    "art": "art"
  }
}
)";

static const char LOCAL_CATEGORY_ICON[] = "/usr/share/icons/unity-icon-theme/places/svg/group-videos.svg";
static const char LOCAL_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium",
    "card-layout": "horizontal"
  },
  "components": {
    "title": "title",
    "art":  {
      "field": "art",
      "fallback": "@FALLBACK@",
      "aspect-ratio": 1.5
    }
  }
}
)";

static const char AGGREGATOR_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "carousel",
    "overlay": true,
    "card-size": "medium"
  },
  "components": {
    "title": "title",
    "art":  {
      "field": "art",
      "fallback": "@FALLBACK@",
      "aspect-ratio": 1.5
    }
  }
}
)";

// Category renderer to use when presenting search results
// FIXME: This should use list category-layout (LP #1279279)
static const char SEARCH_CATEGORY_DEFINITION[] = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "small"
  },
  "components": {
    "title": "title",
    "art": {
      "field": "art",
      "fallback": "@FALLBACK@"
    }
  }
}
)";

enum class VideoType {
    ALL,
    CAMERA,
    DOWNLOADS,
};

void VideoScope::start(std::string const&) {
    init_gettext(*this);
    store.reset(new MediaStore(MS_READ_ONLY));
}

void VideoScope::stop() {
    store.reset();
}

SearchQueryBase::UPtr VideoScope::search(CannedQuery const &q,
                                         SearchMetadata const& hints) {
    SearchQueryBase::UPtr query(new VideoQuery(*this, q, hints));
    return query;
}

PreviewQueryBase::UPtr VideoScope::preview(Result const& result,
                                    ActionMetadata const& hints) {
    PreviewQueryBase::UPtr previewer(new VideoPreview(*this, result, hints));
    return previewer;
}

VideoQuery::VideoQuery(VideoScope &scope, CannedQuery const& query, SearchMetadata const& hints)
    : SearchQueryBase(query, hints),
      scope(scope) {
}

void VideoQuery::cancelled() {
}

static bool from_camera(const std::string &filename) {
    static const boost::regex pattern(R"(.*/video\d{8}_\d{4,}\.mp4$)");
    return boost::regex_match(filename, pattern);
}

void VideoQuery::run(SearchReplyProxy const&reply) {
    const bool surfacing = query().query_string() == "";
    const bool is_aggregated = search_metadata().is_aggregated();

    const bool empty_db = is_database_empty();

    if (empty_db)
    {
        if (!is_aggregated) {
            const CategoryRenderer renderer(GET_STARTED_CATEGORY_DEFINITION);
            auto cat = reply->register_category("myvideos-getstarted", "", "", renderer);
            CategorisedResult res(cat);
            res.set_uri(query().to_uri());
            res.set_title(_("Get started!"));
            res["summary"] = _("Drag and drop items from another devices. Alternatively, load your files onto a SD card.");
            res.set_art(scope.scope_directory() + "/" + "getstarted.svg");
            reply->push(res);
        } else if (surfacing) {
            const CategoryRenderer renderer(GET_STARTED_AGG_CATEGORY_DEFINITION);
            auto cat = reply->register_category("myvideos-getstarted", "", "", renderer);
            CategorisedResult res(cat);
            res.set_uri("appid://com.ubuntu.camera/camera/current-user-version");
            res.set_art(scope.scope_directory() + "/camera-app.png");
            res.set_title(_("Nothing here yet...\nMake a video!"));
            reply->push(res);
        }
        return;
    }

    if (!is_aggregated) {
        Department::SPtr root_dept = Department::create("", query(), _("Everything"));
        root_dept->set_subdepartments({
                Department::create("camera", query(), _("My Roll")),
                Department::create("downloads", query(), _("Downloaded")),
                });

        reply->register_departments(root_dept);
    }

    VideoType department = VideoType::ALL;
    if (query().department_id() == "camera") {
        department = VideoType::CAMERA;
    } else if (query().department_id() == "downloads") {
        department = VideoType::DOWNLOADS;
    }

    Category::SCPtr cat;
    if (is_aggregated) {
        cat = reply->register_category(
            "local", _("My Videos"), LOCAL_CATEGORY_ICON,
            CannedQuery(query().scope_id(), query().query_string(), ""),
            make_renderer(surfacing ? AGGREGATOR_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION, MISSING_VIDEO_ART));
    } else {
        cat = reply->register_category(
            "local", _("My Videos"), LOCAL_CATEGORY_ICON,
            make_renderer(surfacing ? LOCAL_CATEGORY_DEFINITION : SEARCH_CATEGORY_DEFINITION, MISSING_VIDEO_ART));
    }
    mediascanner::Filter filter;
    filter.setLimit(MAX_RESULTS);
    for (const auto &media : scope.store->query(query().query_string(), VideoMedia, filter)) {
        // Filter results if we are in a department
        switch (department) {
        case VideoType::ALL:
            break;
        case VideoType::CAMERA:
            if (!from_camera(media.getFileName())) {
                continue;
            }
            break;
        case VideoType::DOWNLOADS:
            if (from_camera(media.getFileName())) {
                continue;
            }
            break;
        }

        const std::string uri = media.getUri();

        CategorisedResult res(cat);
        res.set_uri(uri);
        res.set_dnd_uri(uri);
        res.set_art(media.getArtUri());
        res.set_title(media.getTitle());

        res["duration"] =media.getDuration();
        // res["width"] = media.getWidth();
        // res["height"] = media.getHeight();

        if(!reply->push(res))
        {
            return;
        }
    }
}

bool VideoQuery::is_database_empty() const
{
    mediascanner::Filter filter;
    filter.setLimit(1);
    return scope.store->query("", VideoMedia, filter).size() == 0;
}

CategoryRenderer VideoQuery::make_renderer(std::string json_text, std::string const& fallback) const
{
    static std::string const placeholder("@FALLBACK@");
    size_t pos = json_text.find(placeholder);
    if (pos != std::string::npos)
    {
        json_text.replace(pos, placeholder.size(), scope.scope_directory() + "/" + fallback);
    }
    return CategoryRenderer(json_text);
}


VideoPreview::VideoPreview(VideoScope &scope, Result const& result, ActionMetadata const& hints)
    : PreviewQueryBase(result, hints),
      scope(scope) {
}

void VideoPreview::cancelled() {
}

void VideoPreview::run(PreviewReplyProxy const& reply)
{
    ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    layout1col.add_column({"video", "header", "actions"});

    layout2col.add_column({"video", "header"});
    layout2col.add_column({"actions"});

    layout3col.add_column({"video", "header"});
    layout3col.add_column({"actions"});
    layout3col.add_column({});
    reply->register_layout({layout1col, layout2col, layout3col});

    PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");

    std::string uri = result().uri();
    if (uri.find("file://") == 0) {
        uri = "video://" + uri.substr(7); // replace file:// with video://
    }

    PreviewWidget video("video", "video");
    video.add_attribute_value("source", Variant(uri));
    video.add_attribute_mapping("screenshot", "art");

    VariantMap share_data;
    share_data["uri"] = Variant(result().uri());
    share_data["content-type"] = Variant("videos");
    video.add_attribute_value("share-data", Variant(share_data));

    PreviewWidget actions("actions", "actions");
    {
        VariantBuilder builder;
        builder.add_tuple({
                {"id", Variant("play")},
                {"uri", Variant(uri)},
                {"label", Variant(_("Play"))}
            });
        actions.add_attribute_value("actions", builder.end());
    }

    reply->push({video, header, actions});
}

extern "C" ScopeBase * UNITY_SCOPE_CREATE_FUNCTION() {
    return new VideoScope;
}

extern "C" void UNITY_SCOPE_DESTROY_FUNCTION(ScopeBase *scope) {
    delete scope;
}
