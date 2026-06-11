#include <stdio.h>
#include <stdlib.h>
#include "record_db.h"

static void print_record_row(const Record *record) {
    printf("<tr>");
    printf("<td style=\"text-align:center;\">%ld</td>", record->id);
    printf("<td style=\"text-align:center;\">%s</td>", record->title);
    printf("<td style=\"text-align:center;\">%s</td>", record->datetime);
    printf("<td style=\"text-align:center;\">%s</td>", record->amount);
    printf("<td style=\"text-align:center;\">%s</td>", record->category);
    printf("<td style=\"text-align:center;\">%s</td>", record->checklist1 ? "true" : "false");
    printf("<td style=\"text-align:center;\">%s</td>", record->checklist2 ? "true" : "false");
    if (record->color[0]) {
        printf("<td style=\"text-align:center;\"><span class=\"swatch\" style=\"background-color:%s\"></span>%s</td>", record->color, record->color);
    } else {
        puts("<td style=\"text-align:center;\">NULL</td>");
    }
    printf("<td style=\"text-align:center;\">%s</td>", record->address);
    printf("<td style=\"text-align:center;\">");
    printf("<button type=button class=\"iconbuttons\" onclick=\"viewitem%ld.requestSubmit();\" title=\"View\">View</button>", record->id);
    printf("<form id=\"viewitem%ld\" method=POST action=\"/cgi-bin/kaiwen/form_view\">\n", record->id);
    printf("<input type=hidden name=id value=\"%ld\">\n", record->id);
    printf("</form>\n");
    printf("</td>");
    puts("</tr>");
}



void TableSorting_Script()
{
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/jquery.dataTables.min.css\">\n");
	printf("<script type=\"text/javascript\" src=\"../js/jquery.js\"></script>\n");
	printf("<script type=\"text/javascript\" language=\"javascript\" src=\"../js/jquery.dataTables.min.js\"></script>\n");

	printf("<script type=\"text/javascript\" class=\"init\">\n");

	printf("	$(document).ready(function() {\n");

	printf("	$('#tblresults').DataTable( {\n");
	printf("	\"columnDefs\": [ \n");
	printf("	{\n");
	printf("		\"targets\": [ 2 ],\n");
	printf("		\"visible\": true,\n");
	printf("		\"searchable\": true\n");
	printf("	},\n");
	printf("	{\n");
	printf("		\"targets\": [ 3 ],\n");
	printf("		\"visible\": true\n");
	printf("	}\n");
	printf("	]} ); \n\n");

	printf(" "	
		"	var table1=$('#tblresults1').DataTable( "
		"	{ordering:true,\n"
		"	\"columnDefs\": [ \n"
		"	{\n"
		"		\"targets\": [ 2 ],\n"
		"		\"visible\": true,\n"
		"		\"searchable\": true\n"
		"	},\n"
		"	{\n"
		"		\"targets\": [ 3 ],\n"
		"		\"visible\": true\n"
		"	}\n"
		"	]} ); \n\n"
		"	table1.order([0,'desc']).draw();"
	      );

	printf(" "	
		"	var table3=$('#tblresults3').DataTable( "
		"	{ordering:false,\n"
		"	\"columnDefs\": [ \n"
		"	{\n"
		"		\"targets\": [ 2 ],\n"
		"		\"visible\": true,\n"
		"		\"searchable\": true\n"
		"	},\n"
		"	{\n"
		"		\"targets\": [ 3 ],\n"
		"		\"visible\": true\n"
		"	}\n"
		"	]} ); \n\n"
	//	"	table3.order([0,'desc']).draw();"
	      );


	printf("	var table2=$('#tblresults2').DataTable( "
			"{ordering:false,\n");
	printf("	\"columnDefs\": [ \n");
	printf("	{\n");
	printf("		\"targets\": [ 2 ],\n");
	printf("		\"visible\": true,\n");
	printf("		\"searchable\": true\n");
	printf("	},\n");
	printf("	{\n");
	printf("		\"targets\": [ 3 ],\n");
	printf("		\"visible\": true\n");
	printf("	}\n");
	printf("	]} ); \n\n");


	printf(" 	var tblverified1=$('#tblverified1').DataTable( "
		"	{ordering:true,\n"
		"	\"columnDefs\": [ \n"
		"	{\n"
		"		\"targets\": [ 2 ],\n"
		"		\"visible\": true,\n"
		"		\"searchable\": true\n"
		"	},\n"
		"	{\n"
		"		\"targets\": [ 3 ],\n"
		"		\"visible\": true\n"
		"	}\n"
		"	]} ); \n\n");

	printf(" 	var tblverified2=$('#tblverified2').DataTable( "
		"	{ordering:true,\n"
		"	\"columnDefs\": [ \n"
		"	{\n"
		"		\"targets\": [ 2 ],\n"
		"		\"visible\": true,\n"
		"		\"searchable\": true\n"
		"	},\n"
		"	{\n"
		"		\"targets\": [ 3 ],\n"
		"		\"visible\": true\n"
		"	}\n"
		"	]} ); \n\n");

	printf(" 	var tblverified3=$('#tblverified3').DataTable( "
		"	{ordering:true,\n"
		"	\"columnDefs\": [ \n"
		"	{\n"
		"		\"targets\": [ 2 ],\n"
		"		\"visible\": true,\n"
		"		\"searchable\": true\n"
		"	},\n"
		"	{\n"
		"		\"targets\": [ 3 ],\n"
		"		\"visible\": true\n"
		"	}\n"
		"	]} ); \n\n");





	// Event listener
	printf(" var tblfilter=document.getElementById('tblfilter');\n");
	printf(" if (tblfilter != null) { \n");
	printf("	tblfilter.addEventListener(\"change\",function() {\n"
		"	table1\n"
		"		.columns( 2 )\n"	
		"		.search( this.value )\n"	
		"		.draw();\n"
		"	} );\n");
	printf("}\n\n");


	printf(" var tblvfilter1=document.getElementById('tblvfilter1');\n");
	printf(" if (tblvfilter1 != null) { \n");
	printf("	tblvfilter1.addEventListener(\"change\",function() {\n"
		"	tblverified1\n"
		"		.columns( 3 )\n"	
		"		.search( this.value )\n"	
		"		.draw();\n"
		"	} );\n");
	printf("}\n\n");

	printf(" var tblvfilter2=document.getElementById('tblvfilter2');\n");
	printf(" if (tblvfilter2 != null) { \n");
	printf("	tblvfilter2.addEventListener(\"change\",function() {\n"
		"	tblverified1\n"
		"		.columns( 9 )\n"	
		"		.search( this.value )\n"	
		"		.draw();\n"
		"	} );\n");
	printf("}\n\n");

	printf(" var tblvfilter3=document.getElementById('tblvfilter3');\n");
	printf(" if (tblvfilter3 != null) { \n");
	printf("	tblvfilter3.addEventListener(\"change\",function() {\n"
		"	tblverified1\n"
		"		.columns( 5 )\n"	
		"		.search( this.value )\n"	
		"		.draw();\n"
		"	} );\n");
	printf("}\n\n");


	printf("	} );\n");
	printf("</script>\n");

}


int main(void) {
    Record *records = NULL;

	printf("Content-Type: text/html\r\n\r\n");
	printf("<!DOCTYPE html>\n");
	printf("<html class=mainbody>\n");
	printf("<head>\n");
	printf("<meta charset=\"UTF-8\">\n");
	printf("<title>Gplex Online - %s</title>\n", "Kai Wen Testing"); // Hardcoded title for testing
	printf("<link rel=\"icon\" type=\"image/png\" href=\"/images/login_logo_v2.png\">\n");
	printf("<link rel=\"stylesheet\" href=\"/css/style.css?v=%d\">\n", 1);

    TableSorting_Script();
	
	// Font Awesome
	printf("<link rel=\"stylesheet\" href=\"../css/font-awesome-4.7.0/css/font-awesome.min.css\">\n");

	printf("<style>\n");
	printf("bookmark[id] { scroll-margin-top: 100px; }\n"); // Offset bookmark
	printf(".iconbuttons { padding:5px;padding-left:10px;padding-right:10px;border:0px;background-color:rgb(0,0,255,0);border-radius:5px;font-size:13px;cursor: pointer; }\n"); 
	printf(".iconbuttons:hover { filter: brightness(120%%); }\n");
	printf("</style>\n");

	printf("</head>");

	printf("<body>\n");

    if (!record_db_select_all(&records)) {
        puts("</div>");
        puts("</div>");
        puts("</body>");
        puts("</html>");
        return EXIT_FAILURE;
    }

	// Start section
	printf("<div style=\"background:white;font-size:12px;padding:20px;width:100%%;max-width:1000px;border-radius: 10px;\">");
	printf("<table id=\"tblresults\" class=resultbox1 style=\"width:100%%;max-width:1200px;\">\n");
	printf("<thead>\n");
	printf("<tr>");
	printf("<th style=\"text-align:center;\">"); printf("id"); printf("</th>");
	printf("<th style=\"text-align:center;\">"); printf("title"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("datetime"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("amount"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("category"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("checklist1"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("checklist2"); printf("</th>");
    printf("<th style=\"text-align:left;\">"); printf("color"); printf("</th>");
    printf("<th style=\"text-align:left;\">"); printf("address"); printf("</th>");
	printf("<th style=\"text-align:center;width:20px;\">"); printf("</th>");
	printf("</tr>");
	printf("</thead>\n");
	printf("<tbody>\n");

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
