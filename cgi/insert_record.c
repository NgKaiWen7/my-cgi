#include <stdio.h>
#include <stdlib.h>

#include "record_db.h"

int main(void) {
    long saved_id = 0;
    Record record = {
        0,
        "Sample Title",
        "2026-06-10 09:00:00",
        "123.45",
        "new",
        true,
        false,
        "#3366ff",
        "Line 1\nLine 2"
    };

    if (!record_db_save(&record, false, &saved_id)) {
        return EXIT_FAILURE;
    }

    printf("Record inserted successfully. id=%ld\n", saved_id);
    return EXIT_SUCCESS;
}
