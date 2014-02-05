#ifndef MUSIC_SCOPE_H
#define MUSIC_SCOPE_H

#include <memory>

#include <mediascanner/MediaStore.hh>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/Variant.h>

class MusicScope : public unity::scopes::ScopeBase
{
    friend class MusicQuery;
public:
    virtual int start(std::string const&, unity::scopes::RegistryProxy const&) override;
    virtual void stop() override;
    virtual unity::scopes::QueryBase::UPtr create_query(std::string const &q,
                                         unity::scopes::VariantMap const& hints) override;
    virtual unity::scopes::QueryBase::UPtr preview(unity::scopes::Result const& result, unity::scopes::VariantMap const& hints) override;

private:
    std::unique_ptr<mediascanner::MediaStore> store;
};

class MusicQuery : public unity::scopes::SearchQuery
{
public:
    MusicQuery(MusicScope &scope, std::string const& query);
    virtual void cancelled() override;
    virtual void run(unity::scopes::SearchReplyProxy const&reply) override;

private:
    const MusicScope &scope;
    const std::string query;
};

class MusicPreview : public unity::scopes::PreviewQuery
{
public:
    MusicPreview(MusicScope &scope, unity::scopes::Result const& result);
    virtual void cancelled() override;
    virtual void run(unity::scopes::PreviewReplyProxy const& reply) override;

private:
    const MusicScope &scope;
    const unity::scopes::Result result;
};

#endif
