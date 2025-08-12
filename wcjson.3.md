# WCJSON(3) - Library Functions Manual

## NAME

**wcjson**,
**wctowcjsons**,
**wctoascjsons**,
**wcjsonstowc**,
**WCJSON\_INITIALIZER** - wide character JSON

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
characters by applying JSON escaping rules (a) for all characters requiring
escaping according to the JSON grammar or additionally (b) for all characters
not compatible with 7bit ASCII.
To ensure the destination
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
characters by unapplying JSON escaping rules.
To ensure the destination
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

The
**WCJSON\_INITIALIZER**()
macro expands to a rvalue expression for initializing a
*wcjson*
structure.

## RETURN VALUES

The functions return 0 on success or a negative value if a deserialization,
decoding or encoding error occurs.
The
**wctowcjsons**(),
**wctoascjsons**()
and
**wcjsonstowc**()
functions set the global variable
*errno*
to indicate the error.
The
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
[RFC 8259](http://www.rfc-editor.org/rfc/rfc8259.html),
December 2017.

J. Klensin,
*ASCII Escaping of Unicode Characters*,
[RFC 5137](http://www.rfc-editor.org/rfc/rfc5137.html),
February 2008.

F. Yergeau,
*UTF-8, a transformation format of ISO 10646*,
[RFC 3629](http://www.rfc-editor.org/rfc/rfc3629.html),
November 2003.

P. Hoffman,
F. Yergeau,
*UTF-16, an encoding of ISO 10646*,
[RFC 2781](http://www.rfc-editor.org/rfc/rfc2781.html),
February 2000.
