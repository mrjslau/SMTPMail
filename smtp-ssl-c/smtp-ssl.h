
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
#include <stdbool.h>
#include <resolv.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "file-util.h"
//#include "base64.h"

#define BASE64LEN 512
#define REQUESTLEN 1024
#define REPLYLEN 1024