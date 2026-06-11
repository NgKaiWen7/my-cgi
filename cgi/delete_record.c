#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record_db.h"

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
    puts("<p><a class=\"button\" href=\"/cgi-bin/kaiwen/list_records\">Back to List</a></p>");
    puts("</div>");
    puts("</body>");
    puts("</html>");
}

int main(void) {
    long id = 0;

    print_page_start();

    if (!read_request_id(&id)) {
        puts("<h1>Invalid Record</h1>");
        puts("<p>Missing or invalid record id.</p>");
        print_page_end();
        return EXIT_FAILURE;
    }

    if (!record_db_delete(id)) {
        puts("<h1>Delete Failed</h1>");
        print_page_end();
        return EXIT_FAILURE;
    }

    puts("<h1>Record Deleted</h1>");
    printf("<p>Delete request completed for id %ld.</p>", id);

    print_page_end();

    return EXIT_SUCCESS;
}
