#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <string.h>

#ifdef _MSC_VER
#include <Windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#define MAXLINESFROMFILE 256
#define MAXMESSAGELENGTH 256

using namespace std;



int PrintFile(char *filename) {
  ifstream file(filename);
  string lines[MAXLINESFROMFILE];
  int size = 0;

  // while lines existing
  while (file.good()) {
    getline(file, lines[size]);
    cout << lines[size] << endl;
    size++;
  }

  return 0;
}

#ifdef _MSC_VER

int SendStringToServer(const char *str) {
  HANDLE hPipe;
  DWORD dwWritten;

  hPipe = CreateFile(TEXT("\\\\.\\pipe\\pipe_chat"),
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL);

  if (hPipe != INVALID_HANDLE_VALUE) {
    WriteFile(hPipe,
              str,
              strlen(str) + 1,   
              &dwWritten,
              NULL);

    CloseHandle(hPipe);
  }

  return 0;
}


int Worked() {
  char filename[256];
  strcpy(filename, "mes.txt");
  char message[1024];

  while (1) {
    PrintFile(filename);
    cout << "> ";
    cin.getline(message, MAXMESSAGELENGTH);
    fflush(stdin);

    if (strcmp(message, "q") == 0) { // quit
      SendStringToServer("client disconnect");
      return 0;
    } else { // send message to server
      SendStringToServer(message);
    }

  }

  system("pause");
  return 0;
}

#else

int SendStringToServer(int fd, const char *str) {
  int procPid = getpid();

  char l = (char)strlen(str);
  write(fd, &l, 1);
  write(fd, &procPid, sizeof(int));
  write(fd, str, strlen(str));

  return 0;
}

int Worked() {
  char* message;
  char filename[256];
  strcpy(filename, "mes.txt");
  message = (char *)malloc(MAXMESSAGELENGTH * sizeof(char));

  // Open write only
  int fd = open("/tmp/bigboss", O_WRONLY);
  if (fd == -1) {    
    return 1;
  }  

  while (1) {
    PrintFile(filename);
    cout << "> ";
    cin.getline(message, MAXMESSAGELENGTH);
    fflush(stdin);

    if (strcmp(message, "q") == 0) { // quit
      SendStringToServer(fd, "client disconnect");
      free(message);
      return 0;
    } else { // send message to server
      SendStringToServer(fd, message);
    }

  }

  free(message);

  close(fd);

  return 0;
}

#endif

int main(int args, char* argv[]) {
  Worked();

  return 0;
}