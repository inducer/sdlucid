// ----------------------------------------------------------------------------
//  Description      : SDL instance
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#include <sdlucid_base.hh>
#include <sdlucid_instance.hh>




using namespace sdl;




// sdl_instance ---------------------------------------------------------------
sdl_instance::sdl_instance(Uint32 flags) {
  if (SDL_Init(flags))
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  }




sdl_instance::~sdl_instance() {
  SDL_Quit();
  }




bool sdl_instance::initializedSubsystem(Uint32 subsys) {
  return SDL_WasInit(subsys);
  }




void sdl_instance::initializeSubsystem(Uint32 subsys) {
  if (SDL_InitSubSystem(subsys))
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  }




void sdl_instance::quitSubsystem(Uint32 subsys) {
  SDL_QuitSubSystem(subsys);
  }
