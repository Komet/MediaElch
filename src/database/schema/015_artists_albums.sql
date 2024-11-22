
DROP TABLE IF EXISTS artists;
-- next
DROP TABLE IF EXISTS albums;
-- next

CREATE TABLE IF NOT EXISTS artists (
    "idArtist" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "content" text NOT NULL,
    "dir" text NOT NULL,
    "path" text NOT NULL
);
-- next

CREATE TABLE IF NOT EXISTS albums (
    "idAlbum" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idArtist" integer NOT NULL,
    "content" text NOT NULL,
    "dir" text NOT NULL,
    "path" text NOT NULL
);
