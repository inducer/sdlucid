// ----------------------------------------------------------------------------
//  Description      : SDL thread wrappers
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_THREADS
#define SDLUCID_THREADS




#include <SDL.h>
#include <sdlucid_base.hh>




namespace sdl {
  // mutex --------------------------------------------------------------------
  class mutex {
    protected:
      SDL_mutex		*Mutex;
    
    public:
      mutex();
      ~mutex();
      void p();
      void v();
    };
  
  
  
  
  
  // mutex_locker -------------------------------------------------------------
  class mutex_locker {
    protected:
      mutex		&Mutex;
    
    public:
      mutex_locker(mutex &mutex)
        : Mutex(mutex) {
        Mutex.p();
        }
      ~mutex_locker() {
        Mutex.v();
        }
    };
  }




#endif
