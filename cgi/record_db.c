#include "record_db.h"

#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db_config.h"

#define SQL_BUFFER_SIZE 20000

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

bool record_db_select_all(Record **records) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char sql[512];
    Record *items = NULL;
    Record *record;

    if (!records) {
        printf("Invalid record output<br>\n");
        return false;
    }

    *records = NULL;

    conn = db_connect();
    if (!conn) {
        return false;
    }

    snprintf(sql, sizeof(sql), "SELECT * FROM %s ORDER BY id ASC", TABLE_NAME);
    if (mysql_query(conn, sql)) {
        printf("DB query error: %s<br>\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    result = mysql_store_result(conn);
    if (!result) {
        printf("DB result error: %s<br>\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    size_t count = mysql_num_rows(result);

    items = malloc((count + 1) * sizeof(Record));
    if (!items) {
        mysql_free_result(result);
        mysql_close(conn);
        printf("Out of memory<br>\n");
        return false;
    }

    memset(items, 0, (count + 1) * sizeof(Record));

    record = items;
    while ((row = mysql_fetch_row(result)) != NULL) {
        fill_record_from_row(record, row);
        record++;
    }

    *records = items;
    mysql_free_result(result);
    mysql_close(conn);
    return true;
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

    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE id=%ld", TABLE_NAME, id);
    if (mysql_query(conn, sql)) {
        printf("DB delete error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    mysql_close(conn);
    return true;
}

bool record_db_save(const Record *record, bool update_existing, long *saved_id) {
    MYSQL *conn;
    char sql[SQL_BUFFER_SIZE];
    char q_title[RECORD_TITLE_SIZE * 2 + 3];
    char q_datetime[RECORD_DATETIME_SIZE * 2 + 3];
    char q_amount[RECORD_AMOUNT_SIZE];
    char q_category[RECORD_CATEGORY_SIZE * 2 + 3];
    char q_color[RECORD_COLOR_SIZE * 2 + 3];
    char q_address[RECORD_ADDRESS_SIZE * 2 + 3];
    char escaped_title[RECORD_TITLE_SIZE * 2 + 1];
    char escaped_datetime[RECORD_DATETIME_SIZE * 2 + 1];
    char escaped_category[RECORD_CATEGORY_SIZE * 2 + 1];
    char escaped_color[RECORD_COLOR_SIZE * 2 + 1];
    char escaped_address[RECORD_ADDRESS_SIZE * 2 + 1];
    char normalized_datetime[RECORD_DATETIME_SIZE];
    char *amount_end = NULL;

    if (!record || !record->title[0]) {
        printf("Title is required<br>\n");
        return false;
    }

    if (record->amount[0]) {
        strtod(record->amount, &amount_end);
        if (!amount_end || *amount_end != '\0') {
            printf("Amount must be numeric<br>\n");
            return false;
        }
        snprintf(q_amount, sizeof(q_amount), "%s", record->amount);
    } else {
        snprintf(q_amount, sizeof(q_amount), "0");
    }

    conn = db_connect();
    if (!conn) {
        return false;
    }

    mysql_real_escape_string(conn, escaped_title, record->title, (unsigned long)strlen(record->title));
    snprintf(q_title, sizeof(q_title), "'%s'", escaped_title);

    if (record->datetime[0]) {
        snprintf(normalized_datetime, sizeof(normalized_datetime), "%s", record->datetime);
        for (char *p = normalized_datetime; *p; p++) {
            if (*p == 'T') {
                *p = ' ';
            }
        }

        if (strlen(normalized_datetime) == 16) {
            snprintf(normalized_datetime + 16, sizeof(normalized_datetime) - 16, ":00");
        }

        mysql_real_escape_string(conn, escaped_datetime, normalized_datetime, (unsigned long)strlen(normalized_datetime));
        snprintf(q_datetime, sizeof(q_datetime), "'%s'", escaped_datetime);
    } else {
        snprintf(q_datetime, sizeof(q_datetime), "CURRENT_TIMESTAMP");
    }

    if (record->category[0]) {
        mysql_real_escape_string(conn, escaped_category, record->category, (unsigned long)strlen(record->category));
        snprintf(q_category, sizeof(q_category), "'%s'", escaped_category);
    } else {
        snprintf(q_category, sizeof(q_category), "NULL");
    }

    if (record->color[0]) {
        mysql_real_escape_string(conn, escaped_color, record->color, (unsigned long)strlen(record->color));
        snprintf(q_color, sizeof(q_color), "'%s'", escaped_color);
    } else {
        snprintf(q_color, sizeof(q_color), "NULL");
    }

    if (record->address[0]) {
        mysql_real_escape_string(conn, escaped_address, record->address, (unsigned long)strlen(record->address));
        snprintf(q_address, sizeof(q_address), "'%s'", escaped_address);
    } else {
        snprintf(q_address, sizeof(q_address), "NULL");
    }

    snprintf(sql, sizeof(sql),
         "INSERT INTO %s (id, title, datetime, amount, category, checklist1, checklist2, color, address) "
         "VALUES (%ld, %s, %s, %s, %s, %d, %d, %s, %s) "
         "ON DUPLICATE KEY UPDATE "
         "title=%s, datetime=%s, amount=%s, category=%s, "
         "checklist1=%d, checklist2=%d, color=%s, address=%s",
         TABLE_NAME,
         record->id, q_title, q_datetime, q_amount,
         q_category, record->checklist1 ? 1 : 0, record->checklist2 ? 1 : 0,
         q_color, q_address,
         q_title, q_datetime, q_amount,
         q_category, record->checklist1 ? 1 : 0, record->checklist2 ? 1 : 0,
         q_color, q_address);

    if (mysql_query(conn, sql)) {
        printf("DB save error: %s<br>\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    if (saved_id) {
        *saved_id = update_existing ? record->id : (long)mysql_insert_id(conn);
    }

    mysql_close(conn);
    return true;
}