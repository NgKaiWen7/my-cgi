#ifndef RECORD_H
#define RECORD_H

#include <stdbool.h>

#define RECORD_TITLE_SIZE 256
#define RECORD_DATETIME_SIZE 64
#define RECORD_AMOUNT_SIZE 64
#define RECORD_CATEGORY_SIZE 64
#define RECORD_COLOR_SIZE 16
#define RECORD_ADDRESS_SIZE 4096

typedef struct {
    long id;
    char title[RECORD_TITLE_SIZE];
    char datetime[RECORD_DATETIME_SIZE];
    char amount[RECORD_AMOUNT_SIZE];
    char category[RECORD_CATEGORY_SIZE];
    bool checklist1;
    bool checklist2;
    char color[RECORD_COLOR_SIZE];
    char address[RECORD_ADDRESS_SIZE];
} Record;

#endif
