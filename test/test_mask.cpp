#include <cairo/cairo.h>

#include <string>
#include <algorithm>

void draw_gradient1(cairo_t *cr)
{
  cairo_pattern_t *pat1;
  pat1 = cairo_pattern_create_linear(0.0, 0.0,  350.0, 350.0);

  double j;
  int count = 1;
  for ( j = 0.1; j < 1; j += 0.1 ) {
      if (( count % 2 ))  {
          cairo_pattern_add_color_stop_rgb(pat1, j, 0, 0, 0);
      } else {
          cairo_pattern_add_color_stop_rgb(pat1, j, 1, 0, 0);
      }
   count++;
  }

  cairo_rectangle(cr, 20, 20, 300, 100);
  cairo_set_source(cr, pat1);
  cairo_fill(cr);

  cairo_pattern_destroy(pat1);
}


int main(int argc, char *argv[]) {


    // create source surface

    cairo_surface_t *osurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 500, 500) ;
    cairo_t *ocr = cairo_create(osurf) ;

    cairo_set_source_rgba(ocr, 0, 0, 0, 0) ;
    cairo_paint(ocr) ;

    cairo_set_source_rgba(ocr, 0, 1, 0, 1) ;
    cairo_rectangle(ocr, 20, 20, 200, 200) ;
    cairo_fill(ocr) ;

    // create mask surface
    cairo_surface_t *msurf = cairo_surface_create_similar(osurf, CAIRO_CONTENT_COLOR_ALPHA, 350, 250) ;
    cairo_t *mcr = cairo_create(msurf) ;

 //   cairo_set_source_rgba(mcr, 0, 0, 0, 0) ;
 //   cairo_paint(mcr) ;

    draw_gradient1(mcr);

    cairo_surface_write_to_png(msurf, "/tmp/mask.png") ;

    // create ouput surface
    cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 500, 500) ;
    cairo_t *cr = cairo_create(surf) ;

    cairo_set_source_surface(cr, osurf, 0, 0);
    cairo_mask_surface(cr, msurf, -60, 0) ;
    cairo_fill(cr) ;

    cairo_surface_destroy(msurf) ;


    cairo_surface_write_to_png(surf, "/tmp/oo.png") ;

}
