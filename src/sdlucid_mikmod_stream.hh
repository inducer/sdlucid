// ----------------------------------------------------------------------------
//  Description      : SDL mod player
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_MIKMOD_STREAM
#define SDLUCID_MIKMOD_STREAM




#include <sdlucid_base.hh>
#ifdef SDLUCID_HAS_LIBMIKMOD
#include <mikmod.h>
#include <iostream>
#include <sdlucid_audio.hh>




namespace sdl {
  using ixion::TSize;

  class audio_mod_stream : public audio_buffered_stream {
    private:
      typedef audio_buffered_stream 	super;
      MODULE				*Module;
      TSize				BytesPerSample;
      
    public:
      audio_mod_stream(std::istream &strm,float vol = 1,float panning = 0);
      ~audio_mod_stream();
      std::string describe();
  
    private:
      void internalStartUp(audio_manager *mgr,audio_format &fmt);
      bool isEndOfStream() const;
      TSize getStreamData(void *data,TSize maxsize);
    };
  }




#endif // SDLUCID_HAS_LIBMIKMOD
#endif
