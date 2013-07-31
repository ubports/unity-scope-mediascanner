#include "config.h"
#include "scope.h"

int
unity_scope_module_get_version ()
{
    return UNITY_SCOPE_API_VERSION;
}

GList *
unity_scope_module_load_scopes (GError **error)
{
    grl_init (NULL, NULL);

    GrlRegistry *registry = grl_registry_get_default ();
    if (!grl_registry_load_plugin_directory (
            registry, "/home/james/src/unity/prefix/lib/grilo-0.2", error)) {
        return NULL;
    }

    GrlSource *source = grl_registry_lookup_source (registry, "grl-hollywood");
    UnityAbstractScope *music = music_scope_new (source);
    UnityAbstractScope *video = video_scope_new (source);

    return g_list_append (g_list_append(NULL, music), video);
}

