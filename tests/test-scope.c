#include <config.h>
#include <glib.h>
#include "../src/scope.h"

/* A UnityResultSet implementation that saves the last added result
 * for inspection */

typedef struct _TestResultSet TestResultSet;
typedef struct _TestResultSetClass TestResultSetClass;

struct _TestResultSet {
    UnityResultSet parent;
    UnityScopeResult last_result;
};

struct _TestResultSetClass {
    UnityResultSetClass parent_class;
};

G_DEFINE_TYPE (TestResultSet, test_result_set, UNITY_TYPE_RESULT_SET);
static UnityResultSetClass *result_set_parent_class;

static void
test_result_set_finalize (GObject *object)
{
    TestResultSet *result_set = (TestResultSet *)object;

    unity_scope_result_destroy (&result_set->last_result);

    G_OBJECT_CLASS (result_set_parent_class)->finalize (object);
}

static void
test_result_set_add_result (UnityResultSet *object, UnityScopeResult *result)
{
    TestResultSet *result_set = (TestResultSet *)object;

    unity_scope_result_copy (result, &result_set->last_result);
}

static void
test_result_set_class_init (TestResultSetClass *class)
{
    result_set_parent_class = g_type_class_peek_parent (class);
    UNITY_RESULT_SET_CLASS (class)->add_result = test_result_set_add_result;
    G_OBJECT_CLASS (class)->finalize = test_result_set_finalize;
}

static void
test_result_set_init (TestResultSet *result_set)
{
}

static TestResultSet *
test_result_set_new ()
{
    return g_object_new (test_result_set_get_type (), NULL);
}


static void
test_music_add_result ()
{
    GrlMedia *media = grl_media_audio_new ();
    TestResultSet *result_set = test_result_set_new ();

    grl_media_set_id (media, "test-id");
    grl_media_set_url (media, "http://example.com/foo.ogg");
    grl_media_set_mime (media, "audio/ogg");
    grl_media_set_title (media, "Title");
    grl_media_set_duration (media, 60);
    grl_media_audio_set_artist (GRL_MEDIA_AUDIO (media), "Artist");
    grl_media_audio_set_album (GRL_MEDIA_AUDIO (media), "Album");
    grl_media_audio_set_track_number (GRL_MEDIA_AUDIO (media), 42);

    music_add_result (UNITY_RESULT_SET (result_set), media);

    UnityScopeResult *result = &result_set->last_result;
    g_assert_cmpstr (result->uri, ==, "http://example.com/foo.ogg");
    g_assert_cmpint (result->result_type, ==, UNITY_RESULT_TYPE_PERSONAL);
    g_assert_cmpstr (result->mimetype, ==, "audio/ogg");
    g_assert_cmpstr (result->title, ==, "Title");
    g_assert_cmpstr (result->dnd_uri, ==, "http://example.com/foo.ogg");

    GVariant *variant;
    variant = g_hash_table_lookup (result->metadata, "duration");
    g_assert_cmpint (g_variant_get_int32 (variant), ==, 60);

    variant = g_hash_table_lookup (result->metadata, "artist");
    g_assert_cmpstr (g_variant_get_string (variant, NULL), ==, "Artist");

    variant = g_hash_table_lookup (result->metadata, "album");
    g_assert_cmpstr (g_variant_get_string (variant, NULL), ==, "Album");

    variant = g_hash_table_lookup (result->metadata, "track-number");
    g_assert_cmpint (g_variant_get_int32 (variant), ==, 42);

    g_object_unref (result_set);
    g_object_unref (media);
}

static void
test_video_add_result ()
{
    GrlMedia *media = grl_media_video_new ();
    TestResultSet *result_set = test_result_set_new ();

    grl_media_set_id (media, "test-id");
    grl_media_set_url (media, "http://example.com/foo.mp4");
    grl_media_set_mime (media, "video/mp4");
    grl_media_set_title (media, "Title");
    grl_media_set_duration (media, 60);

    video_add_result (UNITY_RESULT_SET (result_set), media);

    UnityScopeResult *result = &result_set->last_result;
    g_assert_cmpstr (result->uri, ==, "http://example.com/foo.mp4");
    g_assert_cmpint (result->result_type, ==, UNITY_RESULT_TYPE_PERSONAL);
    g_assert_cmpstr (result->mimetype, ==, "video/mp4");
    g_assert_cmpstr (result->title, ==, "Title");
    g_assert_cmpstr (result->dnd_uri, ==, "http://example.com/foo.mp4");

    GVariant *variant;
    variant = g_hash_table_lookup (result->metadata, "duration");
    g_assert_cmpint (g_variant_get_int32 (variant), ==, 60);

    g_object_unref (result_set);
    g_object_unref (media);
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    grl_init (&argc, &argv);

    g_test_add_func ("/Music/AddResult", test_music_add_result);
    g_test_add_func ("/Video/AddResult", test_video_add_result);

    g_test_run ();
    return 0;
}
