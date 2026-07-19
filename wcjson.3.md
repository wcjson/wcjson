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

The
*wcjson*
structure is defined as follows:

	struct wcjson {
		enum wcjson_status status;
		int errnum;
	};

The elements of this structure are defined as follows:

*status*

> Constant indicating status of an operation.

> `WCJSON_OK`

> > An operation completed successfully.

> `WCJSON_ABORT_ERROR`

> > An operation aborted due to an error.
> > The
> > *errnum*
> > member is describing the error.

> `WCJSON_ABORT_INVALID`

> > An operation aborted due to invalid JSON text.

> `WCJSON_ABORT_END_OF_INPUT`

> > An operation aborted due to an unexpected end of input.

*errnum*

> Code describing the error in case of
> `WCJSON_ABORT_ERROR`.

The
`WCJSON_INITIALIZER`
macro expands to a rvalue expression initializing a
*wcjson*
structure.

The
*wcjson\_ops*
structure is defined as follows:

	struct wcjson_ops {
		void *(*object_start)(struct wcjson *ctx, void *doc,
		    void *parent);
	
		void (*object_add)(struct wcjson *ctx, void *doc,
		    void *obj, void *key,
	
		void (*object_end)(struct wcjson *ctx, void *doc,
		    void *obj);
	
		void *(*array_start)(struct wcjson *ctx, void *doc,
		    void *parent);
	
		void (*array_add)(struct wcjson *ctx, void *doc,
		    void *arr, void *value);
	
		void (*array_end)(struct wcjson *ctx, void *doc,
		    void *arr);
	
		void *(*string_value)(struct wcjson *ctx, void *doc,
		    const wchar_t *str, const size_t len,
		    const bool escaped);
	
		void *(*number_value)(struct wcjson *ctx, void *doc,
		    const wchar_t *num, const size_t len);
	
		void *(*bool_value)(struct wcjson *ctx, void *doc,
		    const bool value);
	
		void *(*null_value)(struct wcjson *ctx, void *doc);
	};

The elements of this structure are defined as follows:

*object\_start*

> Called whenever an opening '{' character of a JSON object has been scanned
> to create a corresponding result node in
> *doc*.
> Except the
> *ctx*
> argument all arguments may be
> `NULL`.
> The function is expected to return a pointer to the result node or
> `NULL`.

*object\_add*

> Called whenever a JSON object's key/value pair has been scanned to add
> to
> *obj*
> with
> *key*
> and
> *value*.
> Except the
> *ctx*
> argument all arguments may be
> `NULL`.

*object\_end*

> Called whenever a closing '}' character of a JSON object has been scanned
> to close
> *obj*.
> Except the
> *ctx*
> argument all arguments may be
> `NULL`.

*array\_start*

> Called whenever an opening '\[' character of a JSON array has been scanned
> to create a corresponding result node in
> *doc*.
> Except the
> *ctx*
> argument all arguments may be
> `NULL`.
> The function is expected to return a pointer to the result node or
> `NULL`.

*array\_add*

> Called whenever a value of a JSON array has been scanned to add to
> *doc*.
> Except the
> *ctx*
> argument all arguments may be
> `NULL`.

*array\_end*

> Called whenever a closing ']' character of a JSON array has been scanned
> to close
> *arr*.
> Except the
> *ctx*
> argument all arguments may be
> `NULL`.

*string\_value*

> Called whenever a JSON string has been scanned to create a corresponding
> result node in
> *doc*
> passing the string having been scanned in
> *str*
> of length
> *len*.
> The
> *escaped*
> flag indicates that string contains escape sequences.
> The
> *doc*
> argument may be
> `NULL`.
> The function is expected to return a pointer to the result node or
> `NULL`.

*number\_value*

> Called whenever a JSON number has been scanned to create a corresponding
> result node in
> *doc*
> passing the number having been scanned in
> *num*
> of length
> *len*.
> The
> *doc*
> argument may be
> `NULL`.
> The function is expected to return a pointer to the result node or
> `NULL`.

*bool\_value*

> Called whenever a JSON boolean has been scanned to create a corresponding
> result node in
> *doc*
> passing the flag having been scanned in
> *value*.
> The
> *doc*
> argument may be
> `NULL`.
> The function is expected to return a pointer to the result node or
> `NULL`.

*null\_value*

> Called whenever a JSON null value has been scanned to create a corresponding
> result node in
> *doc*.
> The
> *doc*
> argument may be
> `NULL`.
> The function is expected to return a pointer to the result node or
> `NULL`.

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
> *\*d\_lenp*
> of a destination
> *d*
> was too small.

\[`EILSEQ`]

> A source
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

## AUTHORS

Christian Schulte &lt;[cs@schulte.it](mailto:cs@schulte.it)&gt;.
