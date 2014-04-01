# -*- coding: utf-8 -*-

from __future__ import unicode_literals

from codecs import open
from glob import iglob
from os import path as op
from subprocess import Popen, PIPE, STDOUT
import unittest as ut

import hoep as h


class MarkdownTestCase(ut.TestCase):
    def setUp(self):
        self._path = op.join(op.dirname(__file__), 'res')
        
        self.md = h.Hoep().render
    
    
    def _get_path(self, *args):
        return op.abspath(op.join(self._path, *args))
    
    def _tidy(self, html):
        args = [
            'tidy',
            '--show-body-only', '1',
            '--quiet', '1',
            '--show-warnings', '0',
            '-utf8'
        ]
        
        tidy = Popen(args, stdout = PIPE, stdin = PIPE, stderr = STDOUT)
        stdout, stderr = tidy.communicate(input = html.encode('utf-8'))
        
        return stdout.decode('utf-8')
    
    
    def runTest(self):
        for suite in ('1.0', '1.0.3'):
            for path in iglob(self._get_path('MarkdownTest_{0}'.format(suite), '*.txt')):
                test = op.splitext(path)[0]
                
                with open('{0}.txt'.format(test), 'r', encoding = 'utf-8') as f:
                    supplied = f.read()
                
                with open('{0}.html'.format(test), 'r', encoding = 'utf-8') as f:
                    expected = f.read()
                
                actual = self.md(supplied)
                
                self.assertEqual(self._tidy(actual), self._tidy(expected))
