#ifndef _AUTH_H_
#define _AUTH_H_

#define TIME_OFFSET 3599

void update_bearer_token(TOKEN_DATA* token_data, char* token);
int get_bearer_token(TOKEN_DATA* token_data);

void* authenticator(void* arg);

#endif
