#ifndef MUSIC_SCOPE_H
#define MUSIC_SCOPE_H

#include <memory>

#include <mediascanner/MediaStore.hh>
#include <scopes/Reply.h>
#include <scopes/ScopeBase.h>
#include <scopes/Variant.h>

class MusicScope : public unity::api::scopes::ScopeBase
{
    friend class MusicQuery;
public:
    virtual int start(std::string const&, unity::api::scopes::RegistryProxy const&) override;
    virtual void stop() override;
    virtual unity::api::scopes::QueryBase::UPtr create_query(std::string const &q,
                                         unity::api::scopes::VariantMap const& hints) override;

private:
    std::unique_ptr<MediaStore> store;
};

class MusicQuery : public unity::api::scopes::QueryBase
{
public:
    MusicQuery(MusicScope &scope, std::string const& query);
    virtual void cancelled() override;
    virtual void run(unity::api::scopes::ReplyProxy const&reply) override;

private:
    const MusicScope &scope;
    const std::string query;
};

#endif
