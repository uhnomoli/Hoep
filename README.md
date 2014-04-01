# Hoep

![Build status][travis-ci]

Hoep is a Python binding for the Hoedown Markdown library.


## Install

Hoep is available on PyPi so installation is only a quick `pip` away.

    $ pip install hoep


## Unicode

Hoep only accepts and returns `unicode` objects in Python 2 and `str` objects in Python 3. This goes for custom renderers as well. All callbacks receive and must return `unicode` or `str` objects, respective to the Python version.


## Basic Usage

Using Hoep is pretty straightforward. You can either use the `render` module function or the `Hoep` module class. The latter is more efficient if you are rendering multiple documents in a single go.

```python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import hoep as h


extensions = h.EXT_STRIKETHROUGH
render_flags = h.HTML_SMARTYPANTS


# Method form.
html = h.render('Some ~~bold~~ __"strong"__ text.', extensions, render_flags)


# Class form.
md = h.Hoep(extensions, render_flags)

html = md.render('Some ~~bold~~ __"strong"__ text.')
```

### Extensions

You can enable multiple extensions by bitwise ORing them.

+ `EXT_AUTOLINK`: Automatically parse URLs into links.
+ `EXT_DISABLE_INDENTED_CODE`: Disables indented code blocks.
+ `EXT_FENCED_CODE`: Enables fenced code blocks.
+ `EXT_FOOTNOTES`: Enables [Markdown Extra style footnotes][syntax-footnotes].
+ `EXT_HIGHLIGHT`: Enables ==marking== text.
+ `EXT_LAX_SPACING`: Removes the need for an empty line between Markdown and raw HTML.
+ `EXT_NO_INTRA_EMPHASIS`: Disables emphasis_between_words.
+ `EXT_QUOTE`: "Quotes" are translated into `<q>` tags.
+ `EXT_SPACE_HEADERS`: ATX style headers require a space after the opening number sign(s).
+ `EXT_STRIKETHROUGH`: Enables \~~striking~~ text.
+ `EXT_SUPERSCRIPT`: Enables super\^script.
+ `EXT_TABLES`: Enables [Markdown Extra style tables][syntax-tables].
+ `EXT_UNDERLINE`: Translates `<em>` tags into `<u>` tags.

### Render flags

You can enable multiple render flags by bitwise ORing them.

+ `HTML_ESCAPE`: All HTML is escaped.
+ `HTML_EXPAND_TABS`: Tabs are expanded to spaces.
+ `HTML_HARD_WRAP`: Line breaks are translated into `<br>` tags.
+ `HTML_SAFELINK`: Only links to safe protocols are allowed.
+ `HTML_SKIP_HTML`: All HTML tags are stripped.
+ `HTML_SKIP_IMAGES`: Images are ignored.
+ `HTML_SKIP_LINKS`: Links are ignored.
+ `HTML_SKIP_STYLE`: `<style>` tags are stripped.
+ `HTML_SMARTYPANTS`: Enables SmartyPants.
+ `HTML_TOC`: Anchors are added to headers.
+ `HTML_USE_XHTML`: Renders XHTML instead of HTML.


## Custom renderer

By subclassing the `Hoep` class, you can create a custom renderer overriding various callbacks.

```python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import hoep as h


class MyRenderer(h.Hoep):
    def __init__(self, extensions = 0, render_flags = 0):
        # If you override the `__init__` function, you must call
        # the parent class's or bad things will happen.
        super(MyRenderer, self).__init__(extensions, render_flags)
        
        self.replace = '[redacted]'
    
    
    def strikethrough(self, text):
        return self.replace


md = MyRenderer(h.EXT_STRIKETHROUGH)

html = md.render('Some ~~bold~~ __"strong"__ text.')
```

### Callbacks

#### Document level

+ `preprocess(str markdown)`
+ `doc_header()`
+ `doc_footer()`
+ `postprocess(str html)`

#### Block level

+ `block_code(str text, str language)`
+ `block_html(str text)`
+ `block_quote(str text)`
+ `footnotes(str text)`
+ `footnote_def(str text, int number)`
+ `header(str text, int level)`
+ `hrule()`
+ `list(str text, bool ordered)`
+ `list_item(str text, bool ordered)`
+ `paragraph(str text)`
+ `table(str header, str body)`
+ `table_row(str text)`
+ `table_cell(str text, int flags)`

#### Span level

+ `autolink(str link, bool is_email)`
+ `codespan(str text)`
+ `double_emphasis(str text)`
+ `emphasis(str text)`
+ `footnote_ref(int number)`
+ `highlight(str text)`
+ `image(str link, mixed title, str alt)`
+ `line_break()`
+ `link(str link, mixed title, str content)`
+ `quote(str text)`
+ `raw_html_tag(str tag)`
+ `strikethrough(str text)`
+ `superscript(str text)`
+ `triple_emphasis(str text)`
+ `underline(str text)`

#### Low-level

+ `entity(str entity)`
+ `normal_text(str text)`


## Support

If you run into any issues or have any questions, either open an [issue][github-issues] or hop in [#mynt][irc-webchat] on irc.freenode.net.


[github-issues]: https://github.com/Anomareh/Hoep/issues
[irc-webchat]: http://webchat.freenode.net/?channels=mynt
[syntax-footnotes]: http://michelf.ca/projects/php-markdown/extra/#footnotes
[syntax-tables]: http://michelf.ca/projects/php-markdown/extra/#table

[travis-ci]: https://api.travis-ci.org/repositories/Anomareh/Hoep.png?branch=master
