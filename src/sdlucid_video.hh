// ----------------------------------------------------------------------------
//  Description      : SDL C++ video wrappers
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_VIDEO
#define SDLUCID_VIDEO




#include <memory>
#include <iostream>
#include <SDL.h>
#include <ixlib_xml.hh>
#include <ixlib_geometry.hh>
#include <ixlib_polygon.hh>
#include <sdlucid_base.hh>
#include <sdlucid_png.hh>




namespace sdl {
  using ixion::TSize;
  
  
  
  
  // types and declarations ---------------------------------------------------
  typedef Uint32				TColor;
  typedef Sint32				TCoordinate;
  typedef ixion::coord_vector<TCoordinate,2>	coordinate_vector;
  typedef ixion::rectangle<TCoordinate>		coordinate_rectangle;
      
  class drawable;
  class bitmap;
  class font;
  



  // affine_transformation ----------------------------------------------------
  struct affine_transformation {
    double Matrix[2][2];
    double Translation[2];
    
    void identity();
    void translate(double x,double y);
    void scale(double x,double y);
    void rotate(double rad);
    void invert();
    
    void transform(double &dest_x,double &dest_y,double x,double y) const;
    void transformLinear(double &dest_x,double &dest_y,double x,double y) const;
    };
  
  
  
  
  // pixel format routines ----------------------------------------------------
  void createPixelFormat(SDL_PixelFormat &fmt,TSize bitdepth,TSize channels = 3,
    TColor colorkey = 0,float surfalpha = 0,SDL_Palette *pal = NULL);
  TColor mapColor(SDL_PixelFormat const &fmt,Uint8 r,Uint8 g,Uint8 b,Uint8 a = SDL_ALPHA_OPAQUE);
  void unmapColor(SDL_PixelFormat const &fmt,TColor color,Uint8 &r,Uint8 &g,Uint8 &b,Uint8 &a);




  // drawable -----------------------------------------------------------------
  class drawable {
      enum TDrawMode {
        COLOR,
        TILE,
        IMAGE,
        }			DrawMode;
      TColor			DrawColor;
      const bitmap		*DrawTile,*DrawPixel;
      TCoordinate		DrawCenterX,DrawCenterY;
      
    protected:
      // This member is read-only in this class. Subclasses need to
      // manage it on their own. The clean way would be a pure-virtual
      // accessor function, but that's probably too slow.
      SDL_Surface		*Surface;

    public:
      drawable();
      
      SDL_Surface *surface() const;
      SDL_PixelFormat const &format() const;
      TSize height() const;
      TSize width() const;
      Uint32 flags() const;
      coordinate_rectangle extent() const;

      coordinate_rectangle clipping();
      void clipping(coordinate_rectangle const &rect);
      void clearClipping();

      void lock();
      void unlock();

      // low-level
      void drawingColor(TColor color);
      void drawingTile(bitmap const *tile,TCoordinate centerx = 0,TCoordinate centery = 0);
      void drawingPixel(bitmap const *pixel,TCoordinate centerx = 0,TCoordinate centery = 0);
      
      TColor getPixel(TCoordinate x,TCoordinate y);
      void setPixel(TCoordinate x,TCoordinate y);
      void drawHLine(TCoordinate x1,TCoordinate y,TCoordinate x2);
      void drawVLine(TCoordinate x,TCoordinate y1,TCoordinate y2);
  
      // high-level
      void drawLine(TCoordinate x1,TCoordinate y1,TCoordinate x2,TCoordinate y2);
      void drawBox(TCoordinate x1,TCoordinate y1,TCoordinate x2,TCoordinate y2);
      void fillBox(TCoordinate x1,TCoordinate y1,TCoordinate x2,TCoordinate y2);
      void drawCircle(TCoordinate x,TCoordinate y,TCoordinate r);
      void fillCircle(TCoordinate x,TCoordinate y,TCoordinate r);
      void drawEllipse(TCoordinate x,TCoordinate y,TCoordinate r_x,TCoordinate r_y);
      void fillEllipse(TCoordinate x,TCoordinate y,TCoordinate r_x,TCoordinate r_y);
      void drawPolygon(ixion::polygon<int> const &poly);
      void fillPolygon(ixion::polygon<int> const &poly);
      
      void blit(drawable &dest,TCoordinate x,TCoordinate y);
      void blit(drawable &dest,TCoordinate x,TCoordinate y,
        coordinate_rectangle const &source);
    };




  // bitmap -------------------------------------------------------------------
  class bitmap : public drawable {
    public:
      bitmap();
      bitmap(SDL_PixelFormat const &fmt,TSize width,TSize height,
        Uint32 flags = SDL_SWSURFACE);
      bitmap(bitmap const &src);
      ~bitmap();
  
      #ifdef SDLUCID_HAS_LIBPNG
      void loadPNG(std::istream &datastrm,png::png_meta_data *meta = NULL);
      void savePNG(std::ostream &datastrm,png::png_meta_data *meta = NULL);
      #endif // SDLUCID_HAS_LIBPNG
      
      void create(SDL_PixelFormat const &fmt,TSize width,TSize height,
        Uint32 flags = SDL_SWSURFACE);
      void copyFrom(bitmap const &src);
      void convertFrom(bitmap const &src,SDL_PixelFormat const &fmt,Uint32 flags);
      void convertForAcceleratedBlitFrom(bitmap const &src);
      void stretchFrom(bitmap const &src,double stretch_x,double stretch_y);
      void transformFrom(bitmap const &src,affine_transformation const &trans,coordinate_vector *origin_shift = NULL);
      
      bool hasColorKey() const;
      TColor colorKey() const;
      void colorKey(TColor key,Uint32 flags = SDL_SRCCOLORKEY);
      void disregardColorKey();
  
      bool hasAlphaChannel() const;
      bool hasSurfaceAlpha() const;
      void setAlphaFlags(Uint32 flags = SDL_SRCALPHA,Uint8 alpha = SDL_ALPHA_OPAQUE);
      void multiplySurfaceAlpha(Uint8 factor);
      void disregardAlpha();
  
    private:
      void setSurface(SDL_Surface *surf);
      SDL_Surface *createSameFormatSurface(TSize width,TSize height) const;
      static void stretchSurface(SDL_Surface *dest,SDL_Surface *src,double stretchx = 1,double stretchy = 1);
      static void transformSurface(SDL_Surface *dest,SDL_Surface *src,affine_transformation const &trans,TColor back_color = 0);
    };




  // display ------------------------------------------------------------------
  class display : public drawable {
    public:
      display(TSize width,TSize height,Uint32 flags,TSize bpp = 0);
      
      void update(coordinate_rectangle const &rect);
      void flip() const;
      void caption(std::string const &caption);
    };




  // font ---------------------------------------------------------------------
  class font {
    public:
      enum TTextAlignment {
        ALIGN_TOP,ALIGN_BASE,ALIGN_BOTTOM
        };

      struct glyph {
        bool			Present;
        int			BaseX;
        int			BaseY;
        unsigned          	Width;
        coordinate_rectangle	Boundary;
        };
      
      TSize static const 	MaxCharacterCount = 256;
  
      std::string             	Name;
      
      // it is implied that the base line is at 0
      int			TopLine;
      int			BottomLine;
      glyph			CharTable[MaxCharacterCount];
      bitmap			Bitmap;
  
      font();
  
      TSize lineHeight() const {
        return BottomLine-TopLine;
	}
      
      void load(std::istream &data,std::istream &bitmap);
      void save(std::ostream &data,std::ostream &bitmap);

      void copyFrom(font const &src);
      void convertFrom(font const &src,SDL_PixelFormat const &fmt,Uint32 flags);
      void convertForAcceleratedBlitFrom(font const &src);

      coordinate_rectangle extent(std::string const &s) const;
      
      void putChar(drawable &dwbl,TCoordinate x,TCoordinate y,unsigned char c,TTextAlignment align = ALIGN_TOP);
      void print(drawable &dwbl,TCoordinate x,TCoordinate y,
        std::string const &text,TTextAlignment align = ALIGN_TOP);
      
    private:
      void copyDataBlock(font const &src);
    };
  // drawable_locker ----------------------------------------------------------
  class drawable_locker {
    protected:
      drawable		*Drawable;

    public:
      drawable_locker( drawable *dwbl );
      ~drawable_locker();
  };




  inline drawable_locker::drawable_locker( drawable * dwbl )
    : Drawable( dwbl ) 
  {
    Drawable->lock();
  }





  inline drawable_locker::~drawable_locker() 
  {
    Drawable->unlock();
  }
}




#endif
