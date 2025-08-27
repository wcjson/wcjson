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
 * @file wcjson.c
 * @brief RFC8259 implementation.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "wcjson.h"

static const wchar_t *const hex_digits = L"0123456789abcdef";

enum token {
  T_OBJ_START,
  T_OBJ_END,
  T_ARR_START,
  T_ARR_END,
  T_COMMA,
  T_COLON,
  T_QUOTE,
  T_TRUE,
  T_FALSE,
  T_NULL,
  T_NUMBER,
  T_UNKNOWN,
};

struct scan_state {
  size_t pos;
  size_t len;
  const wchar_t *txt;
  bool escaped;
};

static void *parse_array(struct scan_state *, struct wcjson *,
                         const struct wcjson_ops *, void *, void *);
static void *parse_object(struct scan_state *, struct wcjson *,
                          const struct wcjson_ops *, void *, void *);

static enum token scan(struct scan_state *ss) {
  switch (ss->txt[ss->pos]) {
  case L'{':
    return T_OBJ_START;
  case L'}':
    return T_OBJ_END;
  case L'[':
    return T_ARR_START;
  case L']':
    return T_ARR_END;
  case L',':
    return T_COMMA;
  case L':':
    return T_COLON;
  case L'"':
    return T_QUOTE;
  case L't':
    return T_TRUE;
  case L'f':
    return T_FALSE;
  case L'n':
    return T_NULL;
  case L'-':
    return T_NUMBER;
  default:
    return (ss->txt[ss->pos] >= L'0' && ss->txt[ss->pos] <= L'9') ? T_NUMBER
                                                                  : T_UNKNOWN;
  }
}

static inline enum wcjson_status scan_ws(struct scan_state *ss) {
  for (; ss->pos < ss->len; ss->pos++) {
    switch (ss->txt[ss->pos]) {
    case L'\t':
    case L'\n':
    case L'\r':
    case L' ':
      break;
    default:
      return WCJSON_OK;
    }

    if (ss->pos == SIZE_MAX)
      return WCJSON_ABORT_END_OF_INPUT;
  }

  return WCJSON_OK;
}

static inline enum wcjson_status
scan_literal(struct scan_state *ss, const wchar_t *lit, const size_t lit_len) {
  size_t i = 0;

  for (; ss->pos < ss->len && i < lit_len; ss->pos++, i++) {
    if (ss->txt[ss->pos] != lit[i])
      return WCJSON_ABORT_INVALID;

    if (ss->pos == SIZE_MAX)
      return WCJSON_ABORT_END_OF_INPUT;
  }

  if (i < lit_len)
    return WCJSON_ABORT_END_OF_INPUT;

  return WCJSON_OK;
}

static void *parse_null(struct scan_state *ss, struct wcjson *ctx,
                        const struct wcjson_ops *ops, void *doc) {
  ctx->status = scan_literal(ss, L"null", 4);
  return ctx->status == WCJSON_OK && ops != NULL ? ops->null_value(ctx, doc)
                                                 : NULL;
}

static void *parse_true(struct scan_state *ss, struct wcjson *ctx,
                        const struct wcjson_ops *ops, void *doc) {
  ctx->status = scan_literal(ss, L"true", 4);
  return ctx->status == WCJSON_OK && ops != NULL
             ? ops->bool_value(ctx, doc, true)
             : NULL;
}

static void *parse_false(struct scan_state *ss, struct wcjson *ctx,
                         const struct wcjson_ops *ops, void *doc) {
  ctx->status = scan_literal(ss, L"false", 5);
  return ctx->status == WCJSON_OK && ops != NULL
             ? ops->bool_value(ctx, doc, false)
             : NULL;
}

static inline enum wcjson_status scan_int(struct scan_state *ss) {
  bool digits = false;
  bool zero = false;
  const size_t start = ss->pos;

  for (; ss->pos < ss->len; ss->pos++) {
    switch (ss->txt[ss->pos]) {
    case L'0':
      if (zero)
        return WCJSON_ABORT_INVALID;

      zero = ss->pos == start;
      digits = true;
      break;
    default:
      if (ss->txt[ss->pos] >= L'1' && ss->txt[ss->pos] <= L'9') {
        if (zero)
          return WCJSON_ABORT_INVALID;

        zero = false;
        digits = true;
        break;
      } else
        goto out;
      break;
    }

    if (ss->pos == SIZE_MAX)
      return WCJSON_ABORT_END_OF_INPUT;
  }

out:
  return digits ? WCJSON_OK : WCJSON_ABORT_INVALID;
}

static inline enum wcjson_status scan_frac(struct scan_state *ss) {
  if (ss->txt[ss->pos] != L'.')
    return WCJSON_OK;

  if (ss->pos == SIZE_MAX)
    return WCJSON_ABORT_END_OF_INPUT;

  ss->pos++;
  bool digits = false;

  for (; ss->pos < ss->len; ss->pos++) {
    if (ss->txt[ss->pos] >= L'0' && ss->txt[ss->pos] <= L'9')
      digits = true;
    else
      goto out;

    if (ss->pos == SIZE_MAX)
      return WCJSON_ABORT_END_OF_INPUT;
  }

out:
  return digits ? WCJSON_OK : WCJSON_ABORT_INVALID;
}

static inline enum wcjson_status scan_exp(struct scan_state *ss) {
  if (ss->txt[ss->pos] != L'e' && ss->txt[ss->pos] != L'E')
    return WCJSON_OK;

  if (ss->pos == SIZE_MAX)
    return WCJSON_ABORT_END_OF_INPUT;

  ss->pos++;
  bool op = false;
  bool digits = false;

  for (; ss->pos < ss->len; ss->pos++) {
    switch (ss->txt[ss->pos]) {
    case L'-':
    case L'+':
      if (op)
        return WCJSON_ABORT_INVALID;

      op = true;
      break;
    default:
      if (ss->txt[ss->pos] >= L'0' && ss->txt[ss->pos] <= L'9')
        digits = true;
      else
        goto out;
      break;
    }

    if (ss->pos == SIZE_MAX)
      return WCJSON_ABORT_END_OF_INPUT;
  }

out:
  return digits ? WCJSON_OK : WCJSON_ABORT_INVALID;
}

static void *parse_number(struct scan_state *ss, struct wcjson *ctx,
                          const struct wcjson_ops *ops, void *doc) {
  const size_t start = ss->pos;

  if (ss->txt[ss->pos] == L'-') {
    if (ss->pos == SIZE_MAX) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }
    ss->pos++;
  }

  if (ss->pos < ss->len) {
    ctx->status = scan_int(ss);

    if (ctx->status != WCJSON_OK)
      return NULL;

  } else {
    ctx->status = WCJSON_ABORT_END_OF_INPUT;
    return NULL;
  }

  if (ss->pos < ss->len) {
    ctx->status = scan_frac(ss);

    if (ctx->status != WCJSON_OK)
      return NULL;
  }

  if (ss->pos < ss->len) {
    ctx->status = scan_exp(ss);

    if (ctx->status != WCJSON_OK)
      return NULL;
  }

  return ops != NULL
             ? ops->number_value(ctx, doc, &ss->txt[start], ss->pos - start)
             : NULL;
}

static inline enum wcjson_status scan_unescaped(struct scan_state *ss) {
#ifdef __limit
#error "__limit macro already defined - rename"
#endif
#if defined(WCHAR_T_UTF32)
#define __limit 0x10ffff
#elif defined(WCHAR_T_UTF16)
#define __limit 0xffff
#elif defined(WCHAR_T_UTF8)
#define __limit 0xff
#elif
#error "Wide character literal encoding implementation not found."
#endif
  for (; ss->pos < ss->len; ss->pos++) {
    if (!((ss->txt[ss->pos] >= (wchar_t)0x20 &&
           ss->txt[ss->pos] <= (wchar_t)0x21) ||
          (ss->txt[ss->pos] >= (wchar_t)0x23 &&
           ss->txt[ss->pos] <= (wchar_t)0x5b) ||
          (ss->txt[ss->pos] >= (wchar_t)0x5d &&
           ss->txt[ss->pos] <= (wchar_t)__limit)))
      break;

    if (ss->pos == SIZE_MAX)
      return WCJSON_ABORT_END_OF_INPUT;
  }

  return WCJSON_OK;
#undef __limit
}

static inline uint16_t scan_hex4(struct scan_state *ss) {
  uint16_t r = 0;

  // Caller needs to check for ss->pos > SIZE_MAX - 4 wrap around.
  for (int e = 3; ss->pos < ss->len && e >= 0; ss->pos++, e--)
    if (ss->txt[ss->pos] >= L'0' && ss->txt[ss->pos] <= L'9')
      r += (ss->txt[ss->pos] - L'0') * (1 << (e << 2));
    else if (ss->txt[ss->pos] >= L'a' && ss->txt[ss->pos] <= L'f')
      r += (ss->txt[ss->pos] - L'a' + 10) * (1 << (e << 2));
    else if (ss->txt[ss->pos] >= L'A' && ss->txt[ss->pos] <= L'F')
      r += (ss->txt[ss->pos] - L'A' + 10) * (1 << (e << 2));
    else
      return r;

  return r;
}

static inline enum wcjson_status scan_escaped(struct scan_state *ss) {
  if (ss->pos == SIZE_MAX || ++ss->pos == ss->len)
    return WCJSON_ABORT_END_OF_INPUT;

  switch (ss->txt[ss->pos]) {
  case L'"':
  case L'\\':
  case L'/':
  case L'b':
  case L'f':
  case L'n':
  case L'r':
  case L't':
    if (ss->pos == SIZE_MAX)
      return WCJSON_ABORT_END_OF_INPUT;

    ++ss->pos;
    return WCJSON_OK;
  case L'u':
    if (ss->pos == SIZE_MAX || ++ss->pos == ss->len)
      return WCJSON_ABORT_END_OF_INPUT;

    size_t start = ss->pos;

    // scan_hex4() does not check for ss->pos wrap around.
    if (ss->pos > SIZE_MAX - (size_t)4)
      return WCJSON_ABORT_INVALID;

    uint16_t unescaped = scan_hex4(ss);

    if (ss->pos - start != (size_t)4 || unescaped < 0x20)
      return WCJSON_ABORT_INVALID;

    if (unescaped >= 0xd800 && unescaped <= 0xdfff) {
      // UTF 16 surrogates
      if (unescaped > 0xdbff || ss->txt[ss->pos] != L'\\')
        return WCJSON_ABORT_INVALID;

      if (ss->pos == SIZE_MAX || ++ss->pos == ss->len)
        return WCJSON_ABORT_END_OF_INPUT;

      if (ss->txt[ss->pos] != L'u')
        return WCJSON_ABORT_INVALID;

      if (ss->pos == SIZE_MAX || ++ss->pos == ss->len)
        return WCJSON_ABORT_END_OF_INPUT;

      start = ss->pos;

      // scan_hex4() does not chek ss->pos for wrap around.
      if (ss->pos > SIZE_MAX - (size_t)4)
        return WCJSON_ABORT_INVALID;

      unescaped = scan_hex4(ss);

      if (ss->pos - start != (size_t)4 || unescaped < 0xdc00 ||
          unescaped > 0xdfff)
        return WCJSON_ABORT_INVALID;
    }

    return WCJSON_OK;
  default:
    return WCJSON_ABORT_INVALID;
  }
}

static void *parse_string(struct scan_state *ss, struct wcjson *ctx,
                          const struct wcjson_ops *ops, void *doc) {
  ss->escaped = false;

  if (ss->pos == SIZE_MAX || ++ss->pos == ss->len) {
    ctx->status = WCJSON_ABORT_END_OF_INPUT;
    return NULL;
  }

  size_t start = ss->pos;

next_part:
  ctx->status = scan_unescaped(ss);

  if (ctx->status != WCJSON_OK)
    return NULL;

  if (ss->pos == ss->len) {
    ctx->status = WCJSON_ABORT_END_OF_INPUT;
    return NULL;
  }

  switch (ss->txt[ss->pos]) {
  case L'"':
    if (ss->pos == SIZE_MAX) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }

    ss->pos++;
    return ops != NULL
               ? ss->pos == start
                     ? ops->string_value(ctx, doc, L"", 0, false)
                     : ops->string_value(ctx, doc, &ss->txt[start],
                                         ss->pos - start - 1, ss->escaped)
               : NULL;

  case L'\\':
    ss->escaped = true;
    ctx->status = scan_escaped(ss);

    if (ctx->status != WCJSON_OK)
      return NULL;

    goto next_part;
  default:
    ctx->status = WCJSON_ABORT_INVALID;
    return NULL;
  }
}

static void *parse_object(struct scan_state *ss, struct wcjson *ctx,
                          const struct wcjson_ops *ops, void *doc,
                          void *parent) {
  void *obj = ops != NULL ? ops->object_start(ctx, doc, parent) : NULL;
  void *key = NULL;
  bool key_seen = false;
  bool value_seen = false;

  if (ss->pos == SIZE_MAX) {
    ctx->status = WCJSON_ABORT_END_OF_INPUT;
    return NULL;
  }

  ++ss->pos;

next_token:
  ctx->status = scan_ws(ss);

  if (ctx->status != WCJSON_OK)
    return NULL;

  if (ss->pos == ss->len) {
    ctx->status = WCJSON_ABORT_END_OF_INPUT;
    return NULL;
  }

  switch (scan(ss)) {
  case T_OBJ_END: {
    if (key_seen) {
      ctx->status = WCJSON_ABORT_INVALID;
      return NULL;
    }

    if (ss->pos == SIZE_MAX) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }

    ss->pos++;
    if (ops != NULL)
      ops->object_end(ctx, doc, obj);

    return obj;
  }
  case T_QUOTE: {
    if (key_seen) {
      ctx->status = WCJSON_ABORT_INVALID;
      return NULL;
    }

    key = parse_string(ss, ctx, ops, doc);

    if (ctx->status != WCJSON_OK)
      return NULL;

    key_seen = true;
    goto next_token;
  }
  case T_COLON: {
    if (!key_seen) {
      ctx->status = WCJSON_ABORT_INVALID;
      return NULL;
    }

    if (ss->pos == SIZE_MAX || ++ss->pos == ss->len) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }

    ctx->status = scan_ws(ss);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ss->pos == ss->len) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }

    switch (scan(ss)) {
    case T_TRUE: {
      void *value = parse_true(ss, ctx, ops, doc);

      if (ctx->status != WCJSON_OK)
        return NULL;

      if (ops != NULL) {
        ops->object_add(ctx, doc, obj, key, value);

        if (ctx->status != WCJSON_OK)
          return NULL;
      }

      key_seen = false;
      value_seen = true;
      break;
    }
    case T_FALSE: {
      void *value = parse_false(ss, ctx, ops, doc);

      if (ctx->status != WCJSON_OK)
        return NULL;

      if (ops != NULL) {
        ops->object_add(ctx, doc, obj, key, value);

        if (ctx->status != WCJSON_OK)
          return NULL;
      }

      key_seen = false;
      value_seen = true;
      break;
    }
    case T_NULL: {
      void *value = parse_null(ss, ctx, ops, doc);

      if (ctx->status != WCJSON_OK)
        return NULL;

      if (ops != NULL) {
        ops->object_add(ctx, doc, obj, key, value);

        if (ctx->status != WCJSON_OK)
          return NULL;
      }

      key_seen = false;
      value_seen = true;
      break;
    }
    case T_NUMBER: {
      void *value = parse_number(ss, ctx, ops, doc);

      if (ctx->status != WCJSON_OK)
        return NULL;

      if (ops != NULL) {
        ops->object_add(ctx, doc, obj, key, value);

        if (ctx->status != WCJSON_OK)
          return NULL;
      }

      key_seen = false;
      value_seen = true;
      break;
    }
    case T_QUOTE: {
      void *value = parse_string(ss, ctx, ops, doc);

      if (ctx->status != WCJSON_OK)
        return NULL;

      if (ops != NULL) {
        ops->object_add(ctx, doc, obj, key, value);

        if (ctx->status != WCJSON_OK)
          return NULL;
      }

      key_seen = false;
      value_seen = true;
      break;
    }
    case T_OBJ_START: {
      void *value = parse_object(ss, ctx, ops, doc, obj);

      if (ctx->status != WCJSON_OK)
        return NULL;

      if (ops != NULL) {
        ops->object_add(ctx, doc, obj, key, value);

        if (ctx->status != WCJSON_OK)
          return NULL;
      }

      key_seen = false;
      value_seen = true;
      break;
    }
    case T_ARR_START: {
      void *value = parse_array(ss, ctx, ops, doc, obj);

      if (ctx->status != WCJSON_OK)
        return NULL;

      if (ops != NULL) {
        ops->object_add(ctx, doc, obj, key, value);

        if (ctx->status != WCJSON_OK)
          return NULL;
      }

      key_seen = false;
      value_seen = true;
      break;
    }
    default:
      ctx->status = WCJSON_ABORT_INVALID;
      return NULL;
    }
    goto next_token;
  }
  case T_COMMA: {
    if (!value_seen) {
      ctx->status = WCJSON_ABORT_INVALID;
      return NULL;
    }
    value_seen = false;

    if (ss->pos == SIZE_MAX) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }

    ++ss->pos;
    goto next_token;
  }
  default:
    ctx->status = WCJSON_ABORT_INVALID;
    return NULL;
  }
}

static void *parse_array(struct scan_state *ss, struct wcjson *ctx,
                         const struct wcjson_ops *ops, void *doc,
                         void *parent) {
  void *arr = ops != NULL ? ops->array_start(ctx, doc, parent) : NULL;
  bool value_seen = false;

  if (ss->pos == SIZE_MAX) {
    ctx->status = WCJSON_ABORT_END_OF_INPUT;
    return NULL;
  }

  ++ss->pos;

next_token:
  ctx->status = scan_ws(ss);

  if (ctx->status != WCJSON_OK)
    return NULL;

  if (ss->pos == ss->len) {
    ctx->status = WCJSON_ABORT_END_OF_INPUT;
    return NULL;
  }

  switch (scan(ss)) {
  case T_ARR_END: {
    if (ss->pos == SIZE_MAX) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }

    ss->pos++;

    if (ops != NULL)
      ops->array_end(ctx, doc, arr);

    return arr;
  }
  case T_COMMA: {
    if (!value_seen) {
      ctx->status = WCJSON_ABORT_INVALID;
      return NULL;
    }
    value_seen = false;

    if (ss->pos == SIZE_MAX) {
      ctx->status = WCJSON_ABORT_END_OF_INPUT;
      return NULL;
    }

    ++ss->pos;
    goto next_token;
  }
  case T_TRUE: {
    void *value = parse_true(ss, ctx, ops, doc);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ops != NULL) {
      ops->array_add(ctx, doc, arr, value);

      if (ctx->status != WCJSON_OK)
        return NULL;
    }

    value_seen = true;
    goto next_token;
  }
  case T_FALSE: {
    void *value = parse_false(ss, ctx, ops, doc);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ops != NULL) {
      ops->array_add(ctx, doc, arr, value);

      if (ctx->status != WCJSON_OK)
        return NULL;
    }

    value_seen = true;
    goto next_token;
  }
  case T_NULL: {
    void *value = parse_null(ss, ctx, ops, doc);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ops != NULL) {
      ops->array_add(ctx, doc, arr, value);

      if (ctx->status != WCJSON_OK)
        return NULL;
    }

    value_seen = true;
    goto next_token;
  }
  case T_NUMBER: {
    void *value = parse_number(ss, ctx, ops, doc);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ops != NULL) {
      ops->array_add(ctx, doc, arr, value);

      if (ctx->status != WCJSON_OK)
        return NULL;
    }

    value_seen = true;
    goto next_token;
  }
  case T_QUOTE: {
    void *value = parse_string(ss, ctx, ops, doc);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ops != NULL) {
      ops->array_add(ctx, doc, arr, value);

      if (ctx->status != WCJSON_OK)
        return NULL;
    }

    value_seen = true;
    goto next_token;
  }
  case T_OBJ_START: {
    void *value = parse_object(ss, ctx, ops, doc, arr);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ops != NULL) {
      ops->array_add(ctx, doc, arr, value);

      if (ctx->status != WCJSON_OK)
        return NULL;
    }

    value_seen = true;
    goto next_token;
  }
  case T_ARR_START: {
    void *value = parse_array(ss, ctx, ops, doc, arr);

    if (ctx->status != WCJSON_OK)
      return NULL;

    if (ops != NULL) {
      ops->array_add(ctx, doc, arr, value);

      if (ctx->status != WCJSON_OK)
        return NULL;
    }

    value_seen = true;
    goto next_token;
  }
  default:
    ctx->status = WCJSON_ABORT_INVALID;
    return NULL;
  }
}

static void parse_json_text(struct scan_state *ss, struct wcjson *ctx,
                            const struct wcjson_ops *ops, void *doc) {
  ctx->status = scan_ws(ss);

  if (ctx->status != WCJSON_OK)
    return;

  if (ss->pos == ss->len)
    return;

  switch (scan(ss)) {
  case T_TRUE:
    parse_true(ss, ctx, ops, doc);
    break;
  case T_FALSE:
    parse_false(ss, ctx, ops, doc);
    break;
  case T_NULL:
    parse_null(ss, ctx, ops, doc);
    break;
  case T_NUMBER:
    parse_number(ss, ctx, ops, doc);
    break;
  case T_QUOTE:
    parse_string(ss, ctx, ops, doc);
    break;
  case T_OBJ_START:
    parse_object(ss, ctx, ops, doc, NULL);
    break;
  case T_ARR_START:
    parse_array(ss, ctx, ops, doc, NULL);
    break;
  default:
    ctx->status = WCJSON_ABORT_INVALID;
    return;
  }

  if (ctx->status != WCJSON_OK)
    return;

  ctx->status = scan_ws(ss);

  if (ctx->status == WCJSON_OK && ss->pos != ss->len)
    ctx->status = WCJSON_ABORT_INVALID;
}

int wcjson(struct wcjson *ctx, const struct wcjson_ops *ops, void *doc,
           const wchar_t *txt, const size_t len) {
  ctx->status = WCJSON_OK;
  ctx->errnum = 0;

  if (txt != NULL && len > 0) {
    struct scan_state ss = {
        .pos = 0,
        .len = len,
        .txt = txt,
        .escaped = false,
    };

    parse_json_text(&ss, ctx, ops, doc);
  } else
    ctx->status = WCJSON_ABORT_INVALID;

  return ctx->status == WCJSON_OK ? 0 : -1;
}

static inline int uhex4(wchar_t n, wchar_t *s, size_t *len) {
  if (*len < 6)
    return -1;

  *s++ = L'\\';
  *s++ = L'u';
  *s++ = hex_digits[(n >> 12) & 0xf];
  *s++ = hex_digits[(n >> 8) & 0xf];
  *s++ = hex_digits[(n >> 4) & 0xf];
  *s++ = hex_digits[n & 0xf];
  *len -= *len - 6;
  return 0;
}

static int wctojsons(const wchar_t *s, size_t s_len, wchar_t *d, size_t *d_lenp,
                     bool ascii) {
#ifdef __escape
#error "__escape macro already defined - rename"
#endif
#define __escape(_c)                                                           \
  do {                                                                         \
    *d++ = L'\\';                                                              \
    if (--d_len == 0)                                                          \
      goto err_range;                                                          \
    *d++ = (_c);                                                               \
    s++;                                                                       \
  } while (0)

  size_t d_len = *d_lenp, u_len;
#if defined(WCHAR_T_UTF8)
  uint32_t cp;
#endif

  if (s_len != 0) {
    if (d_len == 0)
      goto err_range;

    do {
      switch (*s) {
      case L'"':
        __escape(L'"');
        break;
      case L'\\':
        __escape(L'\\');
        break;
      case L'/':
        __escape(L'/');
        break;
      case L'\b':
        __escape(L'b');
        break;
      case L'\f':
        __escape(L'f');
        break;
      case L'\n':
        __escape(L'n');
        break;
      case L'\r':
        __escape(L'r');
        break;
      case L'\t':
        __escape(L't');
        break;
      default:
        if (*s < 0x20)
          goto err_ilseq;

        if (ascii && *s > 0x7f) {
#if defined(WCHAR_T_UTF32)
          if (*s < 0x10000) {
            u_len = d_len;
            if (uhex4(*s, d, &u_len))
              goto err_range;

          } else {
            // UTF 16 surrogates
            u_len = d_len;
            if (uhex4(0xd800 | (((*s - 0x10000) >> 10) & 0b1111111111), d,
                      &u_len))
              goto err_range;

            d_len -= u_len;
            d += u_len;
            u_len = d_len;
            if (uhex4(0xdc00 | (*s & 0b1111111111), d, &u_len))
              goto err_range;
          }
          d_len -= u_len - 1;
          d += u_len;
          s++;
#elif defined(WCHAR_T_UTF16)
          if (*s >= 0xd800 && *s <= 0xdfff) {
            // UTF 16 surrogates
            if (*s > 0xdbff)
              goto err_ilseq;

            u_len = d_len;
            if (uhex4(*s, d, &u_len))
              goto err_range;

            d_len -= u_len;
            d += u_len;

            s++;

            if (--s_len == 0)
              goto err_ilseq;

            if (*s < 0xdc00 || *s > 0xdfff)
              goto err_ilseq;

            u_len = d_len;
            if (uhex4(*s, d, &u_len))
              goto err_range;

          } else {
            u_len = d_len;
            if (uhex4(*s, d, &u_len))
              goto err_range;
          }
          d_len -= u_len - 1;
          d += u_len;
          s++;
#elif defined(WCHAR_T_UTF8)
          // Decode UTF 8
          if ((*s & 0b11110000) == 0b11110000) {
            if (4 > s_len || (s[3] & 0b10000000) != 0b10000000 ||
                (s[2] & 0b10000000) != 0b10000000 ||
                (s[1] & 0b10000000) != 0b10000000)
              goto err_ilseq;

            cp = ((s[0] & 0b111) << 18) | ((s[1] & 0b111111) << 12) |
                 ((s[2] & 0b111111) << 6) | (s[3] & 0b111111);

            s_len -= 3;
            s += 4;
          } else if ((*s & 0b11100000) == 0b11100000) {
            if (3 > s_len || (s[2] & 0b10000000) != 0b10000000 ||
                (s[1] & 0b10000000) != 0b10000000)
              goto err_ilseq;

            cp = ((s[0] & 0b1111) << 12) | ((s[1] & 0b111111) << 6) |
                 (s[2] & 0b111111);

            s_len -= 2;
            s += 3;
          } else if ((*s & 0b11000000) == 0b11000000) {
            if (2 > s_len || (s[1] & 0b10000000) != 0b10000000)
              goto err_ilseq;

            cp = ((s[0] & 0b111111) << 6) | (s[1] & 0b11111);

            s_len -= 1;
            s += 2;
          } else
            goto err_ilseq;

          if (cp < 0x10000) {
            u_len = d_len;
            if (uhex4(cp, d, &u_len))
              goto err_range;

          } else {
            // UTF 16 surrogates
            u_len = d_len;
            if (uhex4(0xd800 | (((cp - 0x10000) >> 10) & 0b1111111111), d,
                      &u_len))
              goto err_range;

            d_len -= u_len;
            d += u_len;
            u_len = d_len;
            if (uhex4(0xdc00 | (cp & 0b1111111111), d, &u_len))
              goto err_range;
          }
          d_len -= u_len - 1;
          d += u_len;
#elif
#error "Wide character literal encoding implementation not found."
#endif
        } else
          *d++ = *s++;
        break;
      }
    } while (--s_len != 0 && --d_len != 0);

    if (s_len != 0)
      goto err_range;
    else
      d_len--;
  }

  *d_lenp -= d_len;
  return 0;
err_range:
  *d_lenp -= d_len;
  errno = ERANGE;
  return -1;
err_ilseq:
  *d_lenp -= d_len;
  errno = EILSEQ;
  return -1;
#undef __escape
}

int wctowcjsons(const wchar_t *s, size_t s_len, wchar_t *d, size_t *d_lenp) {
  return wctojsons(s, s_len, d, d_lenp, false);
}

int wctoascjsons(const wchar_t *s, size_t s_len, wchar_t *d, size_t *d_lenp) {
  return wctojsons(s, s_len, d, d_lenp, true);
}

int wcjsonstowc(const wchar_t *s, size_t s_len, wchar_t *d, size_t *d_lenp) {
  struct scan_state ss = {0};
  uint32_t cp;
  uint16_t ls;
  size_t d_len = *d_lenp;

  if (s_len != 0) {
    if (d_len == 0)
      goto err_range;

    do {
      if (*s == L'\\') {
        if (--s_len == 0)
          goto err_ilseq;

        s++;

        switch (*s) {
        case L'"':
          *d++ = L'"';
          s++;
          break;
        case L'\\':
          *d++ = L'\\';
          s++;
          break;
        case L'/':
          *d++ = L'/';
          s++;
          break;
        case L'b':
          *d++ = L'\b';
          s++;
          break;
        case L'f':
          *d++ = L'\f';
          s++;
          break;
        case L'n':
          *d++ = L'\n';
          s++;
          break;
        case L'r':
          *d++ = L'\r';
          s++;
          break;
        case L't':
          *d++ = L'\t';
          s++;
          break;
        case L'u':
          if (--s_len == 0)
            goto err_ilseq;

          s++;

          ss.pos = 0;
          ss.txt = s;
          ss.len = s_len;

          cp = scan_hex4(&ss);

          if (ss.pos != (size_t)4 || cp < 0x20)
            goto err_ilseq;

          s += ss.pos;
          s_len -= ss.pos;

          if (cp >= 0xd800 && cp <= 0xdfff) {
            // UTF 16 surrogates
            if (cp > 0xdbff || s_len == 0 || --s_len == 0 || *s != L'\\')
              goto err_ilseq;

            s++;

            if (*s != L'u' || --s_len == 0)
              goto err_ilseq;

            s++;

            ss.pos = 0;
            ss.txt = s;
            ss.len = s_len;

            ls = scan_hex4(&ss);

            if (ss.pos != (size_t)4)
              goto err_ilseq;

            s += ss.pos;
            s_len -= ss.pos;

            if (ls < 0xdc00 || ls > 0xdfff)
              goto err_ilseq;

            cp = (((cp & 0b1111111111) << 10) | (ls & 0b1111111111)) + 0x10000;
          }
#if defined(WCHAR_T_UTF32)
          *d++ = (wchar_t)cp;
#elif defined(WCHAR_T_UTF16)
          if (cp > 0xffff) {
            // UTF 16 surrogates
            *d++ = (wchar_t)(0xd800 | (((cp - 0x10000) >> 10) & 0b1111111111));

            if (--d_len == 0)
              goto err_range;

            *d++ = (wchar_t)(0xdc00 | ((cp - 0x10000) & 0b1111111111));
          } else
            *d++ = cp;
#elif defined(WCHAR_T_UTF8)
          if (cp < 0x80) {
            *d++ = (wchar_t)cp & 0xff;
          } else if (cp >= 80 && cp <= 0x7ff) {
            if (2 > d_len)
              goto err_range;

            d[1] = (wchar_t)(0b10000000 | (cp & 0b111111));
            d[0] = (wchar_t)(0b11000000 | ((cp & 0b11111000000) >> 6));
            d += 2;
            d_len -= 1;
          } else if (cp >= 0x800 && cp <= 0xffff) {
            if (3 > d_len)
              goto err_range;

            d[2] = (wchar_t)(0b10000000 | (cp & 0b111111));
            d[1] = (wchar_t)(0b10000000 | ((cp & 0b111111000000) >> 6));
            d[0] = (wchar_t)(0b11100000 | ((cp & 0b1111000000000000) >> 12));
            d += 3;
            d_len -= 2;
          } else if (cp >= 0x10000 && cp <= 0x10ffff) {
            if (4 > d_len)
              goto err_range;

            d[3] = (wchar_t)(0b10000000 | (cp & 0b111111));
            d[2] = (wchar_t)(0b10000000 | ((cp & 0b111111000000) >> 6));
            d[1] = (wchar_t)(0b10000000 | ((cp & 0b111111000000000000) >> 12));
            d[0] =
                (wchar_t)(0b11110000 | ((cp & 0b111000000000000000000) >> 18));
            d += 4;
            d_len -= 3;
          } else
            goto err_ilseq;
#elif
#error "Wide character literal encoding implementation not found."
#endif
          if (s_len == SIZE_MAX)
            goto err_range;

          s_len++;
          break;
        default:
          goto err_ilseq;
        }
      } else
        *d++ = *s++;

    } while (--s_len != 0 && --d_len != 0);

    if (s_len != 0 || d_len == 0)
      goto err_range;
    else
      d_len--;
  }

  *d_lenp -= d_len;
  return 0;
err_range:
  *d_lenp -= d_len;
  errno = ERANGE;
  return -1;
err_ilseq:
  *d_lenp -= d_len;
  errno = EILSEQ;
  return -1;
}

#ifdef __cplusplus
}
#endif
