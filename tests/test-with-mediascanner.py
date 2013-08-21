#!/usr/bin/python3

import os
import shutil
import subprocess
import tempfile
import unittest

import dbusmock
from gi.repository import Unity

class LoggingResultSet(Unity.ResultSet):
    def __init__(self):
        super(LoggingResultSet, self).__init__()
        self.results = []

    def do_add_result(self, result):
        self.results.append(result.copy())


class ScopeTestCase(dbusmock.DBusTestCase):

    @classmethod
    def setUpClass(cls):
        super(ScopeTestCase, cls).setUpClass()
        cls.tempdir = tempfile.mkdtemp(dir=os.getcwd())
        cls.configdir = os.path.join(cls.tempdir, 'config')
        os.mkdir(cls.configdir)
        os.environ['XDG_CONFIG_HOME'] = cls.configdir
        cls.cachedir = os.path.join(cls.tempdir, 'cache')
        os.mkdir(cls.cachedir)
        os.environ['XDG_CACHE_HOME'] = cls.cachedir
        cls.mediadir = os.path.join(cls.tempdir, 'media')
        os.mkdir(cls.mediadir)

        # Set up a private session bus
        cls.start_session_bus();
        cls.session_bus = cls.get_dbus();

        # Start the media scanner on the private bus
        cls.mediascanner = subprocess.Popen(
            ['mediascanner-service',
             '--disable-volume-monitor',
             cls.mediadir])
        cls.wait_for_bus_object('com.canonical.MediaScanner',
                                '/com/canonical/MediaScanner')

        # Load the scopes
        loader = Unity.ScopeLoader()
        [cls.music_scope, cls.video_scope] = loader.get_scopes(
            '../src/unity-scope-mediascanner.so', 'C')

    @classmethod
    def tearDownClass(cls):
        cls.music_scope = None
        cls.video_scope = None
        cls.mediascanner.kill()
        shutil.rmtree(cls.tempdir, ignore_errors=True)
        super(ScopeTestCase, cls).tearDownClass()

    def perform_search(self, scope, search_query):
        filter_set = scope.get_filters()
        result_set = LoggingResultSet()
        context = Unity.SearchContext.create(
            "", 0, filter_set, None, result_set, None)
        search = scope.create_search_for_query(context)
        search.run()
        return result_set.results

    def test_music_search(self):
        results = self.perform_search(self.music_scope, "")
        self.assertEqual(results, [])

    def test_video_search(self):
        results = self.perform_search(self.video_scope, "")
        self.assertEqual(results, [])

if __name__ == '__main__':
    unittest.main()
