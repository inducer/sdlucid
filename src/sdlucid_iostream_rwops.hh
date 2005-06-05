// ----------------------------------------------------------------------------
//  Description      : SDL c++ stream to rwops wrapper
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_IOSTREAM_RWOPS
#define SDLUCID_IOSTREAM_RWOPS




#include <SDL.h>
#include <iostream>



namespace sdl {
  SDL_RWops *RWopsFromStream(std::istream *istr,bool assume_ownership = true);
  }




#endif
