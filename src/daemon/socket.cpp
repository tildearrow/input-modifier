#include "imodd.h"

bool SocketInterface::init() {
  int berrno;
  imLogI("initializing socket interface...\n");
  addr.family=AF_UNIX;
  if (getenv("XDG_RUNTIME_DIR")!=NULL) {
    snprintf(addr.path,PATH_MAX,"%s/imod-%d",getenv("XDG_RUNTIME_DIR"),getuid());
  } else {
    snprintf(addr.path,PATH_MAX,"/tmp/imod-%d",getuid());
  }
  imLogD("socket path: %s\n",addr.path);
  fd=socket(AF_UNIX,SOCK_STREAM,0);
  if (fd<0) {
    imLogE("couldn't create socket: %s\n",strerror(errno));
    return false;
  }
  while (bind(fd,(const struct sockaddr*)&addr,strlen(addr.path)+sizeof(sa_family_t)+1)<0) {
    berrno=errno;
    if (connect(fd,(const struct sockaddr*)&addr,strlen(addr.path)+sizeof(sa_family_t)+1)<0) {
      imLogI("removing remnants of dead daemon...\n");
      if (unlink(addr.path)<0) {
        imLogE("hey, I can't! %s!\n",strerror(errno));
        imLogE("unusual bind error. %s\n",strerror(berrno));
        return false;
      }
      continue;
    }
    imLogE("bind error. it is very likely you are already running the input-modifier daemon! (%s)\n",strerror(berrno));
    return false;
  }
  if (listen(fd,16)<0) {
    imLogE("I am deaf: %s\n",strerror(errno));
    return false;
  }
  chmod(addr.path,0600);
  return activate();
}

void* sockThread(void* sock) {
  ((SocketInterface*)sock)->run();
  return NULL;
}

void* clientThread(void* cli) {
  SocketInterface::Connection* client=(SocketInterface::Connection*)cli;
  char* buffer;
  int length;
  imLogD("this is the client thread.\n");
  buffer=new char[16384];
  write(client->fd,"hello!\n",strlen("hello!\n"));
  while (1) {
    if ((length=read(client->fd,buffer,16384))<=0) break;
    imLogD("i have read\n");
    if (write(client->fd,buffer,length)<0) {
      imLogW("write error %d\n",strerror(errno));
    }
  }
  close(client->fd);
  imLogD("my job is done.\n");
  return NULL;
}

bool SocketInterface::activate() {
  if (pthread_create(&listenThread,NULL,sockThread,this)!=0) {
    imLogW("couldn't create thread for socket interface: %s\n",strerror(errno));
    return false;
  }
  return true;
}

void SocketInterface::run() {
  Connection client;
  while (1) {
    client.fd=accept(fd,(struct sockaddr*)&client.addr,&client.size);
    imLogI("a client is connecting...\n");
    if (pthread_create(&client.dataThread,NULL,clientThread,new Connection(client))!=0) {
      imLogW("couldn't create thread for client %d: %s\n",client.fd,strerror(errno));
      close(client.fd);
    }
  }
}
