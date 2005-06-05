// ----------------------------------------------------------------------------
//  Description      : Frontend to libpng.
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#include <memory>
#include <ixlib_array.hh>
#include <sdlucid_png.hh>
#include <csetjmp>




using namespace std;
using namespace ixion;
using namespace png;




// Plain text rendering table -------------------------------------------------
#define TPNG_GENERAL      	"General PNG error"
#define TPNG_INVALIDINFO	"Invalid information requested"




static char *(PNGPlainText[]) =
  { TPNG_GENERAL,TPNG_INVALIDINFO };




// png_exception --------------------------------------------------------------
char *png_exception::getText() const {
  return PNGPlainText[Error];
  }




// Crappy callback stuff ------------------------------------------------------
#define PNG_SETJMP \
  if (setjmp(PngStruct->jmpbuf)) \
    EXPNG_THROWINFO(ECPNG_GENERAL,error_msg)




void png_user_error_fn(png_structp png_ptr,png_const_charp error_msg) {
  png_base::error_msg = error_msg;
  longjmp(png_ptr->jmpbuf,1);
  }




void png_user_warning_fn(png_structp png_ptr,png_const_charp warning_msg){
  png_base::error_msg = warning_msg;
  longjmp(png_ptr->jmpbuf,1);
  }




void png_user_read_data(png_structp png_ptr,png_bytep data,unsigned int length) {
  png_reader *reader = (png_reader *) png_get_io_ptr(png_ptr);
  reader->internalRead(data,length);
  }




void png_user_write_data(png_structp png_ptr,png_bytep data,unsigned int length) {
  png_writer *writer = (png_writer *) png_get_io_ptr(png_ptr);
  writer->internalWrite(data,length);
  }




void png_user_flush_data(png_structp png_ptr) {
  png_writer *writer = (png_writer *) png_get_io_ptr(png_ptr);
  writer->internalFlush();
  }




// png_base -------------------------------------------------------------------
char const *png_base::error_msg = NULL;




png_base::png_base()
  : PngStruct(NULL),Info(NULL),EndInfo(NULL) {
  }




TSize png_base::getChannels() {
  return png_get_channels(PngStruct,Info);
  }




TSize png_base::getRowBytes() {
  return png_get_rowbytes(PngStruct,Info);
  }




TSize png_base::getAlignedRowBytes(TSize align) {
  return ((getRowBytes()+align-1) / align) * align;
  }




TSize png_base::getImageBytes() {
  return getRowBytes()*getHeight();
  }




TSize png_base::getWidth() {
  return png_get_image_width(PngStruct,Info);
  }




TSize png_base::getHeight() {
  return png_get_image_height(PngStruct,Info);
  }




TSize png_base::getBitDepth() {
  return png_get_bit_depth(PngStruct,Info);
  }




int png_base::getColorType() {
  return png_get_color_type(PngStruct,Info);
  }




TSize png_base::getPalette(png_color *&palette) {
  if (!png_get_valid(PngStruct,Info,PNG_INFO_PLTE))
    EXPNG_THROWINFO(ECPNG_INVALIDINFO,"palette")
  int color_count;
  png_get_PLTE(PngStruct,Info,&palette,&color_count);
  return color_count;
  }




double png_base::getGamma() {
  if (!png_get_valid(PngStruct,Info,PNG_INFO_gAMA))
    EXPNG_THROWINFO(ECPNG_INVALIDINFO,"gamma")
  double gamma;
  png_get_gAMA(PngStruct,Info,&gamma);
  return gamma;
  }




void png_base::getSignificantBits(png_color_8p &sigbits) {
  if (!png_get_valid(PngStruct,Info,PNG_INFO_sBIT))
    EXPNG_THROWINFO(ECPNG_INVALIDINFO,"significant bits")
  png_get_sBIT(PngStruct,Info,&sigbits);
  }




TSize png_base::getTransparencyIndexed(png_byte *&transentries) {
  if (!png_get_valid(PngStruct,Info,PNG_INFO_tRNS))
    EXPNG_THROWINFO(ECPNG_INVALIDINFO,"transparency")
  int color_count;
  png_get_tRNS(PngStruct,Info,&transentries,&color_count,NULL);
  return color_count;
  }




TSize png_base::getTransparencyTrueColor(png_color_16p &transvalue) {
  if (!png_get_valid(PngStruct,Info,PNG_INFO_tRNS))
    EXPNG_THROWINFO(ECPNG_INVALIDINFO,"transparency")
  int color_count;
  png_get_tRNS(PngStruct,Info,NULL,&color_count,&transvalue);
  return color_count;
  }




png_color_16 png_base::getBackground() {
  if (!png_get_valid(PngStruct,Info,PNG_INFO_bKGD))
    EXPNG_THROWINFO(ECPNG_INVALIDINFO,"background")
  png_color_16 *background;
  png_get_bKGD(PngStruct,Info,&background);
  return *background;
  }




void png_base::getMetaData(png_meta_data &meta) {
  meta.clear();
  png_textp texts;
  int dummy;
  int text_count = png_get_text(PngStruct,Info,&texts,&dummy);
  if (!texts) return;
  
  for (int i = 0;i<text_count;i++,texts++)
    meta[texts->key] = texts->text;
  }
  
  
  
  
void png_base::setInvertAlpha() {
  png_set_invert_alpha(PngStruct);
  }




void png_base::setPacking() { // use packed-pixel format
  png_set_packing(PngStruct);
  }




void png_base::setFiller(int filler,int flags) {
  png_set_filler(PngStruct,filler,flags);
  }




void png_base::setBGR() {
  png_set_bgr(PngStruct);
  }




void png_base::setPackSwap() { // swap pixel pack order
  png_set_packswap(PngStruct);
  }




void png_base::setInvertMono() {
  png_set_invert_mono(PngStruct);
  }




// png_reader -----------------------------------------------------------------
png_reader::png_reader() {
  PngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,
    png_user_error_fn,png_user_warning_fn);
  
  if (!PngStruct) 
    EXPNG_THROWINFO(ECPNG_GENERAL,"unable to create read struct");

  Info = png_create_info_struct(PngStruct);
  if (!Info) {
    png_destroy_read_struct(&PngStruct,NULL,NULL);
    EXPNG_THROWINFO(ECPNG_GENERAL,"unable to create read info struct");
    }
  
  EndInfo = png_create_info_struct(PngStruct);
  if (!EndInfo) {
    png_destroy_read_struct(&PngStruct,&Info,NULL);
    EXPNG_THROWINFO(ECPNG_GENERAL,"unable to create read end info struct");
    }
    
  png_set_read_fn(PngStruct,this,png_user_read_data);
  }




png_reader::~png_reader() {
  png_destroy_read_struct(&PngStruct,&Info,&EndInfo);
  }




void png_reader::setExpand() { // expand indexed to 8 bpp, add alpha
  png_set_expand(PngStruct);
  }




void png_reader::setStrip16() { // forget about 16 bpc
  png_set_strip_16(PngStruct);
  }




void png_reader::setStripAlpha() { // combine with background
  png_set_strip_alpha(PngStruct);
  }




void png_reader::setSwapAlpha() { // ARGB instead of RGBA
  png_set_swap_alpha(PngStruct);
  }




void png_reader::setGrayToRGB() {
  png_set_gray_to_rgb(PngStruct);
  }




void png_reader::setRGBToGray(int error_action,float red_weight,float green_weight) {
  png_set_rgb_to_gray(PngStruct,error_action,red_weight,green_weight);
  }




void png_reader::setGamma(float screen_gamma,float image_gamma) {
  png_set_gamma(PngStruct,screen_gamma,image_gamma);
  }




void png_reader::setSwap() { // swap byte order
  png_set_swap(PngStruct);
  }




void png_reader::setInterlaceHandling() {
  png_set_interlace_handling(PngStruct);
  }




void png_reader::updateInfo() {
  png_read_update_info(PngStruct,Info);
  }




void png_reader::readInfo() {
  PNG_SETJMP

  png_read_info(PngStruct,Info);
  }
  
  
  
  
void png_reader::readImage(TByte *data,TSize align) {
  readImageRowBytes(data,getAlignedRowBytes(align));
  }




void png_reader::readImageRowBytes(TByte *data,TSize rowbytes) {
  PNG_SETJMP

  TSize height = getHeight();
  if (rowbytes < getRowBytes())
    EXPNG_THROWINFO(ECPNG_GENERAL,"png/argument row bytes mismatch");

  auto_array<TByte *> rowpointers(height);
  for (TSize row = 0;row<height;row++) {
    rowpointers[row] = data;
    data += rowbytes;
    }
  png_read_image(PngStruct,rowpointers.get());
  }




void png_reader::readEnd() {
  PNG_SETJMP

  png_read_end(PngStruct,EndInfo);
  }
  
  
  
  

// png_writer -----------------------------------------------------------------
png_writer::png_writer() {
  PngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,
    png_user_error_fn,png_user_warning_fn);
  
  if (!PngStruct) 
    EXPNG_THROWINFO(ECPNG_GENERAL,"unable to create write struct");

  Info = png_create_info_struct(PngStruct);
  if (!Info) {
    png_destroy_write_struct(&PngStruct,NULL);
    EXPNG_THROWINFO(ECPNG_GENERAL,"unable to create write info struct");
    }
  
  EndInfo = NULL;
    
  png_set_write_fn(PngStruct,this,png_user_write_data,png_user_flush_data);
  }




png_writer::~png_writer() {
  png_destroy_write_struct(&PngStruct,&Info);
  }




void png_writer::setCompressionLevel(int level) {
  png_set_compression_level(PngStruct,level);
  }




void png_writer::setInfo(TSize width,TSize height,TSize bit_depth,
  int color_type,int interlace_type,int compression_type,int filter_type) {
  png_set_IHDR(PngStruct,Info,width,height,bit_depth,color_type,
    interlace_type,compression_type,filter_type);
  }



void png_writer::setPalette(png_color *palette,TSize colcount) {
  png_set_PLTE(PngStruct,Info,palette,colcount);
  }




void png_writer::setGamma(double gamma) {
  png_set_gAMA(PngStruct,Info,gamma);
  }




void png_writer::setSignificantBits(png_color_8 &sig_bits) {
  png_set_sBIT(PngStruct,Info,&sig_bits);
  }




void png_writer::setTransparencyIndexed(png_byte *transentries,TSize color_count) {
  png_set_tRNS(PngStruct,Info,transentries,color_count,NULL);
  }




void png_writer::setTransparencyTrueColor(png_color_16 &transvalue) {
  png_set_tRNS(PngStruct,Info,NULL,1,&transvalue);
  }




void png_writer::setBackground(png_color_16 &background) {
  png_set_bKGD(PngStruct,Info,&background);
  }



void png_writer::setMetaData(png_meta_data const &meta) {
  if (meta.size() == 0) return;
  png_textp texts = new png_text[meta.size()];
  png_textp textrun = texts;

  FOREACH_CONST(first,meta,png_meta_data) {
    textrun->compression = PNG_TEXT_COMPRESSION_NONE;
    textrun->key = const_cast<char *>(first->first.c_str());
    textrun->text = const_cast<char *>(first->second.c_str());
    textrun->text_length = first->second.size();
    
    textrun++;
    }
  png_set_text(PngStruct,Info,texts,meta.size());

  delete texts;
  }
  
  
  
  
void png_writer::flush() {
  png_write_flush(PngStruct);
  }




void png_writer::autoFlushAfter(TSize rows) {
  png_set_flush(PngStruct,rows);
  }




void png_writer::writeInfo() {
  PNG_SETJMP
  
  png_write_info(PngStruct,Info);
  }




void png_writer::writeImage(TByte *data,TSize align) {
  writeImageRowBytes(data,getAlignedRowBytes(align));
  }




void png_writer::writeImageRowBytes(TByte *data,TSize rowbytes) {
  PNG_SETJMP
  
  TSize height = getHeight();
  if (rowbytes < getRowBytes())
    EXPNG_THROWINFO(ECPNG_GENERAL,"png/argument row bytes mismatch")

  auto_array<TByte *> rowpointers(height);
  for (TSize row = 0;row<height;row++) {
    rowpointers[row] = data;
    data += rowbytes;
    }
  png_write_image(PngStruct,rowpointers.get());
  }




void png_writer::writeEnd() {
  PNG_SETJMP
  
  png_write_end(PngStruct,EndInfo);
  }




// png_stream_reader ----------------------------------------------------------
void png_stream_reader::internalRead(png_bytep data,png_uint_32 length) {
  IStream.read((char *) data,length);
  }




// png_stream_writer ----------------------------------------------------------
void png_stream_writer::internalWrite(png_bytep data,png_uint_32 length) {
  OStream.write((char *) data,length);
  }




void png_stream_writer::internalFlush() {
  OStream.flush();
  }
