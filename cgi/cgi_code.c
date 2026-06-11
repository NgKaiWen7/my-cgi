#include "all_header.h"

int PrintPage(char *message)
{
	printf("<!DOCTYPE html>\n");
	printf("<html class=searchbody>\n");
	printf("<head>\n");
	printf("<meta charset=\"UTF-8\">\n");
	printf("<title>Gplex Online</title>\n");
	printf("<link rel=\"icon\" type=\"image/png\" href=\"/images/login_logo_v2.png\">\n");
	printf("<link rel=\"stylesheet\" href=\"/css/style.css\">\n");
	printf("</head>\n");
	printf("<body>\n");

	// See if need to print extra message
	if (strlen(message) > 0)
		printf("<div style=\"color:red;text-align:center;font-weight:bold;\">%s</div>\n", message);

	printf("</body>\n");
	printf("</html>\n");
	fflush(stdout);
	return 0;
}


void PrintTopMenuHTML(char *toptitle)
{
    printf("<table width=100%% border=1 class=\"topmenuNEW\">");
    printf("<tr>");

    printf("<td width=100 style=\"text-align:center;\">");
    printf("<img border=0 class=goslogo src=\"/images/GO_logo_dev.jpg\" title=\"Go to main page\">");
    printf("</td>");

    printf("<td style=\"text-align:left;padding:0px 15px;\">");
    printf("<font color=black class=\"menuPagetitle\">%s</font>\n", toptitle);
    printf("</td>");

    printf("<td width=100 class=topmenubutton>");
    printf("</td>");

    printf("</tr>");
    printf("</table>");

    printf("<div class=\"menubufferNEW\"></div>");
}
