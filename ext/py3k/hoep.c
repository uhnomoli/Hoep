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
    const char *md;
    const uint8_t *sp;
    Py_ssize_t md_size, sp_size;
    PyObject *smartypants, *html;
    
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|II", kwparams, &md, &extensions, &render_flags)) {
        return NULL;
    }
    
    renderer = hoedown_html_renderer_new(render_flags, 0);
    document = hoedown_document_new(renderer, extensions, 16);
    buffer = hoedown_buffer_new(16);
    
    /* markdown */
    md_size = (Py_ssize_t) strlen(md);
    
    hoedown_document_render(document, buffer, (const uint8_t *) md, md_size);
    
    /* smartypants */
    if (render_flags & HOEP_HTML_SMARTYPANTS) {
        smartypants = PyUnicode_DecodeUTF8((const char *) buffer->data, buffer->size, "strict");
        
        sp = (const uint8_t *) PyUnicode_AsUTF8AndSize(smartypants, &sp_size);
        
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
    const char *str;
    const uint8_t *md, *sp;
    Py_ssize_t str_size, md_size, sp_size;
    PyObject *markdown, *smartypants, *html, *message;
    
    
    if (!PyArg_ParseTuple(args, "s", &str)) {
        return NULL;
    }
    
    str_size = (Py_ssize_t) strlen(str);
    
    /* preprocess */
    if (PyObject_HasAttrString((PyObject *) self, "preprocess")) {
        markdown = PyObject_CallMethod((PyObject *) self, "preprocess", "(s)", str);
        if (markdown == NULL) return NULL;
        if (!PyUnicode_Check(markdown)) goto exc_pre;
        
        md = (const uint8_t *) PyUnicode_AsUTF8AndSize(markdown, &md_size);
        
        hoedown_document_render(self->document, self->buffer, (const uint8_t *) md, md_size);
        
        Py_DECREF(markdown);
    } else {
        hoedown_document_render(self->document, self->buffer, (const uint8_t *) str, str_size);
    }
    
    /* smartypants */
    if (self->render_flags & HOEP_HTML_SMARTYPANTS) {
        smartypants = PyUnicode_DecodeUTF8((const char *) self->buffer->data, self->buffer->size, "strict");
        
        sp = (const uint8_t *) PyUnicode_AsUTF8AndSize(smartypants, &sp_size);
        
        hoedown_buffer_reset(self->buffer);
        hoedown_html_smartypants(self->buffer, sp, sp_size);
        
        Py_DECREF(smartypants);
    }
    
    /* postprocess */
    if (PyObject_HasAttrString((PyObject *) self, "postprocess")) {
        html = PyObject_CallMethod((PyObject *) self, "postprocess", "(s)", self->buffer->data);
        if (html == NULL) return NULL;
        if (!PyUnicode_Check(html)) goto exc_post;
    } else {
        html = PyUnicode_DecodeUTF8((const char *) self->buffer->data, self->buffer->size, "strict");
    }
    
    hoedown_buffer_reset(self->buffer);
    
    return html;
    
    exc_post:;
        message = PyUnicode_FromFormat("must return str, %s received", Py_TYPE(html)->tp_name);
        
        PyErr_SetString(PyExc_TypeError, PyUnicode_AsUTF8(message));
        
        hoedown_buffer_reset(self->buffer);
        
        Py_DECREF(message);
        Py_DECREF(html);
        
        return NULL;
    exc_pre:
        Py_DECREF(markdown);
        
        return NULL;
}

static void Hoep_dealloc(Hoep *self) {
    hoedown_buffer_free(self->buffer);
    if (self->document != NULL) hoedown_document_free(self->document);
    if (self->renderer != NULL) hoedown_html_renderer_free(self->renderer);
    
    Py_TYPE(self)->tp_free((PyObject *) self);
}


static PyMethodDef module_functions[] = {
    {"render", (PyCFunction) render, METH_VARARGS | METH_KEYWORDS, "Render Markdown."},
    {NULL, NULL, 0, NULL}
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
    PyVarObject_HEAD_INIT(NULL, 0)
    "hoep.Hoep",                                /* tp_name */
    sizeof(Hoep),                               /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor) Hoep_dealloc,                  /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash  */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
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

static PyModuleDef hoep_module = {
    PyModuleDef_HEAD_INIT,
    "hoep",
    "Hoep is a Python binding for the Hoedown Markdown library.",
    -1,
    module_functions,
    NULL,
    NULL,
    NULL,
    NULL
};


PyMODINIT_FUNC PyInit_hoep(void) {
    PyObject *mod;
    
    HoepType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&HoepType) < 0) return NULL;
    
    mod = PyModule_Create(&hoep_module);
    if (mod == NULL) return NULL;
    
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
    
    return mod;
}
