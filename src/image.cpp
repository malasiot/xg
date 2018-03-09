#include <xg/image.hpp>

#include <png.h>
#include <cassert>
#include <sstream>

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

class PNGStringReader {

public:

    PNGStringReader(const string &str): data_(str) { }

    static void callback(png_structp  png_ptr, png_bytep data, png_size_t length) {
        PNGStringReader *p = (PNGStringReader *)png_get_io_ptr(png_ptr);

        p->data_.read((char *)data, length) ;
     //   p->data_.append((char *)data, length) ;
    }

    istringstream data_ ;
    uint idx_ ;

};


class PNGFileReader {

public:

    PNGFileReader(FILE *file): file_(file) {}

    static void callback(png_structp  png_ptr, png_bytep data, png_size_t length) {
        PNGFileReader *p = (PNGFileReader *)png_get_io_ptr(png_ptr);
        fread((char *)data, length, 1, p->file_) ;
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
        } else {
            png_bytep data = new png_byte [width_ * 3], dst = data ;
            unsigned char *src = (unsigned char *)p ;
            for( int col = 0 ; col < width_ ; col++, src += 3 ) {
                *dst++ = src[0] ;
                *dst++ = src[1] ;
                *dst++ = src[2] ;
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

template <typename R>
Image Image::png_read(R &reader) {

    unsigned int sig_read = 0;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_bytep *row_pointers = nullptr ;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,  NULL, NULL, NULL);

    if ( png_ptr == NULL ) return Image() ;

    png_infop info_ptr = png_create_info_struct((png_structp)png_ptr);

    if ( info_ptr == nullptr ) {
        png_destroy_read_struct((png_structpp)&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return Image() ;
    }

    if (setjmp(png_jmpbuf((png_structp)png_ptr)))  {
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct((png_structpp)&png_ptr, (png_infopp)&info_ptr, png_infopp_NULL);
        if ( row_pointers ) delete [] row_pointers ;
        return Image() ;
    }

    png_set_read_fn(png_ptr, &reader, R::callback);

//    png_init_io((png_structp)png_ptr, fp);

    /* If we have already read some of the signature */
    png_set_sig_bytes((png_structp)png_ptr, sig_read);
    png_read_info((png_structp)png_ptr, (png_infop)info_ptr);

    png_get_IHDR((png_structp)png_ptr, (png_infop)info_ptr, &width, &height, &bit_depth, &color_type,
                 &interlace_type, int_p_NULL, int_p_NULL);

    if ( color_type & PNG_COLOR_TYPE_PALETTE )
        png_set_palette_to_rgb(png_ptr);

    if (color_type & PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);

    if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

    if ( bit_depth < 8 ) png_set_packing(png_ptr);

    if ( color_type == PNG_COLOR_TYPE_RGB_ALPHA )  png_set_swap_alpha(png_ptr);

    if ( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
             png_set_gray_to_rgb(png_ptr);

    ImageFormat fmt = ( color_type & PNG_COLOR_MASK_ALPHA ) ? ImageFormat::ARGB32 : ImageFormat::RGB24 ;

    Image res(width, height, fmt) ;

    row_pointers = new png_bytep [height];

    char *p = res.pixels() ;

    for (int row = 0; row < height; row++, p += res.stride()  )
         row_pointers[row] = (png_bytep)p ;

    png_read_image((png_structp)png_ptr, row_pointers);
    png_read_end((png_structp)png_ptr, (png_infop)info_ptr);

    delete [] row_pointers ;

    png_destroy_read_struct((png_structpp)&png_ptr, (png_infopp)&info_ptr, png_infopp_NULL);

    return res ;

}

Image Image::loadPNG(const string &filename) {
    FILE *fp = fopen(filename.c_str(), "rb") ;
    if ( !fp ) return Image() ;
    PNGFileReader reader(fp) ;
    return png_read(reader) ;
}

Image Image::loadPNGBuffer(const string &buffer)
{
    PNGStringReader reader(buffer) ;
    return png_read(reader) ;

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
