#include <Python.h>
#include <structmember.h>

#include "../hoedown/html.h"

#include "wrapper.h"


typedef enum {
    HOEP_HTML_SMARTYPANTS = (1 << 10)
} hoep_html_render_mode;

typedef struct {
    PyObject_HEAD
    unsigned int extensions;
    unsigned int render_flags;
    hoedown_renderer *renderer;
    hoedown_document *document;
    hoedown_buffer *buffer;
} Hoep;


static PyObject *render(PyObject *self, PyObject *args, PyObject *kwargs) {
    static char *kwparams[] = {"markdown", "extensions", "render_flags", NULL};
    
    hoedown_renderer *renderer;
    hoedown_document *document;
    hoedown_buffer *buffer;
    unsigned int extensions = 0, render_flags = 0;
    const uint8_t *md, *sp;
    Py_ssize_t md_size, sp_size;
    PyObject *unicode, *markdown, *smartypants, *html;
    
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "U|II", kwparams, &unicode, &extensions, &render_flags)) {
        return NULL;
    }
    
    renderer = hoedown_html_renderer_new(render_flags, 0);
    document = hoedown_document_new(renderer, extensions, 16);
    buffer = hoedown_buffer_new(16);
    
    /* markdown */
    markdown = PyUnicode_AsUTF8String(unicode);
    
    md = (const uint8_t *) PyString_AsString(markdown);
    md_size = PyString_Size(markdown);
    
    hoedown_document_render(document, buffer, md, md_size);
    
    Py_DECREF(markdown);
    
    /* smartypants */
    if (render_flags & HOEP_HTML_SMARTYPANTS) {
        smartypants = PyString_FromStringAndSize((const char *) buffer->data, buffer->size);
        
        sp = (const uint8_t *) PyString_AsString(smartypants);
        sp_size = PyString_Size(smartypants);
        
        hoedown_buffer_reset(buffer);
        hoedown_html_smartypants(buffer, sp, sp_size);
        
        Py_DECREF(smartypants);
    }
    
    html = PyUnicode_DecodeUTF8((const char *) buffer->data, buffer->size, "strict");
    
    hoedown_buffer_free(buffer);
    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);
    
    return html;
}


static int Hoep_init(Hoep *self, PyObject *args, PyObject *kwargs) {
    static char *kwparams[] = {"extensions", "render_flags", NULL};
    
    hoedown_html_renderer_state *state;
    void **hd_callbacks, **hp_callbacks;
    int i;
    
    
    self->extensions = 0;
    self->render_flags = 0;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|II", kwparams, &self->extensions, &self->render_flags)) {
        return -1;
    }
    
    self->renderer = hoedown_html_renderer_new(self->render_flags, 0);
    
    state = self->renderer->opaque;
    state->opaque = self;
    
    hd_callbacks = (void **) self->renderer;
    hp_callbacks = (void **) &hoep_callbacks;
    
    for (i = 0; method_names[i] != NULL; i++) {
        if (PyObject_HasAttrString((PyObject *) self, (const char *) method_names[i])) {
            hd_callbacks[i + 1] = hp_callbacks[i + 1];
        }
    }
    
    self->document = hoedown_document_new(self->renderer, self->extensions, 16);
    self->buffer = hoedown_buffer_new(16);
    
    return 0;
}

static PyObject *Hoep_render(Hoep *self, PyObject *args) {
    const uint8_t *md, *sp;
    Py_ssize_t md_size, sp_size;
    PyObject *unicode, *pre, *markdown, *smartypants, *html, *post, *message;
    
    
    if (!PyArg_ParseTuple(args, "U", &unicode)) {
        return NULL;
    }
    
    /* preprocess */
    if (PyObject_HasAttrString((PyObject *) self, "preprocess")) {
        pre = PyObject_CallMethod((PyObject *) self, "preprocess", "(O)", unicode);
        if (pre == NULL) return NULL;
        
        markdown = PyUnicode_AsUTF8String(pre);
        if (markdown == NULL) goto exc_pre;
        
        Py_DECREF(pre);
    } else {
        markdown = PyUnicode_AsUTF8String(unicode);
    }
    
    /* markdown */
    md = (const uint8_t *) PyString_AsString(markdown);
    md_size = PyString_Size(markdown);
    
    hoedown_document_render(self->document, self->buffer, md, md_size);
    
    Py_DECREF(markdown);
    
    /* smartypants */
    if (self->render_flags & HOEP_HTML_SMARTYPANTS) {
        smartypants = PyString_FromStringAndSize((const char *) self->buffer->data, self->buffer->size);
        
        sp = (const uint8_t *) PyString_AsString(smartypants);
        sp_size = PyString_Size(smartypants);
        
        hoedown_buffer_reset(self->buffer);
        hoedown_html_smartypants(self->buffer, sp, sp_size);
        
        Py_DECREF(smartypants);
    }
    
    /* postprocess */
    if (PyObject_HasAttrString((PyObject *) self, "postprocess")) {
        post = PyUnicode_DecodeUTF8((const char *) self->buffer->data, self->buffer->size, "strict");
        
        html = PyObject_CallMethod((PyObject *) self, "postprocess", "(O)", post);
        if (html == NULL) goto exc_post;
        if (!PyUnicode_Check(html)) goto exc_post_u;
        
        Py_DECREF(post);
    } else {
        html = PyUnicode_DecodeUTF8((const char *) self->buffer->data, self->buffer->size, "strict");
    }
    
    hoedown_buffer_reset(self->buffer);
    
    return html;
    
    exc_post_u:;
        message = PyString_FromFormat("must return unicode, %s received", html->ob_type->tp_name);
        
        PyErr_SetString(PyExc_TypeError, PyString_AsString(message));
        
        Py_DECREF(message);
        Py_DECREF(html);
    exc_post:
        Py_DECREF(post);
        
        hoedown_buffer_reset(self->buffer);
        
        return NULL;
    exc_pre:
        Py_DECREF(pre);
        
        return NULL;
}

static void Hoep_dealloc(Hoep *self) {
    hoedown_buffer_free(self->buffer);
    if (self->document != NULL) hoedown_document_free(self->document);
    if (self->renderer != NULL) hoedown_html_renderer_free(self->renderer);
    
    self->ob_type->tp_free((PyObject *) self);
}


static PyMethodDef module_functions[] = {
    {"render", (PyCFunction) render, METH_VARARGS | METH_KEYWORDS, "Render Markdown."},
    {NULL}
};

static PyMemberDef Hoep_members[] = {
    {"extensions", T_UINT, offsetof(Hoep, extensions), READONLY, "Markdown extension bit flags."},
    {"render_flags", T_UINT, offsetof(Hoep, render_flags), READONLY, "HTML renderer bit flags."},
    {NULL}
};

static PyMethodDef Hoep_methods[] = {
    {"render", (PyCFunction) Hoep_render, METH_VARARGS, "Render Markdown."},
    {NULL}
};

static PyTypeObject HoepType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /* ob_size */
    "hoep.Hoep",                                /* tp_name */
    sizeof(Hoep),                               /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor) Hoep_dealloc,                  /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags*/
    "Hoedown's HTML renderer.",                 /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    Hoep_methods,                               /* tp_methods */
    Hoep_members,                               /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc) Hoep_init,                       /* tp_init */
    0,                                          /* tp_alloc */
    0                                           /* tp_new */
};


#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC inithoep(void) {
    PyObject *mod;
    
    HoepType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&HoepType) < 0) return;
    
    mod = Py_InitModule3("hoep", module_functions,
        "Hoep is a Python binding for the Hoedown Markdown library.");
    
    Py_INCREF(&HoepType);
    
    PyModule_AddIntConstant(mod, "EXT_AUTOLINK", HOEDOWN_EXT_AUTOLINK);
    PyModule_AddIntConstant(mod, "EXT_DISABLE_INDENTED_CODE", HOEDOWN_EXT_DISABLE_INDENTED_CODE);
    PyModule_AddIntConstant(mod, "EXT_FENCED_CODE", HOEDOWN_EXT_FENCED_CODE);
    PyModule_AddIntConstant(mod, "EXT_FOOTNOTES", HOEDOWN_EXT_FOOTNOTES);
    PyModule_AddIntConstant(mod, "EXT_HIGHLIGHT", HOEDOWN_EXT_HIGHLIGHT);
    PyModule_AddIntConstant(mod, "EXT_LAX_SPACING", HOEDOWN_EXT_LAX_SPACING);
    PyModule_AddIntConstant(mod, "EXT_NO_INTRA_EMPHASIS", HOEDOWN_EXT_NO_INTRA_EMPHASIS);
    PyModule_AddIntConstant(mod, "EXT_QUOTE", HOEDOWN_EXT_QUOTE);
    PyModule_AddIntConstant(mod, "EXT_SPACE_HEADERS", HOEDOWN_EXT_SPACE_HEADERS);
    PyModule_AddIntConstant(mod, "EXT_STRIKETHROUGH", HOEDOWN_EXT_STRIKETHROUGH);
    PyModule_AddIntConstant(mod, "EXT_SUPERSCRIPT", HOEDOWN_EXT_SUPERSCRIPT);
    PyModule_AddIntConstant(mod, "EXT_TABLES", HOEDOWN_EXT_TABLES);
    PyModule_AddIntConstant(mod, "EXT_UNDERLINE", HOEDOWN_EXT_UNDERLINE);
    
    PyModule_AddIntConstant(mod, "HTML_ESCAPE", HOEDOWN_HTML_ESCAPE);
    PyModule_AddIntConstant(mod, "HTML_EXPAND_TABS", HOEDOWN_HTML_EXPAND_TABS);
    PyModule_AddIntConstant(mod, "HTML_HARD_WRAP", HOEDOWN_HTML_HARD_WRAP);
    PyModule_AddIntConstant(mod, "HTML_SAFELINK", HOEDOWN_HTML_SAFELINK);
    PyModule_AddIntConstant(mod, "HTML_SKIP_HTML", HOEDOWN_HTML_SKIP_HTML);
    PyModule_AddIntConstant(mod, "HTML_SKIP_IMAGES", HOEDOWN_HTML_SKIP_IMAGES);
    PyModule_AddIntConstant(mod, "HTML_SKIP_LINKS", HOEDOWN_HTML_SKIP_LINKS);
    PyModule_AddIntConstant(mod, "HTML_SKIP_STYLE", HOEDOWN_HTML_SKIP_STYLE);
    PyModule_AddIntConstant(mod, "HTML_SMARTYPANTS", HOEP_HTML_SMARTYPANTS);
    PyModule_AddIntConstant(mod, "HTML_TOC", HOEDOWN_HTML_TOC);
    PyModule_AddIntConstant(mod, "HTML_USE_XHTML", HOEDOWN_HTML_USE_XHTML);
    
    PyModule_AddIntConstant(mod, "TABLE_ALIGN_CENTER", HOEDOWN_TABLE_ALIGN_CENTER);
    PyModule_AddIntConstant(mod, "TABLE_ALIGN_LEFT", HOEDOWN_TABLE_ALIGN_LEFT);
    PyModule_AddIntConstant(mod, "TABLE_ALIGN_RIGHT", HOEDOWN_TABLE_ALIGN_RIGHT);
    PyModule_AddIntConstant(mod, "TABLE_ALIGNMASK", HOEDOWN_TABLE_ALIGNMASK);
    PyModule_AddIntConstant(mod, "TABLE_HEADER", HOEDOWN_TABLE_HEADER);
    
    PyModule_AddObject(mod, "Hoep", (PyObject *) &HoepType);
}
