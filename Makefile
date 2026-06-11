CC ?= cc
CFLAGS ?= -Wall -Wextra -O2
MYSQL_FLAGS := $(shell mysql_config --cflags --libs 2>/dev/null)
DB_SRC :=cgi/record_db.c
CGI_SRC :=cgi/cgi_code.c

TARGETS := list_records form_view edit_form delete_record insert_record

all: $(TARGETS)

list_records:cgi/list_records.c $(DB_SRC)
	$(CC) $(CFLAGS) -o $@ cgi/list_records.c $(DB_SRC) $(MYSQL_FLAGS)

form_view:cgi/form_view.c $(DB_SRC) $(CGI_SRC)
	$(CC) $(CFLAGS) -o $@ cgi/form_view.c $(DB_SRC) $(CGI_SRC) $(MYSQL_FLAGS)

edit_form:cgi/edit_form.c $(DB_SRC) $(CGI_SRC)
	$(CC) $(CFLAGS) -o $@ cgi/edit_form.c $(DB_SRC) $(CGI_SRC) $(MYSQL_FLAGS)

delete_record:cgi/delete_record.c $(DB_SRC)
	$(CC) $(CFLAGS) -o $@ cgi/delete_record.c $(DB_SRC) $(MYSQL_FLAGS)

insert_record:cgi/insert_record.c $(DB_SRC)
	$(CC) $(CFLAGS) -o $@ cgi/insert_record.c $(DB_SRC) $(MYSQL_FLAGS)

install: list_records form_view edit_form delete_record insert_record
	cp list_records /var/www/GOS/cgi-bin/kaiwen/list_records
	cp form_view /var/www/GOS/cgi-bin/kaiwen/form_view
	cp edit_form /var/www/GOS/cgi-bin/kaiwen/edit_form
	cp delete_record /var/www/GOS/cgi-bin/kaiwen/delete_record
	cp insert_record /var/www/GOS/cgi-bin/kaiwen/insert_record

clean:
	rm -f $(TARGETS)
