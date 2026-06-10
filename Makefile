CC ?= cc
CFLAGS ?= -Wall -Wextra -O2
MYSQL_FLAGS := $(shell mysql_config --cflags --libs 2>/dev/null)

TARGETS := list_records.cgi form_view.cgi edit_form.cgi delete_record.cgi insert_record.cgi

all: $(TARGETS)

list_records.cgi: cgi/list_records.c
	$(CC) $(CFLAGS) -o $@ $< $(MYSQL_FLAGS)

form_view.cgi: cgi/form_view.c
	$(CC) $(CFLAGS) -o $@ $< $(MYSQL_FLAGS)

edit_form.cgi: cgi/edit_form.c
	$(CC) $(CFLAGS) -o $@ $< $(MYSQL_FLAGS)

delete_record.cgi: cgi/delete_record.c
	$(CC) $(CFLAGS) -o $@ $< $(MYSQL_FLAGS)

insert_record.cgi: cgi/insert_record.c
	$(CC) $(CFLAGS) -o $@ $< $(MYSQL_FLAGS)

install: list_records.cgi form_view.cgi edit_form.cgi delete_record.cgi insert_record.cgi
	sudo cp list_records.cgi /usr/lib/cgi-bin/list_records.cgi
	sudo cp form_view.cgi /usr/lib/cgi-bin/form_view.cgi
	sudo cp edit_form.cgi /usr/lib/cgi-bin/edit_form.cgi
	sudo cp delete_record.cgi /usr/lib/cgi-bin/delete_record.cgi
	sudo cp insert_record.cgi /usr/lib/cgi-bin/insert_record.cgi

clean:
	rm -f $(TARGETS)
