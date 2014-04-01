# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import unittest as ut

import hoep as h


class SmartyPantsTestCase(ut.TestCase):
    def setUp(self):
        self.md = h.Hoep(render_flags = h.HTML_SMARTYPANTS).render
    
    
    def test_apostrophe(self):
        supplied = 'What\'s with apostrophes?'
        expected = '<p>What&rsquo;s with apostrophes?</p>\n'
        
        self.assertEqual(self.md(supplied), expected)
    
    def test_double_quotes(self):
        supplied = '"Air quotes are obnoxious."'
        expected = '<p>&ldquo;Air quotes are obnoxious.&rdquo;</p>\n'
        
        self.assertEqual(self.md(supplied), expected)
    
    def test_ellipsis(self):
        supplied = 'One of these days...'
        expected = '<p>One of these days&hellip;</p>\n'
        
        self.assertEqual(self.md(supplied), expected)
    
    def test_em_dash(self):
        supplied = 'In five minutes the---'
        expected = '<p>In five minutes the&mdash;</p>\n'
        
        self.assertEqual(self.md(supplied), expected)
    
    def test_en_dash(self):
        supplied = 'Non--zero.'
        expected = '<p>Non&ndash;zero.</p>\n'
        
        self.assertEqual(self.md(supplied), expected)
