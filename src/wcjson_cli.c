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
 * @file wcjson_cli.c
 * @brief wcjson command line interface
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef CLI_DEFAULT_LIMIT
#define CLI_DEFAULT_LIMIT 16384
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "wcjson.h"

extern char *__progname;
int ascii = 0;

static void *object_start_cli(struct wcjson *, void *, void *);
static void object_add_cli(struct wcjson *, void *, void *, void *, void *);
static void object_end_cli(struct wcjson *, void *, void *);
static void *array_start_cli(struct wcjson *, void *, void *);
static void array_add_cli(struct wcjson *, void *, void *, void *);
static void array_end_cli(struct wcjson *, void *, void *);
static void *string_value_cli(struct wcjson *, void *, const wchar_t *,
                              const size_t, const bool);
static void *number_value_cli(struct wcjson *, void *, const wchar_t *,
                              const size_t);
static void *bool_value_cli(struct wcjson *, void *, const bool);
static void *null_value_cli(struct wcjson *, void *);

static struct wcjson_ops cli_ops = {
    .object_start = &object_start_cli,
    .object_add = &object_add_cli,
    .object_end = &object_end_cli,
    .array_start = &array_start_cli,
    .array_add = &array_add_cli,
    .array_end = &array_end_cli,
    .string_value = &string_value_cli,
    .number_value = &number_value_cli,
    .bool_value = &bool_value_cli,
    .null_value = &null_value_cli,
};

enum cli_node_type {
  T_NULL,
  T_BOOLEAN,
  T_STRING,
  T_NUMBER,
  T_OBJECT,
  T_ARRAY,
  T_PAIR,
};

struct cli_node {
  enum cli_node_type type;
  const wchar_t *txt;
  size_t len;
  bool escaped;
  struct cli_node *parent, *value, *first, *last, *next;
  bool flag;
};

struct cli_document {
  struct cli_node *nodes;
  size_t node;
  size_t size;
  struct cli_node *root;
};

static _Noreturn void fail(struct wcjson *ctx) {
  int ret = 3;

  switch (ctx->status) {
  case WCJSON_ABORT_INVALID:
    fputws(L"wcjson: Invalid JSON text\n", stderr);
    ret = 1;
    break;
  case WCJSON_ABORT_END_OF_INPUT:
    fputws(L"wcjson: Unexpected end of input\n", stderr);
    ret = 2;
    break;
  case WCJSON_ABORT_ERROR:
    perror("wcjson");
    break;
  default:
    break;
  }

  exit(ret);
}

static inline struct cli_node *alloc_node(struct wcjson *ctx,
                                          struct cli_document *doc) {
  if (doc->node == doc->size) {
    ctx->status = WCJSON_ABORT_ERROR;
    ctx->errnum = ENOMEM;
    fail(ctx);
  }

  struct cli_node *n = &doc->nodes[doc->node++];
  n->type = T_NULL;
  n->txt = NULL;
  n->len = 0;
  n->escaped = false;
  n->parent = NULL;
  n->value = NULL;
  n->first = NULL;
  n->last = NULL;
  n->next = NULL;
  n->flag = false;

  return n;
}

static void *object_start_cli(struct wcjson *ctx, void *doc, void *node) {
  struct cli_document *cli_doc = doc;
  struct cli_node *cli_obj = alloc_node(ctx, cli_doc);
  cli_obj->type = T_OBJECT;

  if (cli_doc->root == NULL)
    cli_doc->root = cli_obj;

  return cli_obj;
}

static void object_add_cli(struct wcjson *ctx, void *doc, void *node, void *key,
                           void *value) {
  struct cli_document *cli_doc = doc;
  struct cli_node *cli_obj = node;
  struct cli_node *cli_key = key;
  struct cli_node *cli_value = value;
  struct cli_node *cli_pair = alloc_node(ctx, cli_doc);
  cli_pair->type = T_PAIR;
  cli_pair->txt = cli_key->txt;
  cli_pair->len = cli_key->len;
  cli_pair->escaped = cli_key->escaped;
  cli_pair->value = cli_value;

  if (cli_obj->first == NULL) {
    cli_obj->first = cli_pair;
    cli_obj->last = cli_pair;
  } else {
    cli_obj->last->next = cli_pair;
    cli_obj->last = cli_pair;
  }
}

static void object_end_cli(struct wcjson *ctx, void *doc, void *node) {}

static void *array_start_cli(struct wcjson *ctx, void *doc, void *node) {
  struct cli_document *cli_doc = doc;
  struct cli_node *cli_arr = alloc_node(ctx, cli_doc);
  cli_arr->type = T_ARRAY;

  if (cli_doc->root == NULL)
    cli_doc->root = cli_arr;

  return cli_arr;
}

static void array_add_cli(struct wcjson *ctx, void *doc, void *node,
                          void *value) {
  struct cli_node *cli_arr = node;
  struct cli_node *cli_value = value;

  if (cli_arr->first == NULL) {
    cli_arr->first = cli_value;
    cli_arr->last = cli_value;
  } else {
    cli_arr->last->next = cli_value;
    cli_arr->last = cli_value;
  }
}

static void array_end_cli(struct wcjson *ctx, void *doc, void *node) {}

static void *string_value_cli(struct wcjson *ctx, void *doc, const wchar_t *str,
                              const size_t len, const bool escaped) {
  struct cli_document *cli_doc = doc;
  struct cli_node *cli_str = alloc_node(ctx, cli_doc);
  cli_str->type = T_STRING;
  cli_str->txt = str;
  cli_str->len = len;
  cli_str->escaped = escaped;

  if (cli_doc->root == NULL)
    cli_doc->root = cli_str;

  return cli_str;
}

static void *number_value_cli(struct wcjson *ctx, void *doc, const wchar_t *num,
                              const size_t len) {
  struct cli_document *cli_doc = doc;
  struct cli_node *cli_num = alloc_node(ctx, cli_doc);
  cli_num->type = T_NUMBER;
  cli_num->txt = num;
  cli_num->len = len;

  if (cli_doc->root == NULL)
    cli_doc->root = cli_num;

  return cli_num;
}

static void *bool_value_cli(struct wcjson *ctx, void *doc, const bool flg) {
  struct cli_document *cli_doc = doc;
  struct cli_node *cli_bool = alloc_node(ctx, cli_doc);
  cli_bool->type = T_BOOLEAN;
  cli_bool->flag = flg;

  if (cli_doc->root == NULL)
    cli_doc->root = cli_bool;

  return cli_bool;
}

static void *null_value_cli(struct wcjson *ctx, void *doc) {
  struct cli_document *cli_doc = doc;
  struct cli_node *cli_null = alloc_node(ctx, cli_doc);
  cli_null->type = T_NULL;

  if (cli_doc->root == NULL)
    cli_doc->root = cli_null;

  return cli_null;
}

static void print_node(FILE *f, struct wcjson *ctx, struct cli_node *,
                       wchar_t *buf, size_t buf_len);

static void print_null(FILE *f, struct wcjson *ctx) {
  if (fputws(L"null", f) == -1) {
    ctx->status = WCJSON_ABORT_ERROR;
    ctx->errnum = errno;
  }
}

static void print_boolean(FILE *f, struct wcjson *ctx, struct cli_node *node) {
  if (fputws(node->flag ? L"true" : L"false", f) == -1) {
    ctx->status = WCJSON_ABORT_ERROR;
    ctx->errnum = errno;
  }
}

static void print_string(FILE *f, struct wcjson *ctx, struct cli_node *node,
                         wchar_t *buf, size_t buf_len) {
  wchar_t *unesc = buf, *esc = NULL;
  size_t unesc_len = buf_len, esc_len;

  if (wcjsonstowc(node->txt, node->len, unesc, &unesc_len)) {
    if (errno == ERANGE)
      errno = ENOMEM;

    goto err;
  }

  if (unesc_len > buf_len) {
    errno = ENOMEM;
    goto err;
  }

  esc_len = buf_len - unesc_len;
  esc = buf + unesc_len;

  if (ascii) {
    if (wctoascjsons(unesc, unesc_len, esc, &esc_len)) {
      if (errno == ERANGE)
        errno = ENOMEM;

      goto err;
    }
  } else {
    if (wctowcjsons(unesc, unesc_len, esc, &esc_len)) {
      if (errno == ERANGE)
        errno = ENOMEM;

      goto err;
    }
  }

  if (putwc(L'"', f) == WEOF)
    goto err;

  for (size_t i = 0; i < esc_len; i++)
    if (putwc(esc[i], f) == WEOF)
      goto err;

  if (putwc(L'"', f) == WEOF)
    goto err;

  return;
err:
  if (ctx->status == WCJSON_OK) {
    ctx->status = WCJSON_ABORT_ERROR;
    ctx->errnum = errno;
  }
}

static void print_number(FILE *f, struct wcjson *ctx, struct cli_node *node) {
  for (size_t i = 0; i < node->len; i++)
    if (putwc(node->txt[i], f) == WEOF)
      goto err;

  return;
err:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = errno;
}

static void print_object(FILE *f, struct wcjson *ctx, struct cli_node *node,
                         wchar_t *buf, size_t buf_len) {
  if (putwc(L'{', f) == WEOF)
    goto err;

  struct cli_node *n = node->first;

  while (n != NULL) {
    print_node(f, ctx, n, buf, buf_len);

    if (ctx->status != WCJSON_OK)
      return;

    n = n->next;

    if (n != NULL && putwc(L',', f) == WEOF)
      goto err;
  }

  if (putwc(L'}', f) == WEOF)
    goto err;

  return;
err:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = errno;
}

static void print_array(FILE *f, struct wcjson *ctx, struct cli_node *node,
                        wchar_t *buf, size_t buf_len) {
  if (putwc(L'[', f) == WEOF)
    goto err;

  struct cli_node *n = node->first;

  while (n != NULL) {
    print_node(f, ctx, n, buf, buf_len);

    if (ctx->status != WCJSON_OK)
      return;

    n = n->next;

    if (n != NULL && putwc(L',', f) == WEOF)
      goto err;
  }

  if (putwc(L']', f) == WEOF)
    goto err;

  return;
err:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = errno;
}

static void print_pair(FILE *f, struct wcjson *ctx, struct cli_node *node,
                       wchar_t *buf, size_t buf_len) {
  if (putwc(L'"', f) == WEOF)
    goto err;

  for (size_t i = 0; i < node->len; i++)
    if (putwc(node->txt[i], f) == WEOF)
      goto err;

  if (putwc(L'"', f) == WEOF)
    goto err;

  if (putwc(L':', f) == WEOF)
    goto err;

  print_node(f, ctx, node->value, buf, buf_len);
  return;
err:
  ctx->status = WCJSON_ABORT_ERROR;
  ctx->errnum = errno;
}

void print_node(FILE *f, struct wcjson *ctx, struct cli_node *node,
                wchar_t *buf, size_t buf_len) {
  if (node != NULL)
    switch (node->type) {
    case T_NULL:
      print_null(f, ctx);
      break;
    case T_BOOLEAN:
      print_boolean(f, ctx, node);
      break;
    case T_STRING:
      print_string(f, ctx, node, buf, buf_len);
      break;
    case T_NUMBER:
      print_number(f, ctx, node);
      break;
    case T_OBJECT:
      print_object(f, ctx, node, buf, buf_len);
      break;
    case T_ARRAY:
      print_array(f, ctx, node, buf, buf_len);
      break;
    case T_PAIR:
      print_pair(f, ctx, node, buf, buf_len);
      break;
    default:
      ctx->status = WCJSON_ABORT_ERROR;
      ctx->errnum = errno;
    }
}

static _Noreturn void usage(void) {
  fprintf(
      stderr,
      "usage: %s [-i file] [-o file] [-d locale] [-e locale] [-a] [-m bytes]\n",
      __progname);
  exit(3);
}

int main(int argc, char *argv[]) {
  int ch;
  char *i = NULL, *o = NULL, *d = NULL, *e = NULL, *ep = NULL;
  size_t limit = CLI_DEFAULT_LIMIT, len, node_cnt, buf_len;
  FILE *in = stdin, *out = stdout;
  void *mem = NULL;
  wint_t wc;

  struct wcjson ctx = {
      .status = WCJSON_OK,
  };

  struct cli_document doc = {
      .nodes = NULL,
  };

  while ((ch = getopt(argc, argv, "i:o:d:e:m:a")) != -1) {
    switch (ch) {
    case 'i':
      i = optarg;
      break;
    case 'o':
      o = optarg;
      break;
    case 'd':
#ifdef HAVE_SETLOCALE
      d = optarg;
      break;
#else
      fputws(L"wcjson: setlocale: -d not supported on this platform\n", stderr);
      exit(3);
#endif
      break;
    case 'e':
#ifdef HAVE_SETLOCALE
      e = optarg;
      break;
#else
      fputws(L"wcjson: setlocale: -e not supported on this platform\n" stderr);
      exit(3);
#endif
    case 'm':
      errno = 0;
      const long long m = strtoll(optarg, &ep, 0);

      if (errno != 0) {
        perror(optarg);
        usage();
      }

      if (ep[0] != '\0' || m <= 0)
        usage();

      limit = m;
      break;
    case 'a':
      ascii = 1;
      break;
    default:
      usage();
    }
  }
  argc -= optind;
  argv += optind;

#ifdef HAVE_SETLOCALE
  if (setlocale(LC_CTYPE, d != NULL ? d : "") == NULL) {
    errno = EINVAL;
    goto err;
  }
#endif

  if (i != NULL)
    in = fopen(i, "r");

  if (in == NULL)
    goto err;

  mem = calloc(sizeof(char), limit);
  if (mem == NULL)
    goto err;

  for (len = 0, wc = getwc(in); wc != WEOF; wc = getwc(in)) {
    ((wchar_t *)mem)[len++] = (wchar_t)wc;

    if (sizeof(wchar_t) * len >= limit) {
      errno = ENOMEM;
      goto err;
    }
  }

  if (ferror(in))
    goto err;

  limit -= sizeof(wchar_t) * len;
  node_cnt = limit / sizeof(struct cli_node);

  if (node_cnt == 0) {
    errno = ENOMEM;
    goto err;
  }

  doc.nodes = (struct cli_node *)&((wchar_t *)mem)[len];
  doc.node = 0;
  doc.size = node_cnt;
  doc.root = NULL;

  if (wcjson(&ctx, &cli_ops, &doc, mem, len))
    goto err;

  if (++doc.node == doc.size) {
    errno = ENOMEM;
    goto err;
  }

  if (sizeof(struct cli_node) * doc.node >= limit) {
    errno = ENOMEM;
    goto err;
  }

  limit -= sizeof(struct cli_node) * doc.node;
  buf_len = limit / sizeof(wchar_t);

  if (buf_len == 0) {
    errno = ENOMEM;
    goto err;
  }

#ifdef HAVE_SETLOCALE
  if (setlocale(LC_CTYPE, e != NULL ? e : "") == NULL) {
    errno = EINVAL;
    goto err;
  }
#endif

  if (o != NULL)
    out = fopen(o, "w");

  if (out == NULL)
    goto err;

  print_node(out, &ctx, doc.root, (wchar_t *)&doc.nodes[doc.node], buf_len);

  if (ferror(out))
    goto err;

  if (ctx.status != WCJSON_OK)
    goto err;

  free(mem);
  fclose(in);
  fclose(out);
  return 0;
err:
  if (ctx.status == WCJSON_OK) {
    ctx.status = WCJSON_ABORT_ERROR;
    ctx.errnum = errno;
  }

  free(mem);
  fclose(in);
  fclose(out);
  fail(&ctx);
}

#ifdef __cplusplus
}
#endif
