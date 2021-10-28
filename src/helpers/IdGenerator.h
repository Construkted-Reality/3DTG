#ifndef __IDGENERATOR_H__
#define __IDGENERATOR_H__

struct IdGenerator {
  typedef unsigned int ID;
  ID id = 0;
  void reset();
  ID next();
};

#endif // __IDGENERATOR_H__