// ----------------------------------------------------------------------------
//  Description      : SDL wrappers base
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_BASE
#define SDLUCID_BASE




#undef PACKAGE
#undef VERSION
#include <sdlucid_config.hh>
#undef PACKAGE
#undef VERSION
#include <ixlib_exbase.hh>




// SDL error codes ------------------------------------------------------------
#define ECSDL_GENERAL		0
#define ECSDL_NODEVICE		1
#define ECSDL_RESOURCEINUSE	2
#define ECSDL_CANNOTCONVERT	3
#define ECSDL_FILEFORMAT	4
#define ECSDL_NOTFOUND		5
#define ECSDL_SOUNDFORMAT	6
#define ECSDL_NOFRAMEBUFFER	7
#define ECSDL_IOERROR		8




// Throw macro ----------------------------------------------------------------
#define EXSDL_THROW(CODE)\
  throw sdl::sdl_exception(CODE,NULL,__FILE__,__LINE__);
#define EXSDL_THROWINFO(CODE,INFO)\
  throw sdl::sdl_exception(CODE,(char const *) INFO,__FILE__,__LINE__);




// sdl_exception --------------------------------------------------------------
#ifdef __cplusplus
namespace sdl {
  struct sdl_exception : public ixion::base_exception {
    sdl_exception(ixion::TErrorCode error, char const *info = NULL, const char *module = NULL,
      ixion::TIndex line = 0)
      : base_exception(error,info,module,line,"SDL") {
      }
    virtual const char *getText() const;
    };
  }
#endif //__cplusplus




// version query interface ----------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
  int sdlucidGetMajorVersion();
  int sdlucidGetMinorVersion();
  int sdlucidGetMicroVersion();
#ifdef __cplusplus
}
#endif // __cplusplus





#endif
