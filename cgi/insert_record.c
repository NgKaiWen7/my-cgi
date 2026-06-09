#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

int main(void) {
    MYSQL *conn;

    const char *server = "localhost";
    const char *user = "root";
    const char *password = "nkw";
    const char *database = "trial";

    conn = mysql_init(NULL);

    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return EXIT_FAILURE;
    }

    if (mysql_real_connect(
            conn,
            server,
            user,
            password,
            database,
            0,
            NULL,
            0
        ) == NULL) {
        fprintf(stderr, "Connection failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    const char *query =
        "INSERT INTO trial_table "
        "(name, created_at, amount, description) "
        "VALUES "
        "('Sample Name', CURRENT_TIMESTAMP, 123.45, 'This is a sample text record')";

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Insert failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("Record inserted successfully.\n");

    mysql_close(conn);
    return EXIT_SUCCESS;
}