#!/bin/bash

echo "#include \"names.h\""; echo

echo "const char* evnames[EV_CNT]={"
cat /usr/include/linux/input-event-codes.h | egrep "^#define EV_[A-Z_0-9]+?[ 	]+([0-9]+|0x[0-9a-fA-F]+)" | sed -r "s/^#define (EV_[A-Z_0-9]+)[ 	]+([0-9]+|0x[0-9a-fA-F]+)/  [\2]=\"\1\",/" | sed -r "s/\/\*.+$//" | sed "/EV_MAX/d"
echo "};"; echo

echo "const char* keynames[KEY_CNT]={"
cat /usr/include/linux/input-event-codes.h | egrep "^#define (KEY|BTN)_[A-Z_0-9]+?[ 	]+([0-9]+|0x[0-9a-fA-F]+)" | sed -r "s/^#define ((KEY|BTN)_[A-Z_0-9]+)[ 	]+([0-9]+|0x[0-9a-fA-F]+)/  [\3]=\"\1\",/" | sed -r "s/\/\*.+$//" | sed "/KEY_MAX/d"
echo "};"; echo

echo "const char* relnames[REL_CNT]={"
cat /usr/include/linux/input-event-codes.h | egrep "^#define REL_[A-Z_0-9]+?[ 	]+([0-9]+|0x[0-9a-fA-F]+)" | sed -r "s/^#define (REL_[A-Z_0-9]+)[ 	]+([0-9]+|0x[0-9a-fA-F]+)/  [\2]=\"\1\",/" | sed -r "s/\/\*.+$//" | sed "/REL_MAX/d"
echo "};"; echo

echo "const char* absnames[ABS_CNT]={"
cat /usr/include/linux/input-event-codes.h | egrep "^#define ABS_[A-Z_0-9]+?[ 	]+([0-9]+|0x[0-9a-fA-F]+)" | sed -r "s/^#define (ABS_[A-Z_0-9]+)[ 	]+([0-9]+|0x[0-9a-fA-F]+)/  [\2]=\"\1\",/" | sed -r "s/\/\*.+$//" | sed "/ABS_MAX/d"
echo "};"; echo

echo "const char* swnames[SW_CNT]={"
cat /usr/include/linux/input-event-codes.h | egrep "^#define SW_[A-Z_0-9]+?[ 	]+([0-9]+|0x[0-9a-fA-F]+)" | sed -r "s/^#define (SW_[A-Z_0-9]+)[ 	]+([0-9]+|0x[0-9a-fA-F]+)/  [\2]=\"\1\",/" | sed -r "s/\/\*.+$//" | sed "/SW_MAX/d"
echo "};"
