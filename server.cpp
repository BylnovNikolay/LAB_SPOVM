#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <vector>

#ifdef _MSC_VER
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

using namespace std;

int WriteToFile(int procPid, char* str) {
  ofstream f("mes.txt", ios_base::out | ios_base::app);
  f << setfill('0');
  f << setw(5) << procPid;
  f << "> " << str << endl;

  f.close();

  return 0;
}

#ifdef _MSC_VER

void Worked() {
  int usersOnline = 0, procPid = GetCurrentProcessId();

  HANDLE hPipe;
  char buffer[1024];
  DWORD dwRead;

  hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\pipe_chat"),
                          PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
                          PIPE_WAIT,
                          1,
                          1024 * 16,
                          1024 * 16,
                          NMPWAIT_USE_DEFAULT_WAIT,
                          NULL);

  while (hPipe != INVALID_HANDLE_VALUE) {
    if (ConnectNamedPipe(hPipe, NULL) != FALSE) { // wait for someone to connect to the pipe
      while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE) {
        // add terminating zero
        buffer[dwRead] = '\0';

        // do something with data in buffer
        if (strcmp(buffer, "client disconnect") == 0) {
          cout << "online: " << --usersOnline << endl;
        } else {
          cout << "server received: " << buffer << endl;
          WriteToFile(procPid, buffer);
        }
      }
    }

    DisconnectNamedPipe(hPipe);
  }
  system("pause");
}

#else

void Worked() {
  
  int usersOnline = 0, fd, res, procPid = getpid();
  char len;
  char stringBuffer[256];
  memset(stringBuffer, 0, 256);

  cout << "Start" << endl;

  
  int code = mkfifo("/tmp/bigboss", 0666);
  if (code == -1) {
    cout << "file may exist" << endl;
  } else {
    cout << "server was created" << endl;    
  }
  
  fd = open("/tmp/bigboss", O_RDONLY); 
  if (fd == -1) {
    cout << "Cannot open for read" << endl;
    return;
  } else {
    cout << "FIFO opened" << endl;
  }

  while (1) {
    res = read(fd, &len, 1); // taking length of the incoming message
    if (res == 0) break; // if 0 - exit
    read(fd, &procPid, sizeof(int)); // taking process pid
    read(fd, stringBuffer, len); // taking incoming message
    stringBuffer[(int)len] = 0; // last symbol - '\0'

    if (strcmp(stringBuffer, "client disconnect") == 0) { // client disconnect
      cout << "online: " << --usersOnline << endl;
    } else {
      cout << "Server received: id[" << procPid << "] " << stringBuffer << endl;
      WriteToFile(procPid, stringBuffer);
    }
  }

  cout << "Exit" << endl;
}

#endif

int main() {
  Worked();

  return 0;
}