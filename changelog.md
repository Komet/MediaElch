# Changelog

## 2.4.3 (*tbd*)

### Features
 
 - Add user documentation (#531)
 - Use HTTPS for scraping (#371)
 - Set UI language in ``advancedsettings.xml`` (#411)
 - Add Kino.de scraper (#370)
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

### Bugfixes

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

### Improvements

 - UI: line break in tree views (#406)
 - UI: add 4k and 8k resolution flag (#446)
 - UI: show green ID flag only if IMDB is valid (#471)


## 2.4.2 - Talax (2016-07-01)

 - Bugfix: UniversalMusicScraper broken


## 2.4.1 - Talax (2016-03-20)

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


## 2.4.0 - Talax (2015-12-06)

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
 - Move tv shows with new items to the top
 - Don't remove custom tags in nfo files
 - StreamDetails: Use Channel(s)_Original if available
 - Improve UI rendering in Windows
 - Get and store IMDB ID for tv show episodes
 - Support for TV show status (continuing/ended)
 - Use lowercase video and audio codec in streamdetails
 - Save all selected items when saving
 - Votes and top 250 for tv shows and episodes
 - TV Shows: Improved UI in listview
 - Support image formats with wrong file extension
 - Musicscraper: Label, release date and artist is not set
 - IMDB: Studios not detected
 - Windows: Black screen when started via RDP
 - Hotmovies scraper broken
 - StreamDetails: Sometimes not loaded in Windows when filename contains special characters
 - Crash when selecting open folder on empty list


## 2.3.2 - Denobula (2015-10-12)

 - Write urls to nfos by default
 - Fixed dependencies for wily


## 2.3.1 - Denobula (2015-10-10)

 - Bugfix on Coverlib.com scraper
 - Fixed package name in debian control file


## 2.3.0 - Denobula (2015-10-10)

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


## 2.2.2 - Sphere Builder (2015-02-02)

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


## 2.2.1 - Sphere Builder (2015-01-25)

 - Media Passion: API URL changed


## 2.2 - Sphere Builder (2015-01-23)

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
 - Fanart.tv: Add support for tv show posters
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


## 2.1.3 - Trill

 - Update package for Trusty


## 2.1.2 - Trill (2014-03-27)

 - Add info about collection
 - Join genres in nfo files
 - Rename tv shows: dialog doesn't open ...
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


## 2.1.1 - Trill (2014-03-17)

 - Filescanner dialog not visible


## 2.1 - Trill (2014-03-16)

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


## 2.0.6 - Risa (2013-10-31)

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


## 2.0 - Risa (2013-10-28)

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
 - Actors for tv show episodes
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
 - Runtime of *.ts files is not detected
 - Images from SMB shares under Windows cannot be chosen
 - Filescanner might skip some movies
 - MediaPassion: Discart sometimes not found
 - MediaPassion: Original title not scraped
 - Some images are not renamed when renaming movies
 - TV Show list jumps to not clicked items


## 1.7 - Bajor (2013-08-08)

 - *no information*


## 1.6 - Romulus (2013-04-23)

 - *no information*


## 1.5 - P'Jem (2013-03-04)

 - *no information*


## 1.4 - Cardassia Prime (2013-02-04)

 - *no information*


## 1.3 (2012-12-06)

 - *no information*


## 1.2 (2012-11-12)

 - *no information*


## 1.1 (2012-10-22)

 - *no information*


## 1.0.0 (2012-10-08)

 - *no information*


## 0.9.6 (2012-09-12)

 - *no information*


## 0.9.4 (2012-06-29)

 - *no information*


## 0.9.3 (2012-06-29)

 - *no information*


## 0.9.2 (2012-06-29)

 - *no information*
