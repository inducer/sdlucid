// ----------------------------------------------------------------------------
//  Description      : WAM image class
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_numconv.hh>
#include "refpoint.hh"




using namespace sdl;




// bitmap_with_reference ---------------------------------------------------------------------
bitmap_with_reference::bitmap_with_reference()
  : ReferencePoint(0,0) {
  }




bitmap_with_reference::bitmap_with_reference(SDL_PixelFormat const &fmt,TSize width,TSize height,
Uint32 flags)
  : super(fmt,width,height,flags),ReferencePoint(0,0) {
  }





bitmap_with_reference::bitmap_with_reference(bitmap_with_reference const &src)
  : super(src),ReferencePoint(src.ReferencePoint) {
  }





coordinate_vector bitmap_with_reference::referencePoint() const {
  return ReferencePoint;
  }




void bitmap_with_reference::referencePoint(coordinate_vector const &refp) {
  ReferencePoint = refp;
  }




coordinate_rectangle bitmap_with_reference::extent() const {
  return super::extent() - ReferencePoint;
  }




void bitmap_with_reference::create(SDL_PixelFormat const &fmt,TSize width,TSize height,
Uint32 flags) {
  super::create(fmt,width,height,flags);
  ReferencePoint.set(0,0);
  }




void bitmap_with_reference::copyFrom(bitmap_with_reference const &src) {
  super::copyFrom(src);
  ReferencePoint = src.ReferencePoint;
  }




void bitmap_with_reference::convertFrom(bitmap_with_reference const &src,SDL_PixelFormat const &fmt,Uint32 flags) {
  super::convertFrom(src,fmt,flags);
  ReferencePoint = src.ReferencePoint;
  }




  
void bitmap_with_reference::convertForAcceleratedBlitFrom(bitmap_with_reference const &src) {
  super::convertForAcceleratedBlitFrom(src);
  ReferencePoint = src.ReferencePoint;
  }




void bitmap_with_reference::stretchFrom(bitmap_with_reference const &src,double stretch_x,double stretch_y) {
  super::stretchFrom(src,stretch_x,stretch_y);
  ReferencePoint[0] = TCoordinate(src.ReferencePoint[0] * stretch_x);
  ReferencePoint[1] = TCoordinate(src.ReferencePoint[1] * stretch_y);
  if (ReferencePoint[0] < 0) ReferencePoint[0] += width();
  if (ReferencePoint[1] < 0) ReferencePoint[1] += height();
  }




void bitmap_with_reference::transformFrom(bitmap_with_reference const &src,affine_transformation const &trans) {
  coordinate_vector origin_shift;
  super::transformFrom(src,trans,&origin_shift);
  double ref_x,ref_y;
  trans.transform(ref_x,ref_y,src.ReferencePoint[0],src.ReferencePoint[1]);
  ReferencePoint[0] = TCoordinate(ref_x)+origin_shift[0];
  ReferencePoint[1] = TCoordinate(ref_y)+origin_shift[1];
  }




void bitmap_with_reference::blit(drawable &dest,TCoordinate x,TCoordinate y) {
  super::blit(dest,x-ReferencePoint[0],y-ReferencePoint[1]);
  }




void bitmap_with_reference::blit(drawable &dest,TCoordinate x,TCoordinate y,
coordinate_rectangle const &source) {
  super::blit(dest,x-ReferencePoint[0],y-ReferencePoint[1]);
  }

