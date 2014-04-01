# -*- coding: utf-8 -*-

from __future__ import unicode_literals

from codecs import open
from os import path as op
import unittest as ut

import hoep as h


class UnicodeTestCase(ut.TestCase):
    def setUp(self):
        self._path = op.join(op.dirname(__file__), 'res')
        
        self.md = h.Hoep().render
    
    
    def _get_path(self, *args):
        return op.abspath(op.join(self._path, *args))
    
    
    def runTest(self):
        with open(self._get_path('Unicode', 'unicode.txt'), 'r', encoding = 'utf-8') as f:
            supplied = f.read()
        
        with open(self._get_path('Unicode', 'unicode.html'), 'r', encoding = 'utf-8') as f:
            expected = f.read()
        
        self.assertEqual(self.md(supplied), expected)
