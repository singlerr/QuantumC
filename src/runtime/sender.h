#ifndef _SENDER_H_
#define _SENDER_H_

#define IBM_API_VERSION "2026-02-01"

char* get_backends_data(char* token, char* crn);
char* select_backend(char* backends_data);

char* sender(TOKEN_DATA* token_data, char* crn);

#endif
