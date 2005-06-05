// ----------------------------------------------------------------------------
//  Description      : SDL c++ stream to rwops wrapper
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------





#include <ixlib_exgen.hh>
#include <sdlucid_base.hh>
#include <sdlucid_iostream_rwops.hh>




using namespace std;




namespace {
  struct stream_rwops {
    istream 	*IStream;
    bool	AssumeOwnership;
    SDL_RWops	RWops;
    
    stream_rwops(istream *istr,bool assume_ownership);
    ~stream_rwops();
    };

  // stream interface functions -------------------------------------------------
  int RWops_seek(SDL_RWops *context, int offset, int whence) {
    stream_rwops *base = (stream_rwops *) context->hidden.unknown.data1;
    
    if (whence == SEEK_SET)
      base->IStream->seekg(offset,ios::beg);
    if (whence == SEEK_CUR)
      base->IStream->seekg(offset,ios::cur);
    if (whence == SEEK_END)
      base->IStream->seekg(offset,ios::end);
    return base->IStream->tellg();
    }
  
  
  
  
  int RWops_read(SDL_RWops *context, void *ptr, int size, int maxnum) {
    stream_rwops *base = (stream_rwops *) context->hidden.unknown.data1;
    
    streampos before = base->IStream->tellg();
    base->IStream->read((char *) ptr,size*maxnum);
    streampos after = base->IStream->tellg();
    if (base->IStream->bad()) return -1;
    else return (after-before)/size;
    }
  
  
  
  
  int RWops_write(SDL_RWops *context, const void *ptr, int size, int num) {
    // *** FIXME unimplemented
    return -1;
    }
  
  
  
  
  int RWops_close(SDL_RWops *context) {
    stream_rwops *base = (stream_rwops *) context->hidden.unknown.data1;
    delete base;
    return 0;
    }
  }




// stream_rwops ------------------------------------------------------------
stream_rwops::stream_rwops(istream *istr,bool assume_ownership)
  : IStream(istr),AssumeOwnership(assume_ownership) {
  RWops.hidden.unknown.data1 = this;
  RWops.seek = RWops_seek;
  RWops.read = RWops_read;
  RWops.write = RWops_write;
  RWops.close = RWops_close;
  }




stream_rwops::~stream_rwops() {
  if (AssumeOwnership)
    delete IStream;
  }




// public interface -----------------------------------------------------------
SDL_RWops *sdl::RWopsFromStream(istream *istr,bool assume_ownership) {
  stream_rwops *rwops = new stream_rwops(istr,assume_ownership);
  return &rwops->RWops;
  }
