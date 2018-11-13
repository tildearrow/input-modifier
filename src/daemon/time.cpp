#include "imodd.h"

// extensions to timespec

struct timespec mkts(time_t sec, long nsec) {
  struct timespec ret;
  ret.tv_sec=sec;
  ret.tv_nsec=nsec;
  return ret;
}

const int pow10[10]={
  1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
};

struct timespec stots(string s) {
  struct timespec ret;
  long int upper, lower;
  bool curStat;
  string upel, lowel;
  curStat=false;
  for (size_t i=0; i<s.length(); i++) {
    if (s[i]=='.' || s[i]==',') {
      if (curStat) {
        throw std::invalid_argument("stots");
      }
      curStat=true;
      continue;
    }
    if (s[i]<'0' || s[i]>'9') {
      throw std::invalid_argument("stots");
    }
    if (curStat) {
      if (lowel.length()<10) {
        lowel+=s[i];
      }
    } else {
      upel+=s[i];
    }
  }
  if (lowel.empty()) lowel="0";
  try {
    upper=stoi(upel);
  } catch (std::exception& err) {
    throw err;
  }
  try {
    lower=stoi(lowel)*pow10[9-lowel.length()];
  } catch (std::exception& err) {
    throw err;
  }
  ret.tv_sec=upper;
  ret.tv_nsec=lower;
  return ret;
}

string tstos(struct timespec ts) {
  string ret;
  int deci;
  deci=ts.tv_nsec;
  while ((deci%10)==0 || deci==0) {
    deci/=10;
  }
  ret+=std::to_string(ts.tv_sec);
  ret+=".";
  ret+=std::to_string(deci);
  return ret;
}

struct timespec curTime(clockid_t clockSource) {
  struct timespec ret;
  clock_gettime(clockSource,&ret);
  return ret;
}

bool operator >(const struct timespec& l, const struct timespec& r) {
  if (l.tv_sec==r.tv_sec) {
    return l.tv_nsec>r.tv_nsec;
  }
  return l.tv_sec>r.tv_sec;
}

bool operator <(const struct timespec& l, const struct timespec& r) {
  if (l.tv_sec==r.tv_sec) {
    return l.tv_nsec<r.tv_nsec;
  }
  return l.tv_sec<r.tv_sec;
}

bool operator ==(const struct timespec& l, const struct timespec& r) {
  return (l.tv_sec==r.tv_sec && l.tv_nsec==r.tv_nsec);
}

struct timespec operator -(const struct timespec& l, const long& r) {
  struct timespec ret;
  ret=l;
  if ((ret.tv_nsec-=r)<0) {
    ret.tv_sec-=1-ret.tv_nsec/1000000000;
    ret.tv_nsec+=(1+(ret.tv_nsec/1000000000))*1000000000;
  }
  return ret;
}

struct timespec operator -(const struct timespec& l, const struct timespec& r) {
  struct timespec ret;
  ret=l;
  ret.tv_sec-=r.tv_sec;
  ret.tv_nsec-=r.tv_nsec;
  while (ret.tv_nsec<0) {
    ret.tv_nsec+=1000000000;
    ret.tv_sec--;
  }
  return ret;
}

struct timespec operator +(const struct timespec& l, const struct timespec& r) {
  struct timespec ret;
  ret.tv_sec=l.tv_sec+r.tv_sec;
  ret.tv_nsec=l.tv_nsec+r.tv_nsec;
  while (ret.tv_nsec>=1000000000) {
    ret.tv_nsec-=1000000000;
    ret.tv_sec++;
  }
  return ret;
}
