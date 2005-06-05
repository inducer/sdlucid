// ----------------------------------------------------------------------------
//  Description      : SDL mp3 stream
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_SMPEG_STREAM
#define SDLUCID_SMPEG_STREAM




#include "sdlucid_base.hh"
#ifdef SDLUCID_HAS_SMPEG
#include <smpeg/smpeg.h>
#include <sdlucid_audio.hh>




namespace sdl {
  class audio_mp3_stream : public audio_buffered_stream {
    private:
      typedef audio_buffered_stream 	Super;
      SMPEG				*Mpeg;
      TSize				BytesPerSample;
      bool				IsEOF;
      
    public:
      audio_mp3_stream(std::istream *strm,bool assume_ownership,float vol = 1,float panning = 0);
      ~audio_mp3_stream();
      std::string describe();
  
    private:
      void internalStartUp(audio_manager *mgr,audio_format &fmt);
      bool isEndOfStream() const;
      TSize getStreamData(void *data,TSize maxsize);
    };
  }




#endif // SDLUCID_HAS_SMPEG
#endif
