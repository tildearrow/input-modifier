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
  bool haveNotFound;
  std::vector<string>* pargs;
  string arg;
  buffer=new char[16384];
  if (write(client->fd,"input-modifier\n",strlen("input-modifier\n"))<0) {
    imLogI("hey! client %d ran away!\n");
    close(client->fd);
    return NULL;
  }
  // thread loop
  imLogI("client %d connected\n",client->fd);
  while (1) {
    if ((length=read(client->fd,buffer,16384))<=0) break;
    buffer[length-1]=0;
    imLogI("client %d: %s\n",client->fd,buffer);
    // parse arguments
    pargs=new std::vector<string>;
    arg="";
    for (int i=0; i<length-1; i++) {
      if (buffer[i]==' ') {
        pargs->push_back(arg);
        arg="";
      } else {
        arg+=buffer[i];
      }
    }
    pargs->push_back(arg);
    for (auto i: *pargs) {
      imLogD("- %s\n",i.c_str());
    }
    haveNotFound=true;
    for (int i=0; cmds[i].n!=NULL; i++) {
      if (strcmp((*pargs)[0].c_str(),cmds[i].n)==0) {
        cmds[i].c(client->fd,pargs);
        haveNotFound=false;
        break;
      }
    }
    delete pargs;
    if (haveNotFound) {
      if (write(client->fd,"Unknown command.\n",strlen("Unknown command.\n"))<0) {
        imLogI("couldn't tell client %d about its mistake...\n",client->fd);
        close(client->fd);
        return NULL;
      }
    }
    if (write(client->fd,"Ready.\n",strlen("Ready.\n"))<0) {
      imLogI("couldn't tell client %d we're ready...\n");
      close(client->fd);
      return NULL;
    }
  }
  close(client->fd);
  imLogI("client %d lost connection\n",client->fd);
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
