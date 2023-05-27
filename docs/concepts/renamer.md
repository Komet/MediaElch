# Renamer

__Status__: Concept, work in progress  
__Last Updated on__: 2023-07-02

New features for our movie/TV show/… renamer were requested on a regular basis.
Over the last few years, those requests have culminated.  This document tries
to propose a new renamer, that is more dynamic than the old one, which required
hard-coded placeholders for both conditions and variables.


## Current State

The following paragraphs describe the current state, including some drawbacks.

### Error Handling

There exists only minimal error handling and no proper heuristics whether
a proposed file name actually makes sense.  For example, renaming a movie that
hasn't been scraped, yet, could lead to a filename that is just ` ().mp4`,
because neither title nor year were loaded.

If a file can't be renamed, we tell the user.  But there are reports that
files on NFS or other remote network drives are not properly renamed, which
leads me to assume that the error handling should be improved.


### Filesystem Limitations

Depending on the filesystem, certain characters can't be used.  I'm yet to find
a filesystem that allows `/` in a filename.  Even Windows does not allow it.

But different filesystems have other limitations.  I hope nobody stores movies
on FAT32, but who knows.

MediaElch does not (properly) sanitize the user's filename input.


### Placeholders

We have multiple placeholders available, depending on the category to
rename (e.g. movies).  All of them are shown in a table when the renamer dialog
is opened.  This includes two rows for the conditional `{tmdbId}…{/tmdbId}`
and its variable `<tmdbId>`.

Example:

```
<title>{tmdbId} tmdbId-<tmdbId>{/tmdbId}{imdbId} imdbId-<imdbId>{/imdbId} (<year>).<extension>
```

Conditionals are an important aspect of the renamer.  It allows to add
additional characters besides the variable's value, but only if the value
exists / is non-empty.  Imagine the example above, but without conditionals:

```
<title> tmdbId-<tmdbId> imdbId-<imdbId> (<year>).<extension>
```

Without an TMDB ID, we would still have the string `tmdbId` in the file's name.

Unknown placeholders such as `<atitle>` are simply removed.  That makes it
difficult to know if a variable is actually empty or if it was simply
misspelled.


### Possible Mistakes

Because of how the renamer works, it could be that, if a movie's title contains
another placeholder, it is replaced itself. For example, if a movie's title is
`Movie about <year>`, then a placeholder `<title>` would be replaced by it,
and only then `<year>` is replaced, changing the title.  Even though this is a
very unlikely edge case, I dislike the idea of having this possibility at all.


## Proposal

### Error Handling

MediaElch should give the user as much detail as possible, _if requested_.
That means, we should keep the simplified table with a success/failure notice,
but when requesting more details (e.g. via double click), we should tell the
user what failed, if possible.  For example, we could store Qt's error message
and print it.

Furthermore, some errors should simply not be possible:  The user should not
have to specify a trailing `.<extension>`.  An extension should always be
set automatically, depending on the file's type.

### UI

We should group the success/failure table by movie/directory.
For example, with a tree view.  We could even collapse all entries by default
to make the output less verbose.  Errors should be shown more prominent.
If a user wants to rename a few hundred movies at once, and one movie failed,
I doubt that the user scrolled through all movies to see which failed.

### Filesystem Limitations

We should have a standard set of known filesystem limitations.  These
limitations should be checked against the user's filename patterns.

Limitations can be found on Wikipedia, but a few are:

 - no `/` or `\`
 - max filename length (255 on some very old filesystems)
 - max path lengths

Some systems are case-insensitive, other case-sensitive.
That means, if we want to rename a filename to something that only differs
in case, the rename-operation could either fail or silently succeed,
without actually renaming something.


### Placeholders

Because the old/current placeholder system has limitations, we want a new one.
It should be backward compatible to the current system, if possible.


#### Variables

Variables are put between `<` and `>`.  In contrast to the old system, the
value is parsed (see following sections about functions and operators) and
interpreted.  This means that simple placeholders such as `<title>` can now
also be written as `<  title  >`, i.e. whitespace is removed.

Variables must exist.  Otherwise, MediaElch must not start the rename-process
and must instead point the user to the incorrect variable.  This should
avoid accidentally misspelled variables.

Variables without any operator or function applied, must resolve to a simple
value.  Variables pointing to lists (e.g. actors/genres) are not allowed.

Variable names are case-sensitive. `<title>` is not the same as `<Title>`.

String values are simply put as-is, including trailing and leading whitespace.
Such whitespace must be removed in the media file's metadata.

Integer values are transformed into strings.

Floating point numbers are transformed into strings, with two places after
the comma.  `1.2345` is transformed into `1.23`, `1` is transformed into
`1.00`.  The representation is not locale aware, meaning that we don't
use `,` as the decimal delimiter in countries such as Germany.

For seldom cases that a floating point number is NaN (not a number) or
+/- Infinity, we use `NaN` and `inf`/`-inf`.


#### Index Operator

The index operator `<list[N]>` can be used to access the N-th value of `list`.
The variable `list` must exist, but if the list does not have N entries,
an empty string is used instead.

To get deterministic results, `list` should be a (sorted) list and not a set
which does not have a defined order.

The operator can also be used to access characters of strings or to create
substrings.  This is similar to how Python (NumPy) handles slices.
`<str[0]>` returns the first character of string `str` or an empty string if
`str` is empty itself.  `<str[:3]>` returns the first three characters of `str`
and is equivalent to `<str[0]><str[1]><str[2]>`.
`[:3]` is short for `[0:3]`, meaning that we want the character sub-range
`[0,2)`.

The index operator can't be used on numbers (integers and floats).

_Implementation remark_:  
We should use `QList`/`QVector` and use the NFO file's order of tags.
For actors, there exists an `order` attribute which should be used
(currently ignored).


#### Number Format

For filenames, we often want numbers that have N digits, e.g. via leading
zeroes.  For floats, we sometimes want at most / minimum two decimal places.

For integers, we support (example number `1234`):

```
{field}     1234
{field:6}   001234
{field:06}  000124
{field:2}   1234
```

Syntax: `{variable:min_digits}` and `{variable:0min_digits}`, meaning that
the number `6` in above example means "at least six digits".  An optional
`0` can be used to make it more clear that `0` is used as a filler.
This could also prove useful if we ever want to allow other filler characters.


For floating point numbers, we support (example number `123.34567`):

```
{field}     123.35
{field:6}   000123.35
{field:06}  000123.35
{field:6,3} 000123.346
{field:6,8} 000123.34567000
{field:2,3} 123.346
```

Syntax:
 - `{variable:min_digits}`
 - `{variable:0min_digits}`
 - `{variable:min_digits,decimal_digits}`
 - `{variable:0min_digits,decimal_digits}`

`min_digits` meaning the same as for integers. `decimal_digits` refers to the
number of digits after the decimal dot.

Default: two decimal places.

If ever required, we could also introduce number signs (`+`/`-`), but I'm not
aware of negative numbers in MediaElch except some default values for
"display season".


#### Field Access

Some fields are themselves structured.  For example, if we want to use an
actors name in a filename, we could use `<actors[0].name>`.  For the actor's
character, one could use `<actors[0].role>`.

In this example, `actors` is a list (see "Index Operator"), and its elements
are "actor" _structures_.

The `.` allows access into a structure's fields.

Imagine that a movie had a structured field `rating` that is not a list.
If we want to access the count of votes, we could use `rating.voteCount`.


#### Conditions

Conditions have two parts: Opening tags are put between `{` and `}` and
closing tags between `{/` and `}`.  Everything between those tags is
the _body_.

If the content of the opening tag evaluates to a _falsey_ value, then
their bodies will _not_ be evaluated.  For _truthy_ values, the body
is evaluated and put into the final filename.

_Truthy_ are, for example, non-empty strings.


#### Functions

Some functions may be useful such as uppercasing a string (e.g. a country).

Syntax: `<title|lower>` vs `<lower(title>`

Functions are added on-demand, but following ones are added by default:

- _`lower`_  
  Makes the string all lowercase. Example: "Harry Potter" -> "harry potter"
- _`upper`_  
  Makes the string all uppercase. Example: "usa" -> "USA"


#### Other Notes

Using `<`, `>`, `{`, and `}` means that such characters can't be used in
the filename.
