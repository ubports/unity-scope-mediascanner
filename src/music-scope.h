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
    friend class MusicPreview;

public:
    virtual void start(std::string const&, unity::scopes::RegistryProxy const&) override;
    virtual void stop() override;
    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const &q,
                                         unity::scopes::SearchMetadata const& hints) override;
    virtual unity::scopes::PreviewQueryBase::UPtr preview(unity::scopes::Result const& result,
                                         unity::scopes::ActionMetadata const& hints) override;

private:
    std::unique_ptr<mediascanner::MediaStore> store;
};

class MusicQuery : public unity::scopes::SearchQueryBase
{
public:
    MusicQuery(MusicScope &scope, unity::scopes::CannedQuery const& query, unity::scopes::SearchMetadata const& hints);
    virtual void cancelled() override;
    virtual void run(unity::scopes::SearchReplyProxy const&reply) override;

private:
    const MusicScope &scope;

    void query_songs(unity::scopes::SearchReplyProxy const&reply) const;
    void query_albums(unity::scopes::SearchReplyProxy const&reply) const;
};

class MusicPreview : public unity::scopes::PreviewQueryBase
{
public:
    MusicPreview(MusicScope &scope, unity::scopes::Result const& result, unity::scopes::ActionMetadata const& hints);
    virtual void cancelled() override;
    virtual void run(unity::scopes::PreviewReplyProxy const& reply) override;

private:
    void song_preview(unity::scopes::PreviewReplyProxy const &reply) const;
    void album_preview(unity::scopes::PreviewReplyProxy const &reply) const;
    const MusicScope &scope;
};

#endif
