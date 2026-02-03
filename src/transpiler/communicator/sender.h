#ifndef _SENDER_H_
#define _SENDER_H_

char* obtain_bearer_token(const char* api_key);
char* authenticate(const char* api_key, const char* crn);

#endif
