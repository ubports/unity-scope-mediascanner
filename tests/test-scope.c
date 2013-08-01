#include <config.h>
#include <glib.h>
#include "../src/scope.h"

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_run ();
    return 0;
}
