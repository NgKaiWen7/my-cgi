#ifndef RECORD_DB_H
#define RECORD_DB_H

#include <stdbool.h>

#include "record.h"

bool record_db_find_by_id(long id, Record *record);
bool record_db_select_all(Record **records);
bool record_db_save(const Record *record, bool update_existing, long *saved_id);
bool record_db_delete(long id);
void record_list_free(Record *records);

#endif
