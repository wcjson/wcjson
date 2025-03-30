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
 * @file wcjson-document.h
 * @brief Document C API header file.
 */
#ifndef WCJSON_DOCUMENT_H
#define WCJSON_DOCUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <wchar.h>

#include "wcjson.h"

/** JSON value. */
struct wcjson_value {
  unsigned is_null : 1;
  unsigned is_boolean : 1;
  unsigned is_true : 1;
  unsigned is_string : 1;
  unsigned is_number : 1;
  unsigned is_object : 1;
  unsigned is_array : 1;
  unsigned is_pair : 1;
  const wchar_t *string;
  size_t s_len;
  size_t idx;
  size_t head_idx;
  size_t tail_idx;
  size_t prev_idx;
  size_t next_idx;
};

/** JSON document. */
struct wcjson_document {
  struct wcjson_value *values;
  size_t v_nitems;
  size_t v_idx;
  wchar_t *strings;
  size_t s_nitems;
  size_t s_idx;
  wchar_t *esc;
  size_t e_nitems;
};

/**
 + Gets the next unused value from a document.
 * @param doc The document to get the next unsed value from.
 * @return An uninitialized value or NULL if the document cannot provide more
 * values.
 */
struct wcjson_value *wcjson_document_nextv(struct wcjson_document *doc);

/**
 * Accessor macro to get the head JSON value from a list of JSON values.
 * @param d The document to query for values.
 * @param v The value to query.
 */
#define wcjson_value_head(d, v)                                                \
  ((v)->head_idx == 0 ? NULL : &(d)->values[(v)->head_idx])

/**
 * Accessor macro to get the next JSON value from a list of JSON values.
 * @param d The document to query for values.
 * @param v The value to query.
 */
#define wcjson_value_next(d, v)                                                \
  ((v)->next_idx == 0 ? NULL : &(d)->values[(v)->next_idx])

/**
 * Accessor macro to get the tail JSON value from a list of JSON values.
 * @param d The document to query for values.
 * @param v The value to query.
 */
#define wcjson_value_tail(d, v)                                                \
  ((v)->tail_idx == 0 ? NULL : &(d)->values[(v)->tail_idx])

/**
 * Accessor macro to get the previous JSON value from a list of JSON values.
 * @param d The document to query for values.
 * @param v The value to query.
 */
#define wcjson_value_prev(d, v)                                                \
  ((v)->prev_idx == 0 ? NULL : &(d)->values[(v)->prev_idx])

/**
 * Iterator macro to iterate a list of JSON values.
 * @param lval The lvalue to assing JSON values to.
 * @param d The document to query for values.
 * @param v The value to iterate.
 */
#define wcjson_value_foreach(lval, d, v)                                       \
  for ((lval) = wcjson_value_head((d), (v)); (lval) != NULL;                   \
       (lval) = wcjson_value_next((d), (lval)))

/**
 * Accessor function to get the JSON value for a given key from a given object.
 * @param doc The document to query.
 * @param obj The JSON object to query.
 * @param key The key to query that object for.
 * @return The value for key of o or NULL if no such value is found.
 */
struct wcjson_value *wcjson_value_pair(const struct wcjson_document *doc,
                                       const struct wcjson_value *obj,
                                       const wchar_t *key);

/**
 * Adds a value to the head of the child value list of an array.
 * @param doc The document containing the array and the value.
 * @param arr The array to add a value to.
 * @param val The value to add.
 * @return 0 on success or <0 if adding fails.
 */
int wcjson_value_arraddhead(struct wcjson_document *doc,
                            struct wcjson_value *arr, struct wcjson_value *val);

/**
 * Adds a value to the tail of the child value list of an array.
 * @param doc The document containing the array and the value.
 * @param arr The array to add a value to.
 * @param val The value to add.
 * @return 0 on success or <0 if adding fails.
 */
int wcjson_value_arraddtail(struct wcjson_document *doc,
                            struct wcjson_value *arr, struct wcjson_value *val);

/**
 * Removes a value from the child value list of an array.
 * @param doc The document containing the array and the value.
 * @param arr The array to remove a value from.
 * @param val The value to remove.
 * @return 0 on success or <0 if removing fails.
 */
int wcjson_value_arremove(struct wcjson_document *doc, struct wcjson_value *arr,
                          struct wcjson_value *val);

/**
 * Adds a key value pair the head of the child value list of an object.
 * @param doc The document containing the object and the value.
 * @param obj The object to add a value to.
 * @param pair The pair to add.
 * @return 0 on success or <0 if adding fails.
 */
int wcjson_value_objaddhead(struct wcjson_document *doc,
                            struct wcjson_value *obj,
                            struct wcjson_value *pair);

/**
 * Adds a key value pair to the tail of the child value list of an object.
 * @param doc The document holding the object and the value.
 * @param obj The object to add a value to.
 * @param pair The pair to add.
 * @return 0 on success or <0 if adding fails.
 */
int wcjson_value_objaddtail(struct wcjson_document *doc,
                            struct wcjson_value *obj, struct wcjson_value *air);

/**
 * Removes a pair from the child value list of an object.
 * @param doc The document containing the object and the pair.
 * @param obj The object to remove a pair from.
 * @param pair The pair to remove.
 * @return 0 on success of <0 if removing fails.
 */
int wcjson_value_objremove(struct wcjson_document *doc,
                           struct wcjson_value *obj, struct wcjson_value *pair);

/**
 * Deserializes JSON text to populate a document.
 * @param ctx Library context.
 * @param doc Document to populate.
 * @param txt JSON text to deserialize.
 * @param len Number of characters to deserialize.
 * @return 0 on success, <0 on failure.
 */
int wcjsondocvalues(struct wcjson *ctx, struct wcjson_document *doc,
                    const wchar_t *txt, const size_t len);

/**
 * Decodes any values in a document by unapplying JSON escaping rules and
 * adding terminating zero characters.
 * @param ctx Library context.
 * @param doc Document to decode.
 * @return 0 on success, <0 on failure.
 */
int wcjsondocstrings(struct wcjson *ctx, struct wcjson_document *doc);

/**
 * Serializes a JSON document to a wide character file.
 * @param f The file to serialize to.
 * @param doc The document to serialize.
 * @param value The value to serialize.
 * @return 0 on success, <0 on failure.
 */
int wcjsondocfprint(FILE *f, const struct wcjson_document *doc,
                    const struct wcjson_value *value);

/**
 * Serializes a JSON document to a 7 bit ASCII file.
 * @param f The file to serialize to.
 * @param doc The document to serialize.
 * @param value The value to serialize.
 * @return 0 on success, <0 on failure.
 */
int wcjsondocfprintasc(FILE *f, const struct wcjson_document *doc,
                       const struct wcjson_value *value);

/**
 * Serializes a JSON document to a wide character string.
 * @param s The string to serialize to.
 * @param lenp Pointer to the number of characters available in s, the number of
 * characters written to s on return.
 * @param doc The document to serialize.
 * @param value The value to serialize.
 * @return 0 on success, <0 on failure.
 */
int wcjsondocsprint(wchar_t *s, size_t *lenp, const struct wcjson_document *doc,
                    const struct wcjson_value *value);

/**
 * Serializes a JSON document to a 7 bit ASCII string.
 * @param s The string to serialize to.
 * @param lenp Pointer to the number of characters available in s, the number of
 * characters written to s on return.
 * @param doc The document to serialize.
 * @param value The value to serialize.
 * @return 0 on success, <0 on failure.
 */
int wcjsondocsprintasc(wchar_t *s, size_t *lenp,
                       const struct wcjson_document *doc,
                       const struct wcjson_value *value);

#ifdef __cplusplus
}
#endif
#endif
