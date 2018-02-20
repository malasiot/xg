#include <xg/canvas.hpp>
#include <xg/backends/cairo/canvas.hpp>
namespace xg {

class Canvas::Impl {
};

// Create an implementation object
Canvas::Canvas(double w, double h): width_(w), height_(h), impl_(new Impl())
{}

Canvas::~Canvas() = default ;

/*
std::unique_ptr<RenderingContext> CairoCanvas::createContext()
{
    assert( handle_ ) ;
    return std::unique_ptr<RenderingContext>(new CairoRenderingContext(handle_)) ;
}

CairoCanvas::~CairoCanvas() {
    assert(handle_) ;

    cairo_surface_finish (handle_);
    cairo_surface_destroy (handle_);
}

CairoImageCanvas::CairoImageCanvas(double w, double h, double dpi): CairoCanvas(w, h), dpi_(dpi) {
    handle_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h) ;
}

void CairoImageCanvas::saveToPng(const std::string &file_name)  {
    cairo_surface_write_to_png (handle_, file_name.c_str());
}
*/
}
