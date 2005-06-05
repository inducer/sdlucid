// ----------------------------------------------------------------------------
//  Description      : SDL wrappers base
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckenr
// ----------------------------------------------------------------------------




#include <sdlucid_threads.hh>




using namespace sdl;




// mutex ---------------------------------------------------------------------
mutex::mutex() {
  Mutex = SDL_CreateMutex();
  if (!Mutex)
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  }




mutex::~mutex() {
  SDL_DestroyMutex(Mutex);
  }




void mutex::p() {
  if (SDL_mutexP(Mutex) == -1)
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  }




void mutex::v() {
  if (SDL_mutexV(Mutex) == -1)
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  }
