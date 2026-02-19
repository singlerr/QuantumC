#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#define REFRESH_TIME 10

char* get_result(TOKEN_DATA* token_data, char* crn, char* job_id);
char* parse_result(char* result);
char* convert_result(char* sample);

char* receiver(TOKEN_DATA* token_data, char* crn, char* job_id);

#endif
