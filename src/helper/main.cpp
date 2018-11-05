#include "uinputHelper.h"

int main(int argc, char** argv) {
  struct group* inputGroup;
  imLogI("input-modifier uinput perms helper\n");
  if (geteuid()!=0) {
    imLogE("you need to set this executable's owner to root,\n");
    imLogE("and set the setuid bit on it.\n");
    return 1;
  }
  // try to fix this problem
  inputGroup=getgrnam("input");
  if (inputGroup==NULL) {
    imLogE("the input group does not exist!\n");
    return 1;
  }
  imLogD("setting owner/group...\n");
  if (chown("/dev/uinput",0,inputGroup->gr_gid)<0) {
    imLogE("can't set group for uinput: %s\n",strerror(errno));
    imLogE("giving up.\n");
    return 1;
  }
  imLogD("setting mod...\n");
  if (chmod("/dev/uinput",0660)<0) {
    imLogE("can't set mod for uinput: %s\n",strerror(errno));
    imLogE("giving up.\n");
    return 1;
  }
  imLogI("done.\n");
}
