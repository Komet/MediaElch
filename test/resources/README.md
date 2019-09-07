# Test Resources

Many of the NFO files in this directory were created using Kodi.
We had to change a few things, though, because of how MediaElch and Qt handle XML:

 - order of tags and attributes (defined by the order in a `QHash<QString>`)
 - replaced `&quot;` with `"` because `QDomText` does not escape them unless necessary
 - replaced `&apos;` with `'` because `QDomText` does not escape them unless necessary
 - replaced `&#x0A;` with linebreaks because `QDomText` does not escape linebreaks
 - strip last zeroes from floating point numbers (e.g. `7.5000` => `7.5`)

## Changes only for movies

 - added empty `<sorttitle>` (not set by Kodi unless specified)
 - removed following tags because they're set by Kodi v17 but not used:
   - see https://kodi.wiki/view/NFO_files/Movies#Known_Issues
   - `<status></status>`
   - `<code></code>`
   - `<aired></aired>`

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

## Changes for TV show episodes

 - add root element `<episodes>` because multiple root elements are expected by Kodi which is actually invalid XML
 - remove `<displayseason>` if its value is `< 0` (i.e. no season)
 - remove `<displayepisode>` if its value is `< 0` (i.e. no episode)
 - remove empty `<outline>`
 - remove empty `<thumb>` tags inside `<actor>`

### Tags to be implemented
```xml
<genre>...</genre>
<tagline></tagline>
<runtime>0</runtime>
<premiered>2005-02-06</premiered>
<year>2005</year>
<status></status>
<code></code>
<trailer></trailer>
<resume>
    <position>0.0</position>
    <total>0.0</total>
</resume>
<dateadded>2019-07-07 13:38:41</dateadded>
```


## Changes only for concerts

 - main tag `<movie>` renamed to `<musicvideo>`


## Changes only for artists (music)

 - removed some tags as they are not yet supported by MediaElch (see next section)
 - removed `<path></path>` tag as it is "Exported but not used on Import"
   (from [Kodi wiki](https://kodi.wiki/view/NFO_files/Music#nfo_Tags))
 - for testing, these tags were added to "AC/DC" (e.g. to have multiple genre tags):
     - added another genre `Rock`
     - added another mood `Really Energetic`
     - added another style `Pop/Rock`

### Tags to be implemented

```xml
<disambiguation></disambiguation>
<gender></gender>
<sortname></sortname>
<instruments></instruments>
```

Furthermore MediaElch should support multiple `yearsactive` tags.

## Changes only for albumbs (music)

 - removed `<path></path>` tag as it is "Exported but not used on Import"
 - removed `<releasetype></releasetype>` as it's "not used when scanned"
 - removed some tags as they are not yet supported by MediaElch (see next section)
 - removed `<artistdesc></artistdesc>` as it is "not merged when scanned"

### Tags to be implemented

```xml
<compilation>false</compilation>
<userrating max="10">-1</userrating>
<rating max="10">8.1</rating> <!-- max attribute -->
<votes>-1</votes>
<!-- MediaElch does not read this tag and instead uses the artist's id -->
<musicBrainzArtistID>66c662b6-6e2f-4930-8610-912e24c63ed1</musicBrainzArtistID>
```
