#include <stdio.h>
#include <stdlib.h>

#include "file-util.h"

int readConfig(char *servName, char *userName, char *userPassword, char *mailFrom, char *mailRcpt, char *mailSubj, char *mailCont, unsigned short *echoServPort, int *sslAuth, int *tlsStart){

  FILE* file = fopen("/Users/<YOURUSRDIR>/Desktop/SMTPconf/App.config", "r");
    
      char fileLine[256];
      char** configPackage;
    
      bool hostIs, portIs, userIs, passIs, fromIs, rcptIs, subjIs, contIs, sslIs, tlsIs, allIs;
      hostIs = portIs = userIs = passIs = fromIs = rcptIs = subjIs = contIs = sslIs = tlsIs = allIs = false;
      
      while (fgets(fileLine, sizeof(fileLine), file)) {
        configPackage = str_split(fileLine, ':');

        printf("CONFIG PACK SPLIT : %s  :  %s\n", configPackage[0], configPackage[1]);
      
        if (strcmp(configPackage[0], "HOST") == 0 && !hostIs) {
          memcpy(servName, configPackage[1], strlen(configPackage[1])-1);
          
          if (strlen(configPackage[1]) > 1) {
            hostIs = true;
            printf("%s\n", servName);
          }
        } 

        else if (strcmp(configPackage[0], "PORT") == 0 && !portIs) {
          *echoServPort = atoi(configPackage[1]);
          if (*echoServPort == 0) {
            *echoServPort = 25;
            printf("Default port: %d\n", *echoServPort);
          }
          
          portIs = true;
          printf("%d\n", *echoServPort);
        } 

        else if (strcmp(configPackage[0], "SSLI") == 0 && !sslIs) {
          if (strlen(configPackage[1]) > 1) {
            sslIs = true;

            if(strcmp(configPackage[1], "TRUE\n") == 0) {
              *sslAuth = 1;
              printf("SSL TRUE\n");
            }
          }
        } 
        
        else if (strcmp(configPackage[0], "TLSI") == 0 && !tlsIs) {
          if (strlen(configPackage[1]) > 1) {
            tlsIs = true;

            if(strcmp(configPackage[1], "TRUE\n") == 0) {
              *tlsStart = 1;
              printf("TSL TRUE\n");
            }
          }
        }

        else if (strcmp(configPackage[0], "USER") == 0 && !userIs) {
          memcpy(userName, configPackage[1], strlen(configPackage[1])-1);
          if (strlen(configPackage[1]) > 1) {
            userIs = true;
            printf("%s\n", userName);
          }
        }
        
        else if (strcmp(configPackage[0], "PASS") == 0 && !passIs) {
          memcpy(userPassword, configPackage[1], strlen(configPackage[1])-1);
          if (strlen(configPackage[1]) > 1) {
            passIs = true;
            printf("%s\n", userPassword);
          }
        } 
        
        else if (strcmp(configPackage[0], "FROM") == 0 && !fromIs) {
          memcpy(mailFrom, configPackage[1], strlen(configPackage[1])-1);
          if (strlen(configPackage[1]) > 1) {
            fromIs = true;
            printf("%s\n", mailFrom);
          }
        } 
        
        else if (strcmp(configPackage[0], "RCPT") == 0 && !rcptIs) {
          memcpy(mailRcpt, configPackage[1], strlen(configPackage[1])-1);
          if (strlen(configPackage[1]) > 1) {
            rcptIs = true;
            printf("%s\n", mailRcpt);
          }
        } 
        
        else if (strcmp(configPackage[0], "SUBJ") == 0 && !subjIs) {
          memcpy(mailSubj, configPackage[1], strlen(configPackage[1])-1);
          if (strlen(configPackage[1]) > 1) {
            subjIs = true;
            printf("%s\n", mailSubj);
          }
        } 
        
        else if (strcmp(configPackage[0], "CONT") == 0 && !contIs) {
          memcpy(mailCont, configPackage[1], strlen(configPackage[1])-1);
          if (strlen(configPackage[1]) > 1) {
            contIs = true;
            printf("%s\n", mailCont);
          }
        } 
        

        
        
        for (int i = 0; *(configPackage + i); i++) { free(*(configPackage + i)); }
        free(configPackage);
        
        
        if (hostIs && portIs && userIs && passIs && fromIs && rcptIs && subjIs && contIs && sslIs && tlsIs) {
          allIs = true;
          break;
        }
      }
      fclose(file);
      
      if (!allIs) {
        fprintf(stderr, "Error: Configuration file is incomplete!\n");
        exit(1);
      }
      
      if (*tlsStart && *sslAuth) {
        fprintf(stderr, "Error: Using SSL communication and STARTTLS command is forbidden!\n");
        exit(1);
      }

  return 0;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char **)malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}