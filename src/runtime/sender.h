#ifndef _SENDER_H_
#define _SENDER_H_

char* get_backends_data(TOKEN_DATA* token_data, char* crn);
char* select_backend(char* backends_data);
char* build_payload(char* backend, char* qasm);
char* submit_job(TOKEN_DATA* token_data, char* crn, char* payload);
char* parse_job_id(char* response);

char* sender(TOKEN_DATA* token_data, char* crn, char* qasm);

#endif
