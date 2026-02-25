#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#define REFRESH_TIME 10

typedef struct SampleCount {
    char* sample;
    int count;
} SAMPLE_COUNT;

bool check_code(char* response);
char* get_job_result(TOKEN_DATA* token_data, char* crn, char* job_id);
char* parse_job_result(char* response);
char* convert_job_result(char* sample);

char* receiver(TOKEN_DATA* token_data, char* crn, char* job_id);

#endif
