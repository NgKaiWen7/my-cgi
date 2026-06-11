#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record_db.h"

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
            if (strcmp(pair, "id") == 0) {
                return parse_id_text(equals + 1, id);
            }
        }
        pair = strtok(NULL, "&");
    }

    return 0;
}

static int get_id_from_request(long *id) {
    const char *method = getenv("REQUEST_METHOD");
    const char *query = getenv("QUERY_STRING");

    if (method && strcmp(method, "POST") == 0) {
        const char *length_text = getenv("CONTENT_LENGTH");
        long length = length_text ? strtol(length_text, NULL, 10) : 0;

        if (length > 0 && length < 1024) {
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
    puts(
        "Content-Type: text/html\r\n\r\n"
        "<!doctype html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "<meta charset=\"utf-8\">\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        "<style>\n"
        "body{font-family:Arial,Helvetica,sans-serif;margin:32px;background:#f6f8fb;color:#1f2937}\n"
        ".top-actions{display:flex;gap:10px;margin-bottom:22px}\n"
        ".button{display:inline-block;padding:8px 14px;border:0;border-radius:4px;background:#1f6feb;color:#fff;text-decoration:none;font-weight:bold;cursor:pointer}\n"
        ".button.secondary{background:#4b5563}\n"
        ".button.danger{background:#b42318}\n"
        ".delete-form{margin:0}\n"
        ".form-view{max-width:900px;background:#fff;border:1px solid #d7dde8;padding:20px}\n"
        ".field{margin-bottom:16px}\n"
        "label{display:block;margin-bottom:6px;font-weight:bold;color:#374151}\n"
        ".value{min-height:22px;padding:10px;border:1px solid #d7dde8;background:#f9fafb;white-space:pre-wrap}\n"
        ".swatch{display:inline-block;width:24px;height:24px;margin-right:8px;border:1px solid #9ca3af;border-radius:4px;vertical-align:middle}\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
    );
}

static void print_page_end(void) {
    puts("</body>");
    puts("</html>");
}

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

static void print_bool_field(const char *label, bool value) {
    print_field(label, value ? "true" : "false");
}

static void print_record(const Record *record) {
    char id_text[32];

    snprintf(id_text, sizeof(id_text), "%ld", record->id);
    print_field("id", id_text);
    print_field("title", record->title);
    print_field("datetime", record->datetime);
    print_field("amount", record->amount);
    print_field("category", record->category);
    print_bool_field("checklist1", record->checklist1);
    print_bool_field("checklist2", record->checklist2);
    print_field("color", record->color);
    print_field("address", record->address);
}

int main(void) {
    Record record;
    long id = 0;

    print_page_start();

    if (!get_id_from_request(&id)) {
        puts("<p>Missing or invalid record id.</p>");
        puts("<p><a class=\"button secondary\" href=\"/cgi-bin/kaiwen/list_records\">Back to List</a></p>");
        print_page_end();
        return EXIT_FAILURE;
    }

    puts("<h1>Record Form View</h1>");
    printf("<div class=\"top-actions\">");
    printf("<a class=\"button secondary\" href=\"/cgi-bin/kaiwen/list_records\">Back to List</a>");
    printf("<a class=\"button\" href=\"/cgi-bin/kaiwen/edit_form?id=%ld\">Edit Record</a>", id);
    printf("<form class=\"delete-form\" method=\"post\" action=\"/cgi-bin/kaiwen/delete_record\">");
    printf("<input type=\"hidden\" name=\"id\" value=\"%ld\">", id);
    printf("<button class=\"button danger\" type=\"submit\">Delete Record</button>");
    printf("</form>");
    puts("</div>");

    if (!record_db_find_by_id(id, &record)) {
        print_page_end();
        return EXIT_FAILURE;
    }

    puts("<div class=\"form-view\">");
    print_record(&record);
    puts("</div>");

    print_page_end();
    return EXIT_SUCCESS;
}
