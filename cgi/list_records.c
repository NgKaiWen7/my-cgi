#include <stdio.h>
#include <stdlib.h>
#include "record_db.h"

static void print_record_row(const Record *record) {
    printf("<tr>");
    printf("<td>%ld</td>", record->id);
    printf("<td>%s</td>", record->title);
    printf("<td>%s</td>", record->datetime);
    printf("<td>%s</td>", record->amount);
    printf("<td>%s</td>", record->category);
    printf("<td>%s</td>", record->checklist1 ? "true" : "false");
    printf("<td>%s</td>", record->checklist2 ? "true" : "false");
    if (record->color[0]) {
        printf("<td><span class=\"swatch\" style=\"background-color:%s\"></span>%s</td>", record->color, record->color);
    } else {
        puts("<td>NULL</td>");
    }
    printf("<td>%s</td>", record->address);
    printf("<td><a class=\"iconbuttons\" href=\"/cgi-bin/kaiwen/form_view?id=%ld\" title=\"View\">View</a></td>", record->id);
    puts("</tr>");
}

int main(void) {
    Record *records = NULL;

    puts(
        "Content-Type: text/html\r\n\r\n"
        "<!doctype html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "<meta charset=\"utf-8\">\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        "<title>Trial Table Records</title>\n"
        "<link rel=\"stylesheet\" href=\"/css/style.css\">\n"
        "<style>\n"
        "body{font-family:Arial,Helvetica,sans-serif;margin:0;background:#f6f8fb;color:#1f2937;font-size:12px}\n"
        ".page-wrap{text-align:center;padding:20px 12px 48px}\n"
        ".panel{display:inline-block;text-align:left;background:#fff;font-size:12px;padding:20px;width:100%;max-width:1000px;border-radius:10px;box-sizing:border-box}\n"
        ".topbar{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:16px}\n"
        "h1{margin:0;color:#1f2937;font-size:20px;line-height:1.2}\n"
        ".actionbutton{display:inline-block;border-radius:7px;background:#009efd;text-decoration:none;border:none;color:#fff;border-bottom:3px solid #585858;cursor:pointer;padding:7px 12px;font-size:13px;font-weight:bold;outline:none}\n"
        ".actionbutton:hover,.iconbuttons:hover{filter:brightness(120%)}\n"
        ".iconbuttons{display:inline-block;padding:5px 10px;border:0;background-color:#1679d6;border-radius:5px;color:#fff;font-size:13px;line-height:1;text-decoration:none;cursor:pointer;white-space:nowrap}\n"
        ".table-scroll{overflow-x:auto}\n"
        "table.resultbox1{width:100%;max-width:1200px;border-collapse:collapse;background:#fff;color:#1f2937}\n"
        ".resultbox1 th,.resultbox1 td{border:1px solid #d7dde8;padding:8px 10px;text-align:left;vertical-align:top}\n"
        ".resultbox1 th{background:#eef2f7;color:#374151;font-weight:bold;text-transform:uppercase}\n"
        ".resultbox1 tbody tr:nth-child(even){background:#f9fafb}\n"
        ".resultbox1 tbody tr:hover{background:#eef6ff}\n"
        ".resultbox1 td:first-child,.resultbox1 th:first-child,.resultbox1 td:nth-child(6),.resultbox1 th:nth-child(6),.resultbox1 td:nth-child(7),.resultbox1 th:nth-child(7),.resultbox1 td:last-child,.resultbox1 th:last-child{text-align:center}\n"
        ".swatch{display:inline-block;width:22px;height:22px;margin-right:8px;border:1px solid #9ca3af;border-radius:4px;vertical-align:middle}\n"
        "@media(max-width:700px){.topbar{align-items:flex-start;flex-direction:column}.panel{padding:14px}.resultbox1 th,.resultbox1 td{padding:7px 8px}}\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<div class=\"page-wrap\">\n"
        "<div class=\"panel\">\n"
        "<div class=\"topbar\">\n"
        "<h1>Trial Table Records</h1>\n"
        "<a class=\"actionbutton\" href=\"/cgi-bin/kaiwen/edit_form?id=null\">Add New Record</a>\n"
        "</div>\n"
    );

    if (!record_db_select_all(&records)) {
        puts("</div>");
        puts("</div>");
        puts("</body>");
        puts("</html>");
        return EXIT_FAILURE;
    }

    puts("<div class=\"table-scroll\">");
    puts("<table class=\"resultbox1\">");
    puts("<thead><tr>");
    puts("<th>id</th>");
    puts("<th>title</th>");
    puts("<th>datetime</th>");
    puts("<th>amount</th>");
    puts("<th>category</th>");
    puts("<th>checklist1</th>");
    puts("<th>checklist2</th>");
    puts("<th>color</th>");
    puts("<th>address</th>");
    puts("<th>Details</th>");
    puts("</tr></thead>");
    puts("<tbody>");

    for (Record *record = records; record->id != 0; record++) {
        print_record_row(record);
    }

    puts("</tbody>");
    puts("</table>");
    puts("</div>");

    free(records);
    puts("</div>");
    puts("</div>");
    puts("</body>");
    puts("</html>");
    return EXIT_SUCCESS;
}
