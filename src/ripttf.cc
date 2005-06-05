// ----------------------------------------------------------------------------
//  Description      : ttf ripping tool
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#include <string>
#include <fstream>
#include <fcntl.h>
#include <ixlib_base.hh>
#include <ixlib_cmdline.hh>
#include <ixlib_re.hh>
#include <ixlib_numconv.hh>
#include <ixlib_geometry.hh>
#include <ixlib_array.hh>
#include <freetype/freetype.h>
#include <sdlucid_video.hh>




#define MAKE_TT_FIX(number) TT_F26Dot6(number * (1 << 6))
#define MAKE_FLOAT(fix) (double(fix) / (1 << 6))
#define MAKE_INT(fix) (int(fix) >> 6)




using namespace std;
using namespace ixion;
using namespace sdl;




int main(int argc,char **argv) {
  try {
    command_line cmdline(argc,argv);
    cout << "ripttf by Andreas Kloeckner, part of SDLucid." << endl;

    if (cmdline.count("--help") || cmdline.count("-h")) {
      cout << "usage: ripttf <parameters>" << endl
	<< "where <parameters> may / must(*) contain" << endl
	<< "  ttf=<file.ttf>               Specify input TrueType font (*)" << endl
	<< "  out=<file.fnt>               Specify output raster font (*)" << endl
	<< "  size=<file.fnt>              Specify output pixel size (default 20)" << endl
	<< "  --fg=<r>,<g>,<b>             Specify foreground color (default white)" << endl
	<< "  --bg=<r>,<g>,<b>             Specify background color (default black)" << endl
	<< "  --key=<r>,<g>,<b>            Specify color key" << endl
	<< "  --alpha                      Make font use alpha" << endl
	<< "  --rim=<size>,<r>,<g>,<b>     Draw a rim" << endl;
      return 0;
      }
    
    // parse command line -----------------------------------------------------
    if (!cmdline.count("ttf=")) EXGEN_THROWINFO(EC_INVALIDPAR,"no ttf= parameter")
    string ttf = cmdline.get("ttf=");
    if (!cmdline.count("out=")) EXGEN_THROWINFO(EC_INVALIDPAR,"no out= parameter")
    string data_name = cmdline.get("out=");
    
    float size = 20;
    
    if (cmdline.count("size=")) 
      size = evalFloat(cmdline.get("size="));
    
    unsigned fg_r = 255,fg_g = 255,fg_b = 255;
    
    if (cmdline.count("--fg=")) {
      regex_string re("^([0-9]+)\\,([0-9]+)\\,([0-9]+)$");

      if (!re.match(cmdline.get("--fg="))) 
        EXGEN_THROWINFO(EC_INVALIDPAR,"invalid fg= format")
      
      fg_r = evalUnsigned(re.getBackref(0));
      fg_g = evalUnsigned(re.getBackref(1));
      fg_b = evalUnsigned(re.getBackref(2));
      }
    
    unsigned bg_r = 0,bg_g = 0,bg_b = 0;
    
    if (cmdline.count("--bg=")) {
      regex_string re("^([0-9]+)\\,([0-9]+)\\,([0-9]+)$");

      if (!re.match(cmdline.get("--bg="))) 
        EXGEN_THROWINFO(EC_INVALIDPAR,"invalid bg= format")
      
      bg_r = evalUnsigned(re.getBackref(0));
      bg_g = evalUnsigned(re.getBackref(1));
      bg_b = evalUnsigned(re.getBackref(2));
      }
    
    bool color_key = false;
    unsigned key_r,key_g,key_b;
    
    if (cmdline.count("--key=")) {
      color_key = true;
      regex_string re("^([0-9]+)\\,([0-9]+)\\,([0-9]+)$");

      if (!re.match(cmdline.get("--key="))) 
        EXGEN_THROWINFO(EC_INVALIDPAR,"invalid key= format")
      
      key_r = evalUnsigned(re.getBackref(0));
      key_g = evalUnsigned(re.getBackref(1));
      key_b = evalUnsigned(re.getBackref(2));
      }
    
    TSize rim_size = 0;
    TSize size_add = 0;
    unsigned rim_r = 0,rim_g = 0,rim_b = 0;
    if (cmdline.count("--rim=")) {
      regex_string re("^([0-9]+)\\,([0-9]+)\\,([0-9]+)\\,([0-9]+)$");

      if (!re.match(cmdline.get("--rim="))) 
        EXGEN_THROWINFO(EC_INVALIDPAR,"invalid rim= format")
      
      size_add = 1;
      rim_size = evalUnsigned(re.getBackref(0));
      rim_r = evalUnsigned(re.getBackref(1));
      rim_g = evalUnsigned(re.getBackref(2));
      rim_b = evalUnsigned(re.getBackref(3));
      }
    
    bool use_alpha = cmdline.count("--alpha") != 0;
    
    // fire up freetype -------------------------------------------------------
    TT_Engine engine;
    if (TT_Init_FreeType(&engine))
      EXGEN_THROWINFO(EC_ERROR,"freetype error")
    
    TT_Face face;
    if (TT_Open_Face(engine,ttf.c_str(),&face))
      EXGEN_THROWINFO(EC_ERROR,"freetype error")
    
    TT_Face_Properties props;
    TT_Get_Face_Properties(face,&props);
    
    string realname = "Unknown";
    { 
      for (int i = 0;i < props.num_Names;i++) {
        TT_UShort platform_id,encoding_id,language_id,name_id;
        TT_Get_Name_ID(face,i,&platform_id,&encoding_id,&language_id,&name_id);
	
	if (name_id == TT_NAME_ID_FULL_NAME) {
          TT_UShort namelen;
	  TT_String *ttf_name = NULL;
	  
          TT_Get_Name_String(face,i,&ttf_name,&namelen);
	  realname = string(ttf_name,namelen);
	  break;
	  }
	}
      }

    cout << "font name: " << realname << endl;
    
    TT_Instance instance;
    TT_New_Instance(face,&instance);
    TT_Set_Instance_CharSize(instance,MAKE_TT_FIX(size));
    
    TT_Glyph glyph;
    TT_New_Glyph(face,&glyph);
    
    TT_CharMap charmap;
    {
      bool success = false;
      for (int i = 0;i < props.num_CharMaps;i++) {
        TT_UShort platform_id,encoding_id;
      
        TT_Get_CharMap_ID(face,i,&platform_id,&encoding_id);
	if (platform_id == TT_PLATFORM_MICROSOFT && encoding_id == TT_MS_ID_UNICODE_CS) {
	  success = true;
	  TT_Get_CharMap(face,i,&charmap);
	  }
        }
      if (!success)
        EXGEN_THROWINFO(EC_ERROR,"freetype error: no appropriate charmap found")
      }

    TT_Byte palette[] = { 0,64,128,192,255 };
    TT_Set_Raster_Gray_Palette(engine,palette);

    // determine metrics ------------------------------------------------------
    cout << "building metrics..." << endl;
    rectangle<int> surface_bbox(0,0,0,0);
    
    for (TT_UShort ch = 32;ch < font::MaxCharacterCount;ch++) {
      TT_UShort glyph_index = TT_Char_Index(charmap,ch);
      if (glyph_index == 0)
	continue;

      TT_Load_Glyph(instance,glyph,glyph_index,TTLOAD_SCALE_GLYPH | TTLOAD_HINT_GLYPH);
      
      TT_Glyph_Metrics metrics;
      TT_Get_Glyph_Metrics(glyph,&metrics);
      
      surface_bbox.B[0] += MAKE_INT(metrics.bbox.xMax-metrics.bbox.xMin)+size_add+2*rim_size;
      if (MAKE_INT(metrics.bbox.yMin) < surface_bbox.A[1]) surface_bbox.A[1] = MAKE_INT(metrics.bbox.yMin);
      if (MAKE_INT(metrics.bbox.yMax) > surface_bbox.B[1]) surface_bbox.B[1] = MAKE_INT(metrics.bbox.yMax);
      }
    surface_bbox.A[1] -= rim_size;
    surface_bbox.B[1] += size_add+rim_size;
    
    // create SDL font --------------------------------------------------------
    cout << "rendering..." << endl;
    sdl::font fnt;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_PixelFormat format;
    if (use_alpha)
      createPixelFormat(format,8,4);
    else 
      createPixelFormat(format,8);
      
    fnt.Bitmap.create(format,surface_bbox.getSizeX(),surface_bbox.getSizeY());

    fnt.Name = realname;
    fnt.TopLine = -surface_bbox.B[1];
    fnt.BottomLine = -surface_bbox.A[1];
    
    if (color_key) {
      fnt.Bitmap.drawingColor(mapColor(format,key_r,key_g,key_b));
      fnt.Bitmap.colorKey(mapColor(format,key_r,key_g,key_b));
      }
    else if (use_alpha)
      fnt.Bitmap.drawingColor(mapColor(format,fg_r,fg_g,fg_b,SDL_ALPHA_TRANSPARENT));
    else
      fnt.Bitmap.drawingColor(mapColor(format,bg_r,bg_g,bg_b));

    fnt.Bitmap.fillBox(0,0,fnt.Bitmap.width(),fnt.Bitmap.height());
    
    TCoordinate draw_at_x = 0;
    
    for (TT_UShort ch = 32;ch < font::MaxCharacterCount;ch++) {
      cout << '[' << (char) ch << "] " << flush;
      TT_UShort glyph_index = TT_Char_Index(charmap,ch);
      if (glyph_index == 0)
	continue;

      TT_Load_Glyph(instance,glyph,glyph_index,TTLOAD_SCALE_GLYPH | TTLOAD_HINT_GLYPH);
      
      TT_Glyph_Metrics metrics;
      TT_Get_Glyph_Metrics(glyph,&metrics);
      
      TT_Raster_Map map;
      map.rows = MAKE_INT(metrics.bbox.yMax-metrics.bbox.yMin);
      map.width = MAKE_INT(metrics.bbox.xMax-metrics.bbox.xMin);
      map.cols = (map.width+3) & ~3;
      map.flow = TT_Flow_Down;
      map.size = map.cols*map.rows;
      
      auto_array<TUnsigned8> bitmap(map.size);
      map.bitmap = bitmap.get();
      memset(bitmap.get(),0,map.cols*map.rows);
      
      TT_Get_Glyph_Pixmap(glyph,&map,
        MAKE_TT_FIX(MAKE_INT(-metrics.bbox.xMin)),
        MAKE_TT_FIX(-MAKE_FLOAT(metrics.bbox.yMin)));

      font::glyph &sdl_glyph = fnt.CharTable[ch];
      
      sdl_glyph.Present = true;
      sdl_glyph.BaseX = MAKE_INT(-metrics.bbox.xMin)+rim_size;
      sdl_glyph.BaseY = MAKE_INT(metrics.bbox.yMax)+rim_size;
      sdl_glyph.Width = MAKE_INT(metrics.advance);
      if (rim_size > 0)
        sdl_glyph.Width += 2*(rim_size-1);
      sdl_glyph.Boundary.A[0] = draw_at_x;
      sdl_glyph.Boundary.A[1] = -fnt.TopLine-MAKE_INT(metrics.bbox.yMax)-rim_size-size_add;
      sdl_glyph.Boundary.B = sdl_glyph.Boundary.A;
      sdl_glyph.Boundary.B[0] += map.width+2*rim_size+size_add;
      sdl_glyph.Boundary.B[1] += map.rows+2*rim_size+size_add;
      
      #define RENDER_PIXELS(BASE_X,BASE_Y,FG_R,FG_G,FG_B,BG_R,BG_G,BG_B,USE_ALPHA) \
        for (int y = 0;y < map.rows;y++) \
          for (int x = 0;x < map.width;x++) { \
            int pixel = bitmap[y*map.cols+x]; \
            \
            if (pixel != 0) { \
              if (USE_ALPHA) { \
                TColor col = fnt.Bitmap.getPixel(sdl_glyph.Boundary.A[0]+(BASE_X)+x,sdl_glyph.Boundary.A[1]+(BASE_Y)+y); \
		Uint8 r,g,b,a; \
		unmapColor(format,col,r,g,b,a); \
                fnt.Bitmap.drawingColor(mapColor(format,FG_R,FG_G,FG_B,NUM_MAX(a,pixel))); \
                fnt.Bitmap.setPixel(sdl_glyph.Boundary.A[0]+(BASE_X)+x,sdl_glyph.Boundary.A[1]+(BASE_Y)+y); \
                } \
              else { \
                int r = (pixel * FG_R + (255-pixel) * BG_R) / 255; \
                int g = (pixel * FG_G + (255-pixel) * BG_G) / 255; \
                int b = (pixel * FG_B + (255-pixel) * BG_B) / 255; \
        	\
                fnt.Bitmap.drawingColor(mapColor(format,r,g,b)); \
                fnt.Bitmap.setPixel(sdl_glyph.Boundary.A[0]+(BASE_X)+x,sdl_glyph.Boundary.A[1]+(BASE_Y)+y); \
                } \
              } \
            }
      
      if (rim_size) {
        for (TIndex rim_x = 0;rim_x < 2*rim_size+size_add;rim_x++)
          for (TIndex rim_y = 0;rim_y < 2*rim_size+size_add;rim_y++)
	    RENDER_PIXELS(rim_x,rim_y,rim_r,rim_g,rim_b,bg_r,bg_g,bg_b,use_alpha)

        RENDER_PIXELS(rim_size,rim_size,fg_r,fg_g,fg_b,rim_r,rim_g,rim_b,false)
	}
      else
        RENDER_PIXELS(0,0,fg_r,fg_g,fg_b,bg_r,bg_g,bg_b,use_alpha)
      
      draw_at_x += sdl_glyph.Boundary.width();
      }
    cout << endl;
    
    // write to disk ----------------------------------------------------------
    cout << "writing files..." << endl;
    string image_name = data_name.substr(0,data_name.size()-4) + ".png";
    ofstream data(data_name.c_str());
    ofstream image(image_name.c_str());
    
    fnt.save(data,image);
    
    // finalize ---------------------------------------------------------------
    SDL_Quit();
    
    
    #if 0
    // this would be the clean way, but it causes segfaults :-(
    TT_Done_Glyph(glyph);
    TT_Done_Instance(instance);
    TT_Close_Face(face);
    TT_Done_FreeType(engine);
    #endif
    }
  catch (exception &ex) {
    cerr << ex.what() << endl;
    }
  catch (...) {
    cerr << "exit via unknown exception" << endl;
    }
  }
