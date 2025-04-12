# WCJSON-DOCUMENT(3) - Library Functions Manual

## NAME

**wcjsondoc\_create\_null**,
**wcjsondoc\_create\_bool**,
**wcjsondoc\_create\_string**,
**wcjsondoc\_create\_number**,
**wcjsondoc\_create\_object**,
**wcjsondoc\_create\_array**,
**wcjson\_value\_head**,
**wcjson\_value\_next**,
**wcjson\_value\_tail**,
**wcjson\_value\_prev**,
**wcjson\_value\_foreach**,
**wcjsondoc\_array\_add\_head**,
**wcjsondoc\_array\_add\_tail**,
**wcjsondoc\_array\_get**,
**wcjsondoc\_array\_remove**,
**wcjsondoc\_object\_add\_head**,
**wcjsondoc\_object\_add\_tail**,
**wcjsondoc\_object\_remove**,
**wcjsondoc\_object\_get**,
**wcjsondocvalues**,
**wcjsondocstrings**,
**wcjsondocmbstrings**,
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
**wcjsondocmbstrings**(*struct wcjson \*ctx*, *struct wcjson\_document \*document*);

*int*  
**wcjsondocfprint**(*FILE \*f*, *const struct wcjson\_document \*document*, *const struct wcjson\_value \*value*);

*int*  
**wcjsondocfprintasc**(*FILE \*f*, *const struct wcjson\_document \*document*, *const struct wcjson\_value \*value*);

*int*  
**wcjsondocsprint**(*wchar\_t \*s*, *size\_t \*lenp*, *const struct wcjson\_document document*, *const struct wcjson\_value \*value*);

*int*  
**wcjsondocsprintasc**(*wchar\_t \*s*, *size\_t \*lenp*, *const struct wcjson\_document document*, *const struct wcjson\_value \*value*);

**wcjson\_value\_head**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_next**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_tail**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_prev**(*struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

**wcjson\_value\_foreach**(*lvalue*, *struct wcjson\_document \*d*, *struct wcjson\_value \*v*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_create\_null**(*struct wcjson\_document \*document*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_create\_bool**(*struct wcjson\_document \*document*, *const bool value*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_create\_string**(*struct wcjson\_document \*document*, *const wchar\_t \*value*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_create\_number**(*struct wcjson\_document \*document*, *const wchar\_t \*value*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_create\_object**(*struct wcjson\_document \*document*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_create\_array**(*struct wcjson\_document \*doc*);

*int*  
**wcjsondoc\_array\_add\_head**(*const struct wcjson\_document \*document*, *struct wcjson\_value \*array*, *struct wcjson\_value \*value*);

*int*  
**wcjsondoc\_array\_add\_tail**(*const struct wcjson\_document \*document*, *struct wcjson\_value \*array*, *struct wcjson\_value \*value*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_array\_get**(*const struct wcjson\_document \*document*, *const struct wcjson\_value \*array*, *const size\_t index*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_array\_remove**(*const struct wcjson\_document \*document*, *struct wcjson\_value \*array*, *const size\_t index*);

*int*  
**wcjsondoc\_object\_add\_head**(*struct wcjson\_document \*document*, *struct wcjson\_value \*object*, *const wchar\_t \*key*, *const struct wcjson\_value \*value*);

*int*  
**wcjsondoc\_object\_add\_tail**(*struct wcjson\_document \*document*, *struct wcjson\_value \*object*, *const wchar\_t \*key*, *const struct wcjson\_value \*value*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_object\_get**(*const struct wcjson\_document \*document*, *const struct wcjson\_value \*object*, *const wchar\_t \*key*);

*struct wcjson\_value \*&zwnj;*  
**wcjsondoc\_object\_remove**(*const struct wcjson\_document \*document*, *struct wcjson\_value \*object*, *const wchar\_t \*key*);

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
		size_t v_idx;
		wchar_t *strings;
		size_t s_nitems;
		size_t s_idx;
		char *mbstrings;
		size_t mb_nitems;
		size_t mb_idx;
		wchar_t *esc;
		size_t e_nitems;
	};

The elements of this structure are defined as follows:

*values*

> Array of values of the document.

*v\_nitems*

> Number of items in the values array.

*v\_idx*

> Index of the next item in the values array.

*strings*

> Array of strings of the document.

*s\_nitems*

> Number of items in the strings array.

*s\_idx*

> Index of the next item in the strings array.

*mbstrings*

> Array of multi byte strings of the document.

*mb\_nitems*

> Number of items in the mbstrings array.

*mb\_idx*

> Index of the next item in the mbstrings array.

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
		const char *mbstring;
		size_t mb_len;
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

*mbstring*

> Array holding the multi byte characters of a JSON string or number value.

*mb\_len*

> Number of items in the mbstring array.

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
*v\_idx*
member holds the number of items used in that array - that is the index of
the next useable item in that array.
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
*s\_idx*
member holds the number of items used in that array - that is the index of
the next useable item in that array.
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
*mb\_nitems*
member is set to the number of items needed in the
*mbstrings*
array to create multi byte strings.

The
**wcjsondocmbstrings**()
function creates multi byte strings by converting all
*string*
members of all
*values*
in a
*document*
to
*mbstring*
multi byte strings.
The
*mbstrings*
member needs to point to useable memory and the
*mb\_nitems*
member needs to be set to the number of items available in that array.
On Successful completion that array holds the multi byte strings and the
*mb\_idx*
member holds the number of items used in that array - that is the index of
the next useable item in that array.
The
*mbstring*
member of any
*wcjson\_value*
in the
*values*
array points to
*mbstrings*.

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
**wcjsondoc\_create\_null**(),
**wcjsondoc\_create\_bool**(),
**wcjsondoc\_create\_string**(),
**wcjsondoc\_create\_number**(),
**wcjsondoc\_create\_object**()
and
**wcjsondoc\_create\_array**()
functions get the next useable value from a
*document*.
The
*values*
member needs to point to useable memory and the
*v\_nitems*
member needs to be set to the number of items available in that array.
On successful completion the
*v\_idx*
member is increased by one indicating the number of elements used in that
array - that is the index of the next usable item in that array.

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
**wcjsondoc\_array\_add\_head**()
and
**wcjsondoc\_array\_add\_tail**()
functions add a value to an array.
The
**wcjsondoc\_array\_remove**()
function removes a value from an array.
The
**wcjsondoc\_array\_get**()
function gets a value from an array.

The
**wcjsondoc\_object\_add\_head**()
and
**wcjsondoc\_object\_add\_tail**()
functions add a key value pair to an object.
The
**wcjsondoc\_object\_remove**()
function removes a key value pair from an object.
The
**wcjsondoc\_object\_get**()
function gets the value of a key value pair from an object.

## RETURN VALUES

The functions return 0 on success or a negative value if an error occurs.
The global variable
*errno*
is set to indicate the error.
The
**wcjsondocvalues**(),
**wcjsondocstrings**()
and
**wcjsondocmbstrings**()
functions provide status via
*ctx*.
The
**wcjsondoc\_object\_get**()
and
**wcjsondoc\_object\_remove**()
functions return the first value matching
*key*
or NULL if no such value is found.
The
**wcjsondoc\_array\_get**()
and
**wcjsondoc\_array\_remove**()
functions return the value at
*index*
or NULL if no such value is found.
The
**wcjsondoc\_create\_null**(),
**wcjsondoc\_create\_bool**(),
**wcjsondoc\_create\_string**(),
**wcjsondoc\_create\_number**(),
**wcjsondoc\_create\_object**()
and
**wcjsondoc\_create\_array**()
functions return the next useable value or NULL if
*document*
cannot provide more values.

## ERRORS

\[`EINVAL`]

> A function was called with an invalid value.

\[`ERANGE`]

> A size of
> *v\_nitems*,
> *s\_nitems*,
> *mb\_nitems*,
> *e\_nitems*
> or
> *\*lenp*
> was too small.

\[`EILSEQ`]

> An input contained illegal data.

## SEE ALSO

[wcstombs(3)](https://man.openbsd.org/wcstombs)

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
