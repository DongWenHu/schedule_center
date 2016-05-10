#ifndef __TYPEDEFS_H_2016_05_06__
#define __TYPEDEFS_H_2016_05_06__

#include <boost/thread/thread.hpp>

namespace mpsp{

#define _SIGNALS  public
#define _SLOTS    public

typedef boost::shared_mutex rwmutex;
typedef boost::shared_lock<rwmutex> readLock;
typedef boost::unique_lock<rwmutex> writeLock;

}

#endif  //  __TYPEDEFS_H_2016_05_06__