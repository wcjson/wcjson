# WCJSON-DOCUMENT(3) - Library Functions Manual

## NAME

**wcjson\_value\_head**,
**wcjson\_value\_next**,
**wcjson\_value\_tail**,
**wcjson\_value\_prev**,
**wcjson\_value\_foreach**,
**wcjson\_value\_pair**,
**wcjsondocvalues**,
**wcjsondocstrings**,
**wcjsondocfprint**,
**wcjsondocfprintasc**,
**wcjsondocsprint**,
**wcjsondocsprintasc** - wide character JSON documents

## SYNOPSIS

**#include &lt;[wcjson-document.h](src/wcjson-document.h)>**

*int*  
**wcjsondocvalues**(*struct wcjson \*ctx*, *struct wcjson\_document \*document*, *const wchar\_t \*text*, *const size\_t len*);

*int*  
**wcjsondocstrings**(*struct wcjson \*ctx*, *struct wcjson\_document \*document*);

*int*  
**wcjsondocfprint**(*FILE \*f*, *const struct wcjson\_document \*document*);

*int*  
**wcjsondocfprintasc**(*FILE \*f*, *const struct wcjson\_document \*document*);

*int*  
**wcjsondocsprint**(*wchar\_t \*s*, *size\_t \*lenp*, *const struct wcjson\_document document*);

*int*  
**wcjsondocsprintasc**(*wchar\_t \*s*, *size\_t \*lenp*, *const struct wcjson\_document document*);

*struct wcjson\_value\*&zwnj;*  
**wcjson\_value\_pair**(*const struct wcjson\_document \*document*, *const struct wcjson\_value \*object*, *const wchar\_t \*key*);

**wcjson\_value\_head**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_next**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_tail**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_prev**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_foreach**(*lvalue*, *struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

## DESCRIPTION

The functions operate on the
*wcjson\_document*
and
*wcjson\_value*
structures.
The
*wcjson\_document*
structure is defined as follows:

	struct wcjson_document {
		struct wcjson_value *values;
		size_t v_nitems;
		wchar_t *strings;
		size_t s_nitems;
		wchar_t *esc;
		size_t e_nitems;
	};

The elements of this structure are defined as follows:

*values*

> Array of values of the document.

*v\_nitems*

> Number of items in the values array.

*strings*

> Array of strings of the document.

*s\_nitems*

> Number of items in the strings array.

*esc*

> Array of escape sequences.

*e\_nitems*

> Number of items in the esc array.

The
*wcjson\_value*
structure is defined as follows:

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

The elements of this structure are defined as follows:

*is\_null*

> Flag indicating the value represents a JSON null literal.

*is\_boolean*

> Flag indicating the value represents a JSON boolean literal.

*is\_true*

> Flag indicating a JSON true or false literal.

*is\_string*

> Flag indicating the value represents a JSON string.

*is\_number*

> Flag indicating the value represents a JSON number.

*is\_object*

> Flag indicating the value represents a JSON object.

*is\_array*

> Flag indicating the value represents a JSON array.

*is\_pair*

> Flag indicating the value represents a key value pair of a JSON object.

*string*

> Array holding the characters of a JSON string or number value.

*s\_len*

> Number of items in the string array.

*idx*

> Index of the value in the docment values array.

*head\_idx*

> Index of the first value of the child value list.

*tail\_idx*

> Index of the last value of the child value list.

*prev\_idx*

> Index of the previous value in the child value list.

*next\_idx*

> Index of the next value in the child value list.

The
**wcjsondocvalues**()
function deserializes
*len*
characters of JSON
*text*
to populate a
*document*.
The
*values*
member of the
*document*
needs to point to useable memory and the
*v\_nitems*
member needs to be set to the number of items available in that array.
On successful completion that array holds the deserialized document structure
and the
*v\_nitems*
member is updated to the number of items used in that array.
The
**wcjsondocvalues**()
function does not decode strings.
The
*string*
member of any
*wcjson\_value*
in the
*values*
array points to
*text*.
Those strings are not zero terminated C strings so that the value of the
*s\_len*
member needs to be used when working with those strings.
Escape sequences will be preserved.
The
*s\_nitems*
member is set to the number of items needed in the
*strings*
array to create zero terminated C strings with any JSON escaping rules
unapplied.

The
**wcjsondocstrings**()
function decodes any
*values*
in a
*document*
by unapplying JSON escaping rules and adding terminating zero characters.
The
*strings*
member needs to point to useable memory and the
*s\_nitems*
member needs to be set to the number of items available in that array.
On successful completion that array holds the decoded strings and the
*s\_nitems*
member is updated to the number of items used in that array.
The
*string*
member of any
*wcjson\_value*
in the
*values*
array points to
*strings*.
The
*e\_nitems*
member is set to the number of items needed in the
*esc*
array to create JSON escape sequences when serializing the document.

The
**wcjsondocfprint**(),
**wcjsondocfprintasc**(),
**wcjsondocsprint**()
and
**wcjsondocsprintasc**()
functions serialize a
*document*
to a file or a string.
The
**wcjsondocfprintasc**()
and
**wcjsondocsprintasc**()
functions serialize to a 7 bit ASCII compatible representation, whereas the
**wcjsondocfprint**()
and
**wcjsondocsprint**()
functions serialize to wide characters with just the standard JSON escaping
rules applied.
The
*esc*
member needs to point to useable memory and the
*e\_nitems*
member needs to be set to the number of items available in that array.
For the
**wcjsondocsprint**()
and
**wcjsondocsprintasc**()
functions the
*s*
array needs to point to useable memory and
*lenp*
needs to be set to the number of items available in that array.
On successful completion
*lenp*
is updated to the number of items used in that array.

The
**wcjson\_value\_head**(),
**wcjson\_value\_next**(),
**wcjson\_value\_tail**()
and
**wcjson\_value\_prev**()
macros expand to accessor rvalue expressions for retrieving values from the
child value list of a value.

The
**wcjson\_value\_foreach**()
macro expands to a loop expression for iterating the child value list of a
value.

The
**wcjson\_value\_pair**()
accessor function gets the value of a key value pair from a given object.

## RETURN VALUES

The functions return 0 on success or a negative value if an error occurs.
The global variable
*errno*
is set to indicate the error.
The
**wcjsondocvalues**()
and
**wcjsondocstrings**()
functions provide status via
*ctx*.
The
**wcjson\_value\_pair**()
function returns the value for the given key or NULL if no such value is found.

## ERRORS

\[`ERANGE`]

> A size of
> *v\_nitems*,
> *s\_nitems*,
> *e\_nitems*
> or
> *\*lenp*
> was too small.

\[`EILSEQ`]

> A given input contained illegal data.

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
