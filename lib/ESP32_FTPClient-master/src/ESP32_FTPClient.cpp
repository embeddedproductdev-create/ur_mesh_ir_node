#include <WiFiClient.h>
#include "ESP32_FTPClient.h"
#include "SPIFFS.h"
#include "Update.h"

WiFiClient ftpClient;

char *host=NULL;
unsigned int DataPort = 1;
String recvBuf;
char ftpRespDataBuf[100] = {0};

bool connectToFTPServer(char* Host,unsigned int ftpPort)
{
  host = Host;
  if (!ftpClient.connect(host, ftpPort)) {
    Serial.println("FTP Server connection failed");
    return false;
  }
  return true;
}

void readFTPResponse()
{
  recvBuf = "";
  String temp;
  char ch;
  for (int i = 0 ; i < 3 ; i++)
  {
    while (ftpClient.available())
    {
      ch = ftpClient.read();//readStringUntil('\r');
      recvBuf += ch;
    }
  }
  Serial.print("> ");
  Serial.print(recvBuf);
  ftpClient.flush();
}

uint8_t sendFTPCommand(String cmd)
{
  int ret = 0;
  Serial.print("< ");
  Serial.println(cmd);
  ftpClient.println(cmd);
  delay(200);
  readFTPResponse();
  if (recvBuf.length())
    sscanf(recvBuf.c_str(), "%d ", &ret);
  return (uint8_t)ret;
}

bool loginFTPServer(String username , String password)
{
  uint8_t ret = 0;
  ret = sendFTPCommand("USER "+username);  
  ret = sendFTPCommand("PASS "+password);
  if (ret == 230 )
  {
    return true;
  }
  return false;
}

bool parseFTPDataPort()
{
  char str[] = "";
  DataPort = 0;
  sscanf(recvBuf.c_str(), "229 Entering Extended Passive Mode (|||%d|)", &DataPort);
  Serial.printf("Port Parsed %d\n", DataPort);
  if (DataPort)
  {
    return true;
  }
  return false;
}

bool initFileDownloadSequence()
{
  return 0;
}

bool downloadFileFromFTP(const char *path)
{
  int socket_fd;
  struct sockaddr_in sa, ra;
  char cmdbuf[100]={0};
  
  int recv_data = 0;
  uint8_t data_buffer[5000];

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }

  socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  if ( socket_fd < 0 )
  {
    printf("socket call failed");
    return false;
  }

  /* Receiver connects to server ip-address. */

  memset(&ra, 0, sizeof(struct sockaddr_in));
  ra.sin_family = AF_INET;
  ra.sin_addr.s_addr = inet_addr(host);
  ra.sin_port = htons(DataPort);

  if (connect(socket_fd, (const sockaddr*)&ra, sizeof(struct sockaddr_in)) < 0)
  {
    printf("connect failed \n");
    close(socket_fd);
    return false;
  }
  sprintf(cmdbuf,"%s %s","RETR",path);
  Serial.println(cmdbuf);
  int recvCount = 0;
  uint32_t t = millis();
  sendFTPCommand(cmdbuf);
  Serial.println("Downloading File");
  while ( (recv_data = recv(socket_fd, data_buffer, sizeof(data_buffer), 0)) > 0)
  {
    file.write(data_buffer, recv_data);
    recvCount += recv_data;
  }
  uint32_t ttotal = (millis() - t);
  Serial.println("File Download Completed");
  file.close();
  close(socket_fd);
  return true;
}

void listDir(const char * dirname, uint8_t levels) 
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = SPIFFS.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println((float)file.size()/(1024 * 1024));
    }
    file = root.openNextFile();
  }
}

void readFile(const char * path) 
{
  Serial.printf("Reading file: %s\n", path);

  File file = SPIFFS.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(const char * path, const char * message)
{
    Serial.printf("Writing file: %s\r\n", path);

    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- frite failed");
    }
}

bool FS_rmFile(const char * path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (SPIFFS.remove(path))
  {
    Serial.println("File deleted");
    return true;
  }
  Serial.println("Delete failed");
  return false;
}

bool Format_SPIFFS()
{
	if( SPIFFS.format() )
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool mount_SPIFFS()
{
	Serial.println(F("Inizializing FS..."));
    if (SPIFFS.begin()){
        Serial.println(F("done."));
    }else{
        Serial.println(F("fail."));
    }
  uint64_t usedBy = SPIFFS.usedBytes ();
  uint64_t cardSize = SPIFFS.totalBytes() / (1024 * 1024);
  uint64_t freeSize = ( cardSize - usedBy) / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB Free Size %lluMB (%f %%)\n", cardSize, freeSize, (float)((freeSize * 100) / cardSize));
  return true;
}

uint64_t Get_File_Size( const char * path)
{
  File file = SPIFFS.open(path, FILE_READ);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return 0;
  }
  uint64_t fileSize = file.size();
  return fileSize;
}
void update_firmware(char* path)
{
  File file = SPIFFS.open(path);
  
  if(!file){
      Serial.println("Failed to open file for reading");
      return;
  }
     
  Serial.println("Starting update..");
      

  size_t fileSize = file.size();

  if(!Update.begin(fileSize)){
     
     Serial.println("Cannot do the update");
     return;
  };

  Update.writeStream(file);

  if(Update.end()){
     
    Serial.println("Successful update");  
  }else {
     
    Serial.println("Error Occurred: " + String(Update.getError()));
    return;
  }
   
  file.close();

  Serial.println("Reset in 4 seconds...");
  delay(4000);

  ESP.restart();
}