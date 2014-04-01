# -*- coding: utf-8 -*-

from __future__ import unicode_literals

from textwrap import dedent
import unittest as ut

import hoep as h


class _IgnoreParagraph(h.Hoep):
    def paragraph(self, text):
        return text


class BlockRenderer(h.Hoep):
    def block_code(self, text, language):
        return '[BLOCK_CODE language={1}] {0}'.format(text, language)
    
    def block_html(self, text):
        return '[BLOCK_HTML] {0}'.format(text)
    
    def block_quote(self, text):
        return '[BLOCK_QUOTE] {0}'.format(text)
    
    def header(self, text, level):
        return '[HEADER level={1}] {0}'.format(text, level)
    
    def hrule(self):
        return '[HRULE]'
    
    def list(self, text, ordered):
        return '[LIST ordered={1}]\n{0}'.format(text, ordered)
    
    def list_item(self, text, ordered):
        return '[LIST_ITEM ordered={1}] {0}'.format(text, ordered)
    
    def table(self, header, body):
        return '[TABLE]\n[HEADER]{0}\n[BODY]{1}'.format(header, body)
    
    def table_row(self, text):
        return '\n[TABLE_ROW]\n{0}'.format(text)
    
    def table_cell(self, text, flags):
        return '[TABLE_CELL text={0}]'.format(text)

class DocRenderer(_IgnoreParagraph):
    def doc_header(self):
        return 'One.\n'
    
    def doc_footer(self):
        return '\nFive.'

class FootnoteRenderer(h.Hoep):
    def footnotes(self, text):
        return '[FOOTNOTES]\n{0}'.format(text)
    
    def footnote_def(self, text, number):
        return '[FOOTNOTE_DEF number={1}] {0}'.format(text, number)
    
    def footnote_ref(self, number):
        return '[FOOTNOTE_REF number={0}]'.format(number)

class LowRenderer(_IgnoreParagraph):
    def entity(self, entity):
        return '[ENTITY] {0}'.format(entity)
    
    def normal_text(self, text):
        return '[NORMAL_TEXT] {0}'.format(text)

class ParagraphRenderer(h.Hoep):
    def paragraph(self, text):
        return '[PARAGRAPH] {0}'.format(text)

class PreRenderer(_IgnoreParagraph):
    def preprocess(self, markdown):
        return 'Nop! {0}'.format(markdown)

class PostRenderer(_IgnoreParagraph):
    def postprocess(self, html):
        return '{0} {1}'.format(html, html[::-1])

class SpanRenderer(_IgnoreParagraph):
    def autolink(self, link, email):
        return '[AUTOLINK email={1}] {0}'.format(link, email)
    
    def codespan(self, text):
        return '[CODESPAN] {0}'.format(text)
    
    def double_emphasis(self, text):
        return '[DOUBLE_EMPHASIS] {0}'.format(text)
    
    def emphasis(self, text):
        return '[EMPHASIS] {0}'.format(text)
    
    def highlight(self, text):
        return '[HIGHLIGHT] {0}'.format(text)
    
    def image(self, link, title, alt):
        return '[IMAGE link={0} title={1} alt={2}]'.format(link, title, alt)
    
    def line_break(self):
        return '[LINE_BREAK]'
    
    def link(self, link, title, content):
        return '[LINK link={0} title={1}] {2}'.format(link, title, content)
    
    def quote(self, text):
        return '[QUOTE] {0}'.format(text)
    
    def raw_html_tag(self, tag):
        return '[RAW_HTML_TAG] {0}'.format(tag)
    
    def strikethrough(self, text):
        return '[STRIKETHROUGH] {0}'.format(text)
    
    def superscript(self, text):
        return '[SUPERSCRIPT] {0}'.format(text)
    
    def triple_emphasis(self, text):
        return '[TRIPLE_EMPHASIS] {0}'.format(text)

class UnderlineRenderer(_IgnoreParagraph):
    def underline(self, text):
        return '[UNDERLINE] {0}'.format(text)


class CustomRendererTestCase(ut.TestCase):
    def setUp(self):
        self.block = BlockRenderer(h.EXT_FENCED_CODE | h.EXT_TABLES)
        self.doc = DocRenderer()
        self.footnote = FootnoteRenderer(h.EXT_FOOTNOTES)
        self.low = LowRenderer()
        self.paragraph = ParagraphRenderer()
        self.post = PostRenderer()
        self.pre = PreRenderer()
        self.span = SpanRenderer(h.EXT_AUTOLINK | h.EXT_HIGHLIGHT | h.EXT_QUOTE | h.EXT_STRIKETHROUGH | h.EXT_SUPERSCRIPT)
        self.underline = UnderlineRenderer(h.EXT_UNDERLINE)
    
    
    def md(self, md, renderer):
        return getattr(self, renderer).render(md)
    
    
    # Document level
    def test_preprocess(self):
        supplied = 'First!'
        expected = 'Nop! First!'
        
        self.assertEqual(self.md(supplied, 'pre'), expected)
    
    def test_doc_header_and_footer(self):
        supplied = 'Two.'
        expected = 'One.\nTwo.\nFive.'
        
        self.assertEqual(self.md(supplied, 'doc'), expected)
    
    def test_postprocess(self):
        supplied = 'echo'
        expected = 'echo ohce'
        
        self.assertEqual(self.md(supplied, 'post'), expected)
    
    
    # Block level
    def test_block_code(self):
        supplied = '```bash\n$ :(){ :|:& };:\n```'
        expected = '[BLOCK_CODE language=bash] $ :(){ :|:& };:\n'
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    def test_block_html(self):
        supplied = '<p>Hi.</p>'
        expected = '[BLOCK_HTML] <p>Hi.</p>\n'
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    def test_block_quote(self):
        supplied = '> Echo.'
        expected = '[BLOCK_QUOTE] <p>Echo.</p>\n'
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    def test_footnotes(self):
        supplied = 'What you looking at? [^1]\n\n[^1]: Yeah, I\'m talking to you pal.'
        expected = dedent('''\
            <p>What you looking at? [FOOTNOTE_REF number=1]</p>
            [FOOTNOTES]
            [FOOTNOTE_DEF number=1] <p>Yeah, I&#39;m talking to you pal.</p>
            ''')
        
        self.assertEqual(self.md(supplied, 'footnote'), expected)
    
    def test_header(self):
        supplied = '## One more to go.'
        expected = '[HEADER level=2] One more to go.'
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    def test_hrule(self):
        supplied = '---'
        expected = '[HRULE]'
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    def test_list_ordered(self):
        supplied = '1. Ehh\n2. Bee\n3. Eee'
        expected = dedent('''\
            [LIST ordered=True]
            [LIST_ITEM ordered=True] Ehh
            [LIST_ITEM ordered=True] Bee
            [LIST_ITEM ordered=True] Eee
            ''')
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    def test_list_unordered(self):
        supplied = '+ One\n+ Two\n+ Five'
        expected = dedent('''\
            [LIST ordered=False]
            [LIST_ITEM ordered=False] One
            [LIST_ITEM ordered=False] Two
            [LIST_ITEM ordered=False] Five
            ''')
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    def test_paragraph(self):
        supplied = 'One might say this is soul sucking...'
        expected = '[PARAGRAPH] One might say this is soul sucking...'
        
        self.assertEqual(self.md(supplied, 'paragraph'), expected)
    
    def test_table(self):
        supplied = dedent(u'''\
            |  1  |  2  |  3  |
            | --- | --- | --- |
            |  X  |  X  |  O  |
            |  O  |  O  |  X  |
            |  X  |  O  |  X  |
            ''')
        expected = dedent('''\
            [TABLE]
            [HEADER]
            [TABLE_ROW]
            [TABLE_CELL text=1][TABLE_CELL text=2][TABLE_CELL text=3]
            [BODY]
            [TABLE_ROW]
            [TABLE_CELL text=X][TABLE_CELL text=X][TABLE_CELL text=O]
            [TABLE_ROW]
            [TABLE_CELL text=O][TABLE_CELL text=O][TABLE_CELL text=X]
            [TABLE_ROW]
            [TABLE_CELL text=X][TABLE_CELL text=O][TABLE_CELL text=X]''')
        
        self.assertEqual(self.md(supplied, 'block'), expected)
    
    
    # Span level
    def test_autolink(self):
        supplied = 'https://github.com/'
        expected = '[AUTOLINK email=False] https://github.com/'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_codespan(self):
        supplied = '`$ rm -Rf tests/`'
        expected = '[CODESPAN] $ rm -Rf tests/'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_double_emphasis(self):
        supplied = '__strong__'
        expected = '[DOUBLE_EMPHASIS] strong'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_emphasis(self):
        supplied = '_wat_'
        expected = '[EMPHASIS] wat'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_highlight(self):
        supplied = '==blink=='
        expected = '[HIGHLIGHT] blink'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_image(self):
        supplied = '![spacer](spacer.gif "spacer")'
        expected = '[IMAGE link=spacer.gif title=spacer alt=spacer]'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_image_ref(self):
        supplied = '![spacer][spacer]\n\n[spacer]: spacer.gif'
        expected = '[IMAGE link=spacer.gif title=None alt=spacer]'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_line_break(self):
        supplied = 'So.  \nTired.'
        expected = 'So.[LINE_BREAK]Tired.'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_link(self):
        supplied = '[GitHub](https://github.com/)'
        expected = '[LINK link=https://github.com/ title=None] GitHub'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_link_ref(self):
        supplied = '[GitHub][github]\n\n[github]: https://github.com/ "GitHub"'
        expected = '[LINK link=https://github.com/ title=GitHub] GitHub'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_quote(self):
        supplied = '"Air quotes are obnoxious."'
        expected = '[QUOTE] Air quotes are obnoxious.'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_raw_html_tag(self):
        supplied = '<halp/>'
        expected = '[RAW_HTML_TAG] <halp/>'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_strikethrough(self):
        supplied = 'I\'m ~~running~~ out of ideas.'
        expected = 'I&#39;m [STRIKETHROUGH] running out of ideas.'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_superscript(self):
        supplied = '^bro'
        expected = '[SUPERSCRIPT] bro'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_triple_emphasis(self):
        supplied = 'Triple emphasis? That\'s ___absurd___.'
        expected = 'Triple emphasis? That&#39;s [TRIPLE_EMPHASIS] absurd.'
        
        self.assertEqual(self.md(supplied, 'span'), expected)
    
    def test_underline(self):
        supplied = 'That\'s _it_?'
        expected = 'That&#39;s [UNDERLINE] it?'
        
        self.assertEqual(self.md(supplied, 'underline'), expected)
    
    
    # Low-level
    def test_low_level(self):
        supplied = '&#9731;'
        expected = '[NORMAL_TEXT] [ENTITY] &#9731;'
        
        self.assertEqual(self.md(supplied, 'low'), expected)
