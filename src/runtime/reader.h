#ifndef _READER_H_
#define _READER_H_

#define CONFIG_FILENAME "config.json"

typedef struct config {
    char* key;
    char* crn;
} CONFIG;

int count_characters(char* filename);

CONFIG* read_config(char* filename);
char* read_qasm(char* filename);

#endif
