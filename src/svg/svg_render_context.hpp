#ifndef __XG_SVG_RENDER_CONTEXT_HPP__
#define __XG_SVG_RENDER_CONTEXT_HPP__

#include <xg/canvas.hpp>
#include "svg_dom.hpp"

namespace xg {
namespace svg {

enum class RenderingMode { Display, BoundingBox, Cliping } ;

class RenderingContext {

public:

      RenderingContext(Canvas &canvas): canvas_(canvas), rendering_mode_(RenderingMode::Display) {}

      void pushState(const Style &) ;
      void popState() ;
      void pushTransform(const Transform &) ;
      void popTransform() ;

      void preRenderShape(const Style &s, const Transform &tr) ;
      void postRenderShape() ;
      void renderShape() ;

      float toPixels(const Length &l, LengthDirection dir, bool scale_to_viewport = true) ;

      void extentBoundingBox(double x1, double x2, double y1, double y2) ;

      void populateRefs(const svg::ElementPtr &root)  ;
      ElementPtr lookupRef(const std::string &name) ;

      void setFilePath(const std::string &path) {
          file_path_ = path ;
      }

      Canvas &canvas_ ;
      std::deque<Style> states_ ;
      std::deque<Transform> transforms_ ;
      std::deque<ViewBox> view_boxes_ ;

      ViewBox obbox_  ;
      RenderingMode rendering_mode_ ;
      std::map<std::string, ElementPtr> refs_ ;
      Matrix2d view2dev_ ;

      double ctx, cty ;
      std::string file_path_ ;
      float doc_width_hint_, doc_height_hint_ ;
      float dpi_x_ = 92, dpi_y_ = 92 ;

};

class RenderVisitor: public Visitor {
public:
    RenderVisitor(RenderingContext &ctx): ctx_(ctx) {}

    void visit(SVGElement &) override ;
    void visit(CircleElement &) override ;
    void visit(LineElement &) override ;
    void visit(PolygonElement &) override ;
    void visit(PolylineElement &) override ;
    void visit(PathElement &) override ;
    void visit(RectElement &) override ;
    void visit(EllipseElement &) override ;
    void visit(DefsElement &) override ;
    void visit(GroupElement &) override ;
    void visit(SymbolElement &) override ;
    void visit(UseElement &) override ;
    void visit(ClipPathElement &) override ;
    void visit(ImageElement &) override ;
    void visit(TextElement &) override ;
    void visit(LinearGradientElement &) override ;
    void visit(RadialGradientElement &) override ;
    void visit(PatternElement &) override ;
    void visit(TextSpanElement &) override ;
    void visit(StyleElement &) override ;

    RenderingContext &ctx_ ;

};

}
}






























#endif
