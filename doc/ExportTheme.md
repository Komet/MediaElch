# Export Theme

MediaElchs export feature allows you to export your whole collection as HTML files. You can also create a new export theme in a few steps:

**Installing the Dummy theme**

At first you should install the "Dummy Theme". The files will be located at the following places:

 - Mac OS: `/Users/username/Library/Application Support/kvibes/MediaElch/export_themes/dummy`
 - Linux: `/home/username/.local/share/data/kvibes/MediaElch/export_themes/dummy`
 - Windows: `C:\Users\username\AppData\Local\kvibes\MediaElch\export_themes\dummy`

Now just rename the dummy folder as you like. The new name should not contain any special characters or spaces and match the name of your theme.

**Adjust the themes metadata**

Open `metadata.xml` and adjust the information inside:

 - **name** should be the name of your theme
 - identifier needs to be an unique identifier and exactly match the folder name you chose before
 - **add** description tags in languages you want
 - **author** should be your name
 - **version** needs to be a numeric
 - **supports** contains the items which could be exported with your theme

**Customizing the templates**

All other files are just plain HTML files with some placeholders.

If you place other files inside your theme folder as the ones mentioned below they will be copied on export. This means you can also create subfolders like css or js and put files inside this folders.

The placeholders will look like {{ MOVIE.TITLE }} (case and spaces matter). If there are blocks of information (a loop) it will be inside a block placeholder {{ BEGIN_BLOCK_GENRES }}...{{ END_BLOCK_GENRES }}.

Images will be resized to the size you want: {{ IMAGE.POSTER[200, 400] }} This will insert an image with the maximum size of 200px width and 400px height, aspect ratio will be kept. For every image you include you should place a default image (png format) in the defaults folder which should be named (for example): movie_poster_200x400.png 

**Movies**

`movies.html` is a list view which should show the movies title.  
`movies/movie.html` is the template for all information about a movie

**TV Shows and Episodes**

`tvshows.html` is a list view which should show the tv shows title.

`tvshows/tvshow.html` is the template for all information about the show and should contain a list of all episodes.

`episodes/episode.html` should contain all information about an episode.

**Concerts**

*concerts.html* is a list view which should show the concerts title.  
`concerts/concert.html` is the template for all information about a concert.

**Publishing your theme**

As MediaElch is available for free it would be nice if you would share your created theme. Just create a zip archive and send it to support@mediaelch.de. Thank you!
