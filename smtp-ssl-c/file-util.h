#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

int readConfig(char *servName, char *userName, char *userPassword, char *mailFrom, char *mailRcpt, char *mailSubj, char *mailCont, unsigned short *echoServPort, int *sslAuth, int *tlsStart);
char** str_split(char* a_str, const char a_delim);