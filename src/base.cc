// ----------------------------------------------------------------------------
//  Description      : SDL wrappers base
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#include <sdlucid_base.hh>




using namespace ixion;
using namespace sdl;




// Plain text rendering table -------------------------------------------------
#define TSDL_GENERAL      	"General SDL error"
#define TSDL_NODEVICE     	"Could not open device"
#define TSDL_RESOURCEINUSE	"Resource in use"
#define TSDL_CANNOTCONVERT	"Conversion impossible"
#define TSDL_FILEFORMAT		"File format error"
#define TSDL_NOTFOUND		"Item not found"
#define TSDL_SOUNDFORMAT	"Unsupported sound format"
#define TSDL_NOFRAMEBUFFER	"Framebuffer has disappeared"
#define TSDL_IOERROR		"I/O error"




static char *SDLPlainText[] = { 
  TSDL_GENERAL,TSDL_NODEVICE,TSDL_RESOURCEINUSE,TSDL_CANNOTCONVERT,
  TSDL_FILEFORMAT,TSDL_NOTFOUND,TSDL_SOUNDFORMAT,TSDL_NOFRAMEBUFFER,
  TSDL_IOERROR
  };




// sdl_exception --------------------------------------------------------------
char *sdl_exception::getText() const {
  return SDLPlainText[Error];
  }




// version query --------------------------------------------------------------
int sdlucidGetMajorVersion() {
  return SDLUCID_MAJOR_VERSION;
  }




int sdlucidGetMinorVersion() {
  return SDLUCID_MINOR_VERSION;
  }




int sdlucidGetMicroVersion() {
  return SDLUCID_MICRO_VERSION;
  }
