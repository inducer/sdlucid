// ----------------------------------------------------------------------------
//  Description      : SDL mod player
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------





#include <iostream>
#include <ixlib_numconv.hh>
#include <sdlucid_mikmod_stream.hh>




#ifndef SDLUCID_HAS_LIBMIKMOD
#error "this module shouldn't even be compiled if we do not have mikmod"
#endif



using namespace std;
using namespace ixion;
using namespace sdl;




namespace {
  struct mikmod_initializer {
    mikmod_initializer() {
      MikMod_RegisterDriver(&drv_nos);
      MikMod_RegisterAllLoaders();
    
      md_mode = DMODE_SOFT_MUSIC | DMODE_16BITS | DMODE_STEREO | DMODE_INTERP;
      md_mixfreq = 44100;
      
      MikMod_Init("");
      md_volume = 128;
      md_musicvolume = 128;
      md_reverb = 0;
      }
    ~mikmod_initializer() {
      MikMod_Exit();
      }
    } Initializer;

  struct stream_MREADER : public MREADER {
    audio_mod_stream		&Stream;
    istream			&MOD;
    
    stream_MREADER(audio_mod_stream &stream,istream &mod);
    };
  




  BOOL wmrSeek(MREADER *mr,long pos,int whence) {
    stream_MREADER &wmr = *static_cast<stream_MREADER *>(mr);
    if (whence == SEEK_SET)
      wmr.MOD.seekg(pos,ios::beg);
    if (whence == SEEK_CUR)
      wmr.MOD.seekg(pos,ios::cur);
    if (whence == SEEK_END)
      wmr.MOD.seekg(pos,ios::end);
    return true;
    }




  long wmrTell(MREADER *mr) {
    stream_MREADER &wmr = *static_cast<stream_MREADER *>(mr);
    return wmr.MOD.tellg();
    }




  BOOL wmrRead(MREADER *mr,void *data,size_t size) {
    stream_MREADER &wmr = *static_cast<stream_MREADER *>(mr);
    streampos before = wmr.MOD.tellg();
    wmr.MOD.read((char *) data,size);
    streampos after = wmr.MOD.tellg();
    if (wmr.MOD.eof()) return EOF;
    else return before-after;
    }




  int wmrGet(MREADER *mr) {
    stream_MREADER &wmr = *static_cast<stream_MREADER *>(mr);
    return wmr.MOD.get();
    }




  BOOL wmrEOF(MREADER *mr) {
    stream_MREADER &wmr = *static_cast<stream_MREADER *>(mr);
    return wmr.MOD.eof();
    }
  }




stream_MREADER::stream_MREADER(audio_mod_stream &stream,istream &mod)
  : Stream(stream),MOD(mod) {
  Seek = wmrSeek;
  Tell = wmrTell;
  Read = wmrRead;
  Get = wmrGet;
  Eof = wmrEOF;
  }




// audio_mod_stream ------------------------------------------------------------
audio_mod_stream::audio_mod_stream(istream &strm,float vol,float panning) 
  : super(vol,panning) {
  stream_MREADER wmr(*this,strm);
  Module = Player_LoadGeneric(&wmr,64,0);
  if (Module == NULL) {
    EXSDL_THROWINFO(ECSDL_IOERROR,MikMod_strerror(MikMod_errno))
    }
  }




audio_mod_stream::~audio_mod_stream() {
  if (Player_Active()) Player_Stop();
  Player_Free(Module);
  }




string audio_mod_stream::describe() {
  return Module->songname+string(" ")+unsigned2dec(Module->sngpos)+"/"+unsigned2dec(Module->numpos);
  }




void audio_mod_stream::internalStartUp(audio_manager *mgr,audio_format &fmt) {
  fmt.set(44100,2,AUDIO_S16);
  BytesPerSample = fmt.getBytesPerSample();
  Player_Start(Module);
  }




bool audio_mod_stream::isEndOfStream() const {
  return Module->sngpos >= Module->numpos;
  }




TSize audio_mod_stream::getStreamData(void *data,TSize maxsize) {
  if (isEndOfStream()) 
    return 0;
  maxsize /= BytesPerSample;
  maxsize *= BytesPerSample;
  if (maxsize != 0)
    VC_WriteBytes((SBYTE *) data,maxsize);
  return maxsize;
  }
