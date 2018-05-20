# Windows: Portable Mode 

To make MediaElch run in "portable mode" place an advancedsettings.xml in MediaElchs application directory with at least the following contents: 

```xml
<advancedsettings>
	<portableMode>true</portableMode>
</advancedsettings>
```

MediaElch will then store its settings, temporary files and caches in its application directory and not in standard locations based on your OS.

The portable mode is only available on Windows.