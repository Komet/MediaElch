
DROP TABLE IF EXISTS movieSubtitles;
-- next

CREATE TABLE IF NOT EXISTS movieSubtitles (
    "idSubtitle" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "idMovie" integer NOT NULL,
    "files" text NOT NULL,
    "language" text NOT NULL,
    "forced" integer NOT NULL
);
-- next

CREATE INDEX id_subtitle_idx ON movieSubtitles(idMovie);
