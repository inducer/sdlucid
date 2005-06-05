// ----------------------------------------------------------------------------
//  Description      : SDL instance
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_INSTANCE
#define SDLUCID_INSTANCE




#include <SDL.h>




namespace sdl {
  class sdl_instance {
    public:
      sdl_instance(Uint32 flags = SDL_INIT_EVERYTHING);
      ~sdl_instance();
      
      bool initializedSubsystem(Uint32 subsys);
      void initializeSubsystem(Uint32 subsys);
      void quitSubsystem(Uint32 subsys);
    };
  }




#endif
