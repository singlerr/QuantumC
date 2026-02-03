#ifndef _SENDER_H_
#define _SENDER_H_

char* get_bearer_token(const char* api_key);
char* get_backends(const char* token, const char* crn);
char* authenticate(const char* api_key, const char* crn);

#endif
