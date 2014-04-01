#include <Python.h>

#include <stdarg.h>

#include "../hoedown/html.h"


static PyObject *to_unicode(const hoedown_buffer *buffer) {
    if (!buffer) {
        Py_RETURN_NONE;
    }
    
    return PyUnicode_DecodeUTF8((const char *) buffer->data, buffer->size, "strict");
}


static PyObject *call_method(void *opaque, const char *method_name, const char *format, va_list va_args) {
    PyObject *self = ((hoedown_html_renderer_state *) opaque)->opaque;
    
    PyObject *method = PyObject_GetAttrString(self, method_name);
    if (method == NULL) return NULL;
    
    PyObject *args = Py_VaBuildValue(format, va_args);
    if (args == NULL) goto exc;
    
    return PyObject_CallObject(method, args);
    
    exc:
        Py_DECREF(method);
        
        return NULL;
}

static int process(void *opaque, hoedown_buffer *buffer, const char *method_name, const char *format, ...) {
    va_list va_args;
    
    va_start(va_args, format);
    
    PyObject *ret = call_method(opaque, method_name, format, va_args);
    
    va_end(va_args);
    
    if (ret == NULL) goto exc;
    
    if (!PyUnicode_Check(ret)) {
        PyObject *message = PyString_FromFormat("must return unicode, %s received", ret->ob_type->tp_name);
        
        PyErr_SetString(PyExc_TypeError, PyString_AsString(message));
        
        Py_DECREF(message);
        
        goto exc_ret;
    }
    
    PyObject *string = PyUnicode_AsUTF8String(ret);
    if (string == NULL) goto exc_ret;
    
    hoedown_buffer_puts(buffer, PyString_AsString(string));
    
    Py_DECREF(string);
    Py_DECREF(ret);
    
    return 1;
    
    exc_ret:
        Py_DECREF(ret);
    exc:;
        PyObject *exc = PyErr_Occurred();
        
        if (exc != NULL) {
            PyErr_Print();
        }
        
        return 0;
}

/* Block level callbacks. */
static void hoep_block_code(hoedown_buffer *buffer, const hoedown_buffer *text, const hoedown_buffer *language, void *opaque) {
    PyObject *utext = to_unicode(text);
    PyObject *ulanguage = to_unicode(language);
    
    process(opaque, buffer, "block_code", "(NN)", utext, ulanguage);
    
    Py_DECREF(ulanguage);
    Py_DECREF(utext);
}

static void hoep_block_html(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "block_html", "(N)", utext);
    
    Py_DECREF(utext);
}

static void hoep_block_quote(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "block_quote", "(N)", utext);
    
    Py_DECREF(utext);
}

static void hoep_footnotes(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "footnotes", "(N)", utext);
    
    Py_DECREF(utext);
}

static void hoep_footnote_def(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int number, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "footnote_def", "(Ni)", utext, number);
    
    Py_DECREF(utext);
}

static void hoep_header(hoedown_buffer *buffer, const hoedown_buffer *text, int level, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "header", "(Ni)", utext, level);
    
    Py_DECREF(utext);
}

static void hoep_hrule(hoedown_buffer *buffer, void *opaque) {
    process(opaque, buffer, "hrule", "()");
}

static void hoep_list(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int flags, void *opaque) {
    PyObject *utext = to_unicode(text);
    PyObject *ordered = flags & HOEDOWN_LIST_ORDERED ? Py_True : Py_False;
    
    process(opaque, buffer, "list", "(NO)", utext, ordered);
    
    Py_DECREF(ordered);
    Py_DECREF(utext);
}

static void hoep_list_item(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int flags, void *opaque) {
    PyObject *utext = to_unicode(text);
    PyObject *ordered = flags & HOEDOWN_LIST_ORDERED ? Py_True : Py_False;
    
    process(opaque, buffer, "list_item", "(NO)", utext, ordered);
    
    Py_DECREF(ordered);
    Py_DECREF(utext);
}

static void hoep_paragraph(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "paragraph", "(N)", utext);
    
    Py_DECREF(utext);
}

static void hoep_table(hoedown_buffer *buffer, const hoedown_buffer *header, const hoedown_buffer *body, void *opaque) {
    PyObject *uheader = to_unicode(header);
    PyObject *ubody = to_unicode(body);
    
    process(opaque, buffer, "table", "(NN)", uheader, ubody);
    
    Py_DECREF(ubody);
    Py_DECREF(uheader);
}

static void hoep_table_row(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "table_row", "(N)", utext);
    
    Py_DECREF(utext);
}

static void hoep_table_cell(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int flags, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "table_cell", "(Ni)", utext, flags);
    
    Py_DECREF(utext);
}


/* Span level callbacks. */
static int hoep_autolink(hoedown_buffer *buffer, const hoedown_buffer *link, enum hoedown_autolink type, void *opaque) {
    PyObject *ulink = to_unicode(link);
    PyObject *is_email = type == HOEDOWN_AUTOLINK_EMAIL ? Py_True : Py_False;
    
    int ret = process(opaque, buffer, "autolink", "(NO)", ulink, is_email);
    
    Py_DECREF(is_email);
    Py_DECREF(ulink);
    
    return ret;
}

static int hoep_codespan(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "codespan", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_double_emphasis(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "double_emphasis", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_emphasis(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "emphasis", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_footnote_ref(hoedown_buffer *buffer, unsigned int number, void *opaque) {
    return process(opaque, buffer, "footnote_ref", "(i)", number);
}

static int hoep_highlight(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "highlight", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_image(hoedown_buffer *buffer, const hoedown_buffer *link, const hoedown_buffer *title, const hoedown_buffer *alt, void *opaque) {
    PyObject *ulink = to_unicode(link);
    PyObject *utitle = to_unicode(title);
    PyObject *ualt = to_unicode(alt);
    
    int ret = process(opaque, buffer, "image", "(NNN)", ulink, utitle, ualt);
    
    Py_DECREF(ualt);
    Py_DECREF(utitle);
    Py_DECREF(ulink);
    
    return ret;
}

static int hoep_line_break(hoedown_buffer *buffer, void *opaque) {
    return process(opaque, buffer, "line_break", "()");
}

static int hoep_link(hoedown_buffer *buffer, const hoedown_buffer *link, const hoedown_buffer *title, const hoedown_buffer *content, void *opaque) {
    PyObject *ulink = to_unicode(link);
    PyObject *utitle = to_unicode(title);
    PyObject *ucontent = to_unicode(content);
    
    int ret = process(opaque, buffer, "link", "(NNN)", ulink, utitle, ucontent);
    
    Py_DECREF(ucontent);
    Py_DECREF(utitle);
    Py_DECREF(ulink);
    
    return ret;
}

static int hoep_quote(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "quote", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_raw_html_tag(hoedown_buffer *buffer, const hoedown_buffer *tag, void *opaque) {
    PyObject *utag = to_unicode(tag);
    
    int ret = process(opaque, buffer, "raw_html_tag", "(N)", utag);
    
    Py_DECREF(utag);
    
    return ret;
}

static int hoep_strikethrough(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "strikethrough", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_superscript(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "superscript", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_triple_emphasis(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "triple_emphasis", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}

static int hoep_underline(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    int ret = process(opaque, buffer, "underline", "(N)", utext);
    
    Py_DECREF(utext);
    
    return ret;
}


/* Low-level callbacks. */
static void hoep_entity(hoedown_buffer *buffer, const hoedown_buffer *entity, void *opaque) {
    PyObject *uentity = to_unicode(entity);
    
    process(opaque, buffer, "entity", "(N)", uentity);
    
    Py_DECREF(uentity);
}

static void hoep_normal_text(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    PyObject *utext = to_unicode(text);
    
    process(opaque, buffer, "normal_text", "(N)", utext);
    
    Py_DECREF(utext);
}


/* Header and footer callbacks. */
static void hoep_doc_header(hoedown_buffer *buffer, void *opaque) {
    process(opaque, buffer, "doc_header", "()");
}

static void hoep_doc_footer(hoedown_buffer *buffer, void *opaque) {
    process(opaque, buffer, "doc_footer", "()");
}



struct hoedown_renderer hoep_callbacks = {
    NULL,
    
    hoep_block_code,
    hoep_block_quote,
    hoep_block_html,
    hoep_header,
    hoep_hrule,
    hoep_list,
    hoep_list_item,
    hoep_paragraph,
    hoep_table,
    hoep_table_row,
    hoep_table_cell,
    hoep_footnotes,
    hoep_footnote_def,
    
    hoep_autolink,
    hoep_codespan,
    hoep_double_emphasis,
    hoep_emphasis,
    hoep_underline,
    hoep_highlight,
    hoep_quote,
    hoep_image,
    hoep_line_break,
    hoep_link,
    hoep_raw_html_tag,
    hoep_triple_emphasis,
    hoep_strikethrough,
    hoep_superscript,
    hoep_footnote_ref,
    
    hoep_entity,
    hoep_normal_text,
    
    hoep_doc_header,
    hoep_doc_footer
};

const char *method_names[] = {
    "block_code",
    "block_quote",
    "block_html",
    "header",
    "hrule",
    "list",
    "list_item",
    "paragraph",
    "table",
    "table_row",
    "table_cell",
    "footnotes",
    "footnote_def",
    
    "autolink",
    "codespan",
    "double_emphasis",
    "emphasis",
    "underline",
    "highlight",
    "quote",
    "image",
    "line_break",
    "link",
    "raw_html_tag",
    "triple_emphasis",
    "strikethrough",
    "superscript",
    "footnote_ref",
    
    "entity",
    "normal_text",
    
    "doc_header",
    "doc_footer",
    
    NULL
};
