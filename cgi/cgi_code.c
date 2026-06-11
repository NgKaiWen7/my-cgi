#include "all_header.h"

int PrintPage(char *message)
{
	printf("<!DOCTYPE html>\n");
	printf("<html class=searchbody>\n");
	printf("<head>\n");
	printf("<meta charset=\"UTF-8\">\n");
	printf("<title>Gplex Online</title>\n");
	printf("<link rel=\"icon\" type=\"image/png\" href=\"/images/login_logo_v2.png\">\n");
	//printf("<link rel=\"stylesheet\" href=\"/css/reset.css\">\n");
	printf("<link rel=\"stylesheet\" href=\"/css/style.css\">\n");
	printf("</head>\n");
	printf("<body>\n");

	// See if need to print extra message
	if (strlen(message) > 0)
		PrintErrorMsg(message,"red");

	printf("</body>\n");
	printf("</html>\n");
	fflush(stdout);
}