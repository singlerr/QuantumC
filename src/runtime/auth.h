#ifndef _AUTH_H_
#define _AUTH_H_

#define OFFSET_TIME 300

char* get_bearer_token(TOKEN_DATA* token_data);
void update_bearer_token(TOKEN_DATA* token_data, char* token);
int parse_bearer_token(TOKEN_DATA* token_data, char* response);

void* authenticator(void* arg);

#endif
