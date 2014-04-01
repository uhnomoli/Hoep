# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import unittest as ut

import hoep as h


class RenderFlagsTestCase(ut.TestCase):
    def setUp(self):
        self.render_flags = {
            'escape': h.HTML_ESCAPE,
            'hard_wrap': h.HTML_HARD_WRAP,
            'safelink': h.HTML_SAFELINK,
            'skip_html': h.HTML_SKIP_HTML,
            'skip_images': h.HTML_SKIP_IMAGES,
            'skip_links': h.HTML_SKIP_LINKS,
            'skip_style': h.HTML_SKIP_STYLE,
            'xhtml': h.HTML_USE_XHTML
        }
    
    
    def md(self, md, flag):
        return h.render(md, render_flags = self.render_flags[flag])
    
    
    def test_hard_wrap(self):
        supplied = 'One.\nAt.\nA.\nTime.'
        expected = '<p>One.<br>\nAt.<br>\nA.<br>\nTime.</p>\n'
        
        self.assertEqual(self.md(supplied, 'hard_wrap'), expected)
    
    def test_html_escaping(self):
        supplied = 'JavaScript is pure <strong style="color: red;">evil</strong>.'
        expected = '<p>JavaScript is pure &lt;strong style=&quot;color: red;&quot;&gt;evil&lt;/strong&gt;.</p>\n'
        
        self.assertEqual(self.md(supplied, 'escape'), expected)
    
    def test_safelink(self):
        supplied = '[alert](javascript:alert(\'it\\\'s a trap\');)'
        expected = '<p>[alert](javascript:alert(&#39;it\&#39;s a trap&#39;);)</p>\n'
        
        self.assertEqual(self.md(supplied, 'safelink'), expected)
    
    def test_safelink_ref(self):
        supplied = '[alert][alert]\n\n[alert]: javascript:alert(\'it\\\'s a trap\');'
        expected = '<p>[alert][alert]</p>\n\n<p>[alert]: javascript:alert(&#39;it\&#39;s a trap&#39;);</p>\n'
        
        self.assertEqual(self.md(supplied, 'safelink'), expected)
    
    def test_skip_html(self):
        supplied = 'It <blink>must be</blink> allergies.'
        expected = '<p>It must be allergies.</p>\n'
        
        self.assertEqual(self.md(supplied, 'skip_html'), expected)
    
    def test_skip_images(self):
        supplied = '![spacer](spacer.gif)'
        expected = '<p>![spacer](spacer.gif)</p>\n'
        
        self.assertEqual(self.md(supplied, 'skip_images'), expected)
    
    def test_skip_images_ref(self):
        # wat?
        supplied = '![spacer][spacer]\n\n[spacer]: spacer.gif'
        expected = '<p>![spacer]<a href="spacer.gif">spacer</a></p>\n'
        
        self.assertEqual(self.md(supplied, 'skip_images'), expected)
    
    def test_skip_links(self):
        supplied = '[GitHub.](https://github.com/)'
        expected = '<p>[GitHub.](https://github.com/)</p>\n'
        
        self.assertEqual(self.md(supplied, 'skip_links'), expected)
    
    def test_skip_links_ref(self):
        supplied = '[GitHub.][github]\n\n[github]: https://github.com/'
        expected = '<p>[GitHub.][github]</p>\n'
        
        self.assertEqual(self.md(supplied, 'skip_links'), expected)
    
    def test_skip_style(self):
        supplied = '<style type="text/css">* { content: \'stylin\\\'\'; }</style>'
        expected = u'<style type="text/css">* { content: \'stylin\\\'\'; }</style>\n'
        
        self.assertEqual(self.md(supplied, 'skip_style'), expected)
    
    def test_xhtml(self):
        supplied = '![spacer](spacer.gif)'
        expected = '<p><img src="spacer.gif" alt="spacer"/></p>\n'
        
        self.assertEqual(self.md(supplied, 'xhtml'), expected)
