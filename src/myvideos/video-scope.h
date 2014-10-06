#ifndef VIDEO_SCOPE_H
#define VIDEO_SCOPE_H

#include <memory>

#include <mediascanner/MediaStore.hh>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/Variant.h>

class VideoScope : public unity::scopes::ScopeBase
{
    friend class VideoQuery;
public:
    virtual void start(std::string const&) override;
    virtual void stop() override;
    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const &q,
                                         unity::scopes::SearchMetadata const& hints) override;
    virtual unity::scopes::PreviewQueryBase::UPtr preview(unity::scopes::Result const& result, unity::scopes::ActionMetadata const& hints) override;

private:
    std::unique_ptr<mediascanner::MediaStore> store;
};

class VideoQuery : public unity::scopes::SearchQueryBase
{
public:
    VideoQuery(VideoScope &scope, unity::scopes::CannedQuery const& query, unity::scopes::SearchMetadata const& hints);
    virtual void cancelled() override;
    virtual void run(unity::scopes::SearchReplyProxy const&reply) override;

private:
    const VideoScope &scope;
};

class VideoPreview : public unity::scopes::PreviewQueryBase
{
public:
    VideoPreview(VideoScope &scope, unity::scopes::Result const& result, unity::scopes::ActionMetadata const& hints);
    virtual void cancelled() override;
    virtual void run(unity::scopes::PreviewReplyProxy const& reply) override;

private:
    const VideoScope &scope;
};

#endif
