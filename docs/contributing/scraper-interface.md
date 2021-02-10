# Scraper Interfaces

This document describes MediaElch's scrapers and their design goals.

## Why was this written down?

Because I (GitHub user `bugwelle`) wanted to write down my thoughts while
developing the new scraper interfaces.

## TV Show Scraper

All TV show scrapers have a common interface.  At the time of writing this
(2020-11-20) there is only one TV show scraper: TheTvDb.  Because Kodi has
switched to TMDb for TV shows, users requested it as well.  But because
TheTvDb is deeply embedded in MediaElch, it became obvious that a rewrite of
the TV scrapers was necessary.

TV show scrapers have three main usages:

 - Search for a TV show by text or IMDb/TheTvDb ID
 - Load metadata for a TV show and its seasons and episodes
 - Load metadata for a single episode (or batch load for multiple episodes)

On top of that, images may be loaded from those providers.

### Meta Information

Scrapers must provide meta information about themselves.
The following details are required:

 - __Identifier__  
   A unique identifier. It is used for storing settings, as user-data in
   drop-down menus and so on. Must be lowercase and must not contain
   spaces or special characters.
 - __Name__  
   User-visible name.
 - __Description__  
   A (translatable) description of the scraper.  Can be obtained from the
   scraper's website.
 - __Website__  
   A URL to the main website of the scraper.
 - __Terms of Service URL__  
   URL to the terms of service (ToS).
 - __Privacy Policy URL__  
   URL to the privacy policy.  Required for GDPR compliance.
   MediaElch should point to it before the scraper is used the first time.  
   _TODO: Currently not implemented._
 - __Help URL__  
   Optional URL to help documents.
 - __Supported TV Show Details__  
   List of TV show details that the scraper supports.
 - __Supported Episode Details__  
   List of episode details that the scraper supports.
 - __Supported Season Order__  
   For example "DVD Order" and "Aired Order".  The latter is the default.
 - __Supported Languages__  
   List of locales that the scraper supports.
 - __Default Language__  
   Language that the user has selected in settings or just a default language.
   Most often `en-US`.


### Scraper Setup

Scrapers may need to be set up to work.  For example TheTvDb v2 requires a JSON
Web Token.  MediaElch has a developer key which is send to the provider and a
token is returned.  This token needs to be passed to each request.

Each scraper must provide the methods `isInitialized()` and `initialize()` as
well as a signal `initialized(bool)`.  When using the scraper, users must
ensure that the scraper is initialized before usage.  Using the scraper before
it is initialized will result in a direct failure (through a signal).


### TV show search

The TV show scraper interface (in the following only "scraper") should provide
a `search()` function that takes the following configuration parameters as one
argument:

 - Search query: may be a scraper specific ID or show title
 - Locale: The user's preferred language
 - Include Adult results: Whether to include NSFW results

These settings are required for all scrapers even if a scraper only supports
one language.  All additional settings must be set otherwise, e.g. through
MediaElch's settings window.  Adapters may need to be written for each scraper
so that it can be initialized with the correct configuration.

The scraper must provide methods to access a list of supported locales.  This
is necessary to create a drop-down menu for users in the search window.
The scraper must also store a default locale which should correspond to the
one that the user has selected in MediaElch's settings if such exist.
Locales must be stored as ISO language-COUNTRY codes (using MediaElch's
`Locale` class).  See [Wikipedia][wiki_locale].

Because network requests should never be blocking, `search()` must return
immediately.  But instead of storing the request information in the scraper
itself, a "search job" object should be returned which is fully self-contained.
Using one large scraper class has been proven to have multiple edge cases.
The following has happened in MediaElch before:

 - scraping is in progress and settings changed
 - scraping or search is in progress and a new search/scrape job has been
   triggered:
   - the currently used TV show changes -> data changed for the wrong show
 - scraping is in progress and all TV shows are reloaded
   - use after free crash because the used pointer becomes invalid

Most of the points above can be avoided by having a self-contained search
object.  The search job class must provide a "finished" signal.  This signal
must take a pointer to the search job itself.  By accessing the pointer,
information can be obtained like the result code, result list and error
messages.

Previous versions of this document suggested to have a signal for success and
error scenarios.  However, this leads to duplicated code for object deletion
and during object creation: Two signals must be connected.  Only passing the
result list or error code is also disadvantageous because then the user has to
store the scrape job (pointer) somewhere to delete it after completion.

The result list is a vector of tuples containing the TV show name, release date
(or year) and an identifier which can be used to load the show, e.g. an URL or
scraper specific id.


### Single Episode scraping

It must be possible to load single episodes. Scrapers must provide a method
`loadEpisode()` which takes an episode identifier, a list of details to load
and a language, similar to `search()`.  All other settings must be stored in
the scraper itself.

The episode identifier may be a specific ID or the show's ID and a season and
episode number.

The load method works similar to the search one.  A scrape job object is
returned for the same reasons as for `search()`.  The signals look the same
as well.

The episode-scrape-object contains an episode which will have the scraped
details.  It can be used to copy details to another episode or its ownership
can be transferred.

The scrape job must be deleted by the caller in the `finished` signal handler.


### TV Show scraping

Scrapers must provide a `loadShow()` method that loads metadata for the
selected show.  As configuration (one argument) it takes a show identifier,
details to load and a language.  Like the previous methods, other settings
must be set on the scraper itself.

The method also returns a scrape job with the same mechanism as the previous
ones.  Like for `loadEpisode()`, the show can be obtained and its ownership
can be taken.


### Season scraping

Scrapers must provide a `loadSeason()` method that takes as arguments the
show and season numbers for which *all* episodes should be loaded, a list of
details to load, the season order and a language.  Like the previous methods,
other settings must be set on the scraper itself.

The method also returns a scrape job with the same mechanism as the previous
ones.

Furthermore a `progress()` signal must be implemented which has two arguments:
Number of currently scraped episodes and the number of *all* episodes to
scrape.  This allows us to show progress bars.


## Handling of DVD, original, anime order

TV shows can have different types of orders. For example a show may premiere on
a TV station which only allows 20min segments.  So the show's episodes are each
20min long.  However, the season finale is 40min long.  So: Is the final episode
split up into two episodes or does it stay as a single one?  In practice, the TV
order would have two episodes for the final and the DVD order would have it as
one.  Also, some episodes are premiered in a different order on TV than they are
listed on DVD.

Furthermore, a special case is anime order which is not yet supported by
MediaElch.  Some animes do not have seasons. Instead they have one large season
with hundreds of episodes.  But because the show is still sold on DVD,
technically seasons *do* exist even though some users do not want to have them
separated into ones.

A problem with TheTvDb is that the DVD order is not available for all TV shows.
Anime order is only available for very few shows. On top of that, even if DVD
order is not available, TheTvDb simply returns an empty list. So MediaElch
needs to handle that as well and should fall back to the default order and/or
emit a user warning.

## TV show preview

Other media managers like TinyMediaManager make it possible to preview a show
scraping it. Such a functionality would be useful for MediaElch as well.

[wiki_locale]: https://en.wikipedia.org/wiki/Language_localisation#Language_tags_and_codes
