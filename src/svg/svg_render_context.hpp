#ifndef __XG_SVG_RENDER_CONTEXT_HPP__
#define __XG_SVG_RENDER_CONTEXT_HPP__

#include <xg/canvas.hpp>
#include "svg_dom.hpp"

namespace xg {
namespace svg {

enum class RenderingMode { Display, BoundingBox, Cliping } ;

class RenderingContext {

public:

      RenderingContext(Canvas &canvas): canvas_(canvas), rendering_mode_(RenderingMode::Display) {
          view_boxes_.push_back({0, 0, (float)canvas.width(), (float)canvas.height()}) ;
      }

      void pushState(const Style &) ;
      void popState() ;
      void pushTransform(const Matrix2d &) ;
      void popTransform() ;

      void preRenderShape(Element &e, const Style &s, const Matrix2d &tr, const Rectangle2d &rect) ;
      void setPaint(Element &e) ;
      void postRenderShape() ;

      void applyClipPath(ClipPathElement *e) ;

      float toPixels(const Length &l, LengthDirection dir, bool scale_to_viewport = true) ;

      void extentBoundingBox(double x1, double x2, double y1, double y2) ;

      void populateRefs(const svg::ElementPtr &root)  ;
      ElementPtr lookupRef(const std::string &name) ;

      void setFilePath(const std::string &path) {
          file_path_ = path ;
      }

      void render(SVGElement &)  ;
      void render(CircleElement &) ;
      void render(LineElement &) ;
      void render(PolygonElement &) ;
      void render(PolylineElement &) ;
      void render(PathElement &) ;
      void render(RectElement &) ;
      void render(EllipseElement &) ;
      //void visit(DefsElement &) override ;
      void render(GroupElement &) ;
      void render(SymbolElement &) ;
      void render(UseElement &) ;
     // void visit(ClipPathElement &) override ;
      void render(ImageElement &) ;
      void render(TextElement &) ;
      void render(TextSpanElement &) ;
      //void visit(StyleElement &) override ;

      void renderChildren(const Element &e) ;

      void setShapeAntialias(ShapeQuality aa);
      void setLinearGradientBrush(LinearGradientElement &e, float a) ;
      void setRadialGradientBrush(RadialGradientElement &e, float a) ;
      void setPatternBrush(PatternElement &e, float a) ;
protected:

      Canvas &canvas_ ;
      std::deque<Style> states_ ;
      std::deque<Matrix2d> transforms_ ;
      std::deque<ViewBox> view_boxes_ ;

      Rectangle2d obbox_  ;
      RenderingMode rendering_mode_ ;
      std::map<std::string, ElementPtr> refs_ ;
      Matrix2d view2dev_ ;

      double ctx, cty ;
      std::string file_path_ ;
      float doc_width_hint_, doc_height_hint_ ;
      float dpi_x_ = 92, dpi_y_ = 92 ;

};


}
}






























#endif
