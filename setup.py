'''
Hoep
====

Hoep is a Python binding for the `Hoedown`_ Markdown library.


Install
-------

Hoep is available on PyPI so installation is only a quick ``pip`` away.

::

    $ pip install hoep


Unicode
-------

Hoep only accepts and returns ``unicode`` objects in Python 2 and ``str`` objects in Python 3. This goes for custom renderers as well. All callbacks receive and must return ``unicode`` or ``str`` objects, respective to the Python version.


Basic Usage
-----------

Using Hoep is pretty straightforward. You can either use the ``render`` module function or the ``Hoep`` module class. The latter is more efficient if you are rendering multiple documents in a single go.

.. code:: python

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

Extensions
~~~~~~~~~~

You can enable multiple extensions by bitwise ORing them.

-  ``EXT_AUTOLINK``: Automatically parse URLs into links.
-  ``EXT_DISABLE_INDENTED_CODE``: Disables indented code blocks.
-  ``EXT_FENCED_CODE``: Enables fenced code blocks.
-  ``EXT_FOOTNOTES``: Enables `Markdown Extra style footnotes`_.
-  ``EXT_HIGHLIGHT``: Enables ==marking== text.
-  ``EXT_LAX_SPACING``: Removes the need for an empty line between Markdown and raw HTML.
-  ``EXT_NO_INTRA_EMPHASIS``: Disables emphasis\_between\_words.
-  ``EXT_QUOTE``: "Quotes" are translated into ``<q>`` tags.
-  ``EXT_SPACE_HEADERS``: ATX style headers require a space after the opening number sign(s).
-  ``EXT_STRIKETHROUGH``: Enables ~~striking~~ text.
-  ``EXT_SUPERSCRIPT``: Enables super^script.
-  ``EXT_TABLES``: Enables `Markdown Extra style tables`_.
-  ``EXT_UNDERLINE``: Translates ``<em>`` tags into ``<u>`` tags.

Render flags
~~~~~~~~~~~~

You can enable multiple render flags by bitwise ORing them.

-  ``HTML_ESCAPE``: All HTML is escaped.
-  ``HTML_EXPAND_TABS``: Tabs are expanded to spaces.
-  ``HTML_HARD_WRAP``: Line breaks are translated into ``<br>`` tags.
-  ``HTML_SAFELINK``: Only links to safe protocols are allowed.
-  ``HTML_SKIP_HTML``: All HTML tags are stripped.
-  ``HTML_SKIP_IMAGES``: Images are ignored.
-  ``HTML_SKIP_LINKS``: Links are ignored.
-  ``HTML_SKIP_STYLE``: ``<style>`` tags are stripped.
-  ``HTML_SMARTYPANTS``: Enables SmartyPants.
-  ``HTML_TOC``: Anchors are added to headers.
-  ``HTML_USE_XHTML``: Renders XHTML instead of HTML.


Custom renderer
---------------

By subclassing the ``Hoep`` class, you can create a custom renderer overriding various callbacks.

.. code:: python

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

Callbacks
~~~~~~~~~

Document level
^^^^^^^^^^^^^^

+  ``preprocess(str markdown)``
+  ``doc_header()``
+  ``doc_footer()``
+  ``postprocess(str html)``

Block level
^^^^^^^^^^^

+  ``block_code(str text, str language)``
+  ``block_html(str text)``
+  ``block_quote(str text)``
+  ``footnotes(str text)``
+  ``footnote_def(str text, int number)``
+  ``header(str text, int level)``
+  ``hrule()``
+  ``list(str text, bool ordered)``
+  ``list_item(str text, bool ordered)``
+  ``paragraph(str text)``
+  ``table(str header, str body)``
+  ``table_row(str text)``
+  ``table_cell(str text, int flags)``

Span level
^^^^^^^^^^

+  ``autolink(str link, bool is_email)``
+  ``codespan(str text)``
+  ``double_emphasis(str text)``
+  ``emphasis(str text)``
+  ``footnote_ref(int number)``
+  ``highlight(str text)``
+  ``image(str link, mixed title, str alt)``
+  ``line_break()``
+  ``link(str link, mixed title, str content)``
+  ``quote(str text)``
+  ``raw_html_tag(str tag)``
+  ``strikethrough(str text)``
+  ``superscript(str text)``
+  ``triple_emphasis(str text)``
+  ``underline(str text)``

Low-level
^^^^^^^^^

+  ``entity(str entity)``
+  ``normal_text(str text)``

Support
-------

If you run into any issues or have any questions, either open an `issue`_ or hop in `#mynt`_ on irc.freenode.net.


.. _#mynt: http://webchat.freenode.net/?channels=mynt
.. _Hoedown: https://github.com/hoedown/hoedown
.. _issue: https://github.com/Anomareh/Hoep/issues
.. _Markdown Extra style footnotes: http://michelf.ca/projects/php-markdown/extra/#footnotes
.. _Markdown Extra style tables: http://michelf.ca/projects/php-markdown/extra/#table
'''
from sys import version_info

from setuptools import Extension, setup


_version = '2' if version_info[0] == 2 else '3'
_sources = [
    'ext/py{0}k/hoep.c'.format(_version),
    'ext/py{0}k/wrapper.c'.format(_version),
    'ext/hoedown/autolink.c',
    'ext/hoedown/buffer.c',
    'ext/hoedown/document.c',
    'ext/hoedown/escape.c',
    'ext/hoedown/html.c',
    'ext/hoedown/html_blocks.c',
    'ext/hoedown/html_smartypants.c',
    'ext/hoedown/stack.c',
    'ext/hoedown/version.c'
]


setup(
    name = 'hoep',
    version = '1.0.2',
    author = 'Andrew Fricke',
    author_email = 'andrew@uhnomoli.com',
    url = 'https://github.com/Anomareh/Hoep',
    description = 'A Python binding for the Hoedown Markdown library.',
    long_description = __doc__,
    license = 'BSD',
    platforms = 'any',
    zip_safe = False,
    ext_modules = [
        Extension(
            'hoep',
            sources = _sources
        )
    ],
    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'Environment :: Web Environment',
        'Intended Audience :: Developers',
        'Intended Audience :: End Users/Desktop',
        'License :: OSI Approved :: BSD License',
        'Operating System :: OS Independent',
        'Programming Language :: C',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Topic :: Internet',
        'Topic :: Internet :: WWW/HTTP',
        'Topic :: Text Processing',
        'Topic :: Utilities'
    ]
)
