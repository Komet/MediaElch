# Music scraping 

Starting with version 2.2 MediaElch is able to scrape information about your music collection.

This feature is currently in a beta state and should be used with some care. Please try it out, check the resulting nfo files and if something goes wrong open a report in the bug reports forum.

To use the music scraping your directory structure must look like the following:

 - /Volumes/Data/Music/Artist1
 - /Volumes/Data/Music/Artist1/Album1
 - /Volumes/Data/Music/Artist1/Album1/song.mp3
 - /Volumes/Data/Music/Artist2
 - /Volumes/Data/Music/Artist2/Album2
 - /Volumes/Data/Music/Artist2/Album2/song.mp3

This means every artist must have its own subdirectory and inside this directory every album must be in another subdirectory. In MediaElchs settings add the toplevel directory of the artists (in the example above this is `/Volumes/Data/Music`).

Scrapers

There is only one scraper for music artists and albums: Universal Music Scraper.
This scraper combines The Audio DB, AllMusic and Discogs for information about artists and albums.
In the settings you can select the language (just used for The Audio DB) and which one you prefer.
All images are scraped from Fanart.tv.
Extra fanart images are also downloaded automatically.
How much images should be loaded can be adjusted in the settings.
Set it to 0 if you don't want extra fanarts to be downloaded automatically.
