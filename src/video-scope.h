#ifndef VIDEO_SCOPE_H
#define VIDEO_SCOPE_H

#include <memory>

#include <mediascanner/MediaStore.hh>
#include <scopes/Reply.h>
#include <scopes/ScopeBase.h>
#include <scopes/Variant.h>

class VideoScope : public unity::api::scopes::ScopeBase
{
    friend class VideoQuery;
public:
    virtual int start(std::string const&, unity::api::scopes::RegistryProxy const&) override;
    virtual void stop() override;
    virtual unity::api::scopes::QueryBase::UPtr create_query(std::string const &q,
                                         unity::api::scopes::VariantMap const& hints) override;

private:
    std::unique_ptr<MediaStore> store;
};

class VideoQuery : public unity::api::scopes::QueryBase
{
public:
    VideoQuery(VideoScope &scope, std::string const& query);
    virtual void cancelled() override;
    virtual void run(unity::api::scopes::ReplyProxy const&reply) override;

private:
    const VideoScope &scope;
    const std::string query;
};

#endif
