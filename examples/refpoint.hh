// ----------------------------------------------------------------------------
//  Description      : Bitmap with reference point
// ----------------------------------------------------------------------------
//  Remarks          :
//    The reference point only comes into play when a bitmap is placed
//    at a certain position in another one. (and even then, only the
//    reference point of the placed-in bitmap is used)
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Andreas Kloeckner
// ----------------------------------------------------------------------------
// I am not quite sure about this interface, so it's not yet included in the 
// main library. Especially loading and saving would require nonstandard
// PNG metadata entries...




#ifndef SDLUCID_BITMAP_WITH_REFERENCE
#define SDLUCID_BITMAP_WITH_REFERENCE




#include <sdlucid_video.hh>




namespace sdl {
  class bitmap_with_reference : public bitmap {
      typedef bitmap		super;
      sdl::coordinate_vector	ReferencePoint;
    
    public:
      bitmap_with_reference();
      bitmap_with_reference(SDL_PixelFormat const &fmt,TSize width,TSize height,
        Uint32 flags = SDL_SWSURFACE);
      bitmap_with_reference(bitmap_with_reference const &src);
  
      coordinate_vector referencePoint() const;
      void referencePoint(coordinate_vector const &refp);
      coordinate_rectangle extent() const;
      
      void create(SDL_PixelFormat const &fmt,TSize width,TSize height,
        Uint32 flags = SDL_SWSURFACE);
      void copyFrom(bitmap_with_reference const &src);
      void convertFrom(bitmap_with_reference const &src,SDL_PixelFormat const &fmt,Uint32 flags);
      void convertForAcceleratedBlitFrom(bitmap_with_reference const &src);
      void stretchFrom(bitmap_with_reference const &src,double stretch_x,double stretch_y);
      void transformFrom(bitmap_with_reference const &src,affine_transformation const &trans);
  
      void blit(drawable &dest,TCoordinate x,TCoordinate y);
      void blit(drawable &dest,TCoordinate x,TCoordinate y,
        coordinate_rectangle const &source);
    };
  }




#endif
