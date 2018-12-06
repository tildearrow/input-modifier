#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string>
#include <vector>

typedef std::string string;

int sock;
struct {
  sa_family_t family;
  char path[PATH_MAX];
} address;

char inputBuf[32768];
int inputBufLen;
fd_set fds;

int c;
char typeBuf[32768];
int typeBufLen;
bool quit;

enum nextCharType {
  Normal=0,
  Escape,
  CSI
};

int nextCharIs;

struct termios termPropNew;
struct termios termPropOld;
struct winsize termSize;

string line;
std::vector<string> history;
int historyPos;

int main(int argc, char** argv) {
  quit=false;
  nextCharIs=Normal;
  // initialize connection
  address.family=AF_UNIX;
  if (getenv("XDG_RUNTIME_DIR")!=NULL) {
    snprintf(address.path,PATH_MAX,"%s/imod-%d",getenv("XDG_RUNTIME_DIR"),getuid());
  } else {
    snprintf(address.path,PATH_MAX,"/tmp/imod-%d",getuid());
  }
  printf("socket path: %s\n",address.path);
  sock=socket(AF_UNIX,SOCK_STREAM,0);
  if (sock<0) {
    perror("couldn't create socket");
    return 1;
  }
  if (connect(sock,(const struct sockaddr*)&address,strlen(address.path)+sizeof(sa_family_t)+1)<0) {
    printf("couldn't connect: ");
    if (errno==EINVAL) {
      printf("Unknown error\n");
    } else {
      printf("%s\n",strerror(errno));
    }
    printf("is the daemon running?\n");
    return 0;
  }
  // initialize terminal
  setvbuf(stdout,NULL,_IONBF,1);
  if (tcgetattr(0,&termPropOld)!=0) {
    printf("warning: could not get console attributes...\n");
  }
  termPropNew=termPropOld;
  termPropNew.c_lflag&=~ECHO;
  termPropNew.c_lflag&=~ICANON;
  if (tcsetattr(0,TCSAFLUSH,&termPropNew)!=0) {
    printf("warning: could not set console attributes...\n");
  }
  ioctl(1,TIOCGWINSZ,&termSize);
  // main loop
  while (1) {
    FD_ZERO(&fds);
    FD_SET(0,&fds);
    FD_SET(sock,&fds);
    select(sock+1,&fds,NULL,NULL,NULL);
    if (FD_ISSET(sock,&fds)) {
      inputBufLen=read(sock,inputBuf,32768);
      if (inputBufLen<=0) {
        printf("\nconnection closed...\n");
        break;
      }
      write(1,inputBuf,inputBufLen);
    }
    if (FD_ISSET(0,&fds)) {
      typeBufLen=read(0,typeBuf,32768);
      if (typeBufLen<=0) {
        quit=true;
      }
      for (int i=0; i<typeBufLen; i++) {
        if (nextCharIs==Normal) {
          switch (typeBuf[i]) {
            case 4: // EOF
              quit=true;
              break;
            case '\x1b': // escape
              nextCharIs=Escape;
              break;
            case '\n':
              write(sock,line.c_str(),line.size()+1);
              printf("> ");
              history.push_back(line);
              historyPos=history.size();
              line="";
              break;
            case '\b': case 127:
              if (!line.empty()) {
                line.erase(line.end()-1);
                printf("\b \b");
              }
              break;
            default:
              line+=typeBuf[i];
              fputc(typeBuf[i],stdout);
              break;
          }
        } else if (nextCharIs==Escape) {
          switch (typeBuf[i]) {
            case '[':
              nextCharIs=CSI;
              break;
            default:
              printf("^[%c",typeBuf[i]);
              nextCharIs=Normal;
              break;
          }
        } else if (nextCharIs==CSI) {
          switch (typeBuf[i]) {
            case 'A': // up
              if (history.empty()) break;
              historyPos--;
              if (historyPos<0) historyPos=0;
              line=history[historyPos];
              printf("\x1b[2K\x1b[G> %s",line.c_str());
              break;
            case 'B': // down
              if (history.empty()) break;
              historyPos++;
              if (historyPos>(int)history.size()) {
                historyPos=history.size();
              } else {
                if (historyPos==(int)history.size()) {
                  line="";
                } else {
                  line=history[historyPos];
                }
              }
              printf("\x1b[2K\x1b[G> %s",line.c_str());
              break;
            case 'C': // right
              printf("rightward\n");
              break;
            case 'D': // left
              printf("leftward\n");
              break;
            default:
              printf("^[[%c",typeBuf[i]);
          }
          nextCharIs=Normal;
        }
      }
    }
    if (quit) break;
  }
  close(sock);
  if (tcsetattr(0,TCSAFLUSH,&termPropOld)!=0) {
    printf("error: could not set console attributes! this means your console is most likely going to misbehave :(\n");
  }
  return 0;
}
