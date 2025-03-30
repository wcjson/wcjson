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

#include "wcjson-document.h"
#include "wcjson.h"

extern char *__progname;
int ascii = 0;
int report = 0;

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
    errno = ctx->errnum;
    perror("wcjson");
    break;
  default:
    break;
  }

  exit(ret);
}

static _Noreturn void usage(void) {
  fprintf(stderr,
          "usage: %s [-i file] [-o file] [-d locale] [-e locale] [-a] [-r] [-m "
          "bytes]\n",
          __progname);
  exit(3);
}

int main(int argc, char *argv[]) {
  int ch;
  char *i = NULL, *o = NULL, *d = NULL, *e = NULL, *ep = NULL;
  size_t limit = CLI_DEFAULT_LIMIT, len, total_bytes = 0;
  FILE *in = stdin, *out = stdout;
  wchar_t *json = NULL, *strings = NULL, *esc = NULL, *outb = NULL;
  struct wcjson_value *values = NULL;
  void *p;
  wint_t wc;
#ifdef HAVE_SETLOCALE
  char *locale;
#endif

  struct wcjson ctx = {
      .status = WCJSON_OK,
  };

  while ((ch = getopt(argc, argv, "i:o:d:e:m:ar")) != -1) {
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
      fputws(L"wcjson: setlocale: -e not supported on this platform\n", stderr);
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
    case 'r':
      report = 1;
      break;
    default:
      usage();
    }
  }
  argc -= optind;
  argv += optind;

#ifdef HAVE_SETLOCALE
  locale = setlocale(LC_CTYPE, d != NULL ? d : "");
  if (locale == NULL) {
    errno = EINVAL;
    goto err;
  }
  if (report)
    fwprintf(stdout, L"Input locale: %s\n", locale);
#endif

  if (i != NULL)
    in = fopen(i, "r");

  if (in == NULL)
    goto err;

  size_t json_len = limit / sizeof(wchar_t);
  if (json_len == 0) {
    errno = ENOMEM;
    goto err;
  }

  json = malloc(json_len * sizeof(wchar_t));
  if (json == NULL)
    goto err;

  for (len = 0, wc = getwc(in); len < json_len && wc != WEOF; wc = getwc(in))
    json[len++] = wc;

  if (ferror(in))
    goto err;

  if (wc != WEOF) {
    errno = ENOMEM;
    goto err;
  }

  p = realloc(json, sizeof(wchar_t) * len);
  if (p == NULL)
    goto err;

  json = p;

  if (report) {
    total_bytes += len * sizeof(wchar_t);
    fwprintf(stdout, L"Input characters: %lld\n", len);
    fwprintf(stdout, L"Input characters (byte): %lld\n", len * sizeof(wchar_t));
  }

  limit -= sizeof(wchar_t) * len;

  size_t v_nitems = limit / sizeof(struct wcjson_value);
  if (v_nitems == 0) {
    errno = ENOMEM;
    goto err;
  }

  values = malloc(v_nitems * sizeof(struct wcjson_value));
  if (values == NULL)
    goto err;

  struct wcjson_document doc = {
      .values = values,
      .v_nitems = v_nitems,
  };

  if (wcjsondocvalues(&ctx, &doc, json, len) < 0) {
    if (ctx.errnum == ERANGE)
      ctx.errnum = ENOMEM;

    goto err;
  }

  p = realloc(values, sizeof(struct wcjson_value) * doc.v_idx);
  if (p == NULL)
    goto err;

  values = p;
  doc.values = values;
  doc.v_nitems = doc.v_idx;

  if (report) {
    total_bytes += doc.v_nitems * sizeof(struct wcjson_value);
    fwprintf(stdout, L"Input values: %lld\n", doc.v_nitems);
    fwprintf(stdout, L"Input values (byte): %lld\n",
             doc.v_nitems * sizeof(struct wcjson_value));
  }

  limit -= sizeof(struct wcjson_value) * doc.v_nitems;

  size_t s_nitems = limit / sizeof(wchar_t);
  if (s_nitems < doc.s_nitems) {
    errno = ENOMEM;
    goto err;
  }

  strings = malloc(doc.s_nitems * sizeof(wchar_t));
  if (strings == NULL)
    goto err;

  doc.strings = strings;

  if (wcjsondocstrings(&ctx, &doc) < 0)
    goto err;

  if (report) {
    total_bytes += doc.s_nitems * sizeof(wchar_t);
    fwprintf(stdout, L"Input string characters: %lld\n", doc.s_nitems);
    fwprintf(stdout, L"Input string characters (byte): %lld\n",
             doc.s_nitems * sizeof(wchar_t));
  }

  limit -= doc.s_nitems * sizeof(wchar_t);

  size_t e_nitems = limit / sizeof(wchar_t);
  if (e_nitems < doc.e_nitems) {
    errno = ENOMEM;
    goto err;
  }

  esc = malloc(doc.e_nitems * sizeof(wchar_t));
  if (esc == NULL)
    goto err;

  doc.esc = esc;

  if (report) {
    total_bytes += doc.e_nitems * sizeof(wchar_t);
    fwprintf(stdout, L"Escape sequences characters: %lld\n", doc.e_nitems);
    fwprintf(stdout, L"Escape sequences characters (byte): %lld\n",
             doc.e_nitems * sizeof(wchar_t));
  }

#ifdef HAVE_SETLOCALE
  locale = setlocale(LC_CTYPE, e != NULL ? e : "");
  if (locale == NULL) {
    errno = EINVAL;
    goto err;
  }
  if (report)
    fwprintf(stdout, L"Output locale: %s\n", locale);
#endif

  if (o != NULL && !report && (out = fopen(o, "w")) == NULL)
    goto err;

  limit -= doc.e_nitems * sizeof(wchar_t);

  size_t o_nitems = limit / sizeof(wchar_t);
  if (o_nitems == 0) {
    errno = ENOMEM;
    goto err;
  }

  if (report || o_nitems > len * WCJSON_ESCAPE_MAX) {
    outb = malloc(o_nitems * sizeof(wchar_t));
    if (outb == NULL)
      goto err;

    if (ascii) {
      if (wcjsondocsprintasc(outb, &o_nitems, &doc, doc.values) < 0) {
        if (errno == ERANGE)
          errno = ENOMEM;

        goto err;
      }
    } else {
      if (wcjsondocsprint(outb, &o_nitems, &doc, doc.values) < 0) {
        if (errno == ERANGE)
          errno = ENOMEM;

        goto err;
      }
    }

    if (report) {
      total_bytes += o_nitems * sizeof(wchar_t);
      fwprintf(stdout, L"Output characters: %lld\n", o_nitems);
      fwprintf(stdout, L"Output characters (byte): %lld\n",
               o_nitems * sizeof(wchar_t));
    } else if (fputws(outb, out) == -1)
      goto err;

  } else {
    if (!report) {
      if (ascii) {
        if (wcjsondocfprintasc(out, &doc, doc.values) < 0)
          goto err;
      } else {
        if (wcjsondocfprint(out, &doc, doc.values) < 0)
          goto err;
      }
    } else {
      fwprintf(stdout, L"Output characters: n/a\n");
      fwprintf(stdout, L"Output characters (byte): n/a\n");
    }
  }

  if (ferror(out))
    goto err;

  if (ctx.status != WCJSON_OK)
    goto err;

  if (report)
    fwprintf(stdout, L"Total bytes: %lld\n", total_bytes);

  free(json);
  free(values);
  free(strings);
  free(esc);
  free(outb);
  fclose(in);
  fclose(out);
  return 0;
err:
  if (ctx.status == WCJSON_OK) {
    ctx.status = WCJSON_ABORT_ERROR;
    ctx.errnum = errno;
  }

  free(json);
  free(values);
  free(strings);
  free(esc);
  free(outb);
  fclose(in);
  fclose(out);
  fail(&ctx);
}

#ifdef __cplusplus
}
#endif
