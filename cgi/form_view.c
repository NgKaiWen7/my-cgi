#include <ctype.h>
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

static int from_hex(int ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return 0;
}

static void url_decode(char *text) {
    char *read = text;
    char *write = text;

    while (*read) {
        if (*read == '+') {
            *write++ = ' ';
            read++;
        } else if (*read == '%' && isxdigit((unsigned char)read[1]) && isxdigit((unsigned char)read[2])) {
            *write++ = (char)((from_hex(read[1]) << 4) | from_hex(read[2]));
            read += 3;
        } else {
            *write++ = *read++;
        }
    }

    *write = '\0';
}

static int get_id_from_query(long *id) {
    const char *query = getenv("QUERY_STRING");
    char query_copy[1024];
    char *pair;

    if (!query || !query[0]) {
        return 0;
    }

    snprintf(query_copy, sizeof(query_copy), "%s", query);
    pair = strtok(query_copy, "&");

    while (pair) {
        char *equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            url_decode(pair);
            url_decode(equals + 1);

            if (strcmp(pair, "id") == 0) {
                char *end = NULL;
                long value = strtol(equals + 1, &end, 10);
                if (end && *end == '\0' && value > 0) {
                    *id = value;
                    return 1;
                }
            }
        }
        pair = strtok(NULL, "&");
    }

    return 0;
}

static void print_page_start(const char *title) {
    printf("Content-Type: text/html\r\n\r\n");
    puts("<!doctype html>");
    puts("<html lang=\"en\">");
    puts("<head>");
    puts("<meta charset=\"utf-8\">");
    puts("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    printf("<title>");
    print_html_escaped(title);
    puts("</title>");
    puts("<style>");
    puts("body{font-family:Arial,Helvetica,sans-serif;margin:32px;background:#f6f8fb;color:#1f2937}");
    puts(".top-actions{display:flex;gap:10px;margin-bottom:22px}");
    puts(".button{display:inline-block;padding:8px 14px;border:0;border-radius:4px;background:#1f6feb;color:#fff;text-decoration:none;font-weight:bold;cursor:pointer}");
    puts(".button.secondary{background:#4b5563}");
    puts(".button.danger{background:#b42318}");
    puts(".delete-form{margin:0}");
    puts(".form-view{max-width:900px;background:#fff;border:1px solid #d7dde8;padding:20px}");
    puts(".field{margin-bottom:16px}");
    puts("label{display:block;margin-bottom:6px;font-weight:bold;color:#374151}");
    puts(".value{min-height:22px;padding:10px;border:1px solid #d7dde8;background:#f9fafb;white-space:pre-wrap}");
    puts(".swatch{display:inline-block;width:24px;height:24px;margin-right:8px;border:1px solid #9ca3af;border-radius:4px;vertical-align:middle}");
    puts("</style>");
    puts("</head>");
    puts("<body>");
}

static void print_page_end(void) {
    puts("</body>");
    puts("</html>");
}

static void print_field(const char *label, const char *value) {
    puts("<div class=\"field\">");
    printf("<label>");
    print_html_escaped(label);
    puts("</label>");
    puts("<div class=\"value\">");
    if (value && strcmp(label, "color") == 0) {
        printf("<span class=\"swatch\" style=\"background-color:");
        print_html_escaped(value);
        printf("\"></span>");
        print_html_escaped(value);
    } else if (value) {
        print_html_escaped(value);
    } else {
        puts("NULL");
    }
    puts("</div>");
    puts("</div>");
}

int main(void) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL_FIELD *fields;
    unsigned int field_count;
    long id = 0;
    char query[256];

    print_page_start("Record Form View");

    if (!get_id_from_query(&id)) {
        puts("<p>Missing or invalid record id.</p>");
        puts("<p><a class=\"button secondary\" href=\"/cgi-bin/list_records.cgi\">Back to List</a></p>");
        print_page_end();
        return EXIT_FAILURE;
    }

    printf("<div class=\"top-actions\">");
    printf("<a class=\"button secondary\" href=\"/cgi-bin/list_records.cgi\">Back to List</a>");
    printf("<a class=\"button\" href=\"/cgi-bin/edit_form.cgi?id=%ld\">Edit Record</a>", id);
    printf("<form class=\"delete-form\" method=\"post\" action=\"/cgi-bin/delete_record.cgi\">");
    printf("<input type=\"hidden\" name=\"id\" value=\"%ld\">", id);
    printf("<button class=\"button danger\" type=\"submit\">Delete Record</button>");
    printf("</form>");
    puts("</div>");

    conn = mysql_init(NULL);
    if (!conn) {
        puts("<p>mysql_init() failed.</p>");
        print_page_end();
        return EXIT_FAILURE;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        printf("<p>Database connection failed: ");
        print_html_escaped(mysql_error(conn));
        puts("</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    snprintf(query, sizeof(query), "SELECT * FROM %s WHERE id=%ld LIMIT 1", TABLE_NAME, id);
    if (mysql_query(conn, query)) {
        printf("<p>Query failed: ");
        print_html_escaped(mysql_error(conn));
        puts("</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    result = mysql_store_result(conn);
    if (!result) {
        printf("<p>Unable to read results: ");
        print_html_escaped(mysql_error(conn));
        puts("</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        puts("<p>Record not found.</p>");
        mysql_free_result(result);
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    field_count = mysql_num_fields(result);
    fields = mysql_fetch_fields(result);

    puts("<h1>Record Form View</h1>");
    puts("<div class=\"form-view\">");
    for (unsigned int i = 0; i < field_count; i++) {
        print_field(fields[i].name, row[i]);
    }
    puts("</div>");

    mysql_free_result(result);
    mysql_close(conn);
    print_page_end();

    return EXIT_SUCCESS;
}
