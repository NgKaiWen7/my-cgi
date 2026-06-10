#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record.h"

#define DB_HOST "localhost"
#define DB_USER "nkw"
#define DB_PASS "nkw"
#define DB_NAME "trial"
#define TABLE_NAME "trial_table"

static void print_html_escaped(const char *text) {
    if (!text) {
        return;
    }

    while (*text) {
        switch (*text) {
            case '&':
                fputs("&amp;", stdout);
                break;
            case '<':
                fputs("&lt;", stdout);
                break;
            case '>':
                fputs("&gt;", stdout);
                break;
            case '"':
                fputs("&quot;", stdout);
                break;
            case '\'':
                fputs("&#39;", stdout);
                break;
            default:
                fputc(*text, stdout);
                break;
        }
        text++;
    }
}

static void print_page_start(void) {
    printf("Content-Type: text/html\r\n\r\n");
    puts("<!doctype html>");
    puts("<html lang=\"en\">");
    puts("<head>");
    puts("<meta charset=\"utf-8\">");
    puts("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    puts("<title>Trial Table Records</title>");
    puts("<style>");
    puts("body{font-family:Arial,Helvetica,sans-serif;margin:32px;background:#f6f8fb;color:#1f2937}");
    puts("table{width:100%;border-collapse:collapse;background:#fff}");
    puts("th,td{border:1px solid #d7dde8;padding:10px;text-align:left;vertical-align:top}");
    puts("th{background:#eef2f7}");
    puts(".button{display:inline-block;padding:7px 12px;border-radius:4px;background:#1f6feb;color:#fff;text-decoration:none;font-weight:bold}");
    puts(".swatch{display:inline-block;width:22px;height:22px;margin-right:8px;border:1px solid #9ca3af;border-radius:4px;vertical-align:middle}");
    puts("</style>");
    puts("</head>");
    puts("<body>");
    puts("<h1>Trial Table Records</h1>");
    puts("<p><a class=\"button\" href=\"/cgi-bin/edit_form.cgi?id=null\">Add New Record</a></p>");
}

static void print_page_end(void) {
    puts("</body>");
    puts("</html>");
}

int main(void) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL_FIELD *fields;
    unsigned int field_count;

    print_page_start();

    // Initialize MySQL connection
    conn = mysql_init(NULL);
    if (!conn) {
        puts("<p>mysql_init() failed.</p>");
        print_page_end();
        return EXIT_FAILURE;
    }

    // Connect to the database
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        puts("<p>Database connection failed</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    // Execute the query to fetch records
    if (mysql_query(conn, "SELECT * FROM " TABLE_NAME " ORDER BY id ASC")) {
        printf("<p>Query failed: ");
        print_html_escaped(mysql_error(conn));
        puts("</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    // Store the result set and check for errors when fetching results
    result = mysql_store_result(conn);
    if (!result) {
        printf("<p>Unable to read results: ");
        print_html_escaped(mysql_error(conn));
        puts("</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    // Output the table header
    field_count = mysql_num_fields(result);
    fields = mysql_fetch_fields(result);
    puts("<table>");
    puts("<thead><tr>");
    for (unsigned int i = 0; i < field_count; i++) {
        puts("<th>");
        print_html_escaped(fields[i].name);
        puts("</th>");
    }

    // Add an extra column for details link
    puts("<th>");
    print_html_escaped("Details");
    puts("</th>");

    // Close the header row
    puts("</tr></thead>");
    puts("<tbody>");

    // Output the table rows
    while ((row = mysql_fetch_row(result)) != NULL) {
        puts("<tr>");
        for (unsigned int i = 0; i < field_count; i++) {
            puts("<td>");
            if (row[i] && strcmp(fields[i].name, "color") == 0) {
                printf("<span class=\"swatch\" style=\"background-color:");
                print_html_escaped(row[i]);
                printf("\"></span>");
                print_html_escaped(row[i]);
            } else if (row[i]) {
                print_html_escaped(row[i]);
            } else {
                puts("NULL");
            }
            puts("</td>");
        }
        puts("<td>");
        printf("<a class=\"button\" href=\"/cgi-bin/form_view.cgi?id=");
        print_html_escaped(row[0]);
        printf("\">View Form</a>");
        puts("</td>");
        puts("</tr>");
    }
    puts("</tbody>");
    puts("</table>");

    mysql_free_result(result);
    mysql_close(conn);
    print_page_end();

    return EXIT_SUCCESS;
}
