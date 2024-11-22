DROP TABLE IF EXISTS movies;
-- next
DROP TABLE IF EXISTS movieFiles;
-- next
DROP TABLE IF EXISTS concerts;
-- next
DROP TABLE IF EXISTS concertFiles;
-- next
DROP TABLE IF EXISTS shows;
-- next
DROP TABLE IF EXISTS showsSettings;
-- next
DROP TABLE IF EXISTS episodes;
-- next
DROP TABLE IF EXISTS showsEpisodes;
-- next
DROP TABLE IF EXISTS episodeFiles;
-- next
DROP TABLE IF EXISTS settings;
-- next
DROP TABLE IF EXISTS importCache;
-- next
DROP TABLE IF EXISTS labels;
-- next

CREATE TABLE IF NOT EXISTS movies (
    "idMovie" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "lastModified" integer NOT NULL,
    "inSeparateFolder" integer NOT NULL,
    "hasPoster" integer NOT NULL,
    "hasBackdrop" integer NOT NULL,
    "hasLogo" integer NOT NULL,
    "hasClearArt" integer NOT NULL,
    "hasCdArt" integer NOT NULL,
    "hasBanner" integer NOT NULL,
    "hasThumb" integer NOT NULL,
    "hasExtraFanarts" integer NOT NULL,
    "discType" integer NOT NULL,
    "path" text NOT NULL
);
-- next

CREATE TABLE IF NOT EXISTS movieFiles(
    "idFile" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idMovie" integer NOT NULL,
    "file" text NOT NULL
);
-- next

CREATE INDEX id_movie_idx ON movieFiles(idMovie);
-- next


CREATE TABLE IF NOT EXISTS concerts (
    "idConcert" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "inSeparateFolder" integer NOT NULL,
    "path" text NOT NULL
);
-- next

CREATE TABLE IF NOT EXISTS concertFiles(
    "idFile" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idConcert" integer NOT NULL,
    "file" text NOT NULL
);
-- next

CREATE INDEX id_concert_idx ON concertFiles(idConcert);
-- next

CREATE TABLE IF NOT EXISTS shows (
    "idShow" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "dir" text NOT NULL,
    "content" text NOT NULL,
    "path" text NOT NULL
);
-- next

CREATE TABLE IF NOT EXISTS showsSettings (
    "idShow" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "tvdbid" text NOT NULL,
    "url" text NOT NULL,
    "showMissingEpisodes" integer NOT NULL,
    "hideSpecialsInMissingEpisodes" integer NOT NULL,
    "dir" text NOT NULL
);
-- next

CREATE TABLE IF NOT EXISTS showsEpisodes (
    "idEpisode" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "idShow" integer NOT NULL,
    "seasonNumber" integer NOT NULL,
    "episodeNumber" integer NOT NULL,
    "tvdbid" text NOT NULL,
    "updated" integer NOT NULL
);
-- next

CREATE TABLE IF NOT EXISTS episodes (
    "idEpisode" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "idShow" integer NOT NULL,
    "seasonNumber" integer NOT NULL,
    "episodeNumber" integer NOT NULL,
    "path" text NOT NULL
);
-- next

CREATE TABLE IF NOT EXISTS episodeFiles(
    "idFile" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idEpisode" integer NOT NULL,
    "file" text NOT NULL
);
-- next

CREATE INDEX id_episode_idx ON episodeFiles(idEpisode);
-- next

CREATE TABLE IF NOT EXISTS labels (
    "idLabel" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "color" integer NOT NULL,
    "fileName" text NOT NULL
);
-- next

CREATE INDEX id_label_filename_idx ON labels(fileName);
-- next

CREATE TABLE IF NOT EXISTS importCache (
    "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "filename" text NOT NULL,
    "type" text NOT NULL,
    "path" text NOT NULL
);
