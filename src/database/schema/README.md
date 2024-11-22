# SQL Database

We version the database.  The SQL statements need to be applied in order.
Because Qt can only execute one statement at a time, we use `-- next`
as a statement delimiter.

The file `999_full_schema.sql` contains the database schema as if all
migrations were executed.  We use SQLite to get the final schema.
It isn't used by MediaElch itself and is only intended for documentation.

The benefit of using migrations is easy: users can keep their cache and
don't have to refresh on each MediaElch update.  By applying migrations one
at a time, we don't need to care if users migrate from database
v14 to v16 or v18.
