/***************************************************************************************
Title		:	Basic Program Template (Sub Sales Prepare PV )
Created		:	
Desription	:	

CHANGE LOG
***************************************************************************************/

#include "all_header.h"
#include "db_code.h"
#include "cgi_code.h"
#include "comm_code.h"
#include <mysql.h>
#include <locale.h>
#include <ccgi.h>
#include <curl/curl.h>

// runlog 0=NO, 1=YES
int	runlog=1;

char	ctoken[100];
char	cuserid[300];
char	page[100];
char	pagetitle[100];
char	word2display[500];
char	languageFile[200];

CGI_varlist *varlist;
CGI_value *value;

// Update this if got changes to CSS/JS to force refresh
char	uniqueNo[100];

int setProgramName(char *fullpath)
{
	char *base_name = strrchr(fullpath, '/'); // For Unix/Linux
    	if (base_name) {
        	base_name++; // Move past the '/' or '\' character
    	} else {
        	base_name = fullpath; // No path, use the full name
    	}

	memset(page,0x00,sizeof(page));
	strcpy(page, base_name);

	return 0;
}

int main(int argc, char *argv[])
{
	char	DBserver[100];
	char	DBuser[50];
	char	DBpassword[50];
	char	DBname[50];;
	MYSQL 	*conn;

	int	totalValue=0;
	int	i=0;
	char	errorMsg[200];
	char 	loginid[21];
	char	temp[200];
	
	char	action[20];
	char	process[50];
	int	NewEditDelView=1; // Default is new

	// 2022-09-26 specialBooking to cater for Create New Case but Basic mode. 
	int	specialBooking=0;

	memset(process,0x00,sizeof(process));
	memset(errorMsg,0x00,sizeof(errorMsg));
	memset(ctoken,0x00,sizeof(ctoken));
	memset(cuserid,0x00,sizeof(cuserid));

	// Set the programname
	setProgramName(argv[0]);

	// Set random unique number (to ensure style sheet is refreshed each time)
	memset(uniqueNo,0x00,sizeof(uniqueNo));
	GetUniqueID(uniqueNo);

	// Start HTML content defination
	printf("Content-type: text/html\n\n"); fflush(stdout);

	// Connect to database
	conn = mysql_init(NULL);
	if (conn == NULL) 
  	{
  	    	printf("ERROR [%s]<br>\n", mysql_error(conn)); fflush(stdout);
		return -1001;
  	}
	memset(DBserver,0x00,sizeof(DBserver));
	memset(DBuser,0x00,sizeof(DBuser));
	memset(DBpassword,0x00,sizeof(DBpassword));
	memset(DBname,0x00,sizeof(DBname));
	GetDBinfo(DBserver,DBuser,DBpassword,DBname);
	if (!mysql_real_connect(conn,DBserver,DBuser,DBpassword,DBname,0,NULL,0)) {
		printf("DBError [01][%s]\n", mysql_error(conn)); fflush(stdout);
		return -1002;
	}

	// Get the token from previous calling page
	ParseGetData(ctoken,cuserid);
	SimpleDecode(cuserid, loginid);
	
	// Verify the session
	i=VerifySessionDB(conn,loginid, ctoken); 
	if (i<0)
	{
		printf("<meta http-equiv=\"refresh\" content=\"2; url=login\">");
		fflush(stdout);	

		if (i == -3)
			PrintPage("User not logged in. Please login.");
		else if (i == -4)
			PrintPage("User session has expired. Please login again.");
		else
			PrintPage("[0] Invalid access. Please login.");
		exit(0);
	}


	// Get the POSTed value from previous form
	char	tmppath[100]; memset(tmppath,0x00,sizeof(tmppath));
	sprintf(tmppath,"/tmp/%s-XXXXXX", page);
	varlist = CGI_get_all(tmppath);

	// Check if Data was posted
	memset(process,0x00,sizeof(process));
	value = CGI_lookup_all(varlist, "process"); 
	if (value!=NULL) 
	{
		memset(process,0x00,sizeof(process));
		memset(action,0x00,sizeof(action));
		value = CGI_lookup_all(varlist, "process"); if (value!=NULL) if (strlen(value[0]) != 0) strcpy(process, value[0]); 
		value = CGI_lookup_all(varlist, "action"); if (value!=NULL) if (strlen(value[0]) != 0) strcpy(action, value[0]); 

		// Determine the action type	
		if (strcmp(action,"New")==0) NewEditDelView=1;
		else if (strcmp(action,"Update")==0) NewEditDelView=2;
		else if (strcmp(action,"Cancel")==0) NewEditDelView=3;
		else if (strcmp(action,"View")==0) NewEditDelView=4;
	}
	if (strlen(process) == 0) sprintf(process,"%s","MAIN");

	// Set the language
	memset(languageFile,0x00,sizeof(languageFile));
	GetLanguageFile(conn, loginid, languageFile);

	// Set page title
	memset(pagetitle,0x00,sizeof(pagetitle));
	memset(word2display,0x00,sizeof(word2display));
	if (GetWord(languageFile,"SSCOMMPAYMENTLOW",word2display) == 0) sprintf(pagetitle,"%s", word2display);
	else sprintf(pagetitle,"%s", "Test Template Program");

	// Get the PROCESS TYPE
	if (strcmp(process,"MAIN") == 0) 
		PrintListPage(conn,loginid);
	else if (strcmp(process,"VIEWDETAILS") == 0) 
	{
		PrintDetails(conn, loginid, "");
	}
	else
	{
		PrintNoAccess();
		exit(0);
	}
	
	fflush(stdout);
}


int PrintDetails(MYSQL *conn, char *loginid, char *errormessage)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	MYSQL_RES *res1;
	MYSQL_ROW row1;

	char 	sqlstat[200000];
	int 	totalRow = 0;
	int	i=0;

	char	tmpbuf[200];
	char	rankid[21];
	char	status[21];
	char	branchid[21];

	printf("<!DOCTYPE html>\n");
	printf("<html class=mainbody>\n");
	printf("<head>\n");
	printf("<meta charset=\"UTF-8\">\n");
	printf("<title>Gplex Online - %s</title>\n", pagetitle);
	printf("<link rel=\"icon\" type=\"image/png\" href=\"/images/login_logo_v2.png\">\n");
	printf("<link rel=\"stylesheet\" href=\"/css/style.css?v=%s\">\n", uniqueNo);

	// Header for mobile
	printf(" <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
                "<meta http-equiv=\"cleartype\" content=\"on\">\n"
                "<meta name=\"MobileOptimized\" content=\"320\">\n"
                "<meta name=\"HandheldFriendly\" content=\"True\">\n"
                "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\">\n"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n");

	// Print Javascript for table sorting and paging
	TableSorting_Script();

	// Font Awesome
	printf("<link rel=\"stylesheet\" href=\"../css/font-awesome-4.7.0/css/font-awesome.min.css\">\n");

	printf("<style>\n");
	printf(".actionbutton {"
        	"border-radius: 7px; background: #009efd; text-decoration: center; border: none;"
        	"color: white; border-bottom: 3px solid #585858; cursor: pointer; margin-top:0px; "
		"padding-top: 5px;padding-bottom: 5px; padding-left:10px;padding-right:10px;font-size: 13px; outline: none; "
		"}\n");
	printf(".actionbutton:hover { filter: brightness(120%%); }\n"); 

	printf(".iconbuttons { padding:5px;padding-left:10px;padding-right:10px;border:0px;background-color:rgb(0,0,255,0);border-radius:5px;font-size:15px;cursor: pointer; }\n"); 
	printf(".iconbuttons:hover { filter: brightness(120%%); }\n");
	printf(".inputtable td { padding:5px; height:20px;}\n");
	printf("</style>\n");

	printf("</head>");
	printf("<body>\n");

	// Print the Top Menu
	memset(tmpbuf,0x00,sizeof(tmpbuf));
	sprintf(tmpbuf,"%s - View Details",pagetitle);
	PrintTopMenuDB(conn, tmpbuf, page,ctoken,cuserid);	
	printf("<center><br>");

	// Basic items
	char	main_stateid[100];
	memset(main_stateid,0x00,sizeof(main_stateid));
	value = CGI_lookup_all(varlist, "main_stateid"); if (value!=NULL) if (strlen(value[0]) != 0) strcpy(main_stateid, value[0]);

	// Display error (if any)
	if (strlen(errormessage)>0)
	{
		printf("<div style=\"text-align:center;background:yellow;color:red;font-size:12px;padding:20px;width:100%%;max-width:600px;border-radius:10px;font-weight:bold;\">");
		printf("%s", errormessage);
		printf("</div>");
		printf("<br><br>");
	
	}

	// Display agent details
	char	main_seq[100], main_statename[200], main_regionid[100];
	memset(main_seq,0x00,sizeof(main_seq));
	memset(main_statename,0x00,sizeof(main_statename));
	memset(main_regionid,0x00,sizeof(main_regionid));

	memset(sqlstat,0x00,sizeof(sqlstat));
	sprintf(sqlstat, 
	"SELECT seq, statename, regionid "
       	"FROM statemaster "
	"WHERE stateid=\"%s\" ", main_stateid);	
	if (mysql_query(conn, sqlstat)) {
		printf("DBerror2ca [%s]\n", mysql_error(conn)); fflush(stdout);
		return -1003;
	}
	res = mysql_store_result(conn);
	totalRow = mysql_num_rows(res);
	if (totalRow > 0) {
		row = mysql_fetch_row(res);
		if (row[0]) strcpy(main_seq, row[0]);
		if (row[1]) strcpy(main_statename, row[1]);
		if (row[2]) strcpy(main_regionid, row[2]);
	}
	mysql_free_result(res);

	printf("<div style=\"text-align:left;background:white;font-size:12px;padding:20px;width:100%%;max-width:600px;border-radius: 10px;\">");
	printf("<table>");
	printf("<tr><td style=\"font-weight:bold;width:150px;\">Sequence</td><td>%s</td></tr>", main_seq); 
	printf("<tr><td style=\"font-weight:bold;width:150px;\">Statename</td><td>%s</td></tr>", main_statename);
	printf("<tr><td style=\"font-weight:bold;width:150px;\">Region ID</td><td>%s</td></tr>", main_regionid);
	printf("</table>");
	printf("</div>");
	printf("<br>");
	fflush(stdout);

	printf("<br>");
	fflush(stdout);

	// Buttons
	printf("<button type=button class=\"actionbutton fa fa-ban\" onclick=\"formcancel.requestSubmit();\" "
		"style=\"background-color:#e13c3c;color:white;\"><font style=\"font-size:13px;\">&nbsp&nbsp;Back</button>");

	printf("<form id=formcancel method=POST action=%s?%s:%s>\n", page,ctoken,cuserid);
	printf("<input type=hidden name=process value=\"MAIN\">\n");
	printf("</form>\n");

	printf("</center>");

	fflush(stdout);

	return 0;
}

int PrintListPage(MYSQL *conn, char *loginid)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	MYSQL_RES *res1;
	MYSQL_ROW row1;

	char 	sqlstat[200000];
	int 	totalRow = 0;
	int	i=0;

	char	main_viewtype[51];	// View cases by Project or Own
	char	tmpbuf[200];

	char	rankid[21];
	char	status[21];
	char	branchid[21];

	printf("<!DOCTYPE html>\n");
	printf("<html class=mainbody>\n");
	printf("<head>\n");
	printf("<meta charset=\"UTF-8\">\n");
	printf("<title>Gplex Online - %s</title>\n", pagetitle);
	printf("<link rel=\"icon\" type=\"image/png\" href=\"/images/login_logo_v2.png\">\n");
	printf("<link rel=\"stylesheet\" href=\"/css/style.css?v=%s\">\n", uniqueNo);

	// Header for mobile
	printf(" <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
                "<meta http-equiv=\"cleartype\" content=\"on\">\n"
                "<meta name=\"MobileOptimized\" content=\"320\">\n"
                "<meta name=\"HandheldFriendly\" content=\"True\">\n"
                "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\">\n"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n");

	// Print Javascript for table sorting and paging
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

	// Print the Top Menu
	memset(tmpbuf,0x00,sizeof(tmpbuf));
	sprintf(tmpbuf,"%s",pagetitle);
	PrintTopMenuDB(conn, tmpbuf, page,ctoken,cuserid);	
	
	printf("<center>\n");
	printf("<br>");

	// Get previous case (if got)
	char	prev_caseid[100];
	memset(prev_caseid,0x00,sizeof(prev_caseid));
	value = CGI_lookup_all(varlist, "main_caseid"); if (value!=NULL) if (strlen(value[0]) != 0) strcpy(prev_caseid, value[0]); 

	// Get the list of cases 
	memset(sqlstat,0x00,sizeof(sqlstat));
	sprintf(sqlstat, 
	"SELECT seq, stateid, statename, regionid "
	"FROM statemaster "	
	"ORDER BY seq "
	);
	if (mysql_query(conn, sqlstat)) {
		printf("DBerror2ca [%s]\n", mysql_error(conn)); fflush(stdout);
		return -1003;
	}
	res = mysql_store_result(conn);
	totalRow = mysql_num_rows(res);

	// Start section
	printf("<div style=\"background:white;font-size:12px;padding:20px;width:100%%;max-width:1000px;border-radius: 10px;\">");
	printf("<table id=\"tblresults\" class=resultbox1 style=\"width:100%%;max-width:1200px;\">\n");
	printf("<thead>\n");
	printf("<tr>");
	printf("<th style=\"text-align:center;\">"); printf("SEQ"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("STATE"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("NAME"); printf("</th>");
	printf("<th style=\"text-align:left;\">"); printf("RIGHT"); printf("</th>");
	printf("<th style=\"text-align:center;width:20px;\">"); printf("</th>");
	printf("</tr>");
	printf("</thead>\n");
	printf("<tbody>\n");

	if (totalRow > 0)
	{
		while ((row = mysql_fetch_row(res)) != NULL)
		{	
			printf("<tr>");

			// No 
			printf("<td style=\"text-align:center;\">");
			if (row[0]) printf("%s", row[0]);
			printf("</td>");

			// State ID
			printf("<td style=\"text-align:center;\">");
			if (row[1]) printf("%s", row[1]);
			printf("</td>");

			// State Name 
			printf("<td style=\"text-align:center;\">");
			if (row[2]) printf("%s", row[2]);
			printf("</td>");

			// State Region 
			printf("<td style=\"text-align:center;\">");
			if (row[3]) printf("%s", row[3]);
			printf("</td>");

			// Action
			printf("<td style=\"text-align:center;\">");
			printf("<button type=button class=\"iconbuttons fa fa-eye\" onclick=\"viewitem%s.requestSubmit();\" "
				"style=\"background-color:#1679d6;color:white;\" title=\"View\"></button>", row[1]);
			printf("</td>");

			// Form for this row
			printf("<form id=\"viewitem%s\" method=POST action=\"%s?%s:%s\">\n", row[1], page,ctoken,cuserid);
			printf("<input type=hidden name=process value=\"%s\">\n", "VIEWDETAILS");
			printf("<input type=hidden name=main_stateid value=\"%s\">\n", row[1]);
			printf("</form>\n");

			printf("</tr>\n");
		}
	}
	mysql_free_result(res);

	printf("</tbody>\n");
	printf("</table>");
	printf("</div>");
	printf("<br>");

	printf("<br><br><br>");
	PrintBottomMenuDB(conn,tmpbuf, page,ctoken,cuserid,0);

	return 0;
}


int TableSorting_Script()
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

int isAgent(MYSQL *conn, char *main_userid)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	char 	sqlstat[500];
	int 	totalRow = 0;

	memset(sqlstat,0x00,sizeof(sqlstat));
	sprintf(sqlstat,
	"SELECT userid "
	"FROM agentmaster "
	"WHERE userid=\"%s\" ",
	main_userid);	
	if (mysql_query(conn, sqlstat)) {
		printf("DBerror [%s]\n", mysql_error(conn));
	    	fflush(stdout);
		return -1003;
	}
	res = mysql_store_result(conn);
	totalRow = mysql_num_rows(res);
	mysql_free_result(res);
	if (totalRow > 0)
		return 0;

	return 1;
}

int writelog(char *userid, char *rawtext)
{
	FILE 	*fptr;
	char	timestamp[100];
	char	todaydate[100];
	char	filename[100];
	char	fullpath[100];

	if (runlog == 0) return 0;

	memset(todaydate,0x00,sizeof(todaydate));
	GetYYYYMMDD(todaydate);


	memset(filename,0x00,sizeof(filename));
	memset(fullpath,0x00,sizeof(fullpath));

	sprintf(filename,"logs-%s-%s.log",page, todaydate);
	sprintf(fullpath,"../logs/%s",filename);

	fptr = fopen(fullpath, "a");
	if (fptr == NULL)
	{
		printf("ERROR, cannot open file!!<br>\n"); fflush(stdout);
		exit(0);
	}

	memset(timestamp,0x00,sizeof(timestamp));
	GetTodayDateTime(timestamp);
	fprintf(fptr,"[%s][%s] %s\n", timestamp, userid, rawtext);

	fclose(fptr);

	return 0;
}


