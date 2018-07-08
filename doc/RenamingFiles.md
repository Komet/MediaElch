# Renaming files and directories 

MediaElch is able to rename your files and folders.
You are able to define how you want your files and folders to be named.
After clicking on "Rename selected files" in the main toolbar you see a bunch of placeholders which can be used for renaming: Some are just placeholders (the ones in `<...>`) which are replaced every time and some are conditional placeholders. The text inside the latter is only used if the condition is true.

Lets do an example: Assume two movies

 1. "My Awesome Movie", year is set to 2000, and it's a 3D movie (selected in the Stream Details tab)
 2. "Ordinary Movie", year is set to 1999.
 3. "Sequel Movie", year is set to 2010, movieset is set to "Movie collection"

Using the following rename pattern

 - Folder: `{movieset}<movieset> - {/movieset}<title> (<year>)`
 - File: `<title>{3D}.3D{/3D}.<extension>`

the movies will be renamed to:

 1. folder: **My Awesome Movie (2000)**, file: `My Awesome Movie.3D.mkv`
 2. folder: **Ordinary Movie (1999)**, file: `Ordinary Movie.mkv`
 3. folder: **Movie Collection - Sequel Movie (2010)**, file: `Sequel Movie.mkv`

The following conditional placeholders are available:

 - `{bluray}{/bluray}` The movie is a bluray structure
 - `{dvd}{/dvd}` The movie is a dvd structure
 - `{3D}{/3D}` It's a 3D movie, you selected a stereo mode in the stream details tab (if not detected automatically)
 - `{movieset}<movieset>{/movieset}` The movieset is not empty
 - `{imdbId}<imdbId>{/imdbId}` The IMDB Id of the movie

If you are unsure if your rename pattern gives the result you want, just hit the "Simulate" button. No file is renamed then but you'll see the resulting folder- and filenames.
