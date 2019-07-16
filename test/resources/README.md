# Test Resources

Many of the NFO files in this directory were created using Kodi.
We had to change a few things, though, because of how MediaElch and Qt handle XML:

 - order of tags and attributes (defined by the order in a `QHash<QString>`)
 - replaced `&quot;` with `"` because `QDomText` does not escape them unless necessary
 - replaced `&apos;` with `'` because `QDomText` does not escape them unless necessary

changes only for movies:

 - added empty `<sorttitle>` (not set by Kodi unless specified)
 - removed following tags because they're set by Kodi v17 but not used:
   - see https://kodi.wiki/view/NFO_files/Movies#Known_Issues
   - `<status></status>`
   - `<code></code>`
   - `<aired></aired>`

and only for TV shows:

 - set `<outline>` with the contents with `<plot>`
 - remove following tags because they are unused in TV show NFO files (may be used in episode files)
   - see https://kodi.wiki/index.php?title=NFO_files/TV_shows#TV_Show_Tags
   - `<outline></outline>`
   - `<displayseason></displayseason>`
   - `<displayepisode></displayepisode>`
   - `<resume></resume>` (only for episodes)
   - `<aired></aired>`
   - `<code></code>`
