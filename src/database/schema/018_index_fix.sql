-- in version 014, the index was not created properly because the
-- SQL contained an issue.  It was fixed, but we need to migrate
-- nonetheless.

DROP INDEX IF EXISTS id_label_filename_idx;
-- next

CREATE INDEX id_label_filename_idx ON labels(fileName);
