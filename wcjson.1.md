WCJSON(1) - General Commands Manual

# NAME

**wcjson** - transcode JSON text

# SYNOPSIS

**wcjson**
\[**-i**&nbsp;*file*]
\[**-d**&nbsp;*locale*]
\[**-o**&nbsp;*file*]
\[**-e**&nbsp;*locale*]
\[**-a**]
\[**-r**]
\[**-m**&nbsp;*bytes*]

# DESCRIPTION

The
**wcjson**
utility transcodes JSON text.

The options are as follows:

**-i** *file*

> Input file to read JSON text from.
> Defaults to standard input.

**-d** *locale*

> Character encoding to use for decoding JSON text.
> See
> *ENVIRONMENT*.

**-o** *file*

> Output file to write JSON text to.
> Defaults to standard output.

**-e** *locale*

> Character encoding to use for encoding JSON text.
> See
> *ENVIRONMENT*.

**-a**

> Flag indicating to encode JSON text for 7bit ASCII compatibility (e.g. when
> using
> **-e**
> C).

**-r**

> Flag indicating to write statistics information to the standard output
> instead of writing JSON text.

**-m** *bytes*

> Maximum amount of memory the utility is allowed to allocate.

# ENVIRONMENT

`LC_CTYPE`

> Character encoding used for decoding and encoding JSON text by default.

# EXIT STATUS

The **wcjson** utility exits&#160;0 on success, and&#160;&gt;0 if an error occurs.

# SEE ALSO

[locale(1)](https://man.openbsd.org/locale),
wcjson(3)

# STANDARDS

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
