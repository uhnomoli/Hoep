#include <Python.h>

#include <stdarg.h>

#include "../hoedown/html.h"


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
        PyObject *message = PyUnicode_FromFormat("must return str, %s received", Py_TYPE(ret)->tp_name);
        
        PyErr_SetString(PyExc_TypeError, PyUnicode_AsUTF8(message));
        
        Py_DECREF(message);
        
        goto exc_ret;
    }
    
    hoedown_buffer_puts(buffer, PyUnicode_AsUTF8(ret));
    
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
    process(opaque, buffer, "block_code", "(s#s#)",
        text->data, text->size,
        language->data, language->size);
}

static void hoep_block_html(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    process(opaque, buffer, "block_html", "(s#)",
        text->data, text->size);
}

static void hoep_block_quote(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    process(opaque, buffer, "block_quote", "(s#)",
        text->data, text->size);
}

static void hoep_footnotes(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    process(opaque, buffer, "footnotes", "(s#)",
        text->data, text->size);
}

static void hoep_footnote_def(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int number, void *opaque) {
    process(opaque, buffer, "footnote_def", "(s#i)",
        text->data, text->size,
        number);
}

static void hoep_header(hoedown_buffer *buffer, const hoedown_buffer *text, int level, void *opaque) {
    process(opaque, buffer, "header", "(s#i)",
        text->data, text->size,
        level);
}

static void hoep_hrule(hoedown_buffer *buffer, void *opaque) {
    process(opaque, buffer, "hrule", "()");
}

static void hoep_list(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int flags, void *opaque) {
    PyObject *ordered = flags & HOEDOWN_LIST_ORDERED ? Py_True : Py_False;
    
    process(opaque, buffer, "list", "(s#O)",
        text->data, text->size,
        ordered);
    
    Py_DECREF(ordered);
}

static void hoep_list_item(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int flags, void *opaque) {
    PyObject *ordered = flags & HOEDOWN_LIST_ORDERED ? Py_True : Py_False;
    
    process(opaque, buffer, "list_item", "(s#O)",
        text->data, text->size,
        ordered);
    
    Py_DECREF(ordered);
}

static void hoep_paragraph(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    process(opaque, buffer, "paragraph", "(s#)",
        text->data, text->size);
}

static void hoep_table(hoedown_buffer *buffer, const hoedown_buffer *header, const hoedown_buffer *body, void *opaque) {
    process(opaque, buffer, "table", "(s#s#)",
        header->data, header->size,
        body->data, body->size);
}

static void hoep_table_row(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    process(opaque, buffer, "table_row", "(s#)",
        text->data, text->size);
}

static void hoep_table_cell(hoedown_buffer *buffer, const hoedown_buffer *text, unsigned int flags, void *opaque) {
    process(opaque, buffer, "table_cell", "(s#i)",
        text->data, text->size,
        flags);
}


/* Span level callbacks. */
static int hoep_autolink(hoedown_buffer *buffer, const hoedown_buffer *link, enum hoedown_autolink type, void *opaque) {
    PyObject *is_email = type == HOEDOWN_AUTOLINK_EMAIL ? Py_True : Py_False;
    
    int ret = process(opaque, buffer, "autolink", "(s#O)",
        link->data, link->size,
        is_email);
    
    Py_DECREF(is_email);
    
    return ret;
}

static int hoep_codespan(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "codespan", "(s#)",
        text->data, text->size);
}

static int hoep_double_emphasis(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "double_emphasis", "(s#)",
        text->data, text->size);
}

static int hoep_emphasis(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "emphasis", "(s#)",
        text->data, text->size);
}

static int hoep_footnote_ref(hoedown_buffer *buffer, unsigned int number, void *opaque) {
    return process(opaque, buffer, "footnote_ref", "(i)", number);
}

static int hoep_highlight(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "highlight", "(s#)",
        text->data, text->size);
}

static int hoep_image(hoedown_buffer *buffer, const hoedown_buffer *link, const hoedown_buffer *title, const hoedown_buffer *alt, void *opaque) {
    if (!title) {
        return process(opaque, buffer, "image", "(s#ss#)",
            link->data, link->size,
            NULL,
            alt->data, alt->size);
    }
    
    return process(opaque, buffer, "image", "(s#s#s#)",
        link->data, link->size,
        title->data, title->size,
        alt->data, alt->size);
}

static int hoep_line_break(hoedown_buffer *buffer, void *opaque) {
    return process(opaque, buffer, "line_break", "()");
}

static int hoep_link(hoedown_buffer *buffer, const hoedown_buffer *link, const hoedown_buffer *title, const hoedown_buffer *content, void *opaque) {
    if (!title) {
        return process(opaque, buffer, "link", "(s#ss#)",
            link->data, link->size,
            NULL,
            content->data, content->size);
    }
    
    return process(opaque, buffer, "link", "(s#s#s#)",
        link->data, link->size,
        title->data, title->size,
        content->data, content->size);
}

static int hoep_quote(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "quote", "(s#)",
        text->data, text->size);
}

static int hoep_raw_html_tag(hoedown_buffer *buffer, const hoedown_buffer *tag, void *opaque) {
    return process(opaque, buffer, "raw_html_tag", "(s#)",
        tag->data, tag->size);
}

static int hoep_strikethrough(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "strikethrough", "(s#)",
        text->data, text->size);
}

static int hoep_superscript(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "superscript", "(s#)",
        text->data, text->size);
}

static int hoep_triple_emphasis(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "triple_emphasis", "(s#)",
        text->data, text->size);
}

static int hoep_underline(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    return process(opaque, buffer, "underline", "(s#)",
        text->data, text->size);
}


/* Low-level callbacks. */
static void hoep_entity(hoedown_buffer *buffer, const hoedown_buffer *entity, void *opaque) {
    process(opaque, buffer, "entity", "(s#)",
        entity->data, entity->size);
}

static void hoep_normal_text(hoedown_buffer *buffer, const hoedown_buffer *text, void *opaque) {
    process(opaque, buffer, "normal_text", "(s#)",
        text->data, text->size);
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
