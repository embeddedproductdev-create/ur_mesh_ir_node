#ifndef __SIMPLE_FTP_CLIENT_H
#define __SIMPLE_FTP_CLIENT_H
#include <Arduino.h>
#include <WiFi.h>
#include <lwip/sockets.h>


bool connectToFTPServer(char* Host,unsigned int ftpPort);
uint8_t sendFTPCommand(String cmd);
bool loginFTPServer(String username , String password);
void readFTPResponse();
bool parseFTPDataPort();
bool downloadFileFromFTP(const char *path);
void listDir(const char * dirname, uint8_t levels);
void readFile(const char * path) ;
void writeFile(const char * path, const char * message);
bool FS_rmFile(const char * path);
bool Format_SPIFFS();
void update_firmware(char* path);
uint64_t Get_File_Size( const char * path);
#endif