// ----------------------------------------------------------------------------
//  Description      : SDL mp3 stream
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------





#include <iostream>
#include <sdlucid_smpeg_stream.hh>
#include <sdlucid_iostream_rwops.hh>




#ifndef SDLUCID_HAS_SMPEG
#error "this modules shouldn't even be compiled if we do not have smpeg"
#endif




using namespace std;
using namespace sdl;




// audio_mp3_stream ------------------------------------------------------------
audio_mp3_stream::audio_mp3_stream(istream *strm,bool assume_ownership,float vol,float panning) 
  : Super(vol,panning),IsEOF(false) {
  SDL_RWops *rwops = RWopsFromStream(strm,assume_ownership);
  
  Mpeg = SMPEG_new_rwops(rwops,NULL,0);
  if (Mpeg == NULL) {
    SDL_RWclose(rwops);
    EXSDL_THROWINFO(ECSDL_IOERROR,"could not open mpeg stream")
    }
  
  SMPEG_enablevideo(Mpeg,0);
  SMPEG_enableaudio(Mpeg,1);
  }




audio_mp3_stream::~audio_mp3_stream() {
  SMPEG_stop(Mpeg);
  SMPEG_delete(Mpeg);
  }




string audio_mp3_stream::describe() {
  return "mp3 audio stream";
  }




void audio_mp3_stream::internalStartUp(audio_manager *mgr,audio_format &fmt) {
  SDL_AudioSpec spec;
  SMPEG_wantedSpec(Mpeg,&spec);
  fmt.getFromSpec(spec);
  BytesPerSample = fmt.getBytesPerSample();
  
  SMPEG_play(Mpeg);
  }




bool audio_mp3_stream::isEndOfStream() const {
  return IsEOF;
  }




TSize audio_mp3_stream::getStreamData(void *data,TSize maxsize) {
  maxsize /= BytesPerSample;
  maxsize *= BytesPerSample;
  
  if (maxsize != 0) {
    memset(data,0,maxsize);
    TSize result = SMPEG_playAudio(Mpeg,(Uint8 *) data,maxsize);
    if (result < maxsize) 
      IsEOF = true;
    return result;
    }
  else
    return 0;
  }
