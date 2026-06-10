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
#define MAX_SQL 20000

static const char *CATEGORY_OPTIONS[] = {"new", "converted", "done"};
static const size_t CATEGORY_OPTION_COUNT = sizeof(CATEGORY_OPTIONS) / sizeof(CATEGORY_OPTIONS[0]);

typedef struct {
    char action[32];
    char id_text[32];
    Record record;
} FormData;

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

static void copy_value(char *dest, size_t dest_size, const char *src) {
    snprintf(dest, dest_size, "%s", src ? src : "");
}

static int parse_id(const char *text, long *id) {
    char *end = NULL;
    long value;

    if (!text || !text[0] || strcmp(text, "null") == 0) {
        return 0;
    }

    value = strtol(text, &end, 10);
    if (!end || *end != '\0' || value <= 0) {
        return 0;
    }

    *id = value;
    return 1;
}

static bool parse_checkbox(const char *value) {
    return value && strcmp(value, "1") == 0;
}

static void set_param(FormData *form, const char *key, const char *value) {
    if (strcmp(key, "action") == 0) {
        copy_value(form->action, sizeof(form->action), value);
    } else if (strcmp(key, "id") == 0) {
        copy_value(form->id_text, sizeof(form->id_text), value);
        parse_id(value, &form->record.id);
    } else if (strcmp(key, "title") == 0) {
        copy_value(form->record.title, sizeof(form->record.title), value);
    } else if (strcmp(key, "datetime") == 0) {
        copy_value(form->record.datetime, sizeof(form->record.datetime), value);
    } else if (strcmp(key, "amount") == 0) {
        copy_value(form->record.amount, sizeof(form->record.amount), value);
    } else if (strcmp(key, "category") == 0) {
        copy_value(form->record.category, sizeof(form->record.category), value);
    } else if (strcmp(key, "checklist1") == 0) {
        form->record.checklist1 = parse_checkbox(value);
    } else if (strcmp(key, "checklist2") == 0) {
        form->record.checklist2 = parse_checkbox(value);
    } else if (strcmp(key, "color") == 0) {
        copy_value(form->record.color, sizeof(form->record.color), value);
    } else if (strcmp(key, "address") == 0) {
        copy_value(form->record.address, sizeof(form->record.address), value);
    }
}

static void parse_params(FormData *form, char *query) {
    char *pair = strtok(query, "&");

    while (pair) {
        char *equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            url_decode(pair);
            url_decode(equals + 1);
            set_param(form, pair, equals + 1);
        }
        pair = strtok(NULL, "&");
    }
}

static void init_form(FormData *form) {
    memset(form, 0, sizeof(*form));
    copy_value(form->record.category, sizeof(form->record.category), "new");
    copy_value(form->record.color, sizeof(form->record.color), "#000000");
}

static void read_request(FormData *form) {
    const char *query = getenv("QUERY_STRING");
    const char *method = getenv("REQUEST_METHOD");

    init_form(form);

    if (query && query[0]) {
        char query_copy[2048];
        snprintf(query_copy, sizeof(query_copy), "%s", query);
        parse_params(form, query_copy);
    }

    if (method && strcmp(method, "POST") == 0) {
        const char *length_text = getenv("CONTENT_LENGTH");
        long length = length_text ? strtol(length_text, NULL, 10) : 0;

        if (length > 0 && length < 65536) {
            char *body = (char *)malloc((size_t)length + 1);
            if (body) {
                size_t read = fread(body, 1, (size_t)length, stdin);
                body[read] = '\0';
                parse_params(form, body);
                free(body);
            }
        }
    }
}

static void escape_sql_value(MYSQL *conn, const char *input, char *output, size_t output_size, int allow_null) {
    char *buffer;
    size_t len;

    if (allow_null && (!input || input[0] == '\0')) {
        snprintf(output, output_size, "NULL");
        return;
    }

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

static int validate_number_value(const char *value) {
    char *end = NULL;

    if (!value || !value[0]) {
        return 1;
    }

    strtod(value, &end);
    return end && *end == '\0';
}

static void normalize_datetime_value(const char *input, char *output, size_t output_size) {
    char temp[64];

    copy_value(temp, sizeof(temp), input);
    for (char *p = temp; *p; p++) {
        if (*p == 'T') {
            *p = ' ';
        }
    }

    if (strlen(temp) == 16) {
        snprintf(output, output_size, "%s:00", temp);
    } else {
        copy_value(output, output_size, temp);
    }
}

static void datetime_value_for_html(const char *input, char *output, size_t output_size) {
    copy_value(output, output_size, input);
    for (char *p = output; *p; p++) {
        if (*p == ' ') {
            *p = 'T';
        }
    }
    if (strlen(output) > 16) {
        output[16] = '\0';
    }
}

static int load_record(MYSQL *conn, FormData *form, long id) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char sql[512];

    snprintf(sql, sizeof(sql),
             "SELECT id, title, datetime, amount, category, checklist1, checklist2, color, address "
             "FROM %s WHERE id=%ld LIMIT 1",
             TABLE_NAME, id);

    if (mysql_query(conn, sql)) {
        return 0;
    }

    result = mysql_store_result(conn);
    if (!result) {
        return 0;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return 0;
    }

    form->record.id = id;
    snprintf(form->id_text, sizeof(form->id_text), "%ld", id);
    copy_value(form->record.title, sizeof(form->record.title), row[1]);
    copy_value(form->record.datetime, sizeof(form->record.datetime), row[2]);
    copy_value(form->record.amount, sizeof(form->record.amount), row[3]);
    copy_value(form->record.category, sizeof(form->record.category), row[4]);
    form->record.checklist1 = row[5] && strcmp(row[5], "0") != 0;
    form->record.checklist2 = row[6] && strcmp(row[6], "0") != 0;
    copy_value(form->record.color, sizeof(form->record.color), row[7]);
    copy_value(form->record.address, sizeof(form->record.address), row[8]);

    mysql_free_result(result);
    return 1;
}

static int save_record(MYSQL *conn, const FormData *form, long *saved_id) {
    char sql[MAX_SQL];
    char q_title[512], q_datetime[128], q_category[256], q_color[64], q_address[RECORD_ADDRESS_SIZE * 2 + 3];
    char normalized_datetime[64];
    long id = 0;
    int has_id = parse_id(form->id_text, &id);

    if (!form->record.title[0] || !validate_number_value(form->record.amount)) {
        return 0;
    }

    normalize_datetime_value(form->record.datetime, normalized_datetime, sizeof(normalized_datetime));
    escape_sql_value(conn, form->record.title, q_title, sizeof(q_title), 0);
    if (normalized_datetime[0]) {
        escape_sql_value(conn, normalized_datetime, q_datetime, sizeof(q_datetime), 0);
    } else {
        snprintf(q_datetime, sizeof(q_datetime), "CURRENT_TIMESTAMP");
    }
    escape_sql_value(conn, form->record.category, q_category, sizeof(q_category), 1);
    escape_sql_value(conn, form->record.color, q_color, sizeof(q_color), 1);
    escape_sql_value(conn, form->record.address, q_address, sizeof(q_address), 1);

    if (has_id) {
        snprintf(sql, sizeof(sql),
                 "UPDATE %s SET title=%s, datetime=%s, amount=%s, category=%s, "
                 "checklist1=%d, checklist2=%d, color=%s, address=%s WHERE id=%ld",
                 TABLE_NAME, q_title, q_datetime,
                 form->record.amount[0] ? form->record.amount : "0",
                 q_category, form->record.checklist1 ? 1 : 0, form->record.checklist2 ? 1 : 0,
                 q_color, q_address, id);
        *saved_id = id;
    } else {
        snprintf(sql, sizeof(sql),
                 "INSERT INTO %s (title, datetime, amount, category, checklist1, checklist2, color, address) "
                 "VALUES (%s, %s, %s, %s, %d, %d, %s, %s)",
                 TABLE_NAME, q_title, q_datetime,
                 form->record.amount[0] ? form->record.amount : "0",
                 q_category, form->record.checklist1 ? 1 : 0, form->record.checklist2 ? 1 : 0,
                 q_color, q_address);
    }

    if (mysql_query(conn, sql)) {
        return 0;
    }

    if (!has_id) {
        *saved_id = (long)mysql_insert_id(conn);
    }

    return 1;
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
    puts(".button,button{display:inline-block;padding:8px 14px;border:0;border-radius:4px;background:#1f6feb;color:#fff;text-decoration:none;font-weight:bold;cursor:pointer}");
    puts(".button.secondary{background:#4b5563}");
    puts(".form{max-width:760px;background:#fff;border:1px solid #d7dde8;padding:20px;display:grid;grid-template-columns:1fr;gap:16px}");
    puts(".field{display:flex;flex-direction:column;gap:6px}");
    puts(".checklist-group{border:1px solid #d7dde8;border-radius:4px;padding:14px;display:grid;gap:10px}");
    puts(".section-title{margin:0 0 2px;font-size:18px;color:#374151}");
    puts("label{font-weight:bold;color:#374151}");
    puts(".checkbox-label{display:flex;align-items:center;gap:8px;font-weight:bold}");
    puts("input,textarea,select{padding:10px;border:1px solid #d7dde8;border-radius:4px;font:inherit}");
    puts("input[type=checkbox]{width:auto}");
    puts(".color-row{display:flex;align-items:center;gap:12px}");
    puts(".color-preview{display:inline-block;width:34px;height:34px;border:1px solid #9ca3af;border-radius:4px}");
    puts(".color-value{color:#374151;font-weight:bold}");
    puts("</style>");
    puts("<script>");
    puts("function updateColorPreview(input){");
    puts("var preview=document.getElementById('color-preview');");
    puts("var value=document.getElementById('color-value');");
    puts("if(preview){preview.style.backgroundColor=input.value;}");
    puts("if(value){value.textContent=input.value;}");
    puts("}");
    puts("</script>");
    puts("</head>");
    puts("<body>");
}

static void print_page_end(void) {
    puts("</body>");
    puts("</html>");
}

static void print_input(const char *label, const char *name, const char *type, const char *value, const char *extra_attr, int required) {
    puts("<div class=\"field\">");
    printf("<label for=\"");
    print_html_escaped(name);
    printf("\">");
    print_html_escaped(label);
    puts("</label>");
    printf("<input id=\"");
    print_html_escaped(name);
    printf("\" name=\"");
    print_html_escaped(name);
    printf("\" type=\"");
    print_html_escaped(type);
    printf("\" value=\"");
    print_html_escaped(value);
    printf("\"");
    if (extra_attr && extra_attr[0]) {
        printf(" %s", extra_attr);
    }
    if (required) {
        printf(" required");
    }
    puts(">");
    puts("</div>");
}

static void print_textarea(const char *label, const char *name, const char *value) {
    puts("<div class=\"field\">");
    printf("<label for=\"");
    print_html_escaped(name);
    printf("\">");
    print_html_escaped(label);
    puts("</label>");
    printf("<textarea id=\"");
    print_html_escaped(name);
    printf("\" name=\"");
    print_html_escaped(name);
    puts("\" rows=\"5\">");
    print_html_escaped(value);
    puts("</textarea>");
    puts("</div>");
}

static void print_color_input(const char *value) {
    const char *color_value = value && value[0] ? value : "#000000";

    puts("<div class=\"field\">");
    puts("<label for=\"color\">color</label>");
    puts("<div class=\"color-row\">");
    printf("<input id=\"color\" name=\"color\" type=\"color\" value=\"");
    print_html_escaped(color_value);
    puts("\" oninput=\"updateColorPreview(this)\">");
    printf("<span id=\"color-preview\" class=\"color-preview\" style=\"background-color:");
    print_html_escaped(color_value);
    puts("\"></span>");
    printf("<span id=\"color-value\" class=\"color-value\">");
    print_html_escaped(color_value);
    puts("</span>");
    puts("</div>");
    puts("</div>");
}

static void print_category_select(const char *value) {
    puts("<div class=\"field\">");
    puts("<label for=\"category\">category</label>");
    puts("<select id=\"category\" name=\"category\">");
    for (size_t i = 0; i < CATEGORY_OPTION_COUNT; i++) {
        printf("<option value=\"");
        print_html_escaped(CATEGORY_OPTIONS[i]);
        printf("\"");
        if (strcmp(value, CATEGORY_OPTIONS[i]) == 0) {
            printf(" selected");
        }
        printf(">");
        print_html_escaped(CATEGORY_OPTIONS[i]);
        puts("</option>");
    }
    puts("</select>");
    puts("</div>");
}

static void print_checkbox(const char *label, const char *name, bool checked) {
    printf("<input type=\"hidden\" name=\"");
    print_html_escaped(name);
    puts("\" value=\"0\">");
    printf("<label class=\"checkbox-label\"><input id=\"");
    print_html_escaped(name);
    printf("\" name=\"");
    print_html_escaped(name);
    printf("\" type=\"checkbox\" value=\"1\"");
    if (checked) {
        printf(" checked");
    }
    printf("> ");
    print_html_escaped(label);
    puts("</label>");
}

static void print_form(const FormData *form) {
    long id = 0;
    int has_id = parse_id(form->id_text, &id);
    char html_datetime[64];

    datetime_value_for_html(form->record.datetime, html_datetime, sizeof(html_datetime));

    puts("<div class=\"top-actions\">");
    puts("<a class=\"button secondary\" href=\"/cgi-bin/list_records.cgi\">Back to List</a>");
    if (has_id) {
        printf("<a class=\"button secondary\" href=\"/cgi-bin/form_view.cgi?id=%ld\">View Record</a>", id);
    }
    puts("</div>");

    printf("<h1>");
    print_html_escaped(has_id ? "Edit Record" : "New Record");
    puts("</h1>");

    puts("<form class=\"form\" method=\"post\" action=\"/cgi-bin/edit_form.cgi\">");
    puts("<input type=\"hidden\" name=\"action\" value=\"save\">");
    printf("<input type=\"hidden\" name=\"id\" value=\"");
    print_html_escaped(form->id_text);
    puts("\">");

    print_input("title", "title", "text", form->record.title, "", 1);
    print_input("datetime", "datetime", "datetime-local", html_datetime, "", 0);
    print_input("amount", "amount", "number", form->record.amount, "step=\"0.01\"", 0);
    print_category_select(form->record.category);

    puts("<section class=\"checklist-group\">");
    puts("<h2 class=\"section-title\">Checklist</h2>");
    print_checkbox("checklist1", "checklist1", form->record.checklist1);
    print_checkbox("checklist2", "checklist2", form->record.checklist2);
    puts("</section>");

    print_color_input(form->record.color);
    print_textarea("address", "address", form->record.address);

    puts("<div class=\"field\">");
    puts("<button type=\"submit\">Save</button>");
    puts("</div>");
    puts("</form>");
}

int main(void) {
    FormData form;
    MYSQL *conn;
    long id = 0;
    long saved_id = 0;
    int has_id;

    read_request(&form);
    has_id = parse_id(form.id_text, &id);

    print_page_start("Edit Record");

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

    if (strcmp(form.action, "save") == 0) {
        if (save_record(conn, &form, &saved_id)) {
            printf("<p>Record saved successfully.</p>");
            printf("<p><a class=\"button\" href=\"/cgi-bin/form_view.cgi?id=%ld\">View Saved Record</a></p>", saved_id);
        } else {
            puts("<p>Unable to save record. Title is required and amount must be numeric.</p>");
            print_form(&form);
        }
    } else {
        if (has_id && !load_record(conn, &form, id)) {
            puts("<p>Record not found.</p>");
            puts("<p><a class=\"button secondary\" href=\"/cgi-bin/list_records.cgi\">Back to List</a></p>");
        } else {
            print_form(&form);
        }
    }

    mysql_close(conn);
    print_page_end();

    return EXIT_SUCCESS;
}
