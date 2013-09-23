#!/usr/bin/python3

import operator
import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

import dbus
import dbusmock
from gi.repository import Gio, Unity

srcdir = os.path.realpath(os.path.dirname(sys.argv[0]))

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
        cls.mediadir = os.path.join(srcdir, 'media')

        # Set up a private session bus
        cls.start_session_bus();
        cls.session_bus = cls.get_dbus();

        # Start the media scanner on the private bus
        cls.mediascanner_proc = subprocess.Popen(
            ['mediascanner-service',
             '--disable-volume-monitor',
             cls.mediadir])
        cls.wait_for_bus_object('com.canonical.MediaScanner',
                                '/com/canonical/MediaScanner')
        cls.mediascanner = dbus.Interface(
            cls.session_bus.get_object(
                'com.canonical.MediaScanner', '/com/canonical/MediaScanner'),
            'com.canonical.MediaScanner')
        cls.ensure_media_available(['song1.ogg', 'song2.ogg', 'video1.ogv'])

        # Load the scopes
        loader = Unity.ScopeLoader()
        [cls.music_scope, cls.video_scope] = loader.get_scopes(
            '../src/unity-scope-mediascanner.so', 'C')

    @classmethod
    def tearDownClass(cls):
        cls.music_scope = None
        cls.video_scope = None
        cls.mediascanner_proc.terminate()
        cls.mediascanner_proc.wait()
        shutil.rmtree(cls.tempdir, ignore_errors=True)
        super(ScopeTestCase, cls).tearDownClass()

    @classmethod
    def ensure_media_available(cls, filenames):
        remaining = set()
        for filename in filenames:
            f = Gio.File.new_for_path(os.path.join(cls.mediadir, filename))
            remaining.add(f.get_uri())
        timeout = 50
        while timeout > 0:
            for uri in list(remaining):
                if cls.mediascanner.MediaInfoExists(uri):
                    remaining.remove(uri)
            if len(remaining) == 0:
                break
            timeout -= 1
            time.sleep(0.1)
        else:
            raise AssertionError('timed out waiting for media info for %r' % remaining)

    def perform_search(self, scope, search_query):
        filter_set = scope.get_filters()
        result_set = LoggingResultSet()
        context = Unity.SearchContext.create(
            search_query, 0, filter_set, None, result_set, None)
        search = scope.create_search_for_query(context)
        search.run()
        return result_set.results

    def test_music_surfacing(self):
        results = self.perform_search(self.music_scope, '')
        self.assertEqual(len(results), 2)
        results.sort(key=operator.attrgetter('uri'))
        [song1, song2] = results
        self.assertEqual(os.path.basename(song1.uri), 'song1.ogg')
        self.assertEqual(song1.title, 'TitleOne')
        self.assertEqual(song1.metadata['duration'].get_int32(), 1)
        self.assertEqual(song1.metadata['artist'].get_string(), 'ArtistOne')
        self.assertEqual(song1.metadata['album'].get_string(), 'AlbumOne')
        self.assertEqual(song1.metadata['track-number'].get_int32(), 2)

        self.assertEqual(os.path.basename(song2.uri), 'song2.ogg')
        self.assertEqual(song2.title, 'TitleTwo')
        self.assertEqual(song2.metadata['duration'].get_int32(), 1)
        self.assertEqual(song2.metadata['artist'].get_string(), 'ArtistTwo')
        self.assertEqual(song2.metadata['album'].get_string(), 'AlbumTwo')
        self.assertEqual(song2.metadata['track-number'].get_int32(), 42)

    def test_music_search(self):
        results = self.perform_search(self.music_scope, 'titleone')
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].title, 'TitleOne')

        results = self.perform_search(self.music_scope, 'albumone')
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].title, 'TitleOne')

        results = self.perform_search(self.music_scope, 'artistone')
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].title, 'TitleOne')

        # Check that substring matches work.
        results = self.perform_search(self.music_scope, 'artiston')
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].title, 'TitleOne')

        results = self.perform_search(self.music_scope, 'unknown')
        self.assertEqual(len(results), 0)

    def test_video_surfacing(self):
        results = self.perform_search(self.video_scope, "")
        self.assertEqual(len(results), 1)
        video = results[0]
        self.assertEqual(os.path.basename(video.uri), 'video1.ogv')
        self.assertEqual(video.title, 'video1')
        self.assertEqual(video.metadata['duration'].get_int32(), 1)
        self.assertEqual(video.metadata['width'].get_int32(), 320)
        self.assertEqual(video.metadata['height'].get_int32(), 240)


if __name__ == '__main__':
    unittest.main()
