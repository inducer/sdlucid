// ----------------------------------------------------------------------------
//  Description      : SDL audio wrappers
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#include <cmath>
#include <iostream>
#include <algorithm>
#include <ixlib_numeric.hh>
#include <ixlib_numconv.hh>
#include <sdlucid_audio.hh>





using namespace std;
using namespace ixion;
using namespace sdl;




// garbage collector ----------------------------------------------------------
IXLIB_GARBAGE_DECLARE_MANAGER(audio_stream)




// #define SDLAUDIO_DEBUG
#define FIXED_PRECISION			10
#define FIXED_DIVSIOR			(1<<FIXED_PRECISION)




// tool functions -------------------------------------------------------------
namespace {
  class audio_locker {
    public:
      audio_locker() {
        SDL_LockAudio();
        }
      ~audio_locker() {
        SDL_UnlockAudio();
        }
    };




  struct conversion_data {
    TSampleValue 	Center;
    TSampleValue	Range;
    TSize		Shift;
    
    conversion_data(TSampleValue min,TSampleValue max);
    };
  
  
  template<class DestData>
  void outputConvert(DestData *dest,TSampleValue *src,TSize size,TSampleValue min,TSampleValue max) {
    conversion_data conv(min,max);
    
    while (size--) {
      TSampleValue value = (*src++ >> conv.Shift + conv.Center);
      value = NUM_MAX(NUM_MIN(max,value),min);
      *dest++ = (DestData) value;
      }
    }
  
  
  template<class SrcData>
  void mixInternal(TSampleValue *dest,SrcData *srcstart,SrcData *srcend,TSize destsamples,
  TSampleValue srcminval,TSampleValue srcmaxval,
  TSize srcchannels,TSize destchannels,float lvolume,float rvolume,float speed,
  void **srcpos,TSize *destcount) {
    conversion_data conv(srcminval,srcmaxval);
    
    TSampleValue lvol = (TSampleValue) (lvolume * (1 << FIXED_PRECISION));
    TSampleValue rvol = (TSampleValue) (rvolume * (1 << FIXED_PRECISION));
    TSampleValue vol = lvol;
    
    unsigned int position = 0; // << FIXED_PRECISION;
    unsigned int increment = (unsigned int) (speed * (1 << FIXED_PRECISION));
    SrcData *src = srcstart;
    
    TSize samples = destsamples;
    
    TSize shiftamount = conv.Shift-FIXED_PRECISION;
    
    if (srcchannels == 1) {
      if (destchannels == 1) {
        // mono --> mono
        while (src < srcend && samples--) {
	  TSampleValue value = (TSampleValue) (*src - conv.Center);
          *dest++ += (value * vol) << shiftamount;

	  position += increment;
	  src = srcstart + (position >> FIXED_PRECISION);
	  }
        }
      else {
        // mono --> stereo
        while (src < srcend && samples--) {
	  TSampleValue value = (TSampleValue) (*src - conv.Center);
          *dest++ += (value * lvol) << shiftamount;
          *dest++ += (value * rvol) << shiftamount;

	  position += increment;
	  src = srcstart + (position >> FIXED_PRECISION);
	  }
        }
      }
    else {
      if (destchannels == 1) {
        // stereo --> mono
        while (src < srcend && samples--) {
	  TSampleValue lvalue = (TSampleValue) (*src++ - conv.Center);
	  TSampleValue rvalue = (TSampleValue) (*src - conv.Center);
	  
          *dest++ += (lvalue * lvol + rvalue * rvol) << (shiftamount-1);

	  position += increment;
	  src = srcstart + 2*(position >> FIXED_PRECISION);
	  }
        }
      else {
        // stereo --> stereo
        while (src < srcend && samples--) {
	  TSampleValue lvalue = (TSampleValue) (*src++ - conv.Center);
	  TSampleValue rvalue = (TSampleValue) (*src - conv.Center);
          *dest++ += (lvalue * lvol) << shiftamount;
          *dest++ += (rvalue * rvol) << shiftamount;

	  position += increment;
	  src = srcstart + 2*(position >> FIXED_PRECISION);
	  }
        }
      }
    if (src>srcend) src = srcend;
    if (srcpos) *srcpos = src;
    if (samples+1 == 0) samples++;
    if (destcount) *destcount = destsamples-samples;
    }
  }




void sdl::mix(TSampleValue *dest,void *srcstart,void *srcend,TSize destsamples,
    TAudioLayout srclayout,TSize srcchannels,TSize destchannels,
    float lvolume,float rvolume,float speed,
    void **srcpos,TSize *destcount) {
  switch (srclayout) {
    case AUDIO_U8:
      mixInternal(dest,(TUnsigned8 *) srcstart,(TUnsigned8 *) srcend,destsamples,
        0,255,srcchannels,destchannels,lvolume,rvolume,speed,srcpos,destcount);
      break;
    case AUDIO_S8:
      mixInternal(dest,(TSigned8 *) srcstart,(TSigned8 *) srcend,destsamples,
        -128,127,srcchannels,destchannels,lvolume,rvolume,speed,srcpos,destcount);
      break;
    case AUDIO_U16:
      mixInternal(dest,(TUnsigned16 *) srcstart,(TUnsigned16 *) srcend,destsamples,
        0,65535,srcchannels,destchannels,lvolume,rvolume,speed,srcpos,destcount);
      break;
    case AUDIO_S16:
      mixInternal(dest,(TSigned16 *) srcstart,(TSigned16 *) srcend,destsamples,
        -32768,32767,srcchannels,destchannels,lvolume,rvolume,speed,srcpos,destcount);
      break;
    default:
      EXSDL_THROW(ECSDL_SOUNDFORMAT)
    }
  }




void sdl::panning2volumes(float vol,float pan,float &lvol,float &rvol) {
  if (NUM_ABS(pan) < 1e-2) {
    rvol = lvol = vol;
    return;
    }
  if (pan < 0) {
    lvol = vol;
    rvol = (1+pan)*vol;
    }
  else {
    rvol = vol;
    lvol = (1-pan)*vol;
    }
  }




void sdl::volumes2panning(float lvol,float rvol,float &vol,float &pan) {
  vol = NUM_MAX(lvol,rvol);
  if (lvol <= rvol)
    pan = 1-(lvol/vol);
  else
    pan = -(1-(rvol/vol));
  }




conversion_data::conversion_data(TSampleValue min,TSampleValue max) {
  Center = (max+min)/2;
  Range = max-min;
  TSampleValue temp = Range+1;
  Shift = SDLUCID_AUDIO_MIXBITS;

  while (temp >>= 1)
    Shift--;
  }




// audio_format ---------------------------------------------------------------
audio_format::audio_format(TSize freq,TSize channels,TAudioLayout layout) {
  set(freq,channels,layout);
  }




void audio_format::set(TSize freq,TSize channels,TAudioLayout layout) {
  Frequency = freq;
  Channels = channels;
  Layout = layout;
  }




void audio_format::getFromSpec(SDL_AudioSpec const &spec) {
  Frequency = spec.freq;
  Channels = spec.channels;
  Layout = spec.format;
  }




void audio_format::fillSpec(SDL_AudioSpec &spec) const {
  spec.freq = Frequency;
  spec.format = Layout;
  spec.channels = Channels;
  }




TSize audio_format::getBytesPerSampleValue() const {
  switch (Layout) {
    case AUDIO_U8:
    case AUDIO_S8:
      return 1;
      break;
    case AUDIO_S16MSB:
    case AUDIO_S16LSB:
    case AUDIO_U16MSB:
    case AUDIO_U16LSB:
      return 2;
      break;
    default:
      EXSDL_THROWINFO(ECSDL_GENERAL,"unknown audio format")
    }
  }



TSize audio_format::getBytesPerSample() const {
  return getBytesPerSampleValue()*Channels;
  }




// audio_data -----------------------------------------------------------------
audio_data::audio_data() 
  : Size(0) {
  Format.set();
  }




audio_data *audio_data::convertTo(audio_format const &fmt) {
  SDL_AudioCVT cvt;
  if (SDL_BuildAudioCVT(&cvt,Format.Layout,Format.Channels,Format.Frequency,
    fmt.Layout,fmt.Channels,fmt.Frequency))
    EXSDL_THROW(ECSDL_CANNOTCONVERT)
  
  
  auto_array<TByte> data(NUM_MAX(getByteSize(),getByteSize()*cvt.len_mult));
  memcpy(data.get(),Data.get(),getByteSize());
  cvt.len = getByteSize(),
  cvt.buf = data.get();
  
  SDL_ConvertAudio(&cvt);
  
  audio_data *copy = new audio_data();
  copy->Format = fmt;
  copy->Size = Size;
  copy->Data.allocate(fmt.getBytesPerSample()*Size);
  
  memcpy(copy->Data.get(),data.get(),fmt.getBytesPerSample()*Size);
  return copy;
  }




namespace {
  typedef char 	wFourCharCode[4];
  struct wWavHeader {
    wFourCharCode 	RIFF;
    Uint32		FileSize;
    wFourCharCode	WAVE;
    wFourCharCode	fmt_;
    Uint32		fmt_Size;
    Uint16		FormatTag;
    Uint16		Channels;
    Uint32		SampleRate,AvgBytesPerSec;
    Uint16		nBlockAlign,nBitsPerSample;
    wFourCharCode	data;
    Uint32		dataSize;	
    };
  }




void audio_data::loadWAV(istream &istr) {
  wWavHeader header;
  istr.read((char *) &header,sizeof(header));
  if (header.FormatTag != 1)
    EXSDL_THROWINFO(ECSDL_FILEFORMAT,"non-pcm files are not supported")
  if (strncmp(header.RIFF,"RIFF",4) || 
      strncmp(header.WAVE,"WAVE",4) ||
      strncmp(header.fmt_,"fmt ",4) ||
      strncmp(header.data,"data",4)) 
    EXSDL_THROWINFO(ECSDL_FILEFORMAT,"magic number mismatch")
  Format.set(header.SampleRate,header.Channels,
    header.nBitsPerSample == 8 ? AUDIO_U8 : AUDIO_S16);
  Size = header.dataSize / Format.getBytesPerSample();
  Data.allocate(Size*Format.getBytesPerSample());
  istr.read((char *) Data.get(),header.dataSize);
  }




// audio_manager --------------------------------------------------------------
namespace {
  void AudioCallback(void *userdata, Uint8 *stream, int len) {
    ((audio_manager *) userdata)->callback(stream,len);
    }
  }




audio_manager::audio_manager(TSize freq,TSize channels,TAudioLayout layout,TSize buffer) 
  : NextId(0),CallbackCalls(0) {
  SDL_AudioSpec desired;
  desired.freq = freq;
  desired.format = layout;
  desired.channels = channels;
  desired.samples = buffer;
  desired.callback = AudioCallback;
  desired.userdata = this;
  #ifndef SDLAUDIO_DEBUG
    if (SDL_OpenAudio(&desired,&InternalPlayFormat))
      EXSDL_THROWINFO(ECSDL_NODEVICE,SDL_GetError())
    PlayFormat.getFromSpec(InternalPlayFormat);
  #else
    PlayFormat.set(freq,channels,layout);
  #endif
  BufferSize = desired.samples;
  
  MixBuffer.allocate(BufferSize*PlayFormat.Channels);
  ConvertedMixBuffer.allocate(BufferSize * PlayFormat.getBytesPerSample());
  }




audio_manager::~audio_manager() {
  #ifndef SDLAUDIO_DEBUG
    SDL_CloseAudio();
  #endif
  }




audio_format const &audio_manager::getPlayFormat() const {
  return PlayFormat;
  }




TSize audio_manager::getMaxRequestLength() const {
  return BufferSize;
  }




ref<audio_stream> audio_manager::getStream(TStreamId id) const {
  return getStreamIterator(id)->Stream;
  }




audio_manager::TStreamId audio_manager::addStream(ref<audio_stream> strm) {
  audio_locker locker;
  strm->startUp(this);
  wStreamInfo info = { NextId++,strm };
  Streams.push_back(info);
  notify(notification_handler::STARTED,info.Id);
  return info.Id;
  }




void audio_manager::removeStream(TStreamId id) {
  audio_locker locker;
  Streams.erase(getStreamIterator(id));
  notify(notification_handler::REMOVED,id);
  }




bool audio_manager::isStreamActive(TStreamId id) const {
  try {
    getStreamIterator(id);
    return true;
    }
  catch (...) {
    return false;
    }
  }




void audio_manager::addNotificationHandler(notification_handler *handler) {
  NotificationHandlerList.push_back(handler);
  }




void audio_manager::removeNotificationHandler(notification_handler *handler) {
  notification_handler_list::iterator item = 
    find(NotificationHandlerList.begin(),NotificationHandlerList.end(),handler);
  if (item == NotificationHandlerList.end())
    EXGEN_THROWINFO(EC_ITEMNOTFOUND,"audio notification handler")
  NotificationHandlerList.erase(item);
  }




void audio_manager::tick() {
  cleanStreams();
  { audio_locker locker;
    FOREACH(first,Streams,stream_list)
      first->Stream->tick();
    }
  }




void audio_manager::pause() {
  SDL_PauseAudio(1);
  }




void audio_manager::play() {
  SDL_PauseAudio(0);
  }




void audio_manager::callback(void *stream,TSize len) {
  CallbackCalls++;
  
  TSize samples = len / PlayFormat.getBytesPerSample();
  TSize sample_values = samples * PlayFormat.Channels;
  memset(MixBuffer.get(),0,sizeof(TSampleValue)*sample_values);

  FOREACH(first,Streams,stream_list)
    first->Stream->mixTo(MixBuffer.get(),samples,PlayFormat.Frequency,PlayFormat.Channels);
  
  switch (PlayFormat.Layout) {
    case AUDIO_U8:
      outputConvert((Uint8 *) stream,MixBuffer.get(),sample_values,0,255);
      break;
    case AUDIO_S8:
      outputConvert((Sint8 *) stream,MixBuffer.get(),sample_values,-128,127);
      break;
    case AUDIO_U16:
      outputConvert((Uint16 *) stream,MixBuffer.get(),sample_values,0,65535);
      break;
    case AUDIO_S16:
      outputConvert((Sint16 *) stream,MixBuffer.get(),sample_values,-32768,32767);
      break;
    default: ;
      // error condition, cannot throw exception though
    }
  }




void audio_manager::notify(notification_handler::TNotificationType type,TStreamId id) const {
  FOREACH_CONST(first,NotificationHandlerList,notification_handler_list)
    (**first)(type,id);
  }




void audio_manager::cleanStreams() {
  audio_locker locker;
  
  stream_list::iterator first = Streams.begin(),last = Streams.end();
  while (first != last) {
    if (first->Stream->isDone()) {
      // we stand a chance that the notification handler has modified Streams 
      TStreamId id = first->Id;
      Streams.erase(first);
      
      notify(notification_handler::ENDED,id);
      
      first = Streams.begin();
      last = Streams.end();
      }
    else
      first++;
    }
  }




audio_manager::iterator audio_manager::getStreamIterator(TStreamId id) {
  FOREACH(first,Streams,stream_list)
    if (first->Id == id) return first;
  EXSDL_THROWINFO(ECSDL_NOTFOUND,"no such stream id")
  }




audio_manager::const_iterator audio_manager::getStreamIterator(TStreamId id) const {
  FOREACH_CONST(first,Streams,stream_list)
    if (first->Id == id) return first;
  EXSDL_THROWINFO(ECSDL_NOTFOUND,"no such stream id")
  }




// audio_stream ---------------------------------------------------------------
audio_stream::audio_stream(float vol,float panning) 
  : Manager(NULL) {
  panning2volumes(vol,panning,LVolume,RVolume);
  }




void audio_stream::setVolume(float vol,float panning) {
  panning2volumes(vol,panning,LVolume,RVolume);
  }




float audio_stream::getVolume() const {
  float vol,panning;
  volumes2panning(LVolume,RVolume,vol,panning);
  return vol;
  }




float audio_stream::getPanning() const {
  float vol,panning;
  volumes2panning(LVolume,RVolume,vol,panning);
  return panning;
  }




void audio_stream::startUp(audio_manager *mgr) {
  Manager = mgr;
  }




// audio_test_stream ----------------------------------------------------------
audio_test_stream::audio_test_stream(float sinefreq,float vol,float panning) 
  : super(vol,panning),Angle(0),SineFrequency(sinefreq) {
  }




string audio_test_stream::describe() {
  return "audio test stream ("+float2dec(SineFrequency,1)+")";
  }




void audio_test_stream::tick() {
  }




void audio_test_stream::mixTo(TSampleValue *data,TSize samples,TSize frequency,TSize channels) {
  while (samples--) {
    float value = sin(Angle)*SDLUCID_AUDIO_SV_MAXIMUM;
    float lvalue = value * LVolume;
    float rvalue = value * RVolume;
    
    *data++ += (TSampleValue) (lvalue);
    if (channels>=2) *data++ += (TSampleValue) (rvalue);

    Angle += 3.1415/frequency*SineFrequency;
    }
  }




// audio_data_stream ----------------------------------------------------------
audio_data_stream::audio_data_stream(audio_data &data,TSize repeatcount,float delay,float vol,float panning) 
  : super(vol,panning),Data(&data),OwnData(false),Position(Data->get()),
  RepeatCount(repeatcount),Delay(delay),Format(Data->getFormat()) {
  if (RepeatCount != SDLUCID_AUDIO_REPEAT_INF) RepeatCount--;
  }




audio_data_stream::audio_data_stream(audio_data *data,TSize repeatcount,float delay,float vol,float panning) 
  : super(vol,panning),Data(data),OwnData(true),Position(Data->get()),
  RepeatCount(repeatcount),Delay(delay),Format(Data->getFormat()) {
  if (RepeatCount != SDLUCID_AUDIO_REPEAT_INF) RepeatCount--;
  }




audio_data_stream::~audio_data_stream() {
  if (OwnData) delete Data;
  }




string audio_data_stream::describe() {
  return "sample data stream";
  }




void audio_data_stream::startUp(audio_manager *mgr) {
  super::startUp(mgr);
  DelaySamples = (TSize) (Delay * Manager->getPlayFormat().Frequency);
  }




void audio_data_stream::tick() {
  }




void audio_data_stream::mixTo(TSampleValue *data,TSize samples,TSize frequency,TSize channels) {
  if (DelaySamples > 0) {
    if (DelaySamples > samples) {
      DelaySamples -= samples;
      return;
      }
    data += DelaySamples;
    samples -= DelaySamples;
    }
  
  mix(data,(void *) Position,(void *) (Data->get()+Data->getByteSize()),
    samples,Format.Layout,Format.Channels,channels,LVolume,RVolume,
    (float) Format.Frequency/frequency,(void **) &Position);
  
  if (RepeatCount && Position == Data->get()+Data->getByteSize()) {
    if (RepeatCount != SDLUCID_AUDIO_REPEAT_INF) RepeatCount--;
    Position = Data->get();
    }
  }




bool audio_data_stream::isDone() const {
  return (Data->get()+Data->getByteSize() == Position) && (RepeatCount == 0);
  }




// audio_buffered_stream ------------------------------------------------------
audio_buffered_stream::audio_buffered_stream(float vol,float panning)
  : super(vol,panning) {
  }




void audio_buffered_stream::startUp(audio_manager *mgr) {
  super::startUp(mgr);

  internalStartUp(mgr,Format);
  BytesPerSample = Format.getBytesPerSample();
  
  audio_format const &playfmt = Manager->getPlayFormat();
  Speed = Format.Frequency/playfmt.Frequency;

  SoundBuffer.allocate(32*Manager->getMaxRequestLength()*BytesPerSample);
  }




void audio_buffered_stream::tick() {
  TSize read_size;
  do {
    TSize canread_size;
    TByte *buffer = SoundBuffer.pushPointer(canread_size);
    read_size = getStreamData(buffer,canread_size);
    SoundBuffer.pushPointerCommit(read_size);
  } while (read_size);
  }




void audio_buffered_stream::mixTo(TSampleValue *data,TSize samples,TSize frequency,TSize channels) {
  TSize canmix;
  
  do {
    TByte *buffer = SoundBuffer.popPointer(canmix);
    
    // must be aligned on sample boundary
    canmix /= BytesPerSample;
    canmix *= BytesPerSample;
    
    if (canmix) {
      void *up_to;
      TSize destmixed;
      mix(data,buffer,buffer+canmix,samples,
	Format.Layout,Format.Channels,channels,LVolume,RVolume,Speed,&up_to,&destmixed);
	
      data += destmixed*channels;
      samples -= destmixed;
      
      SoundBuffer.popPointerCommit((TByte *) up_to-buffer);
      }
  } while (samples && canmix);
  }




bool audio_buffered_stream::isDone() const {
  return isEndOfStream() && SoundBuffer.size() == 0;
  }
