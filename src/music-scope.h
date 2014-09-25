#ifndef MUSIC_SCOPE_H
#define MUSIC_SCOPE_H

#include <memory>
#include <atomic>

#include <mediascanner/MediaStore.hh>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/Variant.h>
#include <core/net/http/client.h>

class MusicScope : public unity::scopes::ScopeBase
{
    friend class MusicQuery;
    friend class MusicPreview;

public:
    virtual void start(std::string const&) override;
    virtual void stop() override;
    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const &q,
                                         unity::scopes::SearchMetadata const& hints) override;
    virtual unity::scopes::PreviewQueryBase::UPtr preview(unity::scopes::Result const& result,
                                         unity::scopes::ActionMetadata const& hints) override;

private:
    void set_api_key();
    std::string make_artist_art_uri(const std::string &artist, const std::string &album) const;

    std::unique_ptr<mediascanner::MediaStore> store;
    std::shared_ptr<core::net::http::Client> client;
    std::string api_key;
};

class MusicQuery : public unity::scopes::SearchQueryBase
{
public:
    MusicQuery(MusicScope &scope, unity::scopes::CannedQuery const& query, unity::scopes::SearchMetadata const& hints);
    virtual void cancelled() override;
    virtual void run(unity::scopes::SearchReplyProxy const&reply) override;

private:
    const MusicScope &scope;
    std::atomic<bool> query_cancelled;

    void populate_departments(unity::scopes::SearchReplyProxy const &reply) const;
    void query_songs(unity::scopes::SearchReplyProxy const&reply) const;
    void query_albums(unity::scopes::SearchReplyProxy const&reply) const;
    void query_genres(unity::scopes::SearchReplyProxy const&reply) const;
    void query_albums_by_genre(unity::scopes::SearchReplyProxy const &reply, const std::string& genre) const;
    void query_albums_by_artist(unity::scopes::SearchReplyProxy const &reply, const std::string& artist) const;
    void query_songs_by_artist(unity::scopes::SearchReplyProxy const &reply, const std::string& artist) const;
    void query_artists(unity::scopes::SearchReplyProxy const& reply) const;
    std::string fetch_biography_sync(const std::string& artist, const std::string &album) const;

    unity::scopes::CategorisedResult create_album_result(unity::scopes::Category::SCPtr const& category, mediascanner::Album const& album) const;
    unity::scopes::CategorisedResult create_song_result(unity::scopes::Category::SCPtr const& category, mediascanner::MediaFile const& media) const;
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
