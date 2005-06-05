// ----------------------------------------------------------------------------
//  Description      : SDL audio wrappers
// ----------------------------------------------------------------------------
//  Remarks          : audio_stream::mixTo is called within the audio thread.
//    Things requiring I/O and similar things had better go into 
//    audio_stream::tick(). Its execution is automatically protected by
//    the audio lock.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_AUDIO
#define SDLUCID_AUDIO




#include <vector>
#include <queue>
#include <list>
#include <SDL.h>
#include <ixlib_array.hh>
#include <ixlib_garbage.hh>
#include <ixlib_string.hh>
#include <ixlib_ring_queue.hh>
#include <sdlucid_base.hh>




namespace sdl {
  using ixion::TSize;
  using ixion::TByte;
  
  // types and constants ------------------------------------------------------
  typedef TSize           	TAudioFrequency;
  typedef Uint16          	TAudioLayout;
  typedef Sint32          	TSampleValue;
  
  
  
  
  #define SDLUCID_AUDIO_MIXBITS			28
  #define SDLUCID_AUDIO_SV_MAXIMUM		(1 << SDLUCID_AUDIO_MIXBITS - 1)
  #define SDLUCID_AUDIO_REPEAT_INF		-1u



  // tool functions -----------------------------------------------------------
  void mix(TSampleValue *dest,void *srcstart,void *srcend,TSize destsamples,
    TAudioLayout srclayout,TSize srcchannels = 1,TSize destchannels = 1,
    float lvolume = 1,float rvolume = 1,float speed = 1,
    void **srcpos = NULL,TSize *destcount = NULL);
  void panning2volumes(float vol,float pan,float &lvol,float &rvol);
  void volumes2panning(float lvol,float rvol,float &vol,float &pan);




  struct audio_format {
    TAudioFrequency	Frequency;
    TSize		Channels;
    TAudioLayout	Layout;
    
    audio_format(ixion::TSize freq = 22050,TSize channels = 2,TAudioLayout layout = AUDIO_S16);
    void set(ixion::TSize freq = 22050,TSize channels = 2,TAudioLayout layout = AUDIO_S16);
    void getFromSpec(SDL_AudioSpec const &spec);
    void fillSpec(SDL_AudioSpec &spec) const;
    TSize getBytesPerSampleValue() const;
    TSize getBytesPerSample() const;
    };
  
  
  
  
  class audio_data {
      ixion::auto_array<TByte>		Data;
      audio_format			Format;
      TSize				Size; // samples
  
    public:
      audio_data();
      TSize getSampleCount() const {
        return Size;
        }
      audio_format const &getFormat() const {
        return Format;
        }
      TByte const *get() const {
        return Data.get();
        }
      
      audio_data *convertTo(audio_format const &fmt);
      void loadWAV(std::istream &istr);
      TSize getByteSize() const {
        return Size * Format.getBytesPerSample();
        }
      float getDuration() const {
        return Size / (float) Format.Frequency;
        }
    };
  
  
  
  
  class audio_stream;
  class audio_manager {
    protected:
    public:
      typedef unsigned			TStreamId;
      struct wStreamInfo {
        TStreamId			Id;
        ixion::ref<audio_stream>	Stream;
        };
      
      struct notification_handler {
        enum TNotificationType { STARTED,ENDED,REMOVED };
        virtual ~notification_handler() {
          }
        virtual void operator()(TNotificationType type,TStreamId id) = 0;
        };
    
    protected:
      // mandatory-lock data
      typedef std::vector<wStreamInfo>		stream_list;
      stream_list				Streams;    
      TStreamId					NextId;
  
      // only accessed in callback
      ixion::auto_array<TSampleValue>		MixBuffer;
      ixion::auto_array<TByte>			ConvertedMixBuffer;
      TSize					CallbackCalls;
      
      // read-only
      TSize					BufferSize; // samples
      SDL_AudioSpec				InternalPlayFormat;
      audio_format				PlayFormat;
  
      typedef std::vector<notification_handler *> notification_handler_list;
      notification_handler_list			NotificationHandlerList;
      
    public:
      typedef stream_list::iterator		iterator;
      typedef stream_list::const_iterator	const_iterator;
      
      audio_manager(TSize freq = 22050,TSize channels = 2,TAudioLayout layout = AUDIO_S16,
        TSize buffer = 1024);
      ~audio_manager();
      
      iterator begin() {
        return Streams.begin();
        }
      const_iterator begin() const {
        return Streams.begin();
        }
      iterator end() {
        return Streams.end();
        }
      const_iterator end() const {
        return Streams.end();
        }
      TSize size() const {
        return Streams.size();
        }
      
      TSize countCallbacks() const {
        return CallbackCalls;
        }
      
      audio_format const &getPlayFormat() const;
      // advisory: how many samples one mixTo will maximally request
      TSize getMaxRequestLength() const;
      
      ixion::ref<audio_stream> getStream(TStreamId id) const;
      TStreamId addStream(ixion::ref<audio_stream> strm);
      void removeStream(TStreamId id);
      bool isStreamActive(TStreamId id) const;
      void addNotificationHandler(notification_handler *handler);
      void removeNotificationHandler(notification_handler *handler);
      
      void tick();
      
      void pause();
      void play();
      
      // private:
      void callback(void *stream,TSize len);
  
    private:
      void notify(notification_handler::TNotificationType type,TStreamId id) const;
      void cleanStreams();
      iterator getStreamIterator(TStreamId id);
      const_iterator getStreamIterator(TStreamId id) const;
    };
  
  
  
  
  class audio_stream {
    protected:
      audio_manager 	*Manager;
      float               LVolume,RVolume;
  
    public:
      audio_stream(float vol = 1,float panning = 0);
      virtual ~audio_stream() {
        }
      virtual std::string describe() = 0;
      
      void setVolume(float vol = 1,float panning = 0);
      float getVolume() const;
      float getPanning() const;
      
    protected:
      virtual void startUp(audio_manager *mgr);
      virtual void tick() = 0;
      virtual void mixTo(TSampleValue *data,TSize samples,TSize frequency,TSize channels) = 0;
      virtual bool isDone() const = 0;
    
    friend class audio_manager;
    };
  
  
  
  
  class audio_test_stream : public audio_stream {
    private:
      typedef audio_stream	super;
      float               	Angle;
      float               	SineFrequency;
  
    public:
      audio_test_stream(float sinefreq = 440,float vol = 1,float panning = 0);
      std::string describe();
      void tick();
      void mixTo(TSampleValue *data,TSize samples,TSize frequency,TSize channels);
      bool isDone() const {
        return false;
        }
    };
  
  
  
  
  class audio_data_stream : public audio_stream {
      typedef audio_stream		super;
      audio_data			*Data;
      bool				OwnData;
      TByte const			*Position;
      TSize				RepeatCount;
      float				Delay;
      TSize				DelaySamples;
      audio_format const		&Format;
  
    public:
      /// Construct audio_data_stream, do not assume ownership of data
      audio_data_stream(audio_data &data,TSize repeatcount = 1,float delay = 0,float vol = 1,float panning = 0);
      /// Construct audio_data_stream, assume ownership of data
      audio_data_stream(audio_data *data,TSize repeatcount = 1,float delay = 0,float vol = 1,float panning = 0);
      ~audio_data_stream();
      std::string describe();
      void startUp(audio_manager *mgr);
      void tick();
      void mixTo(TSampleValue *data,TSize samples,TSize frequency,TSize channels);
      bool isDone() const;
      
      audio_data const &getData() const {
        return *Data;
        }
    }; 
  
  
  
  
  class audio_buffered_stream : public audio_stream {
    private:
      typedef audio_stream	super;
      float               	Speed;
      ixion::ring_queue<TByte>	SoundBuffer;
      audio_format		Format;
      TSize			BytesPerSample;
      
    public:
      audio_buffered_stream(float vol = 1,float panning = 0);
      void startUp(audio_manager *mgr);
      void tick();
      void mixTo(TSampleValue *data,TSize samples,TSize frequency,TSize channels);
      bool isDone() const;
  
    protected:
      virtual void internalStartUp(audio_manager *mgr,audio_format &fmt) = 0;
      virtual bool isEndOfStream() const = 0;
      virtual TSize getStreamData(void *data,TSize maxsize) = 0;
    };
  }
  
  
  
#endif
