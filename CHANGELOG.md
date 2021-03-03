# Changelog

## 2.8.7 - tbd

### Bugfixes

 - Renamer: On macOS, the renamer was sometimes so large that buttons were not visible (#1227)
 - Local trailers for movies are now detected if the filename contains square brackets such as `Movie[BLURAY]` (#1231)
 - Fix translations on some Windows systems (#1191)
 - Concert: Ratings were not properly written to NFO files

### Changes

 - The experimental CSV Exporter now supports streamdetails as well (#1204)
 - `<episodeguide>` tags in TV shows will now use TMDb first and will only use TheTvDb as a fallback (#1233)
 - Settings: If a new movie folder is added, "in separate folders" is enabled by default.

### Added

 - A quick-open menu for movies was added. Open it by clicking <kbd>Ctrl+O</kbd> (<kbd>⌘+O</kbd> on macOS).
   It uses a Fuzzy matching algorithm. The menu was inspired and partially taken from the
   [Kate editor](https://invent.kde.org/utilities/kate/-/merge_requests/179).
   Our gratitude goes to the Kate team and to Ahmed Waqar for allowing us to use it!

### Removed

 - *tbd*

### Internal Improvements and Changes

 - The Kodi generators now use `QXmlStreamWriter` instead of `QDomDocument`.  
   This has the side-effect that unrecognized tags will be removed.
   All episode xml writers already used this. Now all other media types use that
   as well.  `QXmlStreamWriter` is faster and the code becomes more maintainable.
 - MediaElch no longer has `*.qm` files in its source tree.  QMake (and CMake) need
   to be able to run `lrelease` to generated translation files.
 - MediaElch now searches for movies in parallel (#1230)  
   Improvements can vary. During tests on different hard drives with
   1000 movie directories, the time improvements were:
   - 3.8s -> 1.3s (SSD)
   - 4.1s -> 1.4s (HDD)
 - Fix unuseful log warning "[KodiXml] NFO file could not be opened for reading" for TV shows that do not have a NFO file.   


## 2.8.6 - Coridian (2021-01-22)

### Bugfixes

 - Audio-channel detection was broken since the previous version (#1172)
 - Translations were updated  
   Due to a bug in our "update translation" script, new strings were not published.
 - Genre/Certification/Studio mappings for all TV show scrapers are now respected, not just for TheTvDb (#1176)
 - FanartTv: The default language is now English (#1190)  
   Previously, the first language in the checkbox was used (i.e. Bulgarian).
 - Kodi: v18 was not selectable in MediaElch's settings for Kodi (#1193)
 - IMDb movie scraper: Search results contain the movie's year again
 - Streamdetails: Detection of runtime is fixed (#923)

### Changes

 - CSV Export: Column names in generated CSV files were renamed
 - TV Show search: If the show's title contains its year, it is removed in the search dialog (#1192)
 - TV Show search: All checkboxes are selected per default on new installations (#1189)
 - File Searcher: A delimiter is expected if a file is split up into multiple parts (#1194)  
   If you have media files that are split up into multiple parts (e.g. CDs, DVDs, ...) then MediaElch
   expects it to be named like `title.part1.mkv` (or `-dvd1`, etc.). `titlepart1.mkv`, i.e. without a
   delimiter worked before but caused false positives. Delimiters are ` `, `.`, `_` and `-`.

### Added

 - CSV Export: The experimental CSV exporter now supports "directory" and "filenames" as options (#1173)
 - TV show: Add original title field (#1180)  
   This field is only supported by the TMDb TV show scraper.


## 2.8.4 - Coridian (2021-01-09)

### Bugfixes

 - TV search on TMDb: It is now possible to search for a show by its TMDb and IMDb ID,
   e.g. "tt0096697" or "456" for "The Simpsons"
 - Fix the immediate "network error"-message in the TV show image dialog (#1124)
 - TV search: Episode and show details are stored per scraper (#801)
 - Movie Sets: Adding a movie works again (#1129)
 - Name Formatter: If an excluded word contains special characters, it is removed again (#1131)
 - Universal Music Scraper: "Formed" field now strips HTML tags (#1152)
 - If you click "Cancel" in the file dialog when adding a new media directory, the home directory is no longer added.
 - ADE: Actor images are loaded again (#1164)

### Changes

 - NFO files now contain a `<generator>` tag which holds meta information about MediaElch.  
   This may be helpful when users complain about Kodi not displaying certain details on the Kodi forum.
 - Renamer: More special characters that are not allowed in filenames are now replaced by a space.  
   Windows disallows the following characters in filenames: `<`, `>`, `:`, `"`, `/`, `\`, `|`, `?`, `*`.
   MediaElch now does a bit of filename sanitization more, even if not a lot.  Previously unchecked was `|`.
   Furthermore, leading dots are removed because they indicate hidden files on Unix systems.
 - On macOS the discrete GPU is no longer required (#702)

### Added

 - Concert: Add IMDb/TMDb ID fields
 - Concert search: Add language dropdown to search dialog
 - Added `*.webm` to default file filters for video files (#1122)
 - Add TVmaze as a TV scraper (#834)  
   TVmaze works great for actor images and season posters.  Due to API
   limitations not all episode details are available that you can see
   on their website.
 - The movie renamer now supports `<studio>` which contains the movie's studio(s) (#1136)  
   If there is more than one studio, all studios are separated by a comma (`,`).
 - The exporter can now be opened using the shortcut <kbd>Ctrl+E</kbd> (<kbd>⌘+E</kbd> on macOS)
 - An experimental CSV exporter can now be opened using the shortcut <kbd>Ctrl+Shift+E</kbd> (<kbd>⌘+⇧+E</kbd> on macOS) (#585)
 - Universal Music Scraper: MusicBrainz was added as a biography source (#1065)  
   Due to API-changes in TheAudioDb, MediaElch no longer scraped artist's biography from
   TheAudioDb but from AllMusic which was a fallback. But AllMusic only supports English.
   To have translated texts again, we now use MusicBrainz as a biography source.
   MusicBrainz uses texts from Wikipedia.

### Removed

 - Support for Kodi v16 NFO files is removed.  MediaElch can still read them but does not write them anymore. (#1142)


## 2.8.2 - Coridian (2020-12-20)

### Bugfixes

 - Revert the use of the user's preferred UI language and not the system language (#1002)  
   The implementation was wrong which lead to users getting MediaElch in English even
   though their system is German.  This needs a do-over.
 - Fix regression for BluRays (#1090)  
   BluRay discs had their `BACKUP` and `STREAM` folders recognized as different movies.
   This is now fixed and those two directories are skiped.

### Changes

 - Image downloads are now run in parallel (up to 6 downloads) (#1077)

### Added

 - A TVmaze ID field was added to the TV show and episode UI (#834)

### Internal Improvements and Changes

 - Better error reporting for TV scrapers


## Older Releases

### 2.8.0 - Coridian (2020-12-13)

This is the next "big" MediaElch version. This version brings fully rewritten TV scrapers including
improved user interfaces, better user experience and a new TV show scraper.

MediaElch now requires Qt 5.6 or later.  Qt 5.6 was released in 2016 and we highly recommend to update
to the latest version if your system supports it (this only affects MediaElch's version for Linux
distributions).  
This means that Ubuntu 16.04 is no longer supported!

Note: You may need to set "DVD order" in your settings again as the internal settings-key changed.

#### Bugfixes

 - Exporter: Fix episode thumbnails in TV show view (#961)
 - Movies: Sub-folders are no longer handled as files (#978)
 - Downloads: Fix stylesheet of import dialog window (#984)
 - Fix ADE actor scraping (#1011)
 - Use user's preferred UI language and not the system language (#1002)
 - The filter bar's `X` button is now visible again and also clears filters (#1025)
 - Reverting changes for episodes did not always work (#1032)
 - Just selecting an episode sometimes marked it as unsaved (#1031)
 - Fix crashes on Manjaro (#976)
 - Fix IMDb poster scraping
 - Fix HotMovies scraper and add backdrop support

#### Changes

 - Concert Export: `{{ CONCERT.FILENAME }}` and `{{ CONCERT.DIR }}` are now supported in themes (#962)
 - IMDb now includes adult search results if adult scrapers are enabled (#992)
 - Export: Image placeholders like `{{ IMAGE.XYZ }}` are now only replaced if the image type is actually used (#961)

#### Added

 - Movie Backdrop: Make it possible to set a random screenshot as the movie's backdrop/fanart (#965)
 - Movies: Add option to ignore duplicate original title (#1006)  
   In v2.6.4 MediaElch started to ignore the original title (i.e. did not save it) if it was the same
   as the "normal" title.  We now added an option to reverse this behavior.
 - TV shows and episodes now support TMDb IDs (#1010)
 - macOS: A simple help menu with useful URLs is added (#1020)
 - Movie Renamer: `<director>` placeholder is now supported
 - Movie: Add buttons that take you to the movie's IMDb/TMDb page (#684)
 - TMDb ID field added to UI for movies (#1022)
 - TMDb ID filter: Add "Has TMDb ID"/"No TMDb ID" movie filters (#684)
 - New TV show search dialog  
   You can now distinguish between episode and TV show details that you want to load using the
   selected scraper.
 - New TV scraper settings  
   The settings dialog for TV scrapers has been completely redesigned.  It now features the
   scraper's website, description, terms of service and more so that you know how MediaElch
   uses the scraper.
 - New Custom TV scraper  
   The custom TV scraper has got a new look and feel.  You can select the scrapers you want for
   episode and TV show details.
 - Advanced Settings: You can now specify a custom stylesheet for MediaElch theme development (#1040)
 - Movie Filter: Add filter by original title (#1057)
 - Episodes now support tags like TV shows do (#1061)  
   Note that tags for episodes are not supported by Kodi, yet.

#### Internal Improvements and Changes

 - Dialogs: Removed singletons for dialog windows (#980)
 - Use combo box for "DVD order" and "Aired order" (#983)
 - Use "Locale" class throughout our code base (#997)
 - Use MediaElch's Meta GitHub repository for export themes
 - Remote themes are now checked for a valid SHA256 checksum


### 2.6.6 - Ferenginar (2020-04-18)

*Note:* This release has changed its internal file and directory handling to
fix Windows-related bugs (#732).  Please report any issues with network drives
or other non-local drives.

#### Bugfixes

 - Fix AEBN crash when scraping a movie (#910)
 - Select correct language for TMDb in the movie search dialog (#916)
 - Windows: Fix scanning of concerts (#814)
 - Downloads Section: Fix crash when importing items (#828)
 - Downloads Section: Fix invalid file sizes (#829)  
   Due to a bug, files above 2GB had the wrong file size. Furthermore MediaElch
   showed file sizes in MiB, GiB, etc. even though MB, GB, etc. were displayed.
 - TV episodes: Manually edited writers and directors were not saved (#933)
 - TV shows: Fix TV shows always being reloaded from disk (#732)  
   Due to a bug in path handling, TV shows were *always* reloaded even though
   the internal cache should be used.
 - UI: Fix text color in messages boxes (#942)  
   On Linux distributions that use the KDE Breeze Dark theme (or other dark
   themes), the text color of message boxes would be white.  Light text on
   light background is not readable. The text is now always set to black.
 - HotMovies: Fix rating scraping  
   Due to a change on their website, the vote count scraping did not work anymore.
 - Trailer Download: Fix downloading trailers for many movies (#940)  
   If the trailer dialog window was closed using the window frame's close
   button ("x") instead of the button labeled "Close", the dialog was destroyed
   and reopening it resulted in an empty dialog window.
 - TV Tunes Download: Fix crash when aborting download and restarting it (#940)  
   If the TV tune dialog is closed before it has finished loading its search
   results and is reopened for another show, MediaElch crashed due to a
   race condition.
 - Movie: Don't save runtime from file if it cannot be detected (#604)  
   If the user changes the movie's runtime and saves it, it may be reset to
   0 if "Automatically load and save stream details from file" is enabled in MediaElch's
   settings. MediaElch now first checks if the detected runtime is greater than 0.
 - TV show file searcher now allows spaces between episode and season (#513)  
   Previously "S01 E02" was not accepted. However, in older versions of MediaElch
   this seemed to have worked and now works again.
 - Image Preview: Fix centering of image dialog (#863)

#### Changes

 - Export: The generated folder name now also contains seconds (#935)  
   It now has the pattern `MediaElch Export yyyy-MM-dd hh-mm-ss`.
   Exporting multiple times whithin a minute otherwise leads to name clashes.

#### Improvements

 - Always write the episode guide URL to TV show NFOs using TheTvDb format (#652)
 - Fanart.tv: Print better error messages for shows and movies that cannot be found (#900)
 - TMDb: Update available languages to support official translations (#901)
 - Movies: If movies are sorted by "name", the movie's sort title is used if
   set and the name otherwise (#919)
 - IMDb: Use higher image resolution for actors (#920)
   IMDb has reduced the image resolution for actor images on a movie's main page.
   MediaElch now loads each actor page and uses an image with higher resolution.
 - Movie Poster: Make it possible to set a random screenshot as the movie's poster (#934)
   Just like for TV show episode thumbnails, a poster can be created which uses a
   random screenshot from the movie file. Movies must have a format that is readable
   by ffmpeg. The resolution is hard coded to 720x1080px which has a typical poster
   aspect ratio of 2:3.
 - TV show: Also load TV show posters when searching for new season posters (#600)
   TheTvDb may not return posters for a certain seasons. However it may still be
   useful to select TV show posters for a season.
 - TV show: Remove suffix (e.g. `.mkv`) from default episode names (#513)  
   Default names are just their file names. MediaElch now removes the file suffix.

#### Internal Improvements and Changes

 - Set MediaElch specific HTTP User-Agent header for most HTTP requests (#912)
 - Updater: Use new MediaElch meta repository for version checks (#896)
 - Download Section: Refactor the file searcher to make it non-blocking and
   improve the overall performance (#830)
 - Logging: Respect `QT_MESSAGE_PATTERN` and use better defaults  
   Previously, MediaElch did not respect Qt's environment variable
   `QT_MESSAGE_PATTERN`.  That environment variable can be used to format logging messages.
   In debug mode the default pattern uses colors if the console supports it.
 - Replace all old-style `SIGNAL`/`SLOT` connections with new-style ones.
 - Update MediaELch's dependencies and use Qt 5.14.2 for Windows (#832)


### 2.6.4 - Ferenginar (2020-02-08)

#### Bugfixes

 - Fix TV shows sorting and possible crashes if "Show missing episodes" is enabled (#789, #883)
   MediaElch could crash under certain circumstances due to improper use of Qt's
   model/view system. You may experience that shows are deselected when " Show missing episodes"
   is clicked the first time.
 - Fix hanging window if the custom movie scraper is selected but no valid scraper is found.  
   This can happen if MediaElch's settings are in an inconsistent state (#792)
 - Fix `{{ *.RATING }}` not being replaced in exports if a media item has no rating
 - Fix movie label color not being shown (#803)
 - Fix default language selection in movie scraper dialog  
   Default-selected language not used when loading from TheMovieDb
 - NFO: Only write `<originaltitle>` if it's different from `<title>` (#812)
 - Fix IMDb runtime scraping (#810)
 - Update TMDb base URL for downloading images (#807)  
   The very old subdomain has been taken offline. We now use the new one.
 - Fix TMDb scraper language in dialog window (#813)  
   Scraper language wasn't the one saved in settings but always "English".
 - About Dialog: Fix MediaInfo version string in developer information (#790)
 - Movie/TvShow: Only parse valid `premiered` tags (#827)
 - Fix crash when no TV show is selected  
   For example: Start MediaElch -> click "TvShow" -> click "Sync Kodi" -> click "Close" -> Crash
 - Fix missing TheTvDb ID in episode NFO files (#788)
 - Don't write TheTvDb v1 episode-guide URLs to TV show NFOs (#652)
 - Show "Dolby TrueHD" media flag for "truehd" audio codec
 - Fix audio codec recognition for newer MediaInfoLib versions (#797)
 - Fix "Add to synchronization queue" feature for episodes and TV shows (#850)
 - Allow IMDb IDs with 8 digits (previously only 7 digits allows) (#855)
 - Fix actors having wrong image after removing one actor (#859)

#### Improvements

 - TMDb: Load more movie collection details (#800)
 - ADA search: Don't filter for DVDs, fix overview scraping of some movies (#819)
 - Movie Search Dialog: Add error message label
 - TheTvDb: Use API v2 (JSON API instead of old XML API) (#487, #432, #528)
 - Episode widget: add TheTvDb ID and IMDb ID fields
 - AdvancedSettings: Better input validation (issues are printed to the debug log) (#743)
 - AdvancedSettings: Add experimental exclude patterns (#840)
 - Add `en_US` language file for better singular/plural handling
 - Better network error reporting for scraping TV shows and movies (#870)
 - Better error reporting in the image dialog (#864, #874)

#### Internal Improvements and Changes

 - CMake: Fix Foundation framework include for macOS
 - Tests: Add more integration and unit tests
 - Downgrade to Qt 5.5 as minimal requirement (#885)


### 2.6.2 - Ferenginar (2019-09-13)

This release will affect your scraper settings. You'll therefore have to
reconfigure your scraper settings. You may also need to delete `MediaElch.sqlite`.

See: https://mediaelch.github.io/mediaelch-doc/faq.html#where-are-mediaelchs-settings-stored

#### Bugfixes

 - Fix IMDb tag scraping (#649)
 - Fix IMDb poster scraping
 - Fix TMDb issue with HTML characters in overview/plot (#651)
 - `Ctrl+A` works in movie and concert section (#647)
 - Fix crash when "Load missing episodes" is enabled (#669)
 - Filter "Movie has no IMDb ID" is inverted (#680)
 - Window positions not saved (#679)
 - Country mappings not used in TMDb (#689)
 - Windows: Stream Details do not load (#688)
 - Crashes on macOS when scanning episodes due to a Qt bug (#641)
 - Fix ADE scraper (#703, #725)
 - Fix incompatibilities with Kodi v17 NFO files (#719)
 - Fix race-condition in DownloadManager (#766)
 - Fix VideoBuster certification scraping

#### Improvements

 - Update French translation (#646)
 - Add more subtitle formats (#661)
 - UI: Fix list/table style for dark themes (#640)
 - UI: Fix stuck splitter cursor (#659)
 - UI: Restructured search windows (#660)
 - Windows: Update MediaInfo (#688)
 - Windows: Update to Qt 5.12 and fix style issues (#678)
 - Kodi: Use new syntax for ratings for movies and TV shows (#516)
 - Kodi: Add missing "aspect" attribute to thumbs (#665)
 - Settings: Color paths red if directories are not readable (#730)
 - Add `{{ MOVIE.LABEL }}` placeholder in templates (#715)
 - Add experimental command line interface (#720)
 - Add `userrating` field to movies
 - Add Kodi v16/v17/v18 switcher (#719)
 - Make episode thumbnail size configurable in `advancedsettings.xml` (#776)
 - Add "avc" to "h264" video codec mapping to defaults (#728)

#### Internal Improvements and Changes

 - Implement ScraperSettings class for better mocking
 - Use `QVector<T>` instead of `QList<T>` as the default container
 - Reorganize project structure
 - Add CMake build system (#700)
 - Add `DISABLE_UPDATER` option in QMake and CMake for package maintainers (#763)
 - Require Qt 5.6 or later (#780)


### 2.6.0 - Ferenginar (2019-01-06)

#### Features
 
 - Add user documentation (#531)
 - Use HTTPS for scraping (#371)
 - Set UI language in `advancedsettings.xml` (#411)
 - Load *all* tags from IMDb (#469)
 - Add more audio and video codecs (#524, #530)
 - Remove deprecated Cinefacts (#370)
 - Remove deprecated Coverlib (#369)
 - Remove deprecated MediaPassion (#449)
 - Remove deprecated MovieMaze (#386)
 - Support KDE Breeze Dark theme (#407)
 - Use new Kodi XML syntax for movie set names (#554)
 - Add context menu in movie duplicate view (#591)
 - Add AEBN genre option (#590)
 - Select scraper language in movie search panel (#442)
 - Concert Renamer (#574)
 - Scrape TV show tags when using IMDb
 - Detect duplicate movies
 - Create subdirectories

#### Bugfixes

 - HD-Trailers scraper broken (#445)
 - OFDB movie scraper crashes MediaElch (#394)
 - IMDB movie poster not loaded (#385)
 - IMDB top 250 not scraped (#468)
 - IMDB outline/plot scraping broken (#456)
 - IMDB genres/studios/... scraping broken (#556)
 - studio name not exported (#392)
 - TvShow status not written to `.nfo` file (#380)
 - HTML entities in export not escaped (#391)
 - multi scraper does not load all episode thumbnails (#415)
 - studio mapping for TvShow episodes (#459)
 - using filters crashes MediaElch (#504)
 - wrong filter tooltips (#506)
 - concert extra fanart not saved (#529)
 - ADE search does not return results (#565)
 - backdrop not loaded using ADE (#519)
 - adult scrapers broken (#367)
 - VideoBuster does not load certification in special cases (#571)
 - *fanart.tv* music scraper broken
 - Placeholder `<showTitle>` does not work for renaming season folders (#553)

#### Improvements

 - UI: line break in tree views (#406)
 - UI: add 4k and 8k resolution flag (#446)
 - UI: show green ID flag only if IMDB is valid (#471)


### 2.4.2 - Talax (2016-07-01)

 - Bugfix: UniversalMusicScraper broken


### 2.4.1 - Talax (2016-03-20)

 - Renamer: Show results in table view
 - Export: Add IMDB ID to template
 - IMDB: Adjusted scraper to new layout
 - StreamDetails: Stereomode not detected
 - IMDB: Rating and votes sometimes not scraped
 - TheTVDB: Get votes for episodes and shows
 - Image capture: Prevent possible crash
 - StreamDetails: Language of subtitle not detected
 - TvShows: Possible crash when scanning for new episodes of a single show
 - TvShowEpisodes: epbookmark not saved
 - AdvancedSettings: Use first studio only not working


### 2.4.0 - Talax (2015-12-06)

 - Custom TV scraper
 - Scrape multiple TV shows/episodes
 - Add option to manually edit the nfo file
 - IMDB scraper for TV Shows
 - Support for external subtitles
 - Handle network timeouts in scrapers
 - Improve ignored words filter
 - Option to open folder for music artists and albums
 - Music: Store release id instead release group id
 - Improve album search query
 - Don't reload after rename
 - Musicscraper: Show musicbrainz release id in search results
 - Move TV shows with new items to the top
 - Don't remove custom tags in nfo files
 - StreamDetails: Use Channel(s)_Original if available
 - Improve UI rendering in Windows
 - Get and store IMDB ID for TV show episodes
 - Support for TV show status (continuing/ended)
 - Use lowercase video and audio codec in streamdetails
 - Save all selected items when saving
 - Votes and top 250 for TV shows and episodes
 - TV Shows: Improved UI in listview
 - Support image formats with wrong file extension
 - Musicscraper: Label, release date and artist is not set
 - IMDB: Studios not detected
 - Windows: Black screen when started via RDP
 - Hotmovies scraper broken
 - StreamDetails: Sometimes not loaded in Windows when filename contains special characters
 - Crash when selecting open folder on empty list


### 2.3.2 - Denobula (2015-10-12)

 - Write urls to nfos by default
 - Fixed dependencies for wily


### 2.3.1 - Denobula (2015-10-10)

 - Bugfix on Coverlib.com scraper
 - Fixed package name in debian control file


### 2.3.0 - Denobula (2015-10-10)

 - Music: Coverlib.com scraper
 - Music: Support for booklets
 - Improve UI on retina displays
 - Drop images directly to image elements
 - Renamer: Options for video/audio codec and number of audio channels
 - Renamer: Support IMDB id on directory renaming
 - Renamer: Show warning when item has been edited
 - Renamer: Support extension also in directory names
 - MediaPassion: API URL changed
 - Improve IMDB outline scraping
 - Detect stream details from BluRay structures
 - Advanced Settings: Option to disable saving of thumbs in nfo
 - Media Status Columns: Add local trailers
 - Play trailer on click
 - TMDB: Show results even if TMDB API delivers wrong page count
 - Music: Added extra fanarts can not be removed
 - Music: Artists and albums with special characters fail to scrape
 - Trailer download sometimes not working
 - IMDB: Possible hang on multiscrape
 - Multiselection on movies when using filters leads to wrong selection
 - TvTunes scraper broken
 - Synchronization: Updating play count on episodes and concerts failed
 - OSX: Settings window not closable in fullscreen
 - VideoBuster scraper broken
 - Adult DVD Empire scraper broken
 - Renamer: Scantype (progressive/interlaced) not respected


### 2.2.2 - Sphere Builder (2015-02-02)

 - UI: Music multi scrape progress bar doesn't stop
 - Renamer: Conditional 3D tag not working
 - StreamDetails UI: Audio label on wrong position
 - Renamer: Resolution tag is empty for very low resolution files
 - Default filename for music disc art should be cdart.png
 - Resolution icon not visible
 - Music: Deleted images show up again sometimes
 - MediaPassion: Images are not loaded
 - StreamDetails not loaded when filename contains special characters (Windows only)
 - Music multiscrape: Artist thumb and logo are not loaded
 - Show IDs in the GUI


### 2.2.1 - Sphere Builder (2015-01-25)

 - Media Passion: API URL changed


### 2.2 - Sphere Builder (2015-01-23)

 - Fanart.tv: Add support for personal API keys
 - Support for music libraries
 - Detect 3D movies from streamdetails
 - Export: Add movie filename and path
 - Fanart.tv: Use API v3
 - Save last used path when selecting actor images
 - Renamer: Add more options
 - Play movie/episode/concert on double click
 - Fanart.tv: Add support for season posters
 - NFO: Correctly save multiple entries
 - Fanart.tv: Add support for TV show posters
 - Rename XBMC to KODI
 - Add icon for DTS-HD HR
 - Use icon font for navbar items
 - IMDB Scraper: Problems with html tags
 - Open Movie Folder not working on smb shares
 - TMDB: Sometimes no results are found
 - MovieMaze Scraper broken
 - List widgets are not resized when resizing main window
 - Adult DVD Empire sometimes loads no data
 - IMDB: Multiple directors and writers are not scraped
 - OSX: Popup window positions are offset (Qt Bug)
 - Several issues in set manager
 - Stream details are not detected when filename contains special characters
 - IMDB Scraper: Genres, Country, Actors sometimes not correctly scraped
 - IMDB: Posters not scraped
 - TV Shows ordered wrong when episode number >= 100
 - IMDB: Just small actor images are scraped
 - IMDB: Outline sometimes not scraped
 - Renaming: TV Show directories are always renamed
 - IMDB: Release date, director and writer not detected
 - New marks stay visible after saving last new item
 - MovieMaze: Scraper broken


### 2.1.3 - Trill

 - Update package for Trusty


### 2.1.2 - Trill (2014-03-27)

 - Add info about collection
 - Join genres in nfo files
 - Rename TV shows: dialog doesn't open ...
 - Custom Scraper: Fanart always from fa...
 - IMDB scraper sometimes displays no re...
 - Only small actor images from imdb
 - Windows: dialogs out of desktop
 - Aspect ratio saved in localized version
 - MediaStatusColumns: Extra Arts don't ...
 - Speed up loading of movies from database
 - Filter for audio formats
 - IMDB scraper doesn't map advanced set...
 - Add stream details to export function  


### 2.1.1 - Trill (2014-03-17)

 - Filescanner dialog not visible


### 2.1 - Trill (2014-03-16)

 - OS X Retina display support
 - MakeMKV integration
 - Option to hide donate button
 - Custom labels for movies
 - Select startup section
 - Export: Remove line breaks genre block
 - Words to exclude: add mkv
 - Automatically guess import type and folder
 - Missing episodes: Hide specials
 - Show adult movies from TMDB
 - Filter for movies with/without rating
 - Replace underscores with spaces in movie names
 - Only update movies with id in custom scraper
 - Use different IMDB Scraper (mymovieapi.com down)
 - Read ratings with comma separated decimals
 - Save last used path when manually selecting images
 - Trim title from the tv db
 - Context menus stay sometimes in front
 - StreamDetails: Aspect ratio sometimes displayed as 0.000
 - XBMC synchronization broken
 - Disable filesystemwatcher for imports
 - Multiscraping: IMDB id is set as TMDB id
 - Settings: Movie set artwork filenames are not display correctly
 - MovieMaze: Wrong encoding
 - Some episode names lead to detecting multiple episodes
 - Extracting password protected files fails
 - Updating new episodes with dvd order fails
 - Trailers are not renamed
 - Scraper adds sometimes "id" before IMDB ids
 - Multiscrape: Only movies with id are scraped


### 2.0.6 - Risa (2013-10-31)

 - Show all banners for seasons
 - Improve tabs ui
 - Import: Disable automatic reload during extraction
 - Streamdetails detects runtime of 0
 - Selected items in scrapers are not saved
 - Episodes can not be selected
 - Custom scraper broken
 - TV Shows: Selected item has no background color in Windows
 - Import: Single subtitles are detected as importable item
 - Second window in Linux visible
 - Choosing local images broken


### 2.0 - Risa (2013-10-28)

 - HD-Trailers.net: Add apple trailer download support
 - Media Passion: Support for Logos and ClearArts
 - Language Support in fanart.tv
 - Option to update only movies with IMDb Id
 - Import movies, episodes and concerts
 - Show missing Episodes
 - Update notification
 - BluRay/DVD Disc option in fanart.tv
 - Scrape posters from fanart.tv
 - Portable mode via advancedsettings.xml
 - Scraper for Adult DVD Empire
 - Scraper for AEBN
 - Scraper for HotMovies
 - Upgrade to Qt5
 - Improve exclude words detection
 - Filter for movie sets
 - Filter for IMDb ID
 - Exclude words also in directory names
 - Search by IMDb id or TMDb id by default
 - Also rename .srt subtitles
 - Actors for TV show episodes
 - TheTvDb: Search by ID
 - Speed up loading items
 - Add warning for extra fanarts when sep. folders is not cheked
 - GUI updates
 - Fix hd-trailers.net scraping
 - Poster naming of stacked files
 - Wrong parsing of filenames from stacked movies
 - Skip dots in movie titles when searching
 - TV show season fanart not found/scraped
 - Media Passion: Titles with accents give no results
 - Renaming fails if new name only differs in capitalization
 - Adjust MovieMaze Scraper
 - Cinefacts scraper broken
 - Check DVD aired order
 - Runtime of `*.ts` files is not detected
 - Images from SMB shares under Windows cannot be chosen
 - Filescanner might skip some movies
 - MediaPassion: Discart sometimes not found
 - MediaPassion: Original title not scraped
 - Some images are not renamed when renaming movies
 - TV Show list jumps to not clicked items


### 1.7 - Bajor (2013-08-08)

 - Store last used scraper in settings
 - media-passion.org scraper
 - Get IMDB Id from filename
 - Add runtime from thetvdb
 - Ability to paste a link to a picture
 - Option to use the IMDB Ratings regardless of which scraper is used
 - Banner for movies
 - XBMC Sync: Clean database
 - Add TV Show Thumbs from Fanart.tv
 - Better keyboard navigation in file lists
 - Remove quit button from toolbar
 - Improve mapping to allow empty values in advanced settings
 - Context Menu: Improve scrape option
 - Strip braces in search term
 - Add video category to .desktop file
 - Allow selecting multiple movies when adding them to sets/genres/certifications
 - Sort title for TV shows
 - Use airs before/airsafter to set display season and episode
 - OFDB: Load movie by id
 - RegEx for "Season X Episode Y"
 - TV Show data sometimes lost
 - IMDB API url changed
 - DVD order wrong
 - Don't save empty actor image and thumb tag
 - TV Show File Scanner detects wrong episode numbers
 - Stacked Files: Duration in Streamdetails
 - Display title of movies with dots
 - Wrong nfo filename when renaming stacked movies
 - Duplicate fileinfo tag
 - Add placeholder for episode and season banner image


### 1.6 - Romulus (2013-04-23)

 - Added filter for local trailers
 - Media Status Column “Trailer” now regards local trailers
 - Bulk loading of streamdetails now overwrites movie runtime
 - Bugfix: Also saved last movie while multi scraping
 - Added media status columns for actors and IMDB Id
 - Bugfix: Correctly load CD art for BluRay and DVD structures
 - Added local image cache
 - Improved matching of season/episode in filenames
 - Bugfix: Skipping of extra files in BluRay and DVD structures
 - Small GUI changes
 - TV Shows: Show all banners not just season specific
 - Bugfix: Opening movie folders in network shares
 - Option to use DVD order when scraping TV shows
 - Save DVD and BluRay fanart inside the directory structures
 - Correctly detect stream details in DVD structures
 - Bugfix: Cinefacts scraper didn’t find year and countries
 - Option to delete images
 - Changed default rename pattern for TV show episodes
 - Added filter items for studios, countries and filename
 - Fixed tag cloud UI bug
 - Bugfix: “Watched” Filter showed unwatched movies
 - Automatically set playcount when watched flag changes
 - Bugfix: Downloading season artwork
 - Added director and tags filter
 - Added downloading of tv themes
 - Bugfix: Missing Qt translation added
 - Double click movie in Genre, Certification or Sets Widget jumps to movie
 - Use locale aware sorting in lists
 - Bugfix: Store tvdbid also in id tag
 - Added support for multi-episode files
 - NFO parser is more tolerant with thousands separators
 - Bugfix: Don’t write illegal last played date
 - Bugfix: Don’t write empty stream details
 - Artwork can be loaded separately
 - Settings: Directory string editable manually
 - Filters for Resolution and format
 - Added option in advancedsettings to only store the first studio (helpful for displaying studio logos in XBMC)
 - Added button to set YouTube dummy link
 - Bugfix: Channel count detection wrong for DTS-HD and others
 - Bugfix: Use correct info labels for audio formats


### 1.5 - P'Jem (2013-03-04)

 - Bugfix: Replace illegal characters in filenames on Windows
 - Reduced memory usage when multi scrape movies
 - Sort lists case insensitive
 - Added .wtv files to scanners
 - Changed default filename from cdart.png to disc.png
 - Movies -> Votes: Display thousands separator
 - Bugfix: Skip downloading of actor images in TV shows if set in settings
 - Correct naming of nfo files and images in stacked movies
 - Fixed opening of movie folders in Windows
 - Save TV Show update option
 - Default exclude words set
 - Show saving progress
 - Bugfix: Saving of poster and fanarts for DVD and BluRay structures
 - Skip extra files in DVD structures
 - Show all fanarts from The TV Db when choosing season fanart
 - Save TV show specials season artwork as `season-special-*.jpg`
 - Use JSON RPC to communicate with XBMC
 - Fixed XBMC sync
 - Moved advanced settings to advancedsettings.xml
 - Option to ignore articles when sorting
 - Map genres while scraping to custom genres
 - Extensions for file scanners are now customizable
 - Added alphabetical scroll list
 - Automatically reload files after saving settings
 - Use correct nfo filename when renaming episodes
 - StreamDetails: Map audio and video codecs to custom values
 - Add genres in genre widget
 - Add certifications in certifications widget
 - Add movie sets in sets widget
 - Remove movie sets in sets widget
 - Rename movie sets in sets widget
 - Bugfix: Correct sorting of stacked files
 - Show multi scrape dialog when multiple movies are selected
 - Map certifications while scraping to custom certifications
 - Choose between English GB and English US certifications for The Movie DB
 - Handling of movie set posters and fanarts (compatible with “Movie Set Artwork Automator”)
 - Media Status Columns added
 - TV Shows: ID Tag get’s no longer removed
 - Bugfix: Loading only fanarts was not possible
 - Bugfix: Studios autocompleter showed duplicates
 - Map Studios and Countries to custom values while scraping


### 1.4 - Cardassia Prime (2013-02-04)

 - Added option to set username and password for XBMC webserver
 - Retrieve play count and last played from XBMC
 - Removed SQL Interface
 - Added .img file extension for movies and concerts
 - Speedup loading items from cache database (Thanks to garretn)
 - Fixed wrong xml tag fileinfos -> fileinfo
 - Added support for wmv, ogm, mov and divx files
 - Removed media center path from settings
 - Store files separately in cache database to avoid "," bug
 - Fixes for Qt5 support
 - Added action in context menu: Open Folder
 - Bugfix: Show resolution for images from TMDB
 - Improved TV Show scraping: select infos to load
 - Added displayseason and displayepisode for TV Show episodes
 - Added option to scrape multiple movies at once
 - Art Widget: Show all images when window is big enough
 - Show badge if local trailer is available
 - Added HD-Trailers.net Scraper
 - Added Trailer Preview
 - Added support for Tags
 - Fixed Cinefacts Scraper
 - Added support for TV Show Season Fanarts and Banners
 - Added option to enable/disable downloading of actor images
 - Added option to choose between default filenames for Eden and Frodo
 - Added option to rename artwork to match Frodo naming scheme
 - Added support for extra fanarts
 - Added Fanart.tv Music Artists as source for concert backdrops and logos
 - Use ScrollAreas in MainWindow to allow smaller window height
 - Renaming of Movies,  TV Shows and Concerts
 - Fixed naming of posters and fanarts with stacked files


### 1.3 - Khitomer (2012-12-06)

 - Ignore .actors folder while scanning
 - Fixed Fanart.tv scraper
 - Fixed VideoBuster scraper
 - Cache the contents of nfo files to speedup start
 - Fixed switched filenames of clearart and logo
 - Small GUI and translation fixes
 - Added support for .rmvb files
 - Only save really changed movies when merging genres or certifications
 - Added support for artist and album in concerts
 - Editable outline in movies
 - Added Trailer Downloader
 - Added IMDB Scraper
 - Added XBMC Sync option
 - Added Portuguese translation (Thanks to Wanilton Campos)
 - Added Rating votes and Top 250 fields for movies
 - Bugfix: Loading of StreamDetails failed in Windows when filenames had special characters


### 1.2 - Qo'noS (2012-11-12)

 - Support for streamdetails (resolution, codec, audio etc)
 - Filenames for nfos and images now fully customizable
 - Added french translation (Many thanks to foX aCe)
 - Mark HD/SD content from Fanart.tv
 - Skip -sample files
 - Change sort order of movies
 - Minor GUI improvements
 - Pickup movie set from TMDb
 - Save episodeguide url in TV show xml
 - Windows: Added missing SSL libraries (needed by VideoBuster Scraper)
 - Bugfix: Possible crash when selecting a movie, TV show or concert


### 1.1 - Andoria (2012-10-22)

 - Fanart.tv: Support for Logos, Clear Arts, CD Arts and Character Arts
 - Improved filter widget
 - Organize movies: move into separate directories (Thanks to googl1)
 - Prefer locale posters from TMDb (Thanks to jhenkens)
 - Show new mark on TV show seasons
 - Added .dat, .ts, .vob and .flv file extensions
 - Simple caching of scanned folder to speedup scanning for files
 - Added director and writer fields for movies
 - A proxy can be used for network request
 - Episodes with naming pattern 1×01 are now recognized
 - Highlight new entries in sidebar
 - Small GUI improvements
 - Increased maximum season and episode numbers
 - Bugfix: Fixed memory problem
 - Bugfix: Handle network timeouts while downloading images


### 1.0.0 - Vulcan (2012-10-08)

 - Now with codename
 - Added Concert/MusicVideo scraping
 - Added keyboard shortcuts
 - Rudimentary command line interface
 - Editing of sort titles possible
 - Option to save trailer urls in YouTube plugin format
 - Choose between different files for nfos, posters, fanarts and banners
 - Settings is now a dialog
 - Added .strm file extension
 - Bugfix: Adding of TV show directories possible again


### 0.9.6 (2012-09-20)

 - Ignore files in directory “Extras” (thanks to KBagust)
 - Allow TV Show episodes to have . inbetween season and episode numbers (thanks to KBagust)
 - Added debug output
 - Bugfix: Toolbar hidden in Mac OS X with small resolutions


### 0.9.5 (2012-09-20)

 - TV show episodes are grouped by seasons
 - Markers for new movies and TV shows
 - Skip empty values and images in TMDb
 - Added support .m4v file extension
 - Increased size of Fanart/Poster dialog
 - Feature: Revert changes
 - Bugfix: Crash when loading MediaElch
 - Bugfix: Scraping issues with special characters in movie titles
 - Bugfix: TV Shows had wrong path when episodes are in subfolders
 - Bugfix: Confusion in SQL interfaces when different episodes have equal filenames
 - Bugfix: Checkbox “separate folders” won’t show when adding a movie directory
 - Bugfix: Changing just poster or fanart in TV shows won’t enable the save button


### 0.9.4 (2012-06-29)

 - Fixed not escaped characters in SQL interface


### 0.9.3 (2012-06-29)

 - Added support for TV Show banners
 - Added Set Editor (add/remove movies from sets, reorder movies, change posters and backdrops)
 - View and change images of actors
 - Skip trailer files when searching for movies
 - Rudimentary support for stub files (need to be created on your own)
 - Switched to TMDb API v3
 - Local certifications from TMDb
 - Support metafiles like movie.nfo, movie.tbn, movie.jpg, fanart.jpg, folder.jpg
 - Select which infos should be loaded from a scraper
 - Fixed a bug when using SQL interfaces and "movies are stored in separate folders" is checked in XBMC


### 0.9.2 (2012-06-29)

 - Bugfixes and some new features
