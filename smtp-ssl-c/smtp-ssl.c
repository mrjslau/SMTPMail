
#include "smtp-ssl.h"

/* Pointers and variables for data from file. */
char *servName;
char *userName;
char *userPassword;
char *mailFrom;
char *mailRcpt;
char *mailSubj;
char *mailCont;
unsigned short *echoServPort;

/* Pointers and variables for connection sockets and types. */
int sockfd = 0;
SSL *ssl;
bool sslAuth = false;
bool tlsStart = false;

int get_tcp_socket(char *ip, int port) {
	int socketNum;

	struct hostent *host;
	struct sockaddr_in dest_addr;

	socketNum = socket(AF_INET, SOCK_STREAM, 0);

	dest_addr.sin_family=AF_INET;
	dest_addr.sin_port=htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(ip);


	memset(&(dest_addr.sin_zero), '\0', 8);


	if (connect(socketNum, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr)) == -1 ) {
		fprintf(stderr, "Error: Cannot connect to host!\n");
	}

	return socketNum;
}


char * concatenateString(char *output, char *str1, char *str2) {
	output = (char *) malloc(1 + strlen(str1) + strlen(str2));
	strcpy(output, str1);
	strcat(output, str2);
	
	return output;
}

void Base64(char *string, char *pointer, int bufferLen) {
	FILE *fp;
	
	char *concatenatePointerTemp;
	char *concatenatePointer;
	concatenatePointerTemp = concatenateString(concatenatePointerTemp, (char*)"echo ", string);
	concatenatePointer = concatenateString(concatenatePointer, concatenatePointerTemp, (char*)" | openssl base64");

	fp = popen(concatenatePointer, "r");
	free(concatenatePointer);
	free(concatenatePointerTemp);

	if (fp == NULL) {
		fprintf(stderr, "Error: Can't run base64 via system command!\n");
		exit(1);
	}

	fgets(pointer, bufferLen-1, fp);
	
	pclose(fp);
}

int hostname_to_ip(char * hostname , char* ip) {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}

void rcvdResponse(int socketNum, char * reply) {
	int bytesRcvd;
	
	if (sslAuth) {
		bytesRcvd = SSL_read(ssl, reply, REPLYLEN - 1);
		reply[bytesRcvd] = '\0';
		printf("%s", reply);
	} else {
		bytesRcvd = recv(socketNum, reply, REPLYLEN - 1, 0); //MSG_CMSG_CLOEXEC
		reply[bytesRcvd] = '\0';
		printf("%s", reply);
	}
	
}

int main(int argc, char **argv) {

		if (argc != 1) {
			fprintf(stderr, "Error: Bad arguments!\n");
			exit(1);
		}

	/* Memory alloc */	
		servName = (char*)malloc(256 * sizeof(char));
		userName = (char*)malloc(256 * sizeof(char));
		userPassword = (char*)malloc(256 * sizeof(char));
		mailFrom = (char*)malloc(256 * sizeof(char));
		mailRcpt = (char*)malloc(256 * sizeof(char));
		mailSubj = (char*)malloc(256 * sizeof(char));
		mailCont = (char*)malloc(256 * sizeof(char));
		echoServPort = (unsigned short*)malloc(sizeof(unsigned short));

	/* Get data from file */
		int *sslAuthp, *tlsStartp;
		sslAuthp = (int*)malloc(sizeof(int));
		tlsStartp = (int*)malloc(sizeof(int));
		*sslAuthp = 0;
		*tlsStartp = 0;
		readConfig(servName, userName, userPassword, mailFrom, mailRcpt, mailSubj, mailCont, echoServPort, sslAuthp, tlsStartp);
		printf("%d\n" , *sslAuthp);
		if(*sslAuthp) {
			printf("trueSSL");
			sslAuth = true;
		}
		else if (*tlsStartp)
		{
			printf("trueTLS");
			tlsStart = true;
		}

	/* Resolving domain name to IP address. */
		char ip[32];
		int hostResult = hostname_to_ip(servName , ip);
		
		if (hostResult == 1) {
			fprintf(stderr, "Error: Bad hostname or Internet connection fail!\n");
			exit(1);
		}
		printf("%s resolved to %s\n" , servName , ip);
		
	
	/* Buffers for request and reply: */
    char request[REQUESTLEN+1];
    char reply[REPLYLEN+1];
	/* Variable and poiter for resolving response code: */
	int intCode;
	char **rcvdCode;
	/* Buffer and poiter for base64 encode string: */
	char baseBuffer[BASE64LEN];
	char * basePointer = &baseBuffer[0];
	/* Connection method unable flags: */
	bool sslUnable = false;
	bool tlsUnable = false;
	
	
	/* SSL connection pointers and variables: */
		BIO *certbio = NULL;
		BIO *outbio = NULL;
		BIO *inpbio = NULL;
		X509 *cert = NULL;
		X509_NAME *certname = NULL;
		SSL_CTX *ctx;
		const SSL_METHOD *method;
		int ret, i;
	
	
	/* Make TCP connection: */
		sockfd = get_tcp_socket(&ip[0], *echoServPort);
		if (sockfd != 0)
			printf("Successfully made the TCP connection.\n");
		
		
	/* Operations if STARTTLS command is required: */
	if (tlsStart) {
		sprintf(request,"EHLO %s\r\n", servName);
		write(sockfd, request, strlen(request));
		rcvdResponse(sockfd, &reply[0]);
		rcvdResponse(sockfd, &reply[0]);
	
		sprintf(request,"STARTTLS\r\n");
		write(sockfd, request, strlen(request));
		rcvdResponse(sockfd, &reply[0]);

		/* Get response status code: */
		memcpy (request, reply, 3);
		request[3] = '\0';
		intCode = atoi(request);
		
		/* If TLS is available: */
		if (intCode == 220) {
			sslAuth = true;
		} else {
			tlsUnable = true;
			
			sprintf(request,"QUIT\r\n");
			if (sslAuth) {
				SSL_write(ssl, request, strlen(request));
			} else {
				write(sockfd, request, strlen(request));
			}
			rcvdResponse(sockfd, &reply[0]);
		}
	}

	/* Establishing encrypted connection: */
	if (sslAuth || (tlsStart && !tlsUnable)) {
		/* ---------------------------------------------------------- *
		* These function calls initialize openssl for correct work.  *
		* ---------------------------------------------------------- */
		OpenSSL_add_all_algorithms();
		ERR_load_BIO_strings();
		ERR_load_crypto_strings();
		SSL_load_error_strings();

		/* ---------------------------------------------------------- *
		* Create the Input/Output BIO's.                             *
		* ---------------------------------------------------------- */
		certbio = BIO_new(BIO_s_file());
		outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

		/* ---------------------------------------------------------- *
		* initialize SSL library and register algorithms             *
		* ---------------------------------------------------------- */
		if(SSL_library_init() < 0)
			BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");

		/* ---------------------------------------------------------- *
		* Set SSLv2 client hello, also announce SSLv3 and TLSv1      *
		* ---------------------------------------------------------- */
		method = SSLv23_client_method();

		/* ---------------------------------------------------------- *
		* Try to create a new SSL context                            *
		* ---------------------------------------------------------- */
		if ( (ctx = SSL_CTX_new(method)) == NULL)
			BIO_printf(outbio, "Unable to create a new SSL context structure.\n");

		/* ---------------------------------------------------------- *
		* Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
		* ---------------------------------------------------------- */
		SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

		/* ---------------------------------------------------------- *
		* Create new SSL connection state object                     *
		* ---------------------------------------------------------- */
		ssl = SSL_new(ctx);
	
	
		/* ---------------------------------------------------------- *
		* Attach the SSL session to the socket descriptor            *
		* ---------------------------------------------------------- */
		SSL_set_fd(ssl, sockfd);

		/* ---------------------------------------------------------- *
		* Try to SSL-connect here, returns 1 for success             *
		* ---------------------------------------------------------- */
		if ( SSL_connect(ssl) != 1 ) {
			BIO_printf(outbio, "Error: Could not build a SSL session.\n");
			sslUnable = true;
		}
		else
			BIO_printf(outbio, "Successfully enabled SSL/TLS session.\n");


		if (!sslUnable) {
			/* ---------------------------------------------------------- *
			* Get the remote certificate into the X509 structure         *
			* ---------------------------------------------------------- */
			cert = SSL_get_peer_certificate(ssl);
			if (cert == NULL)
				BIO_printf(outbio, "Error: Could not get a certificate from.\n");
			else
				BIO_printf(outbio, "Retrieved the server's certificate from.\n");

			/* ---------------------------------------------------------- *
			* extract various certificate information                    *
			* -----------------------------------------------------------*/
			certname = X509_NAME_new();
			certname = X509_get_subject_name(cert);

			/* ---------------------------------------------------------- *
			* display the cert subject here                              *
			* -----------------------------------------------------------*/
			BIO_printf(outbio, "Displaying the certificate subject data:\n");
			X509_NAME_print_ex(outbio, certname, 0, 0);
			BIO_printf(outbio, "\n");
		}
	}

		
	// SMTP conversation (sending email):
	if (!sslUnable && !tlsUnable) {
	
		if (sslAuth && !tlsStart) {
			sprintf(request,"HELO %s\r\n", servName);
			SSL_write(ssl, request, strlen(request));
			rcvdResponse(sockfd, &reply[0]);
			rcvdResponse(sockfd, &reply[0]);
		} else {
			sprintf(request,"EHLO %s\r\n", servName);
			if (tlsStart) {
				SSL_write(ssl, request, strlen(request));
			} else {
				write(sockfd, request, strlen(request));
				rcvdResponse(sockfd, &reply[0]);
			}
			
			rcvdResponse(sockfd, &reply[0]);
		}


		Base64((char *)"test.txt", basePointer, BASE64LEN);
		if (strcmp(basePointer, "") == 0) {
			fprintf(stderr, "\n==> Error: Put base64 executable in the same directory!\n");
		} else {

			sprintf(request,"AUTH LOGIN\r\n");
			if (sslAuth) {
				SSL_write(ssl, request, strlen(request));
			} else {
				write(sockfd, request, strlen(request));
			}
		
			rcvdResponse(sockfd, &reply[0]);
		
			Base64(userName, basePointer, BASE64LEN);
			sprintf(request, "%s", basePointer);
			if (sslAuth) {
				SSL_write(ssl, request, strlen(request));
			} else {
				write(sockfd, request, strlen(request));
			}
			
			if (tlsStart && strcmp(servName, "smtp.live.com") == 0) {
				sprintf(request, "\r\n");
				SSL_write(ssl, request, strlen(request));
			}
		
			rcvdResponse(sockfd, &reply[0]);
		
			Base64(userPassword, basePointer, BASE64LEN);
			sprintf(request, "%s", basePointer);
			if (sslAuth) {
				SSL_write(ssl, request, strlen(request));
			} else {
				write(sockfd, request, strlen(request));
			}
			
			if (tlsStart && strcmp(servName, "smtp.live.com") == 0) {
				sprintf(request, "\r\n");
				SSL_write(ssl, request, strlen(request));
			}
		
			rcvdResponse(sockfd, &reply[0]);
			
			/* Get response status code: */
			memcpy (request, reply, 3);
			request[3] = '\0';
			intCode = atoi(request);
			
			/* If login success: */
			if (intCode == 235) {

				sprintf(request,"MAIL FROM:<%s>\r\n", mailFrom);
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}
				rcvdResponse(sockfd, &reply[0]);
	
				sprintf(request,"RCPT TO:<%s>\r\n", mailRcpt);
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}
				rcvdResponse(sockfd, &reply[0]);
	
				sprintf(request,"DATA\r\n");
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}
				rcvdResponse(sockfd, &reply[0]);
	
				sprintf(request,"Content-Type: text/plain; charset=UTF-8\r\n");
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}
			
				sprintf(request,"Content-transfer-encoding: quoted-printable\r\n");
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}
	
				sprintf(request,"From: %s\r\n", mailFrom);
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}

	
				sprintf(request,"To: %s\r\n", mailRcpt);
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}

	
				sprintf(request,"Subject: %s\r\n", mailSubj);
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}

	
				sprintf(request,"\n\r\n");
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}

	
				sprintf(request,"%s\r\n", mailCont);
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}

	
				sprintf(request,"\r\n.\r\n");
				if (sslAuth) {
					SSL_write(ssl, request, strlen(request));
				} else {
					write(sockfd, request, strlen(request));
				}
				rcvdResponse(sockfd, &reply[0]);
		
				/* Get response status code: */
				memcpy (request, reply, 3);
				request[3] = '\0';
				intCode = atoi(request);
		
				/* If message sent:  */
				if (intCode == 250) {
					printf("\n==> Success: Message sent!\n");
				}
	
			} else {
				fprintf(stderr, "\n==> Error: Incorrect username or password!\n");
			}
		}
	
		sprintf(request,"QUIT\r\n");
		if (sslAuth) {
			SSL_write(ssl, request, strlen(request));
		} else {
			write(sockfd, request, strlen(request));
		}
		rcvdResponse(sockfd, &reply[0]);
	}



	/* Doing memory deallocation: */
		free(servName);
		free(userName);
		free(userPassword);
		free(mailFrom);
		free(mailRcpt);
		free(mailSubj);
		free(mailCont);

		if (sslAuth) {
			SSL_free(ssl);
			X509_free(cert);
			SSL_CTX_free(ctx);
			BIO_printf(outbio, "Finished SSL/TLS connection.\n");
		}
	
	close(sockfd);
	printf("TCP socket closed.\n");
	
	return 0;
}
