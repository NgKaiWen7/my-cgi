# C MySQL Record Editor

This workspace contains a small C command-line program that supports:

- list rows in a table
- show a single row
- insert a new row
- update an existing row
- delete a row

## 1. Create the MySQL table from terminal

Use your MySQL terminal and run:

```sql
SOURCE schema.sql;
```

Or from the shell:

```bash
mysql -u root -p < schema.sql
```

The script creates the database `trial` and the table `trial_table`.

## 2. Build the program

```bash
make
```

This builds `record_editor` from `cgi/record_editor.c`.

If your MySQL credentials are different, edit the `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`, and `TABLE_NAME` constants in the source file.

## 3. Run it

Examples:

```bash
./record_editor list
./record_editor insert --title "Hello" --amount 12.5
./record_editor update --id 1 --title "Updated" --amount 13.0
./record_editor delete --id 1
./record_editor show --id 1
```
