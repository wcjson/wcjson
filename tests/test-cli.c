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
  struct wcjson_value *obj = wcjson_value_object(doc);
  if (obj == NULL)
    return -1;

  struct wcjson_value *arr = wcjson_value_array(doc);
  if (arr == NULL)
    return -1;

  struct wcjson_value *n = wcjson_value_null(doc);
  if (n == NULL)
    return -1;

  if (wcjson_array_add_head(doc, arr, n) < 0)
    return -1;

  if (wcjson_object_add_head(doc, obj, L"key", 3, arr) < 0)
    return -1;

  return 0;
}

static int doc_add(struct wcjson_document *doc) {
  if (doc_create(doc) < 0)
    return -1;

  struct wcjson_value *t1 = wcjson_value_string(doc, L"abc", 3);
  if (t1 == NULL)
    return -1;

  struct wcjson_value *t2 = wcjson_value_number(doc, L"123", 3);
  if (t2 == NULL)
    return -1;

  struct wcjson_value *t3 = wcjson_value_string(doc, L"def", 3);
  if (t3 == NULL)
    return -1;

  struct wcjson_value *t4 = wcjson_value_number(doc, L"456", 3);
  if (t4 == NULL)
    return -1;

  struct wcjson_value *arr = wcjson_object_get(doc, doc->values, L"key", 3);
  if (arr == NULL)
    return -1;

  if (wcjson_array_add_head(doc, arr, t1) < 0)
    return -1;

  if (wcjson_array_add_tail(doc, arr, t2) < 0)
    return -1;

  if (wcjson_object_add_head(doc, doc->values, L"key1", 4, t3) < 0)
    return -1;

  if (wcjson_object_add_tail(doc, doc->values, L"key2", 4, t4) < 0)
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
      .v_next = 0,
      .strings = strings,
      .s_nitems = nitems(strings),
      .s_next = 0,
      .esc = esc,
      .e_nitems = nitems(esc),
  };

  struct wcjson ctx = WCJSON_INITIALIZER;

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
      .v_next = 0,
      .strings = strings,
      .s_nitems = nitems(strings),
      .s_next = 0,
      .esc = esc,
      .e_nitems = nitems(esc),
  };

  struct wcjson ctx = WCJSON_INITIALIZER;

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
      .v_next = 0,
      .strings = strings,
      .s_nitems = nitems(strings),
      .s_next = 0,
      .esc = esc,
      .e_nitems = nitems(esc),
  };

  struct wcjson ctx = WCJSON_INITIALIZER;

  if (doc_add(&doc) < 0)
    return -1;

  struct wcjson_value *arr = wcjson_object_get(&doc, doc.values, L"key", 3);
  if (arr == NULL)
    return -1;

  if (wcjson_array_remove(&doc, arr, 1) == NULL)
    return -1;

  if (wcjson_object_remove(&doc, doc.values, L"key2", 4) == NULL)
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
