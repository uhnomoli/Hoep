# -*- coding: utf-8 -*-

from __future__ import unicode_literals

from textwrap import dedent
import unittest as ut

import hoep as h


class ExtensionsTestCase(ut.TestCase):
    def setUp(self):
        self.extensions = {
            'autolink': h.EXT_AUTOLINK,
            'fenced_code': h.EXT_FENCED_CODE,
            'footnotes': h.EXT_FOOTNOTES,
            'highlight': h.EXT_HIGHLIGHT,
            'lax_spacing': h.EXT_LAX_SPACING,
            'no_indented_code': h.EXT_DISABLE_INDENTED_CODE,
            'no_intra_emphasis': h.EXT_NO_INTRA_EMPHASIS,
            'quote': h.EXT_QUOTE,
            'space_headers': h.EXT_SPACE_HEADERS,
            'strikethrough': h.EXT_STRIKETHROUGH,
            'superscript': h.EXT_SUPERSCRIPT,
            'tables': h.EXT_TABLES,
            'underline': h.EXT_UNDERLINE
        }
    
    
    def md(self, md, extension):
        return h.render(md, self.extensions[extension])
    
    
    def test_autolink(self):
        supplied = 'https://github.com/'
        expected = '<p><a href="https://github.com/">https://github.com/</a></p>\n'
        
        self.assertEqual(self.md(supplied, 'autolink'), expected)
    
    def test_fenced_code(self):
        supplied = '```\n$ :(){ :|:& };:\n```'
        expected = '<pre><code>$ :(){ :|:&amp; };:\n</code></pre>\n'
        
        self.assertEqual(self.md(supplied, 'fenced_code'), expected)
    
    def test_fenced_code_lang(self):
        supplied = '```bash\n$ :(){ :|:& };:\n```'
        expected = '<pre><code class="language-bash">$ :(){ :|:&amp; };:\n</code></pre>\n'
        
        self.assertEqual(self.md(supplied, 'fenced_code'), expected)
    
    def test_footnotes(self):
        supplied = 'What you looking at? [^1]\n\n[^1]: Yeah, I\'m talking to you pal.'
        expected = dedent('''\
            <p>What you looking at? <sup id="fnref1"><a href="#fn1" rel="footnote">1</a></sup></p>
            
            <div class="footnotes">
            <hr>
            <ol>
            
            <li id="fn1">
            <p>Yeah, I&#39;m talking to you pal.&nbsp;<a href="#fnref1" rev="footnote">&#8617;</a></p>
            </li>
            
            </ol>
            </div>
            ''')
        
        self.assertEqual(self.md(supplied, 'footnotes'), expected)
    
    def test_highlight(self):
        supplied = 'I\'m ==special==.'
        expected = '<p>I&#39;m <mark>special</mark>.</p>\n'
        
        self.assertEqual(self.md(supplied, 'highlight'), expected)
    
    def test_lax_spacing(self):
        supplied = 'This extension is madness.\n<p>Truly.</p>\n'
        expected = '<p>This extension is madness.</p>\n\n<p>Truly.</p>\n'
        
        self.assertEqual(self.md(supplied, 'lax_spacing'), expected)
    
    def test_no_indented_code(self):
        supplied = '    $ :(){ :|:& };:'
        expected = '<p>$ :(){ :|:&amp; };:</p>\n'
        
        self.assertEqual(self.md(supplied, 'no_indented_code'), expected)
    
    def test_no_intra_emphasis(self):
        supplied = 'Sorry,_my_space_key_is_broken.'
        expected = '<p>Sorry,_my_space_key_is_broken.</p>\n'
        
        self.assertEqual(self.md(supplied, 'no_intra_emphasis'), expected)
    
    def test_quote(self):
        supplied = '"Air quotes are obnoxious."'
        expected = '<p><q>Air quotes are obnoxious.</q></p>\n'
        
        self.assertEqual(self.md(supplied, 'quote'), expected)
    
    def test_space_headers(self):
        supplied = '#Are you listening to me?!'
        expected = '<p>#Are you listening to me?!</p>\n'
        
        self.assertEqual(self.md(supplied, 'space_headers'), expected)
    
    def test_strikethrough(self):
        supplied = 'I\'m ~~running~~ out of ideas.'
        expected = '<p>I&#39;m <del>running</del> out of ideas.</p>\n'
        
        self.assertEqual(self.md(supplied, 'strikethrough'), expected)
    
    def test_superscript(self):
        supplied = '^bro'
        expected = '<p><sup>bro</sup></p>\n'
        
        self.assertEqual(self.md(supplied, 'superscript'), expected)
    
    def test_tables(self):
        supplied = dedent(u'''\
            |  1  |  2  |  3  |
            | --- | --- | --- |
            |  X  |  X  |  O  |
            |  O  |  O  |  X  |
            |  X  |  O  |  X  |
            ''')
        expected = dedent('''\
            <table><thead>
            <tr>
            <th>1</th>
            <th>2</th>
            <th>3</th>
            </tr>
            </thead><tbody>
            <tr>
            <td>X</td>
            <td>X</td>
            <td>O</td>
            </tr>
            <tr>
            <td>O</td>
            <td>O</td>
            <td>X</td>
            </tr>
            <tr>
            <td>X</td>
            <td>O</td>
            <td>X</td>
            </tr>
            </tbody></table>
            ''')
        
        self.assertEqual(self.md(supplied, 'tables'), expected)
    
    def test_underline(self):
        supplied = 'What do you _mean_?'
        expected = '<p>What do you <u>mean</u>?</p>\n'
        
        self.assertEqual(self.md(supplied, 'underline'), expected)
