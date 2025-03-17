# WCJSON(1) - General Commands Manual

## NAME

**wcjson** - transcode JSON text

## SYNOPSIS

**wcjson**
\[**-i**&nbsp;*file*]
\[**-d**&nbsp;*locale*]
\[**-o**&nbsp;*file*]
\[**-e**&nbsp;*locale*]
\[**-a**]
\[**-m**&nbsp;*bytes*]

## DESCRIPTION

The
**wcjson**
utility transcodes JSON text.

The options are as follows:

**-i** *file*

> Input file to read JSON text from. Defaults to standard input.

**-d** *locale*

> Character encoding to use for decoding JSON text. See
> *ENVIRONMENT*.

**-o** *file*

> Output file to write JSON text to. Defaults to standard output.

**-e** *locale*

> Character encoding to use for encoding JSON text. See
> *ENVIRONMENT*.

**-a**

> Flag indicating to encode JSON text for 7bit ASCII compatibility (e.g. when
> using
> **-e**
> C).

**-m** *bytes*

> Maximum amount of memory the utility is allowed to allocate.

## EXIT STATUS

The **wcjson** utility exits&#160;0 on success, and&#160;&gt;0 if an error occurs.

## ENVIRONMENT

`LC_CTYPE`

> Character encoding used for decoding and encoding JSON text by default.

## SEE ALSO

[locale(1)](https://man.openbsd.org/locale)
wcjson(3)

# WCJSON(3) - Library Functions Manual

## NAME

**wcjson**,
**wctowcjsons**,
**wctoascjsons**,
**wcjsonstowc** - wide character JSON

## SYNOPSIS

**#include &lt;[wcjson.h](src/wcjson.h)>**

*int*  
**wcjson**(*struct wcjson \*ctx*, *const struct wcjson\_ops \*ops*, *void \*document*, *const wchar\_t \*text*, *const size\_t len*);

*int*  
**wctowcjsons**(*const wchar\_t \*s*, *size\_t s\_len*, *wchar\_t \*d*, *size\_t \*d\_lenp*);

*int*  
**wctoascjsons**(*const wchar\_t \*s*, *size\_t s\_len*, *wchar\_t \*d*, *size\_t \*d\_lenp*);

*int*  
**wcjsonstowc**(*const wchar\_t \*s*, *size\_t s\_len*, *wchar\_t \*d*, *size\_t \*d\_lenp*);

## DESCRIPTION

The
**wcjson**()
function deserializes
*len*
characters JSON
*text*
by passing
*ctx*
and
*document*
to callback functions
*ops*.

The (a)
**wctowcjsons**()
and (b)
**wctoascjsons**()
functions encode
*s\_len*
characters from
*s*
to
*d*
capable of storing
*\*d\_lenp*
characters by applying JSON escaping rules (a) for all characters requiring escaping
according to the JSON grammar or additionally (b) for all characters not compatible with
7bit ASCII. To ensure the destination
*d*
is capable of storing the complete encoded representation of
*s*
the
`WCJSON_ESCAPE_MAX`
constant can be used such that \*d\_lenp &gt;= s\_len \*
`WCJSON_ESCAPE_MAX`.

The
**wcjsonstowc**()
function decodes
*s\_len*
characters from
*s*
to
*d*
capable of storing
*\*d\_lenp*
characters by unapplying JSON escaping rules. To ensure the destination
*d*
is capable of storing the complete decoded representation of
*s*
that destination needs to be capable of storing at least
*s\_len*
characters such that \*d\_lenp &gt;= s\_len.

The
**wctowcjsons**(),
**wctoascjsons**()
and
**wcjsonstowc**()
functions set
*\*d\_lenp*
to the number of characters written to
*d*
on return.

## RETURN VALUES

The functions return 0 on success, or a negative value if a deserialization, decoding or
encoding error occurs.
The
**wctowcjsons**(),
**wctoascjsons**()
and
**wcjsonstowc**()
functions set the global variable
*errno*
to indicate the error. The
**wcjson**()
function provides status via
*ctx*.

## ERRORS

\[`ERANGE`]

> The size
> *d\_len*
> of the destination
> *d*
> was too small.

\[`EILSEQ`]

> The source
> *s*
> cointained invalid data.

## STANDARDS

T. Bray, Ed.,
*The JavaScript Object Notation (JSON) Data Interchange Format*,
[RFC 8259](https://www.rfc-editor.org/info/rfc8259),
December 2017.

J. Klensin,
*ASCII Escaping of Unicode Characters*,
[RFC 5137](https://www.rfc-editor.org/info/rfc5137),
February 2008.

F. Yergeau,
*UTF-8, a transformation format of ISO 10646*,
[RFC 3629](https://www.rfc-editor.org/info/rfc3629),
November 2003.

P. Hoffman,
F. Yergeau,
*UTF-16, an encoding of ISO 10646*,
[RFC 2781](https://www.rfc-editor.org/info/rfc2781),
February 2000.
