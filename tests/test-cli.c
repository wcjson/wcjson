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
 * @file test-cli.c
 * @brief Testsuite.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "wcjson-document.h"

static int test_create(int argc, char *argv[]);
static int test_add(int argc, char *argv[]);
static int test_remove(int argc, char *argv[]);

#define nitems(a) (sizeof((a)) / sizeof((a)[0]))

struct {
  const char *name;
  int (*test)(int argc, char *argv[]);
} testsuite[] = {
    {
        .name = "create",
        .test = test_create,
    },
    {
        .name = "add",
        .test = test_add,
    },
    {
        .name = "remove",
        .test = test_remove,
    },
};

static int doc_create(struct wcjson_document *doc) {
  struct wcjson_value *obj = wcjsondoc_create_object(doc);
  if (obj == NULL)
    return -1;

  struct wcjson_value *arr = wcjsondoc_create_array(doc);
  if (arr == NULL)
    return -1;

  struct wcjson_value *n = wcjsondoc_create_null(doc);
  if (n == NULL)
    return -1;

  if (wcjsondoc_array_add_head(doc, arr, n) < 0)
    return -1;

  if (wcjsondoc_object_add_head(doc, obj, L"key", arr) < 0)
    return -1;

  return 0;
}

static int doc_add(struct wcjson_document *doc) {
  if (doc_create(doc) < 0)
    return -1;

  struct wcjson_value *t1 = wcjsondoc_create_string(doc, L"abc");
  if (t1 == NULL)
    return -1;

  struct wcjson_value *t2 = wcjsondoc_create_number(doc, L"123");
  if (t2 == NULL)
    return -1;

  struct wcjson_value *t3 = wcjsondoc_create_string(doc, L"def");
  if (t3 == NULL)
    return -1;

  struct wcjson_value *t4 = wcjsondoc_create_number(doc, L"456");
  if (t4 == NULL)
    return -1;

  struct wcjson_value *arr = wcjsondoc_object_get(doc, doc->values, L"key");
  if (arr == NULL)
    return -1;

  if (wcjsondoc_array_add_head(doc, arr, t1) < 0)
    return -1;

  if (wcjsondoc_array_add_tail(doc, arr, t2) < 0)
    return -1;

  if (wcjsondoc_object_add_head(doc, doc->values, L"key1", t3) < 0)
    return -1;

  if (wcjsondoc_object_add_tail(doc, doc->values, L"key2", t4) < 0)
    return -1;

  return 0;
}

static int test_create(int argc, char *argv[]) {
  struct wcjson_value values[4];
  wchar_t strings[4];
  wchar_t esc[4 * WCJSON_ESCAPE_MAX];
  struct wcjson_document doc = {
      .values = values,
      .v_nitems = nitems(values),
      .strings = strings,
      .s_nitems = nitems(strings),
      .esc = esc,
      .e_nitems = nitems(esc),
  };
  struct wcjson ctx = {
      .status = WCJSON_OK,
  };

  if (doc_create(&doc) < 0)
    return -1;

  if (wcjsondocstrings(&ctx, &doc) < 0)
    return -1;

  if (wcjsondocfprint(stdout, &doc, doc.values) < 0)
    return -1;

  return 0;
}

static int test_add(int argc, char *argv[]) {
  struct wcjson_value values[10];
  wchar_t strings[30];
  wchar_t esc[5 * WCJSON_ESCAPE_MAX];
  struct wcjson_document doc = {
      .values = values,
      .v_nitems = nitems(values),
      .strings = strings,
      .s_nitems = nitems(strings),
      .esc = esc,
      .e_nitems = nitems(esc),
  };
  struct wcjson ctx = {
      .status = WCJSON_OK,
  };

  if (doc_add(&doc) < 0)
    return -1;

  if (wcjsondocstrings(&ctx, &doc) < 0)
    return -1;

  if (wcjsondocfprint(stdout, &doc, doc.values) < 0)
    return -1;

  return 0;
}

static int test_remove(int argc, char *argv[]) {
  struct wcjson_value values[10];
  wchar_t strings[30];
  wchar_t esc[5 * WCJSON_ESCAPE_MAX];
  struct wcjson_document doc = {
      .values = values,
      .v_nitems = nitems(values),
      .strings = strings,
      .s_nitems = nitems(strings),
      .esc = esc,
      .e_nitems = nitems(esc),
  };
  struct wcjson ctx = {
      .status = WCJSON_OK,
  };

  if (doc_add(&doc) < 0)
    return -1;

  struct wcjson_value *arr = wcjsondoc_object_get(&doc, doc.values, L"key");
  if (arr == NULL)
    return -1;

  if (wcjsondoc_array_remove(&doc, arr, 1) == NULL)
    return -1;

  if (wcjsondoc_object_remove(&doc, doc.values, L"key2") == NULL)
    return -1;

  if (wcjsondocstrings(&ctx, &doc) < 0)
    return -1;

  if (wcjsondocfprint(stdout, &doc, doc.values) < 0)
    return -1;

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    return EXIT_FAILURE;

  for (size_t i = 0; i < nitems(testsuite); i++)
    if (strcmp(testsuite[i].name, argv[1]) == 0)
      return testsuite[i].test(argc, argv);

  return 1;
}
