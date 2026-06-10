#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record.h"

static void escape_sql_value(MYSQL *conn, const char *input, char *output, size_t output_size) {
    char *buffer;
    size_t len;

    if (!input) {
        input = "";
    }

    len = strlen(input);
    buffer = (char *)malloc(len * 2 + 1);
    if (!buffer) {
        snprintf(output, output_size, "''");
        return;
    }

    mysql_real_escape_string(conn, buffer, input, (unsigned long)len);
    snprintf(output, output_size, "'%s'", buffer);
    free(buffer);
}

int main(void) {
    MYSQL *conn;
    char sql[4096];
    char q_title[512], q_datetime[128], q_category[256], q_color[64], q_address[2048];
    Record record = {
        0,
        "Sample Title",
        "2026-06-10 09:00:00",
        "123.45",
        "new",
        true,
        false,
        "#3366ff",
        "Line 1\nLine 2"
    };

    const char *server = "localhost";
    const char *user = "nkw";
    const char *password = "nkw";
    const char *database = "trial";

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return EXIT_FAILURE;
    }

    if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "Connection failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    escape_sql_value(conn, record.title, q_title, sizeof(q_title));
    escape_sql_value(conn, record.datetime, q_datetime, sizeof(q_datetime));
    escape_sql_value(conn, record.category, q_category, sizeof(q_category));
    escape_sql_value(conn, record.color, q_color, sizeof(q_color));
    escape_sql_value(conn, record.address, q_address, sizeof(q_address));

    snprintf(sql, sizeof(sql),
             "INSERT INTO trial_table "
             "(title, datetime, amount, category, checklist1, checklist2, color, address) "
             "VALUES "
             "(%s, %s, %s, %s, %d, %d, %s, %s)",
             q_title,
             q_datetime,
             record.amount,
             q_category,
             record.checklist1 ? 1 : 0,
             record.checklist2 ? 1 : 0,
             q_color,
             q_address);

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "Insert failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("Record inserted successfully.\n");

    mysql_close(conn);
    return EXIT_SUCCESS;
}
