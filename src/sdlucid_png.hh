// ----------------------------------------------------------------------------
//  Description      : C++ wrapper for libpng.
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 Andreas Kloeckner
// ----------------------------------------------------------------------------




#ifndef SDLUCID_PNG
#define SDLUCID_PNG




#include <png.h>
#include <fstream>
#include <map>
#include <ixlib_exbase.hh>
#include <ixlib_string.hh>




namespace png {
  using ixion::TSize;
  using ixion::TByte;
  
  
  
  
  typedef std::map<std::string,std::string>	png_meta_data;
  
  
  
  
  #define ECPNG_GENERAL		0
  #define ECPNG_INVALIDINFO	1
  
  
  
  
  struct png_exception : public ixion::base_exception {
    png_exception(ixion::TErrorCode error, char const *info = NULL, char const *module = NULL,
      ixion::TIndex line = 0)
      : base_exception(error,info,module,line,"PNG") {
      }
    virtual char const *getText() const;
  };
  
  
  
  
  #define EXPNG_THROW(CODE)\
    throw png::png_exception(CODE,NULL,__FILE__,__LINE__);
  #define EXPNG_THROWINFO(CODE,INFO)\
    throw png::png_exception(CODE,(char const *) INFO,__FILE__,__LINE__);
  
  
  
  
  // Classes --------------------------------------------------------------------
  class png_base {
    public:
      png_structp PngStruct;
      png_infop Info,EndInfo;
      
      static char const *error_msg;
      
      png_base();
        
      // information retrieval 
      TSize getChannels();
      TSize getRowBytes();
      TSize getAlignedRowBytes(TSize align);
      TSize getImageBytes();
      TSize getWidth();
      TSize getHeight();
      TSize getBitDepth();
      int getColorType();
  
      TSize getPalette(png_color *&palette);
      double getGamma();
      void getSignificantBits(png_color_8p &sigbits);
      TSize getTransparencyIndexed(png_byte *&transentries);
      TSize getTransparencyTrueColor(png_color_16p &transvalue);
      png_color_16 getBackground();
      void getMetaData(png_meta_data &meta);
  
      // transformations
      void setInvertAlpha();
      void setPacking(); // use packed-pixel format
      void setFiller(int filler,int flags);
      void setBGR();
      void setPackSwap(); // swap pixel pack order
      void setInvertMono();
    };
    
    
    
    
  class png_reader : public png_base {
    public:
      png_reader();
      virtual ~png_reader();
  
      // transformations 
      void setExpand(); // expand indexed to 8 bpp, add alpha
      void setStrip16(); // forget about 16 bpp
      void setStripAlpha(); // combine with background
      void setSwapAlpha(); // ARGB instead of RGBA
      void setGrayToRGB();
      void setRGBToGray(int error_action = 0,float red_weight = 0,float green_weight = 0);
      void setGamma(float screen_gamma,float image_gamma = 0.45455);
      void setSwap(); // swap byte order
      void setInterlaceHandling();
        
      void updateInfo();
        
      void readInfo();
      void readImage(TByte *data,TSize align = 1);
      void readImageRowBytes(TByte *data,TSize rowbytes);
      void readEnd();
  
      // internal - don't call
      virtual void internalRead(png_bytep data,png_uint_32 length) = 0;
    };
  
  
  
  
  class png_writer : public png_base {
    public:
      png_writer();
      virtual ~png_writer();
  
      // Information setting
      void setCompressionLevel(int level = Z_BEST_COMPRESSION);
      void setInfo(TSize width,TSize height,TSize bit_depth = 8,
        int color_type = PNG_COLOR_TYPE_RGB, 
        int interlace_type = PNG_INTERLACE_NONE,
        int compression_type = PNG_COMPRESSION_TYPE_DEFAULT,
        int filter_type = PNG_FILTER_TYPE_DEFAULT);
      void setPalette(png_color *palette,TSize colcount);
      void setGamma(double gamma);
      void setSignificantBits(png_color_8 &sig_bits);
      void setTransparencyIndexed(png_byte *transentries,TSize color_count);
      void setTransparencyTrueColor(png_color_16 &transvalue);
      void setBackground(png_color_16 &background);
      void setMetaData(png_meta_data const &meta);
  
      void flush();
      void autoFlushAfter(TSize rows);
        
      void writeInfo();
      void writeImage(TByte *data,TSize align = 1);
      void writeImageRowBytes(TByte *data,TSize rowbytes);
      void writeEnd();
  
      // internal - don't call
      virtual void internalWrite(png_bytep data,png_uint_32 length) = 0;
      virtual void internalFlush() = 0;
    };
  
  
  
  
  class png_stream_reader : public png_reader {
    protected:
      std::istream 	&IStream;
    
    public:
      png_stream_reader(std::istream &istr)
        : IStream(istr) {
        }
        
    protected:
      virtual void internalRead(png_bytep data,png_uint_32 length);
    };
  
  
  
  
  class png_stream_writer : public png_writer {
    protected:
      std::ostream 	&OStream;
    
    public:
      png_stream_writer(std::ostream &ostr)
        : OStream(ostr) {
        }
        
    protected:
      virtual void internalWrite(png_bytep data,png_uint_32 length);
      virtual void internalFlush();
    };
  }




#endif
