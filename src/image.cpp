#include <xg/image.hpp>

#include <png.h>
#include <cassert>
using namespace std ;
namespace xg {

class PNGStringWriter {

public:

    PNGStringWriter(string &str): data_(str) {}

    static void callback(png_structp  png_ptr, png_bytep data, png_size_t length) {
        PNGStringWriter *p = (PNGStringWriter *)png_get_io_ptr(png_ptr);
        p->data_.append((char *)data, length) ;
    }

    static void flush(png_structp png_ptr) {

    }

    string &data_ ;
};


class PNGFileWriter {

public:

    PNGFileWriter(FILE *file): file_(file) {}

    static void callback(png_structp  png_ptr, png_bytep data, png_size_t length) {
        PNGFileWriter *p = (PNGFileWriter *)png_get_io_ptr(png_ptr);
        fwrite((char *)data, length, 1, p->file_) ;
    }

    static void flush(png_structp png_ptr) {
        PNGFileWriter *p = (PNGFileWriter *)png_get_io_ptr(png_ptr);
        fflush(p->file_) ;
    }

    FILE *file_ ;
};

template <typename W>
bool Image::png_write(W &writer) {
    assert(pixels_) ;
    png_uint_32 width, height;
    int bit_depth = 8, color_type ;

    if ( format_ == ImageFormat::RGB24 )
        color_type = PNG_COLOR_TYPE_RGB ;
    else if ( format_ == ImageFormat::ARGB32 )
        color_type = PNG_COLOR_TYPE_RGBA ;

    bit_depth = 8 ;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if ( !png_ptr ) return false ;

    /* Allocate/initialize the image information data.  REQUIRED */
    png_infop info_ptr = png_create_info_struct((png_structp)png_ptr);

    if ( !info_ptr ) {
        //  png_destroy_write_struct((png_structpp)&png_ptr,  png_infopp_NULL);
        png_destroy_write_struct((png_structpp)&png_ptr,  NULL);
        return false ;
    }

    /* Set error handling.  REQUIRED if you aren't supplying your own
      * error handling functions in the png_create_write_struct() call.
      */
    if (setjmp(png_jmpbuf((png_structp)png_ptr))) {
        png_destroy_write_struct((png_structpp)&png_ptr, (png_infopp)&info_ptr);
        return false;
    }

    /* set up the output control if you are using standard C streams */
 //   png_init_io((png_structp)png_ptr, fp);
    png_set_write_fn(png_ptr, &writer, W::callback, W::flush);

    png_set_IHDR((png_structp)png_ptr, (png_infop)info_ptr, width_, height_, bit_depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info((png_structp)png_ptr, (png_infop)info_ptr);

    png_bytep *row_pointers = new png_bytep [height_];

    char *p = (char *)pixels_.get() ;

    for (int row = 0; row < height_; row++, p += stride_ ) {
        if ( format_ == ImageFormat::ARGB32 ) {
            png_bytep data = new png_byte [width_ * 4], dst = data ;
            unsigned char *src = (unsigned char *)p ;
            for( int col = 0 ; col < width_ ; col++, src += 4 ) {
                *dst++ = src[2] ;
                *dst++ = src[1] ;
                *dst++ = src[0] ;
                *dst++ = src[3] ;
            }

            row_pointers[row] = (png_bytep)data ;
        }
    }

    png_write_image((png_structp)png_ptr, row_pointers);
    /* It is REQUIRED to call this to finish writing the rest of the file */
    png_write_end((png_structp)png_ptr, (png_infop)info_ptr);

    for (int row = 0; row < height_; row++)
        delete [] row_pointers[row] ;

    delete [] row_pointers ;

    /* clean up after the write, and free any memory allocated */
    png_destroy_write_struct((png_structpp)&png_ptr, (png_infopp)&info_ptr);

    return true ;
}

bool Image::saveToPNG(const string &filename) {
    FILE *fp = fopen(filename.c_str(), "wb") ;
    if ( !fp ) return false ;
    PNGFileWriter w(fp) ;
    return png_write(w) ;
}

bool Image::saveToPNGBuffer(string &buffer) {
    PNGStringWriter w(buffer) ;
    return png_write(w) ;
}

inline static unsigned bytes_per_line(unsigned width, unsigned bit_depth, unsigned spp) {
   return (((( width * bit_depth * spp ) + 63) & ~63) >> 3) ;
}

Image::Image(unsigned int width, unsigned int height, ImageFormat fmt): width_(width), height_(height), format_(fmt)
{
    switch( format_) {
    case ImageFormat::RGB24:
        stride_ = bytes_per_line(width_, 8, 3) ;
        break ;
    case ImageFormat::ARGB32:
        stride_ = bytes_per_line(width_, 8, 4) ;
        break ;
    }

    pixels_.reset(new char [height_ * stride_]) ;
}




}
