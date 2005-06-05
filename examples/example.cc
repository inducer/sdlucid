#include <sdlucid_base.hh>
#ifdef SDLUCID_HAS_LIBPNG
#include <cmath>
#include <fstream>
#include <sdlucid_instance.hh>
#include <sdlucid_video.hh>
#include <sdlucid_audio.hh>
#include <sdlucid_mikmod_stream.hh>
#include "refpoint.hh"




#define OPEN_MODE (ios::in | ios::binary)
#define SCROLLER_TEXT \
  "Welcome to " \
  "SDLucid! " \
  "                   " \
  "SDLucid is a feature-enhanced C++ wrapper for SDL, " \
  "written by Andreas Klöckner and Hardy Kahl. " \
  "                   " \
  "SDL is the world's coolest cross-platform multimedia library." \
  "It was written by Sam Latinga and the wacky folks at LokiGames, Inc." \
  "                   " \
  "Credits for this demo: " \
  "code: Andreas - gfx: asmodean/x.themes.org - music: jester/sanity" \
  "                   " \
  "http://sdlucid.sourceforge.net" \
  "                   " \
  "The SDLucid project is generously hosted by SourceForge, " \
  "courtesy of VA Linux, Inc." \
  "                   " \
  "SDLucid is licensed under the GNU Library General Public License"



using namespace sdl;
using namespace std;




extern "C" int main() {
  try {
    // initialize everything
    sdl_instance my_sdl;
    
    display dpy(640,480,SDL_HWPALETTE | SDL_HWSURFACE | SDL_DOUBLEBUF);
    dpy.caption("SDLucid example");
    
    // load the font
    font fnt;
    ifstream fnt_data("tahoma.fnt",OPEN_MODE);
    ifstream fnt_bitmap("tahoma.png",OPEN_MODE);
    fnt.load(fnt_data,fnt_bitmap);
    
    // convert the font
    { font fnt_conv;
      fnt_conv.convertForAcceleratedBlitFrom(fnt);
      fnt.copyFrom(fnt_conv);
      }
    
    fnt.print(dpy,10,10,"Loading...");
    dpy.flip();

    // precalculate some scroller stuff
    coordinate_rectangle ext = fnt.extent(SCROLLER_TEXT);
    
    // load the background bitmap
    bitmap back_image;
    ifstream back_image_file("night-in-samarkand.png",OPEN_MODE);
    back_image.loadPNG(back_image_file);
    
    // scale the background bitmap
    { bitmap back_image_stretch;
      back_image_stretch.stretchFrom(back_image,
        (float) dpy.width()/back_image.width(),
        (float) dpy.height()/back_image.height());
	
      back_image.convertForAcceleratedBlitFrom(back_image_stretch);
      }
    
    // load the logo
    bitmap_with_reference logo;
    ifstream logo_file("logo.png",OPEN_MODE);
    logo.loadPNG(logo_file);
    logo.referencePoint(coordinate_vector(logo.width()/2,logo.height()/2));
    
    // convert the logo
    { bitmap_with_reference logo_conv;
      logo_conv.convertForAcceleratedBlitFrom(logo);
      logo.copyFrom(logo_conv);
      }
    
    // start the audio
    #ifdef SDLUCID_HAS_LIBMIKMOD
    audio_manager audio;

    ifstream mod_file("vgademo.mod",OPEN_MODE);
    audio.addStream(new audio_mod_stream(mod_file));
    
    audio.play();
    #endif // SDLUCID_HAS_LIBMIKMOD

    // main loop
    SDL_Event event;
    bool event_available;
    double sine_argument_1 = 0,sine_argument_2 = 0,sine_argument_3 = 0;
    float scroller_x = dpy.width();
    int last_tick_start = SDL_GetTicks();
    
    do {
      float tick_duration = (float) (SDL_GetTicks() - last_tick_start)/1000;
      last_tick_start = SDL_GetTicks();
      
      back_image.blit(dpy,0,0);

      affine_transformation tx;
      tx.identity();
      tx.scale(1.2*sin(sine_argument_2),1.2*sin(sine_argument_3));
      tx.rotate(sin(sine_argument_1) * 0.5 );
      bitmap_with_reference transformed_logo;
      transformed_logo.transformFrom(logo,tx);
      transformed_logo.blit(dpy,dpy.width()/2,dpy.height()/2);
      
      fnt.print(dpy,(TCoordinate) scroller_x,dpy.height()-4,SCROLLER_TEXT,font::ALIGN_BOTTOM);
      
      sine_argument_1 += .8*tick_duration;
      sine_argument_2 += 1.2*tick_duration;
      sine_argument_3 += tick_duration;
      scroller_x -= 40*tick_duration;
      
      if (scroller_x < -ext.width()) 
        scroller_x = dpy.width();
      
      dpy.flip();
      
      #ifdef SDLUCID_HAS_LIBMIKMOD
      audio.tick();
      
      if (audio.size() == 0) {
        cout << "restarting mod" << endl;
        audio.addStream(new audio_mod_stream(mod_file));
	}
      #endif // SDLUCID_HAS_LIBMIKMOD      
      
      event_available = SDL_PollEvent(&event);
      
      // save a screenshot if a mouse button is pressed
      if (event_available && event.type == SDL_MOUSEBUTTONDOWN) {
        ofstream shot_file("shot.png",ios::out | ios::binary);
	SDL_PixelFormat fmt;
	createPixelFormat(fmt,8,3);
	bitmap bmp(fmt,dpy.width(),dpy.height());
	dpy.blit(bmp,0,0);
	bmp.savePNG(shot_file);
        }
    } while (!event_available || event.type != SDL_KEYDOWN);
    }
  catch (exception &ex) {
    cerr << "fatal exception: " << ex.what();
    return 1;
    }
  return 0;
  }




#else
#include <iostream>
using namespace std;
#warning The demo program cannot be compiled without libpng, sorry.
int main() {
  cout << "The demo program cannot be compiled without libpng, sorry." << endl;
  }
#endif
