#include <ctype.h>
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int parse_id_text(const char *text, long *id) {
    char *end = NULL;
    long value;

    if (!text || !text[0]) {
        return 0;
    }

    value = strtol(text, &end, 10);
    if (!end || *end != '\0' || value <= 0) {
        return 0;
    }

    *id = value;
    return 1;
}

static int read_id_from_params(char *params, long *id) {
    char *pair = strtok(params, "&");

    while (pair) {
        char *equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            url_decode(pair);
            url_decode(equals + 1);
            if (strcmp(pair, "id") == 0) {
                return parse_id_text(equals + 1, id);
            }
        }
        pair = strtok(NULL, "&");
    }

    return 0;
}

static int read_request_id(long *id) {
    const char *method = getenv("REQUEST_METHOD");
    const char *query = getenv("QUERY_STRING");

    if (method && strcmp(method, "POST") == 0) {
        const char *length_text = getenv("CONTENT_LENGTH");
        long length = length_text ? strtol(length_text, NULL, 10) : 0;

        if (length > 0 && length < 4096) {
            char *body = (char *)malloc((size_t)length + 1);
            if (!body) {
                return 0;
            }

            size_t read = fread(body, 1, (size_t)length, stdin);
            body[read] = '\0';
            int ok = read_id_from_params(body, id);
            free(body);
            return ok;
        }
    }

    if (query && query[0]) {
        char query_copy[1024];
        snprintf(query_copy, sizeof(query_copy), "%s", query);
        return read_id_from_params(query_copy, id);
    }

    return 0;
}

static void print_page_start(void) {
    printf("Content-Type: text/html\r\n\r\n");
    puts("<!doctype html>");
    puts("<html lang=\"en\">");
    puts("<head>");
    puts("<meta charset=\"utf-8\">");
    puts("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    puts("<title>Delete Record</title>");
    puts("<style>");
    puts("body{font-family:Arial,Helvetica,sans-serif;margin:32px;background:#f6f8fb;color:#1f2937}");
    puts(".box{max-width:760px;background:#fff;border:1px solid #d7dde8;padding:22px}");
    puts(".button{display:inline-block;padding:8px 14px;border-radius:4px;background:#1f6feb;color:#fff;text-decoration:none;font-weight:bold}");
    puts("</style>");
    puts("</head>");
    puts("<body>");
    puts("<div class=\"box\">");
}

static void print_page_end(void) {
    puts("<p><a class=\"button\" href=\"/cgi-bin/list_records.cgi\">Back to List</a></p>");
    puts("</div>");
    puts("</body>");
    puts("</html>");
}

int main(void) {
    MYSQL *conn;
    long id = 0;
    char sql[256];

    print_page_start();

    if (!read_request_id(&id)) {
        puts("<h1>Invalid Record</h1>");
        puts("<p>Missing or invalid record id.</p>");
        print_page_end();
        return EXIT_FAILURE;
    }

    conn = mysql_init(NULL);
    if (!conn) {
        puts("<h1>Delete Failed</h1>");
        puts("<p>mysql_init() failed.</p>");
        print_page_end();
        return EXIT_FAILURE;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        puts("<h1>Delete Failed</h1>");
        printf("<p>Database connection failed: ");
        print_html_escaped(mysql_error(conn));
        puts("</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE id=%ld LIMIT 1", TABLE_NAME, id);
    if (mysql_query(conn, sql)) {
        puts("<h1>Delete Failed</h1>");
        printf("<p>Query failed: ");
        print_html_escaped(mysql_error(conn));
        puts("</p>");
        mysql_close(conn);
        print_page_end();
        return EXIT_FAILURE;
    }

    if (mysql_affected_rows(conn) == 0) {
        puts("<h1>Record Not Found</h1>");
        printf("<p>No record was deleted for id %ld.</p>", id);
    } else {
        puts("<h1>Record Deleted</h1>");
        printf("<p>Record id %ld was deleted.</p>", id);
    }

    mysql_close(conn);
    print_page_end();

    return EXIT_SUCCESS;
}
