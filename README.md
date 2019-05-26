# SMTPMail

1. MacOS app for authentication, GUI and sending custom email.
2. SMTP implementation in C language.

## Prerequisites
MacOS
XCode
OpenSSL

## Compile and prepare for running

1. cd into smtp-ssl-c folder
2. Compile with gcc on Unix:
```
$ gcc -o smtp-ssl.o smtp-ssl.c -lssl -lcrypto file-util.c
```
3. Place smtp-ssl, base64 executables and App.config and App.text
   file into folder SMTPconf and place in on the Desktop
4. Fill App.config and App.text with email info
5. Change to your user directory in file-util.c string

6. cd into xcode macos project location and change string to your 
   user directory in Runscript.command  


## Running
Run the app on XCode or export and run as SMTPMailer.app
