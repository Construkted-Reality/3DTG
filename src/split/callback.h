#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include <functional>

#include "./../loaders/Loader.h"
#include "./../helpers/IdGenerator.h"

typedef std::function<void (GroupObject object, IdGenerator::ID targetId, IdGenerator::ID parentId, unsigned int level, bool indexedGeometry)> ResultCallback;

#endif // __CALLBACK_H__