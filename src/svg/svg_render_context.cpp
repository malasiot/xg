#include "svg_render_context.hpp"

using namespace std ;

namespace xg {
namespace svg {

float RenderingContext::toPixels(const Length &l, LengthDirection dir, bool scale_to_viewport) {
    double factor = 1.0 ;

    static const double default_font_size = 12.0 ;

    switch( l.units() )  {
    case LengthUnitType::CM:
        factor = 1/2.54 ;
        break ;
    case LengthUnitType::MM:
        factor = 1/25.4 ;
        break ;
    case LengthUnitType::PT:
        factor = 1/72.0 ;
        break ;
    case LengthUnitType::PC:
        factor = 1/6.0 ;
        break ;
    }

    switch( l.units() )
    {
    case LengthUnitType::Number:
    case LengthUnitType::PX:
        return l.value() ;
    case LengthUnitType::CM:
    case LengthUnitType::MM:
    case LengthUnitType::IN:
    case LengthUnitType::PT:
    case LengthUnitType::PC:
        if ( dir == LengthDirection::Horizontal  ) return  l.value() * factor * dpi_x_ ;
        else if ( dir == LengthDirection::Horizontal ) return  l.value() * factor * dpi_y_ ;
        else if ( dir == LengthDirection::Absolute ) return  l.value() * factor*sqrt(dpi_x_ * dpi_y_) ;
    case LengthUnitType::EMS:
        return l.value() * default_font_size ;
    case LengthUnitType::EXS:
        return l.value() * default_font_size * 0.5;
    case LengthUnitType::Percentage: {
        float fx = ( scale_to_viewport ) ? view_boxes_.back().width_ : obbox_.width_ ;
        float fy = ( scale_to_viewport ) ? view_boxes_.back().height_ : obbox_.height_ ;

        if ( dir == LengthDirection::Horizontal  ) return  l.value() *  fx ;
        else if ( dir == LengthDirection::Horizontal ) return  l.value() * fy ;
        else if ( dir == LengthDirection::Absolute ) return  l.value() * sqrt(fx * fy) ;
    }
    }

    return 0 ;
}

void RenderVisitor::visit(SVGElement &e) {
    ctx_.pushState(e.style_) ;

    float xx, yy, sw, sh ;

    if ( e.x_.unknown() ) xx = 0 ;
    else xx = ctx_.toPixels(e.x_, LengthDirection::Horizontal) ;

    if ( e.y_.unknown() ) yy = 0 ;
    else yy = ctx_.toPixels(e.y_, LengthDirection::Vertical) ;

    if ( e.width_.unknown() ) {
        if ( e.parent_ )
            sw = ctx_.toPixels(1.0_perc, LengthDirection::Horizontal) ;
        else sw = ctx_.doc_width_hint_ ;
    }
    else
        sw = ctx_.toPixels(e.width_, LengthDirection::Horizontal) ;

    if ( e.height_.unknown() ) {
        if ( e.parent_ )
            sh = ctx_.toPixels(1.0_perc, LengthDirection::Vertical) ;
        else sh = ctx_.doc_height_hint_ ;
    }
    else
       sh = ctx_.toPixels(e.height_, LengthDirection::Horizontal) ;


    ViewBox vbox = e.view_box_ ;

    if ( vbox.width_ == 0 ) vbox.width_ = sw ;
    if ( vbox.height_ == 0 ) vbox.height_ = sh ;

    ctx_.view_boxes_.push_back(vbox) ;

    Matrix2d trs = e.preserve_aspect_ratio_.getViewBoxTransform(sw, sh, vbox.width_, vbox.height_, vbox.x_, vbox.y_) ;

    ctx_.view2dev_ = trs ;

    ctx_.canvas_.save() ;
    ctx_.canvas_.setTransform(trs) ;

    visitChildren(&e);

    ctx_.canvas_.restore() ;
    ctx_.popState();
    ctx_.view_boxes_.pop_back();
}


void RenderingContext::pushState(const Style &st)
{
    // inherit last state
    if ( states_.empty() ) states_.push_back(Style()) ;
    else states_.push_back(states_.back()) ;

    Style &style = states_.back() ;

    //style.resetNonInheritable()

    style.extend(st) ;
}


void RenderingContext::popState() {
    states_.pop_back() ;
}

void RenderingContext::pushTransform(const Matrix2d &trs) {
    transforms_.push_back(trs) ;
}

void RenderingContext::popTransform() {
    transforms_.pop_back() ;
}

ElementPtr RenderingContext::lookupRef(const std::string &name) {
    auto it = refs_.find(name) ;

    if ( it == refs_.end() ) return nullptr ;
    else return (*it).second ;
}

void RenderingContext::populateRefs(const ElementPtr &root) {
    if ( root->getType() == Element::DocumentElement )
    {
        Document *pElem = dynamic_cast<Document *>(root.get()) ;

        for( int i=0 ; i<pElem->children_.size() ; i++ )
        {
            ElementPtr el = pElem->children_[i] ;

            string id = el->id_ ;

            if ( !id.empty() ) refs_['#' + id] = el ;

            if ( el->getType() == Element::DocumentElement ||
                 el->getType() == Element::GroupElement ||
                 el->getType() == Element::DefsElement ) populateRefs(el) ;
        }
    }
    else if ( root->getType() == Element::GroupElement )
    {
        Group *pElem = dynamic_cast<Group *>(root.get()) ;

        for( int i=0 ; i<pElem->children_.size() ; i++ )
        {
            ElementPtr el = pElem->children_[i] ;

            string id = el->id_ ;

            if ( !id.empty() ) refs_['#' + id] = el ;

            if ( el->getType() == Element::DocumentElement ||
                 el->getType() == Element::GroupElement ||
                 el->getType() == Element::DefsElement ) populateRefs(el) ;
        }
    }
    else if ( root->getType() == Element::DefsElement )
    {
        Defs *pElem = dynamic_cast<Defs *>(root.get()) ;

        for( int i=0 ; i<pElem->children_.size() ; i++ )
        {
            ElementPtr el = pElem->children_[i] ;

            string id = el->id_ ;

            if ( !id.empty() ) refs_['#' + id] = el ;

            if ( el->getType() == Element::DocumentElement ||
                 el->getType() == Element::GroupElement ||
                 el->getType() == Element::DefsElement ) populateRefs(el) ;
        }
    }
}

void RenderingContext::preRenderShape(const Style &s, const Matrix2d &t)
{
    pushState(s) ;
    pushTransform(t) ;

    Style &st = states_.back() ;

    canvas_.save() ;
    canvas_.setTransform(t) ;

    if ( rendering_mode_ == RenderingMode::Display ) {
        string cp_id = st.getClipPath() ;
        if ( !cp_id.empty() ) {
          ElementPtr p = lookupRef(cp_id) ;

          if ( p ) {
              auto e = std::dynamic_pointer_cast<ClipPathElement>(p) ;
          }
      }
    }
}

void RenderingContext::postRenderShape()
{

}

void RenderingContext::renderShape()
{

}

void RenderVisitor::visit(CircleElement &)
{

}

void RenderVisitor::visit(LineElement &)
{

}

void RenderVisitor::visit(PolygonElement &)
{

}

void RenderVisitor::visit(PolylineElement &)
{

}

void RenderVisitor::visit(PathElement &)
{

}

void RenderVisitor::visit(RectElement &rect)
{
    ctx_.preRenderShape(rect.style_, rect.trans_.m_) ;

    double rxp = ctx_.toPixels(rect.rx_, LengthDirection::Horizontal) ;
    double ryp = ctx_.toPixels(rect.ry_, LengthDirection::Vertical) ;
    double xp = ctx_.toPixels(rect.x_, LengthDirection::Horizontal) ;
    double yp = ctx_.toPixels(rect.y_, LengthDirection::Vertical) ;
    double wp = ctx_.toPixels(rect.width_, LengthDirection::Horizontal) ;
    double hp = ctx_.toPixels(rect.height_, LengthDirection::Vertical) ;

    if (rxp > fabs (wp / 2.)) rxp = fabs (wp / 2.);
    if (ryp > fabs (hp / 2.)) ryp = fabs (hp / 2.);

    if (rxp == 0) rxp = ryp;
    else if (ryp == 0) ryp = rxp ;

    if ( wp != 0.0 && hp != 0.0 ) {
        if ( rxp == 0.0 || ryp == 0.0 )
            ctx_.canvas_.drawRect(xp, yp, wp, hp) ;
        else {
            Path rrect ;
            rrect.moveTo(xp + rxp, yp) ;
            rrect.lineTo(xp + wp - rxp, yp) ;
            rrect.arcTo(rxp, ryp, M_PI/2.0, false, false, xp + wp, yp + ryp) ;
            rrect.lineTo(xp + wp, yp + hp - ryp) ;
            rrect.arcTo(rxp, ryp, M_PI/2.0, false, false, xp + wp -rxp, yp + hp) ;
            rrect.lineTo(xp + rxp, yp + hp) ;
            rrect.arcTo(rxp, ryp, M_PI/2.0, false, false, xp, yp + hp - ryp) ;
            rrect.lineTo(xp, yp + ryp) ;
            rrect.arcTo(rxp, ryp, M_PI/2.0, false, false, xp + rxp, yp) ;

            ctx_.canvas_.drawPath(rrect) ;
        }
    }

    ctx_.renderShape() ;
    ctx_.postRenderShape() ;
}

void RenderVisitor::visit(EllipseElement &)
{

}

void RenderVisitor::visit(DefsElement &)
{

}

void RenderVisitor::visit(GroupElement &)
{

}

void RenderVisitor::visit(SymbolElement &)
{

}

void RenderVisitor::visit(UseElement &)
{

}

void RenderVisitor::visit(ClipPathElement &)
{

}

void RenderVisitor::visit(ImageElement &)
{

}

void RenderVisitor::visit(TextElement &)
{

}

void RenderVisitor::visit(LinearGradientElement &)
{

}

void RenderVisitor::visit(RadialGradientElement &)
{

}

void RenderVisitor::visit(PatternElement &)
{

}

void RenderVisitor::visit(TextSpanElement &)
{

}

void RenderVisitor::visit(StyleElement &)
{

}



}
}
