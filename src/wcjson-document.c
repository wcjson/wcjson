/*
 * Copyright (c) 2025 Christian Schulte <cs@schulte.it>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file wcjson-document.c
 * @brief Document implementation.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "wcjson-document.h"
#include "wcjson.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef DEFAULT_NESTING_LIMIT
#define DEFAULT_NESTING_LIMIT 256
#endif

#define VALUE_IS_VALID(v)                                                      \
  ((v)->is_null || (v)->is_boolean || (v)->is_array || (v)->is_object ||       \
   (((v)->is_string || (v)->is_number || (v)->is_pair) &&                      \
    (v)->string != NULL))

#define VALUE_IS_CHILD(v) ((v)->prev_idx != 0 || (v)->next_idx != 0)

static void *doc_object_start(struct wcjson *, void *, void *);
static void doc_object_add(struct wcjson *, void *, void *, void *, void *);
static void doc_object_end(struct wcjson *, void *, void *);
static void *doc_array_start(struct wcjson *, void *, void *);
static void doc_array_add(struct wcjson *, void *, void *, void *);
static void doc_array_end(struct wcjson *, void *, void *);
static void *doc_string_value(struct wcjson *, void *, const wchar_t *,
                              const size_t, const bool);
static void *doc_number_value(struct wcjson *, void *, const wchar_t *,
                              const size_t);
static void *doc_bool_value(struct wcjson *, void *, const bool);
static void *doc_null_value(struct wcjson *, void *);

static struct wcjson_ops doc_ops = {
    .object_start = doc_object_start,
    .object_add = doc_object_add,
    .object_end = doc_object_end,
    .array_start = doc_array_start,
    .array_add = doc_array_add,
    .array_end = doc_array_end,
    .string_value = doc_string_value,
    .number_value = doc_number_value,
    .bool_value = doc_bool_value,
    .null_value = doc_null_value,
};

struct doc_impl {
  struct wcjson_document *d;
  size_t nlimit;
};

static struct wcjson_value *wcjson_document_nextv(struct wcjson_document *doc) {
  if (doc->v_idx == doc->v_nitems)
    goto err_range;

  struct wcjson_value *v = &doc->values[doc->v_idx];
  v->is_null = 0;
  v->is_boolean = 0;
  v->is_true = 0;
  v->is_string = 0;
  v->is_number = 0;
  v->is_object = 0;
  v->is_array = 0;
  v->is_pair = 0;
  v->string = NULL;
  v->s_len = 0;
  v->idx = doc->v_idx++;
  v->head_idx = 0;
  v->tail_idx = 0;
  v->prev_idx = 0;
  v->next_idx = 0;
  return v;
err_range:
  errno = ERANGE;
  return NULL;
}

static inline struct wcjson_value *doc_next_value(struct wcjson *ctx,
                                                  struct doc_impl *doc) {
  struct wcjson_value *v = wcjson_document_nextv(doc->d);

  if (v == NULL) {
    ctx->status = WCJSON_ABORT_ERROR;
    ctx->errnum = errno;
  }

  return v;
}

struct wcjson_value *wcjsondoc_create_null(struct wcjson_document *doc) {
  struct wcjson_value *v = wcjson_document_nextv(doc);

  if (v != NULL)
    v->is_null = 1;

  return v;
}

struct wcjson_value *wcjsondoc_create_bool(struct wcjson_document *doc,
                                           bool val) {
  struct wcjson_value *v = wcjson_document_nextv(doc);

  if (v != NULL) {
    v->is_boolean = 1;
    v->is_true = val ? 1 : 0;
  }

  return v;
}

struct wcjson_value *wcjsondoc_create_string(struct wcjson_document *doc,
                                             const wchar_t *val) {
  struct wcjson_value *v = wcjson_document_nextv(doc);

  if (v != NULL) {
    v->is_string = 1;
    v->string = val;
    v->s_len = wcslen(val);
  }

  return v;
}

struct wcjson_value *wcjsondoc_create_number(struct wcjson_document *doc,
                                             const wchar_t *val) {
  struct wcjson_value *v = wcjson_document_nextv(doc);

  if (v != NULL) {
    v->is_number = 1;
    v->string = val;
    v->s_len = wcslen(val);
  }

  return v;
}

struct wcjson_value *wcjsondoc_create_object(struct wcjson_document *doc) {
  struct wcjson_value *v = wcjson_document_nextv(doc);

  if (v != NULL)
    v->is_object = 1;

  return v;
}

static struct wcjson_value *
wcjsondoc_create_pair(struct wcjson_document *doc, const wchar_t *key,
                      const struct wcjson_value *val) {
  struct wcjson_value *v = wcjson_document_nextv(doc);

  if (v != NULL) {
    v->is_pair = 1;
    v->string = key;
    v->s_len = wcslen(key);
    v->head_idx = val->idx;
    v->tail_idx = val->idx;
  }

  return v;
}

struct wcjson_value *wcjsondoc_create_array(struct wcjson_document *doc) {
  struct wcjson_value *v = wcjson_document_nextv(doc);

  if (v != NULL)
    v->is_array = 1;

  return v;
}

int wcjsondoc_array_add_head(const struct wcjson_document *doc,
                             struct wcjson_value *arr,
                             struct wcjson_value *val) {
  if (!(arr->is_array && VALUE_IS_VALID(val)) || VALUE_IS_CHILD(val))
    goto err_inval;

  if (arr->head_idx == 0) {
    arr->head_idx = val->idx;
    arr->tail_idx = val->idx;
  } else {
    doc->values[arr->head_idx].prev_idx = val->idx;
    val->next_idx = arr->head_idx;
    arr->head_idx = val->idx;
  }

  return 0;
err_inval:
  errno = EINVAL;
  return -1;
}

int wcjsondoc_array_add_tail(const struct wcjson_document *doc,
                             struct wcjson_value *arr,
                             struct wcjson_value *val) {
  if (!(arr->is_array && VALUE_IS_VALID(val)) || VALUE_IS_CHILD(val))
    goto err_inval;

  if (arr->head_idx == 0) {
    arr->head_idx = val->idx;
    arr->tail_idx = val->idx;
  } else {
    doc->values[arr->tail_idx].next_idx = val->idx;
    val->prev_idx = arr->tail_idx;
    arr->tail_idx = val->idx;
  }

  return 0;
err_inval:
  errno = EINVAL;
  return -1;
}

struct wcjson_value *wcjsondoc_array_get(const struct wcjson_document *doc,
                                         const struct wcjson_value *arr,
                                         const size_t idx) {
  if (arr->is_array) {
    struct wcjson_value *v;
    size_t i = 0;

    wcjson_value_foreach(v, doc, arr) {
      if (i++ == idx)
        return v;
    }
  }

  return NULL;
}

struct wcjson_value *wcjsondoc_array_remove(const struct wcjson_document *doc,
                                            struct wcjson_value *arr,
                                            const size_t idx) {
  struct wcjson_value *val = wcjsondoc_array_get(doc, arr, idx);

  if (val != NULL) {
    if (val->next_idx != 0)
      doc->values[val->next_idx].prev_idx = val->prev_idx;

    if (val->prev_idx != 0)
      doc->values[val->prev_idx].next_idx = val->next_idx;
  }

  return val;
}

int wcjsondoc_object_add_head(struct wcjson_document *doc,
                              struct wcjson_value *obj, const wchar_t *key,
                              const struct wcjson_value *val) {
  if (!(obj->is_object && VALUE_IS_VALID(val)) || VALUE_IS_CHILD(val))
    goto err_inval;

  struct wcjson_value *pair = wcjsondoc_create_pair(doc, key, val);
  if (pair == NULL)
    goto err;

  if (obj->head_idx == 0) {
    obj->head_idx = pair->idx;
    obj->tail_idx = pair->idx;
  } else {
    doc->values[obj->head_idx].prev_idx = pair->idx;
    pair->next_idx = obj->head_idx;
    obj->head_idx = pair->idx;
  }

  return 0;
err:
  return -1;
err_inval:
  errno = EINVAL;
  return -1;
}

int wcjsondoc_object_add_tail(struct wcjson_document *doc,
                              struct wcjson_value *obj, const wchar_t *key,
                              const struct wcjson_value *val) {
  if (!(obj->is_object && VALUE_IS_VALID(val)) || VALUE_IS_CHILD(val))
    goto err_inval;

  struct wcjson_value *pair = wcjsondoc_create_pair(doc, key, val);
  if (pair == NULL)
    goto err;

  if (obj->head_idx == 0) {
    obj->head_idx = pair->idx;
    obj->tail_idx = pair->idx;
  } else {
    doc->values[obj->tail_idx].next_idx = pair->idx;
    pair->prev_idx = obj->tail_idx;
    obj->tail_idx = pair->idx;
  }

  return 0;
err:
  return -1;
err_inval:
  errno = EINVAL;
  return -1;
}

struct wcjson_value *wcjsondoc_object_remove(const struct wcjson_document *doc,
                                             struct wcjson_value *obj,
                                             const wchar_t *key) {
  struct wcjson_value *v;
  const size_t key_len = wcslen(key);

  wcjson_value_foreach(v, doc, obj) {
    if (v->s_len == key_len && wcsncmp(v->string, key, v->s_len) == 0) {
      if (v->next_idx != 0)
        doc->values[v->next_idx].prev_idx = v->prev_idx;

      if (v->prev_idx != 0)
        doc->values[v->prev_idx].next_idx = v->next_idx;

      return wcjson_value_head(doc, v);
    }
  }

  return NULL;
}

struct wcjson_value *wcjsondoc_object_get(const struct wcjson_document *doc,
                                          const struct wcjson_value *obj,
                                          const wchar_t *key) {
  const size_t key_len = wcslen(key);
  struct wcjson_value *v;

  if (obj->is_object)
    wcjson_value_foreach(v, doc, obj) {
      if (v->s_len == key_len && wcsncmp(v->string, key, v->s_len) == 0)
        return wcjson_value_head(doc, v);
    }

  return NULL;
}

static void *doc_object_start(struct wcjson *ctx, void *doc, void *parent) {
  struct doc_impl *d = doc;
  struct wcjson_value *v = doc_next_value(ctx, d);

  if (v != NULL) {
    v->is_object = 1;

    if (d->nlimit-- == 0)
      goto err_range;
  }
  return v;
err_range:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = ERANGE;
  return v;
}

static void doc_object_add(struct wcjson *ctx, void *doc, void *obj, void *key,
                           void *value) {
  struct doc_impl *d = doc;
  struct wcjson_value *o = obj;
  struct wcjson_value *pair = key;
  struct wcjson_value *v = value;

  pair->is_string = 0;
  pair->is_pair = 1;
  pair->head_idx = v->idx;
  pair->tail_idx = v->idx;

  if (o->head_idx == 0) {
    o->head_idx = pair->idx;
    o->tail_idx = pair->idx;
  } else {
    d->d->values[o->tail_idx].next_idx = pair->idx;
    pair->prev_idx = o->tail_idx;
    o->tail_idx = pair->idx;
  }
}

static void doc_object_end(struct wcjson *ctx, void *doc, void *obj) {
  struct doc_impl *d = doc;
  d->nlimit++;
}

static void *doc_array_start(struct wcjson *ctx, void *doc, void *parent) {
  struct doc_impl *d = doc;
  struct wcjson_value *v = doc_next_value(ctx, d);

  if (v != NULL) {
    v->is_array = 1;

    if (d->nlimit-- == 0)
      goto err_range;
  }

  return v;
err_range:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = ERANGE;
  return v;
}

static void doc_array_add(struct wcjson *ctx, void *doc, void *arr,
                          void *value) {
  struct doc_impl *d = doc;
  struct wcjson_value *a = arr;
  struct wcjson_value *v = value;

  if (a->head_idx == 0) {
    a->head_idx = v->idx;
    a->tail_idx = v->idx;
  } else {
    d->d->values[a->tail_idx].next_idx = v->idx;
    v->prev_idx = a->tail_idx;
    a->tail_idx = v->idx;
  }
}

static void doc_array_end(struct wcjson *ctx, void *doc, void *arr) {
  struct doc_impl *d = doc;
  d->nlimit++;
}

static void *doc_string_value(struct wcjson *ctx, void *doc, const wchar_t *str,
                              const size_t len, const bool escaped) {
  struct doc_impl *d = doc;
  struct wcjson_value *v = doc_next_value(ctx, d);

  if (v != NULL) {
    v->is_string = 1;
    v->string = str;
    v->s_len = len;
    d->d->s_nitems += len + 1;
    d->d->e_nitems = MAX((len + 1) * WCJSON_ESCAPE_MAX, d->d->e_nitems);
  }

  return v;
}

static void *doc_number_value(struct wcjson *ctx, void *doc, const wchar_t *num,
                              const size_t len) {
  struct doc_impl *d = doc;
  struct wcjson_value *v = doc_next_value(ctx, d);

  if (v != NULL) {
    v->is_number = 1;
    v->string = num;
    v->s_len = len;
    d->d->s_nitems += len + 1;
  }

  return v;
}

static void *doc_bool_value(struct wcjson *ctx, void *doc, const bool value) {
  struct doc_impl *d = doc;
  struct wcjson_value *v = doc_next_value(ctx, d);

  if (v != NULL) {
    v->is_boolean = 1;
    v->is_true = value ? 1 : 0;
  }

  return v;
}

static void *doc_null_value(struct wcjson *ctx, void *doc) {
  struct doc_impl *d = doc;
  struct wcjson_value *v = doc_next_value(ctx, d);

  if (v != NULL)
    v->is_null = 1;

  return v;
}

static int doc_unesc(struct wcjson *ctx, struct doc_impl *d,
                     struct wcjson_value *v) {
  if (v->is_string || v->is_pair) {
    size_t dst_len = d->d->s_nitems - d->d->s_idx;
    wchar_t *dst = &d->d->strings[d->d->s_idx];

    if (wcjsonstowc(v->string, v->s_len, dst, &dst_len) < 0)
      goto err_decode;

    if (d->d->s_idx + dst_len + 1 > d->d->s_nitems)
      goto err_range;

    dst[dst_len] = L'\0';

    v->string = dst;
    v->s_len = dst_len;

    d->d->s_idx += dst_len + 1;
    d->d->e_nitems = MAX((dst_len + 1) * WCJSON_ESCAPE_MAX, d->d->e_nitems);
    if (v->is_pair && doc_unesc(ctx, d, wcjson_value_head(d->d, v)) < 0)
      goto err;

  } else if (v->is_number) {
    size_t dst_len = d->d->s_nitems - d->d->s_idx;
    wchar_t *dst = &d->d->strings[d->d->s_idx];

    if (dst_len < v->s_len + 1)
      goto err_range;

    wmemcpy(dst, v->string, v->s_len);
    dst[v->s_len] = L'\0';

    v->string = dst;
    d->d->s_idx += v->s_len + 1;
  } else if (v->is_array) {
    for (struct wcjson_value *n = wcjson_value_tail(d->d, v); n != NULL;
         n = wcjson_value_prev(d->d, n))
      if (doc_unesc(ctx, d, n) < 0)
        goto err;
  } else if (v->is_object) {
    for (struct wcjson_value *n = wcjson_value_tail(d->d, v); n != NULL;
         n = wcjson_value_prev(d->d, n))
      if (doc_unesc(ctx, d, n) < 0)
        goto err;
  }

  return 0;
err:
  return -1;
err_range:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = ERANGE;
  return -1;
err_decode:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = errno;
  return -1;
}

static int doc_fprint(FILE *f, bool asc, const struct wcjson_document *d,
                      const struct wcjson_value *v) {
  if (v->is_null) {
    if (fputws(L"null", f) == -1)
      goto err;

  } else if (v->is_boolean) {
    if (fputws(v->is_true ? L"true" : L"false", f) == -1)
      goto err;

  } else if (v->is_string || v->is_pair) {
    size_t d_len = d->e_nitems;

    if (asc) {
      if (wctoascjsons(v->string, v->s_len, d->esc, &d_len) < 0)
        goto err;

    } else {
      if (wctowcjsons(v->string, v->s_len, d->esc, &d_len) < 0)
        goto err;
    }

    if (d->e_nitems - d_len == 0)
      goto err_range;

    d->esc[d_len] = L'\0';

    if (putwc(L'"', f) == WEOF)
      goto err;

    if (fputws(d->esc, f) == -1)
      goto err;

    if (putwc(L'"', f) == WEOF)
      goto err;

    if (v->is_pair) {
      if (putwc(L':', f) == WEOF)
        goto err;

      if (doc_fprint(f, asc, d, wcjson_value_head(d, v)) < 0)
        goto err;
    }
  } else if (v->is_number) {
    if (fputws(v->string, f) == -1)
      goto err;

  } else if (v->is_array) {
    if (putwc(L'[', f) == WEOF)
      goto err;

    struct wcjson_value *n = wcjson_value_head(d, v);
    while (n != NULL) {
      if (doc_fprint(f, asc, d, n) < 0)
        goto err;

      n = wcjson_value_next(d, n);

      if (n != NULL && putwc(L',', f) == WEOF)
        goto err;
    }

    if (putwc(L']', f) == WEOF)
      goto err;

  } else if (v->is_object) {
    if (putwc(L'{', f) == WEOF)
      goto err;

    struct wcjson_value *n = wcjson_value_head(d, v);
    while (n != NULL) {
      if (doc_fprint(f, asc, d, n) < 0)
        goto err;

      n = wcjson_value_next(d, n);

      if (n != NULL && putwc(L',', f) == WEOF)
        goto err;
    }

    if (putwc(L'}', f) == WEOF)
      goto err;

  } else
    goto err_inval;

  return 0;
err:
  return -1;
err_inval:
  errno = EINVAL;
  return -1;
err_range:
  errno = ERANGE;
  return -1;
}

int static doc_sprint(wchar_t *s, size_t *lenp, bool asc,
                      const struct wcjson_document *d,
                      const struct wcjson_value *v) {
#ifdef __copy
#error "__copy macro already defined - rename"
#else
#define __copy(_s, _l)                                                         \
  do {                                                                         \
    if (s_len < (_l))                                                          \
      goto err_range;                                                          \
    wmemcpy(s, (_s), (_l));                                                    \
    s_len -= (_l);                                                             \
    s += (_l);                                                                 \
  } while (0)
#endif

  size_t s_len = *lenp;

  if (v->is_null) {
    __copy(L"null", 4);
  } else if (v->is_boolean) {
    if (v->is_true) {
      __copy(L"true", 4);
    } else {
      __copy(L"false", 5);
    }
  } else if (v->is_string || v->is_pair) {
    size_t d_len = d->e_nitems;

    if (asc) {
      if (wctoascjsons(v->string, v->s_len, d->esc, &d_len) < 0)
        goto err;

    } else {
      if (wctowcjsons(v->string, v->s_len, d->esc, &d_len) < 0)
        goto err;
    }

    if (d->e_nitems - d_len == 0)
      goto err_range;

    d->esc[d_len] = L'\0';

    __copy(L"\"", 1);
    __copy(d->esc, d_len);
    __copy(L"\"", 1);

    if (v->is_pair) {
      __copy(L":", 1);

      size_t t_len = s_len;
      if (doc_sprint(s, &t_len, asc, d, wcjson_value_head(d, v)) < 0)
        goto err;

      s_len -= t_len;
      s += t_len;
    }
  } else if (v->is_number) {
    __copy(v->string, v->s_len);
  } else if (v->is_array) {
    __copy(L"[", 1);

    struct wcjson_value *n = wcjson_value_head(d, v);
    while (n != NULL) {
      size_t t_len = s_len;
      if (doc_sprint(s, &t_len, asc, d, n) < 0)
        goto err;

      s_len -= t_len;
      s += t_len;

      n = wcjson_value_next(d, n);

      if (n != NULL)
        __copy(L",", 1);
    }

    __copy(L"]", 1);
  } else if (v->is_object) {
    __copy(L"{", 1);

    struct wcjson_value *n = wcjson_value_head(d, v);
    while (n != NULL) {
      size_t t_len = s_len;
      if (doc_sprint(s, &t_len, asc, d, n) < 0)
        goto err;

      s_len -= t_len;
      s += t_len;

      n = wcjson_value_next(d, n);

      if (n != NULL)
        __copy(L",", 1);
    }

    __copy(L"}", 1);
  } else
    goto err_inval;

  *lenp -= s_len;
  return 0;
err:
  return -1;
err_inval:
  errno = EINVAL;
  return -1;
err_range:
  errno = ERANGE;
  return -1;
#undef __copy
}

int wcjsondocvalues(struct wcjson *ctx, struct wcjson_document *doc,
                    const wchar_t *txt, const size_t len) {
  struct doc_impl d = {
      .d = doc,
      .nlimit = DEFAULT_NESTING_LIMIT,
  };
  d.d->v_idx = 0;
  d.d->s_idx = 0;
  d.d->s_nitems = 0;
  d.d->e_nitems = 0;
  return wcjson(ctx, &doc_ops, &d, txt, len);
}

int wcjsondocstrings(struct wcjson *ctx, struct wcjson_document *doc) {
  struct doc_impl d = {
      .d = doc,
      .nlimit = DEFAULT_NESTING_LIMIT,
  };
  d.d->s_idx = 0;
  d.d->e_nitems = 0;
  return doc_unesc(ctx, &d, d.d->values);
}

int wcjsondocfprint(FILE *f, const struct wcjson_document *doc,
                    const struct wcjson_value *value) {
  return doc_fprint(f, false, doc, value);
}

int wcjsondocfprintasc(FILE *f, const struct wcjson_document *doc,
                       const struct wcjson_value *value) {
  return doc_fprint(f, true, doc, value);
}

int wcjsondocsprint(wchar_t *s, size_t *lenp, const struct wcjson_document *doc,
                    const struct wcjson_value *value) {
  size_t s_len = *lenp;
  size_t t_len = s_len;
  int r = doc_sprint(s, &t_len, false, doc, value);

  if (r < 0)
    return -1;

  s_len -= t_len;

  if (s_len < 1)
    goto err_range;

  s[t_len] = L'\0';
  *lenp -= s_len;
  return 0;
err_range:
  errno = ERANGE;
  return -1;
}

int wcjsondocsprintasc(wchar_t *s, size_t *lenp,
                       const struct wcjson_document *doc,
                       const struct wcjson_value *value) {
  size_t s_len = *lenp;
  size_t t_len = s_len;
  int r = doc_sprint(s, &t_len, true, doc, value);

  if (r < 0)
    return -1;

  s_len -= t_len;

  if (s_len < 1)
    goto err_range;

  s[t_len] = L'\0';
  *lenp -= s_len;
  return 0;
err_range:
  errno = ERANGE;
  return -1;
}

#ifdef __cplusplus
}
#endif
