// temporarily copied from taebron because I am running out of time!
// but will be replaced with a better thing really soon!
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char** argv) {
  struct sockaddr_un serverAddr;
  int server;
  char* inputBuf; int inputSize;
  char* outputBuf; int outputSize;
  char SOCKPATH[4096];
  int nextChar;
  _Bool finish;
  if (getenv("XDG_RUNTIME_DIR")!=NULL) {
    sprintf(SOCKPATH,"%s/imod-%d",getenv("XDG_RUNTIME_DIR"),getuid());
  } else {
    sprintf(SOCKPATH,"/tmp/imod-%d",getuid());
  }
  inputBuf=malloc(4096);
  outputBuf=malloc(4096);
  inputSize=0;
  outputSize=0;
  finish=0;
  serverAddr.sun_family=AF_UNIX;
  strcpy(serverAddr.sun_path,SOCKPATH);
  if ((server=socket(AF_UNIX,SOCK_STREAM,0))<0) {
    perror("error while creating socket");
    free(inputBuf);
    free(outputBuf);
    return 1;
  }
  if (connect(server,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_un))<0) {
    perror("can't connect");
    free(inputBuf);
    free(outputBuf);
    return 1;
  }
  inputSize=read(server,inputBuf,4096);
  inputBuf[inputSize]=0;
  if (strcmp(inputBuf,"input-modifier\n")!=0) {
    fprintf(stderr,"not the input-modifier daemon\n");
    close(server);
    free(inputBuf);
    free(outputBuf);
    return 1;
  }
  while (1) {
    outputSize=0;
    fprintf(stderr,"> ");
    while (1) {
      nextChar=getchar();
      if (nextChar==EOF) {
        finish=1;
        break;
      }
      if (nextChar=='\n') {
        outputBuf[outputSize++]=nextChar;
        outputBuf[outputSize]=0;
        break;
      } else {
        if (outputSize<4096) {
          outputBuf[outputSize++]=nextChar;
        }
      }
    }
    if (finish) {
      break;
    }
    if (outputBuf[0]=='\n') {
      continue;
    }
    write(server,outputBuf,outputSize);
    do {
      inputSize=read(server,inputBuf,4096);
      if (inputSize<=0) {
        finish=1;
        break;
      }
      inputBuf[inputSize]=0;
      printf("%s",inputBuf);
    } while (strcmp(inputBuf+inputSize-7,"Ready.\n")!=0);
    if (finish) {
      break;
    }
  }
  putchar('\n');
  close(server);
  free(inputBuf);
  free(outputBuf);
  return 0;
}
