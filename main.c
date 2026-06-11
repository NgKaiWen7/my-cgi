#include <mysql.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi/db_config.h"
#include "cgi/record.h"

static MYSQL *db_connect(void) {
    MYSQL *conn = mysql_init(NULL);

    if (!conn) {
        printf("mysql_init() failed<br>\n");
        return NULL;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        printf("DB connect error: %s<br>\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}


static void fill_record_from_row(Record *record, MYSQL_ROW row) {
    memset(record, 0, sizeof(*record));
    if (row[0]) {
        record->id = strtol(row[0], NULL, 10);
    } else {
        record->id = 0;
    }
    if (row[1]) {
        snprintf(record->title, sizeof(record->title), "%s", row[1]);
    } else {
        snprintf(record->title, sizeof(record->title), "%s", "");
    }

    if (row[2]) {
        snprintf(record->datetime, sizeof(record->datetime), "%s", row[2]);
    } else {
        snprintf(record->datetime, sizeof(record->datetime), "%s", "");
    }

    if (row[3]) {
        snprintf(record->amount, sizeof(record->amount), "%s", row[3]);
    } else {
        snprintf(record->amount, sizeof(record->amount), "%s", "");
    }

    if (row[4]) {
        snprintf(record->category, sizeof(record->category), "%s", row[4]);
    } else {
        snprintf(record->category, sizeof(record->category), "%s", "");
    }

    record->checklist1 = row[5] && strcmp(row[5], "0") != 0; // convert to boolean
    record->checklist2 = row[6] && strcmp(row[6], "0") != 0; // convert to boolean

    if (row[7]) {
        snprintf(record->color, sizeof(record->color), "%s", row[7]);
    } else {
        snprintf(record->color, sizeof(record->color), "%s", "");
    }

    if (row[8]) {
        snprintf(record->address, sizeof(record->address), "%s", row[8]);
    } else {
        snprintf(record->address, sizeof(record->address), "%s", "");
    }
}
bool record_db_find_by_id(long id, Record *record) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char sql[512];
    bool ok = false;

    if (!record || id <= 0) {
        printf("Invalid record id<br>\n");
        return false;
    }

    conn = db_connect();
    if (!conn) {
        return false;
    }

    snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE id=%ld LIMIT 1", TABLE_NAME, id);

    if (mysql_query(conn, sql)) {
        printf("DB query error: %s<br>\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    result = mysql_store_result(conn);
    if (!result) {
        printf("DB result error: %s", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    row = mysql_fetch_row(result);
    if (row) {
        fill_record_from_row(record, row);
        ok = true;
    } else {
        printf("Record not found<br>\n");
    }

    mysql_free_result(result);
    mysql_close(conn);
    return ok;
}

bool record_db_delete(long id) {
    MYSQL *conn;
    char sql[256];

    if (id <= 0) {
        printf("Invalid record id<br>\n");
        return false;
    }

    conn = db_connect();
    if (!conn) {
        return false;
    }

    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE id=%ld LIMIT 1", TABLE_NAME, id);
    if (mysql_query(conn, sql)) {
        printf("DB delete error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    mysql_close(conn);
    return true;
}
int main(int argc, char *argv[])
{
    Record *record = malloc(sizeof(Record));

    (void)argc;
    (void)argv;

    memset(record, 0, sizeof(Record));
    if (!record_db_find_by_id(2, record)) { // Replace 1 with the desired record ID
        free(record);
        mysql_library_end();
        return EXIT_FAILURE;
    }

    printf("%ld - %s - %s - %s - %s - %d - %d - %s - %s\n", record->id, record->title, record->datetime, record->amount, record->category, record->checklist1, record->checklist2, record->color, record->address);

    free(record);
    mysql_library_end();
    return EXIT_SUCCESS;
}
