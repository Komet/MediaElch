-- GENERATED BY testSchema.cpp
-- DO NOT EDIT MANUALLY!
--
-- instead, introduce new migration scripts and run the
-- tests to update the reference file.

CREATE TABLE movies (
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

CREATE TABLE sqlite_sequence(name,seq);

CREATE TABLE movieFiles(
    "idFile" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idMovie" integer NOT NULL,
    "file" text NOT NULL
);

CREATE INDEX id_movie_idx ON movieFiles(idMovie);

CREATE TABLE concerts (
    "idConcert" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "inSeparateFolder" integer NOT NULL,
    "path" text NOT NULL
);

CREATE TABLE concertFiles(
    "idFile" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idConcert" integer NOT NULL,
    "file" text NOT NULL
);

CREATE INDEX id_concert_idx ON concertFiles(idConcert);

CREATE TABLE shows (
    "idShow" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "dir" text NOT NULL,
    "content" text NOT NULL,
    "path" text NOT NULL
);

CREATE TABLE episodes (
    "idEpisode" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "idShow" integer NOT NULL,
    "seasonNumber" integer NOT NULL,
    "episodeNumber" integer NOT NULL,
    "path" text NOT NULL
);

CREATE TABLE episodeFiles(
    "idFile" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idEpisode" integer NOT NULL,
    "file" text NOT NULL
);

CREATE INDEX id_episode_idx ON episodeFiles(idEpisode);

CREATE TABLE labels (
    "idLabel" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "color" integer NOT NULL,
    "fileName" text NOT NULL
);

CREATE TABLE importCache (
    "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "filename" text NOT NULL,
    "type" text NOT NULL,
    "path" text NOT NULL
);

CREATE TABLE settings (
    "idSettings" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "value" text NOT NULL
);

CREATE TABLE artists (
    "idArtist" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "dir" text NOT NULL,
    "path" text NOT NULL
);

CREATE TABLE albums (
    "idAlbum" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idArtist" integer NOT NULL,
    "content" text NOT NULL,
    "dir" text NOT NULL,
    "path" text NOT NULL
);

CREATE TABLE movieSubtitles (
    "idSubtitle" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idMovie" integer NOT NULL,
    "files" text NOT NULL,
    "language" text NOT NULL,
    "forced" integer NOT NULL
);

CREATE INDEX id_subtitle_idx ON movieSubtitles(idMovie);

CREATE TABLE showsEpisodes (
    "idEpisode" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "idShow" integer NOT NULL,
    "seasonNumber" integer NOT NULL,
    "episodeNumber" integer NOT NULL,
    "tmdbid" text NOT NULL,
    "updated" integer NOT NULL
);

CREATE TABLE showsSettings (
    "idShow" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "tmdbid" text NOT NULL,
    "url" text NOT NULL,
    "showMissingEpisodes" integer NOT NULL,
    "hideSpecialsInMissingEpisodes" integer NOT NULL,
    "dir" text NOT NULL
);

CREATE INDEX id_label_filename_idx ON labels(fileName)