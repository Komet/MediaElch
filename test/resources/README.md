# Test Resources

Many of the NFO files in this directory were created using Kodi.
We had to change a few things, though, because of how MediaElch and Qt handle XML:

 - order of tags and attributes (defined by the order in a `QHash<QString>`)
 - replaced `&quot;` with `"` because `QDomText` does not escape them unless necessary
 - replaced `&apos;` with `'` because `QDomText` does not escape them unless necessary

## Changes only for movies

 - added empty `<sorttitle>` (not set by Kodi unless specified)
 - removed following tags because they're set by Kodi v17 but not used:
   - see https://kodi.wiki/view/NFO_files/Movies#Known_Issues
   - `<status></status>`
   - `<code></code>`
   - `<aired></aired>`
 - changed resume time to != 0.0

## Changes only for TV shows

 - set `<outline>` with the contents with `<plot>`
 - remove following tags because they are unused in TV show NFO files (may be used in episode files)
   - see https://kodi.wiki/index.php?title=NFO_files/TV_shows#TV_Show_Tags
   - `<outline></outline>`
   - `<displayseason></displayseason>`
   - `<displayepisode></displayepisode>`
   - `<resume></resume>` (only for episodes)
   - `<aired></aired>`
   - `<code></code>`
 - add common URL prefix to `<fanart>`; reason: MediaElch may add fanart from different domains so we don't add a `url` tag to `<fanart>` like Kodi does
 - change order of thumb tag to ascending order of seasons
 - set `<episode>` and `<season>` to `0` as they are determined by the actual number of scanned episodes and not a tag
 - remove following tags as they are not yet supported by MediaElch:
   - `<playcount>0</playcount>`
   - `<lastplayed></lastplayed>`
   - `<tagline></tagline>`
 - removed ` post="yes" cache="auth.json"` from `<episodeguide>` as it is a TVDBv2 feature which we don't yet support;
   see https://forum.kodi.tv/showthread.php?tid=323588

## Changes only for concerts

 - main tag `<movie>` renamed to `<musicvideo>`
