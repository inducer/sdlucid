// ----------------------------------------------------------------------------
//  Description      : SDL video wrappers
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#include <string>
#include <cmath>
#include <ixlib_array.hh>
#include <ixlib_numconv.hh>
#include <ixlib_drawing_functions.hh>
#include <ixlib_geometry_impl.hh>
#include <ixlib_polygon_impl.hh>
#include <sdlucid_video.hh>
#include <sdlucid_png.hh>




using namespace std;
using namespace ixion;
using namespace sdl;
using namespace png;




// keywords for XML font description file -------------------------------------
#define XML_FONT_TAG		"font"
#define XML_FONT_NAME		"name"
#define XML_FONT_TOPLINE	"topline"
#define XML_FONT_BOTTOMLINE	"bottomline"
#define XML_FONT_COLORKEY       "colorkey"
#define XML_GLYPH_TAG		"glyph"
#define XML_GLYPH_CODE		"code"
#define XML_GLYPH_BASE_X	"base_x"
#define XML_GLYPH_BASE_Y	"base_y"
#define XML_GLYPH_WIDTH		"width"
#define XML_GLYPH_BOUND_X	"bound_x"
#define XML_GLYPH_BOUND_Y	"bound_y"
#define XML_GLYPH_BOUND_W	"bound_w"
#define XML_GLYPH_BOUND_H	"bound_h"




// Stretch helpers ------------------------------------------------------------
signed long roundToZero(double x) {
  if (x >= 0)
    return (signed long) floor(x);
  else
    return -(signed long) floor(-x);
  }




// Mask creation functions ----------------------------------------------------
static inline Uint32 getMask(TSize bits,TSize offset) {
  return (((Uint32) 1 << bits) - 1) << offset;
  }
  
  
  
  
// affine_transformation ------------------------------------------------------
void affine_transformation::identity() {
  Matrix[0][0] = 1; Matrix[0][1] = 0;
  Matrix[1][0] = 0; Matrix[1][1] = 1;
  Translation[0] = 0;
  Translation[1] = 0;
  }




void affine_transformation::translate(double x,double y) {
  Translation[0] += x;
  Translation[1] += y;
  }




void affine_transformation::scale(double x,double y) {
  Matrix[0][0] *= x;
  Matrix[0][1] *= x;
  Matrix[1][0] *= y;
  Matrix[1][1] *= y;
  Translation[0] *= x;
  Translation[1] *= y;
  }




void affine_transformation::rotate(double rad) {
  affine_transformation rot;
  rot.identity();
  rot.Matrix[0][0] = cos(rad); rot.Matrix[0][1] = sin(rad);
  rot.Matrix[1][0] = -sin(rad); rot.Matrix[1][1] = cos(rad);
  
  rot.transform(Matrix[0][0],Matrix[1][0],Matrix[0][0],Matrix[1][0]);
  rot.transform(Matrix[0][1],Matrix[1][1],Matrix[0][1],Matrix[1][1]);
  
  rot.transform(Translation[0],Translation[1],Translation[0],Translation[1]);
  }




void affine_transformation::invert() {
  double det_inv = 1/(Matrix[0][0]*Matrix[1][1] - Matrix[0][1]*Matrix[1][0]);
  
  affine_transformation tx;
  tx.Matrix[0][0] = det_inv * Matrix[1][1];   
  tx.Matrix[1][1] = det_inv * Matrix[0][0];

  tx.Matrix[1][0] = -det_inv * Matrix[1][0];   
  tx.Matrix[0][1] = -det_inv * Matrix[0][1];
  
  tx.transformLinear(Translation[0],Translation[1],-Translation[0],-Translation[1]);
  memcpy( &Matrix, &tx.Matrix, sizeof( Matrix ) );
  }




void affine_transformation::transform(double &dest_x,double &dest_y,double x,double y) const {
  double temp_x = Matrix[0][0]*x + Matrix[0][1]*y + Translation[0];
  dest_y = Matrix[1][0]*x + Matrix[1][1]*y + Translation[1];
  dest_x = temp_x;
  }




void affine_transformation::transformLinear(double &dest_x,double &dest_y,double x,double y) const {
  double temp_x = Matrix[0][0]*x + Matrix[0][1]*y;
  dest_y = Matrix[1][0]*x + Matrix[1][1]*y;
  dest_x = temp_x;
  }




// surface_locker ------------------------------------------------------------
namespace {
  class surface_locker {
    protected:
      SDL_Surface		*Surface;
    
    public:
      surface_locker(SDL_Surface *dwbl);
      ~surface_locker();
    };




  surface_locker::surface_locker(SDL_Surface *surf)
    : Surface(surf) {
    if (SDL_LockSurface(Surface))
      EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
    }
  
  
  
  
  
  surface_locker::~surface_locker() {
    SDL_UnlockSurface(Surface);
    }
  }
  
  
  
  
// pixel format routines ----------------------------------------------------
void sdl::createPixelFormat(SDL_PixelFormat &fmt,TSize bitdepth,TSize channels,
TColor colorkey,float surfalpha,SDL_Palette *pal) {
  bool havealpha = channels >= 4;
  
  fmt.palette = pal;
  fmt.BitsPerPixel = bitdepth*channels;
  fmt.BytesPerPixel = (fmt.BitsPerPixel+7)/8;
  
  fmt.Rloss = 8-bitdepth;
  fmt.Gloss = 8-bitdepth;
  fmt.Bloss = 8-bitdepth;
  fmt.Aloss = havealpha ? 8-bitdepth : 8;
  fmt.Rshift = 0;
  fmt.Gshift = bitdepth;
  fmt.Bshift = 2*bitdepth;
  fmt.Ashift = havealpha ? 3*bitdepth : 0;
  fmt.Rmask = getMask(bitdepth,0);
  fmt.Gmask = getMask(bitdepth,bitdepth);
  fmt.Bmask = getMask(bitdepth,2*bitdepth);
  fmt.Amask = havealpha ? getMask(bitdepth,3*bitdepth) : 0;
  
  fmt.colorkey = colorkey;
  fmt.alpha = (Uint8) (255 * surfalpha);
  }
  



TColor sdl::mapColor(SDL_PixelFormat const &fmt,Uint8 r,Uint8 g,Uint8 b,Uint8 a) {
  TColor pixel;

  if (fmt.palette == NULL)
    pixel = 
      ((r >> fmt.Rloss) << fmt.Rshift) |
      ((g >> fmt.Gloss) << fmt.Gshift) |
      ((b >> fmt.Bloss) << fmt.Bshift) |
      ((a >> fmt.Aloss) << fmt.Ashift);
  else 
    pixel = SDL_MapRGB(const_cast<SDL_PixelFormat *>(&fmt),r,g,b);

  return pixel;
  }




void sdl::unmapColor(SDL_PixelFormat const &fmt,TColor color,Uint8 &r,Uint8 &g,Uint8 &b,Uint8 &a) {
  if (fmt.palette == NULL) {
    r = ((color & fmt.Rmask) >> fmt.Rshift) << fmt.Rloss;
    g = ((color & fmt.Gmask) >> fmt.Gshift) << fmt.Gloss;
    b = ((color & fmt.Bmask) >> fmt.Bshift) << fmt.Bloss;
    a = ((color & fmt.Amask) >> fmt.Ashift) << fmt.Aloss;
    }
  else {
    if (color >= TSize(fmt.palette->ncolors)) 
      EXGEN_THROWINFO(EC_INDEX,"Palette index out of range")
    
    SDL_Color &palette_entry = fmt.palette->colors[color];
    r = palette_entry.r;
    g = palette_entry.g;
    b = palette_entry.b;
    a = 0;
    }
  }





// drawable -------------------------------------------------------------------
drawable::drawable()
  : DrawMode(COLOR),DrawColor(0) {
  }




SDL_Surface *drawable::surface() const {
  return Surface;
  }




SDL_PixelFormat const &drawable::format() const {
  return *(Surface->format);
  }




TSize drawable::height() const {
  if ( Surface ) 
    return Surface->h;
  else
    return 0;
  }




TSize drawable::width() const {
  if ( Surface )
    return Surface->w;
  else
    return 0;
  }




Uint32 drawable::flags() const {
  return Surface->flags;
  }




coordinate_rectangle drawable::extent() const {
  return coordinate_rectangle(0,0,width(),height());
  }




coordinate_rectangle drawable::clipping() {
  SDL_Rect &cr = Surface->clip_rect;
  return coordinate_rectangle(cr.x,cr.y,cr.x+cr.w,cr.y+cr.h);
  }




void drawable::clipping(coordinate_rectangle const &rect) {
  SDL_Rect cr;
  cr.x = rect.A[0];
  cr.y = rect.A[1];
  cr.w = rect.width();
  cr.h = rect.height();
  if ( Surface )
    SDL_SetClipRect(Surface,&cr);
  }




void drawable::clearClipping() {
  if ( Surface )
    SDL_SetClipRect(Surface,NULL);
  }




void drawable::lock()
{
  if ( Surface )
    if ( SDL_LockSurface( Surface ) )
      EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError());
}




void drawable::unlock()
{
  if ( Surface )
    SDL_UnlockSurface( Surface );
}




// low-level-------------------------------------------------------------------
void drawable::drawingColor(TColor color) {
  DrawMode = COLOR;
  DrawColor = color;
  }




void drawable::drawingTile(bitmap const *tile,TCoordinate centerx,TCoordinate centery) {
  DrawMode = TILE;
  DrawTile = tile;
  DrawCenterX = centerx;
  DrawCenterY = centery;
  }




void drawable::drawingPixel(bitmap const *pixel,TCoordinate centerx,TCoordinate centery) {
  DrawMode = IMAGE;
  DrawPixel = pixel;
  DrawCenterX = centerx;
  DrawCenterY = centery;
  }




TColor drawable::getPixel(TCoordinate x,TCoordinate y) {
  TColor mask = Surface->format->Rmask | Surface->format->Gmask | Surface->format->Bmask | Surface->format->Amask;
  surface_locker lock(Surface);
  
  TColor *pixel = (TColor *) 
    ((TByte *) Surface->pixels+Surface->pitch*y+Surface->format->BytesPerPixel*x);
  return *pixel & mask;
  }
  
  


void drawable::setPixel(TCoordinate x,TCoordinate y) {
  switch (DrawMode) {
    case COLOR: {
      SDL_Rect dest_rect = { x,y,1,1 };
      if (SDL_FillRect(Surface,&dest_rect,DrawColor))
        EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
      break;
      }
    case TILE: {
      SDL_Rect src_rect = { x + DrawCenterX,y + DrawCenterY,1,1 };
      SDL_Rect dest_rect = { x,y,1,1 };
      while (src_rect.x < 0) src_rect.x += DrawTile->width();
      while (src_rect.y < 0) src_rect.y += DrawTile->height();
      src_rect.x %= DrawTile->width();
      src_rect.y %= DrawTile->height();
      if (SDL_BlitSurface(DrawTile->Surface,&src_rect,Surface,&dest_rect))
        EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
      break;
      }
    case IMAGE: {
      SDL_Rect dest_rect = { x-DrawCenterX,y-DrawCenterY,
        DrawPixel->width(),DrawPixel->height() };
      if (SDL_BlitSurface(DrawPixel->Surface,NULL,Surface,&dest_rect))
        EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
      break;
      }
    }
  }




void drawable::drawHLine(TCoordinate x1,TCoordinate y,TCoordinate x2) {
  switch (DrawMode) {
    case COLOR: {
      SDL_Rect dest_rect = { x1,y,x2-x1,1 };
      if (SDL_FillRect(Surface,&dest_rect,DrawColor))
        EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
      break;
      }
    case TILE: {
      int src_x_offset = x1 + DrawCenterX;
      int src_y_offset = y + DrawCenterY;
      
      while (src_x_offset < 0) src_x_offset += DrawTile->width();
      while (src_y_offset < 0) src_y_offset += DrawTile->height();
      src_x_offset %= DrawTile->width();
      src_y_offset %= DrawTile->height();

      int remaining_width = x2-x1;

      while (remaining_width > 0) {
        int blit_width = DrawTile->width() - src_x_offset;
	blit_width = NUM_MIN(blit_width,remaining_width);
      
        SDL_Rect src_rect = { src_x_offset,src_y_offset,blit_width,1 };
        SDL_Rect dest_rect = { x1,y,blit_width,1 };
        if (SDL_BlitSurface(DrawTile->Surface,&src_rect,Surface,&dest_rect))
          EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
	
	x1 += blit_width;

	src_x_offset = 0;
	remaining_width -= blit_width;
        }
      break;
      }
    case IMAGE:
      EXGEN_NYI
      break;
    }
  }




void drawable::drawVLine(TCoordinate x,TCoordinate y1,TCoordinate y2) {
  switch (DrawMode) {
    case COLOR: {
      SDL_Rect dest_rect = { x,y1,1,y2-y1 };
      if (SDL_FillRect(Surface,&dest_rect,DrawColor))
        EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
      break;
      }
    case TILE: {
      int src_x_offset = x + DrawCenterX;
      int src_y_offset = y1 + DrawCenterY;
      
      while (src_x_offset < 0) src_x_offset += DrawTile->width();
      while (src_y_offset < 0) src_y_offset += DrawTile->height();
      src_x_offset %= DrawTile->width();
      src_y_offset %= DrawTile->height();

      int remaining_height = y2-y1;

      while (remaining_height > 0) {
        int blit_height = DrawTile->height() - src_y_offset;
	blit_height = NUM_MIN(blit_height,remaining_height);
      
        SDL_Rect src_rect = { src_x_offset,src_y_offset,1,blit_height };
        SDL_Rect dest_rect = { x,y1,1,blit_height };
        if (SDL_BlitSurface(DrawTile->Surface,&src_rect,Surface,&dest_rect))
          EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())

	y1 += blit_height;

	src_y_offset = 0;
	remaining_height -= blit_height;
        }
      break;
      }
    case IMAGE:
      EXGEN_NYI
      break;
    }
  }




void drawable::drawLine(TCoordinate x1,TCoordinate y1,TCoordinate x2,TCoordinate y2) {
  drawing_functions::drawLine(*this,x1,y1,x2,y2);
  }




void drawable::drawBox(TCoordinate x1,TCoordinate y1,TCoordinate x2,TCoordinate y2) {
  drawing_functions::drawBox(*this,x1,y1,x2,y2);
  }




void drawable::fillBox(TCoordinate x1,TCoordinate y1,TCoordinate x2,TCoordinate y2) {
  switch (DrawMode) {
    case COLOR: {
      SDL_Rect dest_rect = { x1,y1,x2-x1,y2-y1 };
      if (SDL_FillRect(Surface,&dest_rect,DrawColor))
        EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
      break;
      }
    case TILE: {
      int src_y_offset = y1 + DrawCenterY;
      int start_src_x_offset = x1 + DrawCenterX;
      
      while (start_src_x_offset < 0) start_src_x_offset += DrawTile->width();
      start_src_x_offset %= DrawTile->width();

      while (src_y_offset < 0) src_y_offset += DrawTile->height();
      src_y_offset %= DrawTile->height();
      
      int remaining_height = y2-y1;
      
      while (remaining_height > 0) {
        int blit_height = DrawTile->height() - src_y_offset;
	blit_height = NUM_MIN(blit_height,remaining_height);
	
	int src_x_offset = start_src_x_offset;
	int x = x1;
	
	int remaining_width = x2-x1;
	
	while (remaining_width > 0) {
          int blit_width = DrawTile->width() - src_x_offset;
          blit_width = NUM_MIN(blit_width,remaining_width);
	  
          SDL_Rect src_rect = { src_x_offset,src_y_offset,blit_width,blit_height };
          SDL_Rect dest_rect = { x,y1,blit_width,blit_height };
	  
          if (SDL_BlitSurface(DrawTile->Surface,&src_rect,Surface,&dest_rect))
            EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())

	  src_x_offset = 0;
	  remaining_width -= blit_width;
	  x += blit_width;
          }
	
	src_y_offset = 0;
	remaining_height -= blit_height;
	y1 += blit_height;
        }
      
      break;
      }
    case IMAGE:
      EXGEN_NYI
      break;
    }
  }




void drawable::drawCircle(TCoordinate x,TCoordinate y,TCoordinate r) {
  drawing_functions::drawCircle(*this,x,y,r);
  }




void drawable::fillCircle(TCoordinate x,TCoordinate y,TCoordinate r) {
  drawing_functions::fillCircle(*this,x,y,r);
  }




void drawable::drawEllipse(TCoordinate x,TCoordinate y,TCoordinate r_x,TCoordinate r_y) {
  drawing_functions::drawEllipse(*this,x,y,r_x,r_y);
  }




void drawable::fillEllipse(TCoordinate x,TCoordinate y,TCoordinate r_x,TCoordinate r_y) {
  drawing_functions::fillEllipse(*this,x,y,r_x,r_y);
  }




void drawable::drawPolygon(polygon<int> const &poly) {
  drawing_functions::drawPolygon(*this,poly);
  }




void drawable::fillPolygon(polygon<int> const &poly) {
  drawing_functions::fillPolygon(*this,poly);
  }




void drawable::blit(drawable &dest,TCoordinate x,TCoordinate y) {
  SDL_Rect dstrect;

  dstrect.x = x;
  dstrect.y = y;
  dstrect.w = width();
  dstrect.h = height();

  if (SDL_BlitSurface(Surface,NULL,dest.surface(),&dstrect))
    EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
  }




void drawable::blit(drawable &dest,TCoordinate x,TCoordinate y,
coordinate_rectangle const &source) {
  SDL_Rect srcrect,dstrect;

  srcrect.x = source.A[0];
  srcrect.y = source.A[1];
  srcrect.w = source.width();
  srcrect.h = source.height();
  
  dstrect.x = x;
  dstrect.y = y;
  dstrect.w = srcrect.w;
  dstrect.h = srcrect.h;

  if (SDL_BlitSurface(Surface,&srcrect,dest.surface(),&dstrect))
    EXSDL_THROWINFO(ECSDL_NOFRAMEBUFFER,SDL_GetError())
  }




// bitmap ---------------------------------------------------------------------
bitmap::bitmap() {
  Surface = NULL;
  }




bitmap::bitmap(SDL_PixelFormat const &fmt,TSize width,TSize height,
Uint32 flags) {
  Surface = NULL;
  create(fmt,width,height,flags);
  }





bitmap::bitmap(bitmap const &src) {
  Surface = NULL;
  copyFrom(src);
  }




bitmap::~bitmap() {
  setSurface(NULL);
  }




#ifdef SDLUCID_HAS_LIBPNG
void bitmap::loadPNG(std::istream &datastrm,png::png_meta_data *meta) {
  png_stream_reader reader(datastrm);
  reader.readInfo();
  if (reader.getBitDepth() > 8) reader.setStrip16();
  if (reader.getBitDepth() < 8) reader.setExpand();
  if (reader.getColorType() == PNG_COLOR_TYPE_GRAY) reader.setGrayToRGB();
  reader.updateInfo();
  
  if (meta)
    reader.getMetaData(*meta);
  
  bool havealpha = reader.getColorType() & PNG_COLOR_MASK_ALPHA;
  
  // we cannot directly load to a hardware surface, because sdl automatically
  // allocates hardware surfaces in the video display format
  Uint32 flags = SDL_SWSURFACE;
  if (havealpha) flags |= SDL_SRCALPHA;
  
  SDL_Surface *surf;
  
  if (reader.getColorType() == PNG_COLOR_TYPE_RGB || 
      reader.getColorType() == PNG_COLOR_TYPE_RGB_ALPHA) {
    surf = SDL_CreateRGBSurface(flags,
      reader.getWidth(),reader.getHeight(),reader.getBitDepth()*reader.getChannels(),
      getMask(8,0),
      getMask(8,8),
      getMask(8,16),
      havealpha ? getMask(8,24) : 0);
    }
  else if (reader.getColorType() == PNG_COLOR_TYPE_GRAY || 
      reader.getColorType() == PNG_COLOR_TYPE_GRAY_ALPHA) 
  {
    EXSDL_THROWINFO(ECSDL_GENERAL,"grayscale images are not supported");
  }
  else {
    surf = SDL_CreateRGBSurface(flags,
      reader.getWidth(),reader.getHeight(),reader.getBitDepth(),0,0,0,0);
    if (surf) {
      png_color *pngpalette;
      TSize colcount = reader.getPalette(pngpalette);
      auto_array<SDL_Color> palette(colcount);
      for (TIndex i = 0;i<colcount;i++) {
        palette[i].r = pngpalette[i].red;
        palette[i].g = pngpalette[i].green;
        palette[i].b = pngpalette[i].blue;
        }
      SDL_SetColors(Surface,palette.get(),0,colcount);
      }
    }
  if (surf == NULL)
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  
  setSurface(surf);
  
  { // we must not do I/O while surface is locked, as critical system locks
    // are held during the time, so we first load the image into a buffer.
    
    TSize datasize = Surface->pitch*Surface->h;
    
    auto_array<TByte> store(datasize);
    reader.readImageRowBytes(store.get(),Surface->pitch);
    reader.readEnd();
    
    surface_locker locker(Surface);
    memcpy(Surface->pixels,store.get(),datasize);
    }
  
  try {
    png_color_16p color;
    reader.getTransparencyTrueColor(color);

    if ( (color->red & 0xff) == 0 && 
	(color->green & 0xff) == 0 && 
	(color->blue & 0xff) == 0 &&
	( color->red || color->green || color->blue ) )
    {
      color->red >>= 8;
      color->green >>= 8;
      color->blue >>= 8;
    }

    colorKey(mapColor(format(),color->red, color->green, color->blue));
    }
  catch (...) { }
  }  




void bitmap::savePNG(std::ostream &datastrm,png::png_meta_data *meta) {
  png_stream_writer writer(datastrm);
  TSize bitdepth;
  bool haspalette = (Surface->format->BitsPerPixel <= 8) && Surface->format->palette;
  bool hasalphachannel = hasAlphaChannel();
  
  if (haspalette) bitdepth = Surface->format->BitsPerPixel;
  else bitdepth = hasalphachannel ? Surface->format->BitsPerPixel/4 : Surface->format->BitsPerPixel/3;
  
  writer.setInfo(Surface->w,Surface->h,bitdepth,
     haspalette ? PNG_COLOR_TYPE_PALETTE : 
      (hasalphachannel ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB ));
  
  if (haspalette) {
    auto_array<png_color> palette;
    TSize colcount = Surface->format->palette->ncolors;
    palette.allocate(colcount);
    SDL_Color *sdlpalette = Surface->format->palette->colors;
    for (TIndex i = 0;i<colcount;i++) {
      palette[i].red = sdlpalette[i].r;
      palette[i].green = sdlpalette[i].g;
      palette[i].blue = sdlpalette[i].b;
      }
    writer.setPalette(palette.get(),colcount);
    }
  
  if (hasColorKey()) {
    Uint8 r,g,b,a;
    unmapColor(format(),colorKey(),r,g,b,a);
    png_color_16 color;
    color.red = png_uint_16(r) << 8;
    color.green = png_uint_16(g) << 8;
    color.blue = png_uint_16(b) << 8;
    writer.setTransparencyTrueColor(color);
    }
  
  // we must not do I/O while surface is locked, as critical system locks
  // are held during the time, so we first put everything into a buffer.
  
  TSize datasize = Surface->pitch*Surface->h;
  
  auto_array<TByte> store(datasize);
  { surface_locker locker(Surface);
    memcpy(store.get(),Surface->pixels,datasize);
    }

  if (meta)
    writer.setMetaData(*meta);

  writer.writeInfo();
  writer.writeImageRowBytes(store.get(),Surface->pitch);
  writer.writeEnd();
  writer.flush();
  }
#endif // SDLUCID_HAS_LIBPNG




void bitmap::create(SDL_PixelFormat const &fmt,TSize width,TSize height,
Uint32 flags) {
  SDL_Surface *surf = SDL_CreateRGBSurface(flags,width,height,
    fmt.BitsPerPixel,
    fmt.Rmask,
    fmt.Gmask,
    fmt.Bmask,
    fmt.Amask);

  if (surf == NULL)
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  else
    setSurface(surf);
  }




void bitmap::copyFrom(bitmap const &src) {
  setSurface(src.createSameFormatSurface(src.width(),src.height()));
  
  surface_locker lock1(Surface),lock2(src.Surface);
  memcpy(Surface->pixels,src.Surface->pixels,src.Surface->pitch*height());
  }




void bitmap::convertFrom(bitmap const &src,SDL_PixelFormat const &fmt,Uint32 flags) {
  SDL_Surface *surf = SDL_ConvertSurface(src.Surface,
    const_cast<SDL_PixelFormat *>(&fmt),flags);

  if (surf)
    setSurface(surf);
  else 
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  }




void bitmap::convertForAcceleratedBlitFrom(bitmap const &src) {
  // try to create an equivalent test hardware surface
  SDL_Surface *hw_surf = SDL_CreateRGBSurface(
    (src.flags() & (SDL_SRCCOLORKEY | SDL_SRCALPHA)) | SDL_HWSURFACE,
    src.width(),
    src.height(),
    src.format().BitsPerPixel, // ignored for SDL_HWSURFACE
    src.format().Rmask, // ignored for SDL_HWSURFACE
    src.format().Gmask, // ignored for SDL_HWSURFACE
    src.format().Bmask, // ignored for SDL_HWSURFACE
    src.format().Amask // ignored for SDL_HWSURFACE
    );

  if (hw_surf == NULL || (hw_surf->flags & SDL_HWSURFACE) != SDL_HWSURFACE) {
    if (hw_surf) 
      SDL_FreeSurface(hw_surf);

    // hardware surface creation failed - fall back to sdl
    SDL_Surface *surf;
    if (src.hasAlphaChannel())
      surf = SDL_DisplayFormatAlpha(src.Surface);
    else
      surf = SDL_DisplayFormat(src.Surface);
  
    if (surf) {
      setSurface(surf);
      return;
      }
    else 
      EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
    }
  
  // hardware surface creation succeeded
  
  // free test surface
  SDL_PixelFormat fmt = *hw_surf->format;
  SDL_FreeSurface(hw_surf);
  
  // create hardware surface analogous to test surface
  convertFrom(src,fmt,(src.flags() & (SDL_SRCCOLORKEY | SDL_SRCALPHA)) | SDL_HWSURFACE);
  }




void bitmap::stretchFrom(bitmap const &src,double stretch_x,double stretch_y) {
  unsigned dest_width = TSize(NUM_ABS(src.width()*stretch_x));
  unsigned dest_height = TSize(NUM_ABS(src.height()*stretch_y));
  setSurface( src.createSameFormatSurface(dest_width,dest_height) );

  if ( dest_height != 0 && dest_width != 0 )
    stretchSurface(Surface,src.Surface,stretch_x,stretch_y);
  }




void bitmap::transformFrom(bitmap const &src,affine_transformation const &trans,coordinate_vector *origin_shift) {
  affine_transformation tx = trans;
  
  // determine boundaries
  double minx,maxx,miny,maxy,tempx,tempy;
  tx.transform(minx,miny,0,0);
  maxx = minx;
  maxy = miny;
  
  #define TRY_TRANS(X,Y) \
    tx.transform(tempx,tempy,X,Y); \
    if (tempx < minx) minx = tempx; \
    if (tempy < miny) miny = tempy; \
    if (tempx > maxx) maxx = tempx; \
    if (tempy > maxy) maxy = tempy;
  
  TRY_TRANS(src.width(),0)
  TRY_TRANS(0,src.height())
  TRY_TRANS(src.width(),src.height())
  #undef TRY_TRANS
  
  double sizexf = maxx-minx,sizeyf = maxy-miny;
  TSize sizex = roundToZero(sizexf+0.99),sizey = roundToZero(sizeyf+0.99);
  
  // make origin of transformed image (0,0)
  tx.translate(-minx,-miny);
  if (origin_shift)
    origin_shift->set((TCoordinate) -minx,(TCoordinate) -miny);
  
  // create surface
  setSurface( src.createSameFormatSurface(sizex,sizey) );
  
  // transform surface
  if ( sizex != 0 && sizey != 0 ) {
    TColor back_color = 0;
    if (src.Surface->flags & SDL_SRCCOLORKEY)
      back_color = src.Surface->format->colorkey;
    transformSurface(Surface,src.Surface,tx,back_color);
    }
  }




bool bitmap::hasColorKey() const {
  return (flags() & SDL_SRCCOLORKEY) != 0;
  }




TColor bitmap::colorKey() const {
  return format().colorkey;
  }




void bitmap::colorKey(TColor key,Uint32 flags) {
  SDL_SetColorKey(Surface,flags,key);
  }




void bitmap::disregardColorKey() {
  SDL_SetColorKey(Surface,0,0);
  }




bool bitmap::hasAlphaChannel() const {
  return format().Amask != 0;
  }




bool bitmap::hasSurfaceAlpha() const {
  return (flags() & SDL_SRCALPHA) != 0 && !hasAlphaChannel();
  }




void bitmap::setAlphaFlags(Uint32 flags,Uint8 alpha) {
  SDL_SetAlpha(Surface,flags,alpha);
  }




void bitmap::multiplySurfaceAlpha(Uint8 factor) {
  TSize bypp = format().BytesPerPixel;
  Uint32 amask = format().Amask;
  Uint8 ashift = format().Ashift;
  
  if (bypp != 4)
    EXSDL_THROWINFO(ECSDL_GENERAL,"non-rgba8888 surfaces not supported for this operation")
  
  surface_locker locker(Surface);
  TByte *line_start = (TByte *) Surface->pixels;
  for (TIndex y = 0;y < height();y++) {
    Uint8 *pixel = line_start;
    for (TIndex x = 0;x < width();x++) {
      Uint32 data = *(Uint32 *) pixel;
      Uint32 alpha = (data & amask) >> ashift;
      alpha *= factor;
      alpha >>= 8;
      data = (data & ~amask) | (alpha << ashift);
      *(Uint32 *) pixel = data;
      pixel += bypp;
      }
    line_start += Surface->pitch;
    }
  }




void bitmap::disregardAlpha() {
  SDL_SetAlpha(Surface,0,0);
  }




void bitmap::setSurface(SDL_Surface *surf) {
  if (Surface) {
    SDL_FreeSurface(Surface);
    Surface = NULL;
    }
  Surface = surf;
  }




SDL_Surface *bitmap::createSameFormatSurface(TSize width,TSize height) const {
  SDL_Surface *result = SDL_CreateRGBSurface(
    Surface->flags & (SDL_SWSURFACE | SDL_HWSURFACE | SDL_ASYNCBLIT),
    width,height,Surface->format->BitsPerPixel,
    Surface->format->Rmask,
    Surface->format->Gmask,
    Surface->format->Bmask,
    Surface->format->Amask);

  if (result == NULL)
    EXSDL_THROWINFO(ECSDL_GENERAL,SDL_GetError())
  
  if (Surface->flags & SDL_SRCCOLORKEY)
    SDL_SetColorKey(result,SDL_SRCCOLORKEY,Surface->format->colorkey);
  if (Surface->flags & SDL_SRCALPHA)
    SDL_SetAlpha(result,SDL_SRCALPHA,Surface->format->alpha);

  return result;
  }




void bitmap::stretchSurface(SDL_Surface *dest,SDL_Surface *src,double stretchx,double stretchy) {
  surface_locker lock1(dest),lock2(src);
  
  const unsigned fix_digits = 10;
  const double factor = 1 << fix_digits;
  signed x_increment = signed((1/stretchx) * factor);
  signed y_increment = signed((1/stretchy) * factor);
  
  TSize bpp = src->format->BytesPerPixel;
  TSize dest_bytes_per_row = dest->w * bpp;
  TSize dest_bytes_add_at_end = dest->pitch - dest->w * bpp;
  
  Uint8 *dest_pixels = (Uint8 *) dest->pixels;
  Uint8 *dest_end = dest_pixels + dest->pitch*(dest->h-1) + dest_bytes_per_row;
  
  signed src_y_fix;
  if (y_increment >= 0)
    src_y_fix = 0;
  else
    src_y_fix = (src->h << fix_digits) + y_increment;

  signed src_x_fix_start;
  if (x_increment >= 0)
    src_x_fix_start = 0;
  else
    src_x_fix_start = (src->w << fix_digits) + x_increment;

  while (dest_pixels < dest_end) {
    Uint8 *src_row = (Uint8 *) src->pixels 
      +(src_y_fix >> fix_digits)*src->pitch;
    Uint8 *dest_end_row = dest_pixels + dest_bytes_per_row;

    signed src_x_fix = src_x_fix_start;
    
    while (dest_pixels < dest_end_row) {
      Uint8 *src_pixel = src_row + (src_x_fix >> fix_digits)*bpp;

      for (unsigned i = 0;i<bpp;i++)
        *dest_pixels++ = *src_pixel++;

      src_x_fix += x_increment;
      }
    src_y_fix += y_increment;
    dest_pixels += dest_bytes_add_at_end;
    }    
  }




void bitmap::transformSurface(SDL_Surface *dest,SDL_Surface *src,affine_transformation const &trans,TColor back_color) {
  surface_locker lock1(dest),lock2(src);

  const unsigned fix_digits = 10;
  const double fix_factor = 1u << fix_digits;
  
  affine_transformation tx = trans;
  tx.invert();
  
  TSize bpp = src->format->BytesPerPixel;
  TSize src_pitch = src->pitch;
  TSize dest_pitch = dest->pitch;
  TSize add_at_eol = dest_pitch - dest->w*bpp;
  
  Uint8 *dest_pixels = (Uint8 *) dest->pixels;
  Uint8 *src_pixels = (Uint8 *) src->pixels;
  Uint8 *src_pixel;
  Uint8 *back_pixel = (Uint8 *) &back_color;
  
  signed src_fix_width = src->w << fix_digits;
  signed src_fix_height = src->h << fix_digits;
 
  double src_xf,src_yf,x_inc_xf,x_inc_yf,y_inc_xf,y_inc_yf;
  tx.transform(src_xf,src_yf,0,0);
  tx.transformLinear(x_inc_xf,x_inc_yf,1,0);
  tx.transformLinear(y_inc_xf,y_inc_yf,0,1);
  
  signed src_fix_x = signed(src_xf * fix_factor);
  signed src_fix_y = signed(src_yf * fix_factor);

  signed x_inc_fix_x = signed(x_inc_xf * fix_factor);
  signed x_inc_fix_y = signed(x_inc_yf * fix_factor);
  signed y_inc_fix_x = signed(y_inc_xf * fix_factor);
  signed y_inc_fix_y = signed(y_inc_yf * fix_factor);

  for (signed y = 0;y < dest->h;y++)  {
    Uint8 *eol = dest_pixels+bpp*dest->w;

    signed line_src_fix_x = src_fix_x;
    signed line_src_fix_y = src_fix_y;
    
    while (dest_pixels < eol) {
      if (line_src_fix_x > 0 && line_src_fix_x < src_fix_width &&
          line_src_fix_y > 0 && line_src_fix_y < src_fix_height) {
	src_pixel = src_pixels 
	  + (line_src_fix_y >> fix_digits) * src_pitch
	  + (line_src_fix_x >> fix_digits) * bpp;
	}
      else 
        src_pixel = back_pixel;

      for (unsigned i = 0;i<bpp;i++)
        *dest_pixels++ = *src_pixel++;
      
      line_src_fix_x += x_inc_fix_x;
      line_src_fix_y += x_inc_fix_y;
      }
    
    src_fix_x += y_inc_fix_x;
    src_fix_y += y_inc_fix_y;
    
    dest_pixels += add_at_eol;
    }
  }




// display --------------------------------------------------------------------
display::display(TSize width,TSize height,Uint32 flags,TSize bpp) {
  if (bpp == 0) {
    SDL_VideoInfo const *videoinfo = SDL_GetVideoInfo();      
    if (!videoinfo || !videoinfo->vfmt) 
      EXSDL_THROWINFO(ECSDL_NODEVICE,SDL_GetError())
    bpp = videoinfo->vfmt->BitsPerPixel;
    }
  Surface = SDL_SetVideoMode(width,height,bpp,flags);
  if (!Surface)     
    EXSDL_THROWINFO(ECSDL_NODEVICE,SDL_GetError())
  }



      
void display::update(coordinate_rectangle const &rect) {
  SDL_UpdateRect(Surface,rect.A[0],rect.A[1],rect.width(),rect.height());
  }




void display::flip() const {
  SDL_Flip(Surface);
  }




void display::caption(std::string const &caption) {
  SDL_WM_SetCaption(caption.c_str(),NULL);
  }




// font ----------------------------------------------------------------------
font::font()
  : Name(""), TopLine(0), BottomLine(0) {
  for (TIndex i = 0;i < MaxCharacterCount;i++) 
    CharTable[i].Present = false;
  }




void font::load(std::istream &data,std::istream &bitmap) {
  Bitmap.loadPNG(bitmap);
  
  xml_file xml;
  xml.read(data);
  xml_file::tag *cfgtag = xml.rootTag();
  
  if (cfgtag->getName() == XML_FONT_TAG) {
    Name = cfgtag->Attributes[XML_FONT_NAME];
    TopLine = evalSigned(cfgtag->Attributes[XML_FONT_TOPLINE]);
    BottomLine = evalSigned(cfgtag->Attributes[XML_FONT_BOTTOMLINE]);

    vector<xml_file::tag *>::iterator begin = cfgtag->Children.begin();
    vector<xml_file::tag *>::iterator end   = cfgtag->Children.end();
    while (begin != end) {
      xml_file::tag *tag = *begin;
      if (tag->getName() == XML_GLYPH_TAG) {
        unsigned char code;
        code = evalUnsigned(tag->Attributes[XML_GLYPH_CODE]);
	CharTable[code].Present = true;
	CharTable[code].BaseX = evalSigned(tag->Attributes[XML_GLYPH_BASE_X]);
	CharTable[code].BaseY = evalSigned(tag->Attributes[XML_GLYPH_BASE_Y]);
	CharTable[code].Width = evalSigned(tag->Attributes[XML_GLYPH_WIDTH]);
	CharTable[code].Boundary.A[0] = evalSigned(tag->Attributes[XML_GLYPH_BOUND_X]);
	CharTable[code].Boundary.A[1] = evalSigned(tag->Attributes[XML_GLYPH_BOUND_Y]);
	CharTable[code].Boundary.B = CharTable[code].Boundary.A;
	CharTable[code].Boundary.B[0] += evalSigned(tag->Attributes[XML_GLYPH_BOUND_W]);
	CharTable[code].Boundary.B[1] += evalSigned(tag->Attributes[XML_GLYPH_BOUND_H]);
	}
      begin++;
      }
    }
  }




void font::save(std::ostream &data,std::ostream &bitmap) {
  Bitmap.savePNG(bitmap);
  
  xml_file::tag *cfgtag = new xml_file::tag(XML_FONT_TAG);
  cfgtag->Attributes[XML_FONT_NAME] = Name;
  cfgtag->Attributes[XML_FONT_TOPLINE] = signed2dec(TopLine);
  cfgtag->Attributes[XML_FONT_BOTTOMLINE] = signed2dec(BottomLine);

  for (unsigned int code = 0;code < MaxCharacterCount;code++) {
    if (CharTable[code].Present) {
      xml_file::tag *tag = new xml_file::tag(XML_GLYPH_TAG);
      tag->Attributes[XML_GLYPH_CODE] = unsigned2dec(code);
      tag->Attributes[XML_GLYPH_BASE_X] = signed2dec(CharTable[code].BaseX);
      tag->Attributes[XML_GLYPH_BASE_Y] = signed2dec(CharTable[code].BaseY);
      tag->Attributes[XML_GLYPH_WIDTH] = signed2dec(CharTable[code].Width);
      tag->Attributes[XML_GLYPH_BOUND_X] = signed2dec(CharTable[code].Boundary.A[0]);
      tag->Attributes[XML_GLYPH_BOUND_Y] = signed2dec(CharTable[code].Boundary.A[1]);
      tag->Attributes[XML_GLYPH_BOUND_W] = signed2dec(CharTable[code].Boundary.width());
      tag->Attributes[XML_GLYPH_BOUND_H] = signed2dec(CharTable[code].Boundary.height());
      cfgtag->appendTag(tag);
      }
    }
  xml_file xml;
  xml.rootTag(cfgtag);
  xml.write(data);
  }




void font::copyFrom(font const &src) {
  Bitmap.copyFrom(src.Bitmap);
  copyDataBlock(src);
  }





void font::convertFrom(font const &src,SDL_PixelFormat const &fmt,Uint32 flags) {
  Bitmap.convertFrom(src.Bitmap,fmt,flags);
  copyDataBlock(src);
  }




void font::convertForAcceleratedBlitFrom(font const &src) {
  Bitmap.convertForAcceleratedBlitFrom(src.Bitmap);
  copyDataBlock(src);
  }




coordinate_rectangle font::extent(std::string const &s) const {
  TCoordinate charpos = 0;
  coordinate_rectangle rect(0,0,0,0);
  
  for (string::size_type i=0; i<s.size(); i++) {
    glyph gph = CharTable[(unsigned char) s[i]];
    rect.unite(gph.Boundary-gph.Boundary.A+coordinate_vector(charpos-gph.BaseX,-gph.BaseY));
    charpos += gph.Width;
    }

  return rect;
  }




void font::putChar(drawable &dwbl,TCoordinate x,TCoordinate y,unsigned char c,TTextAlignment align) {
  glyph &gph = CharTable[c];
  if (gph.Present) {
    int posx = x - gph.BaseX;
    int posy = y - gph.BaseY;
    
    switch (align) {
      case ALIGN_TOP: posy -= TopLine; break;
      case ALIGN_BASE: break;
      case ALIGN_BOTTOM: posy -= BottomLine; break;
      }

    Bitmap.blit(dwbl,posx,posy,gph.Boundary);
    }
  }




void font::print(drawable &dwbl,TCoordinate x,TCoordinate y,
std::string const &text,TTextAlignment align) {
  FOREACH_CONST(first,text,string) {
    unsigned char ch = *first;
    putChar(dwbl,x,y,ch,align);
    x += CharTable[ch].Width;
    }
  }




void font::copyDataBlock(font const &src) {
  Name = src.Name;
  TopLine = src.TopLine;
  BottomLine = src.BottomLine;
  for (TIndex i = 0;i < MaxCharacterCount;i++)
    CharTable[i] = src.CharTable[i];
  }
