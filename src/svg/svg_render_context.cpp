#include "svg_render_context.hpp"

namespace xg {
namespace svg {

float RenderingContext::toPixels(const Length &l, LengthDirection dir, bool scale_to_viewport = true) {
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

    /*
    st.visit<StyleAttributeType::Fill>([&](){
        style.setAttribute<Paint::Fill
    }) ;

    for( int i=0 ; i<st.flags_.size() ; i++ )
    {
        Style::Flag flag = st.flags_[i] ;

        if ( flag == Style::FillState )
        {
            style.fill_paint_type_ = st.fill_paint_type_ ;
            if ( style.fill_paint_type_ == Style::SolidColorPaint )
              style.fill_paint_.clr_ = st.fill_paint_.clr_ ;
            else if ( style.fill_paint_type_ == Style::PaintServerPaint )
                style.fill_paint_.paint_server_id_ = strdup(st.fill_paint_.paint_server_id_) ;

        }
        else if ( flag == Style::FillOpacityState )
        {
            style.fill_opacity_ = st.fill_opacity_ ;
        }
        else if ( flag == Style::FillRuleState )
        {
            style.fill_rule_ = st.fill_rule_ ;
        }
        else if ( flag == Style::StrokeState )
        {
            style.stroke_paint_type_ = st.stroke_paint_type_ ;
            if ( style.stroke_paint_type_ == Style::SolidColorPaint )
              style.stroke_paint_.clr_ = st.stroke_paint_.clr_ ;
            else if ( style.stroke_paint_type_ == Style::PaintServerPaint )
                style.stroke_paint_.paint_server_id_ = strdup(st.stroke_paint_.paint_server_id_) ;

        }
        else if ( flag == Style::StrokeWidthState )
        {
            style.stroke_width_ = st.stroke_width_ ;
        }
        else if ( flag == Style::StrokeDashArrayState )
        {
            style.dash_array_ = st.dash_array_ ;
            style.solid_stroke_ = st.solid_stroke_ ;
        }
        else if ( flag == Style::StrokeOpacityState )
        {
            style.stroke_opacity_ = st.stroke_opacity_ ;
        }
        else if ( flag == Style::OpacityState )
        {
            style.opacity_ = st.opacity_ ;
        }
        else if ( flag == Style::StrokeDashOffsetState )
        {
            style.dash_offset_ = st.dash_offset_ ;
        }
        else if ( flag == Style::StrokeLineCapState )
        {
            style.line_cap_ = st.line_cap_ ;
        }
        else if ( flag == Style::StrokeLineJoinState )
        {
            style.line_join_ = st.line_join_ ;
        }
        else if ( flag == Style::DisplayState )
        {
            style.display_mode_ = st.display_mode_ ;
        }
        else if ( flag == Style::VisibilityState )
        {
            style.visibility_mode_ = st.visibility_mode_ ;
        }
        else if ( flag == Style::FontFamilyState )
        {
            style.font_family_ = st.font_family_ ;
        }
        else if ( flag == Style::FontSizeState )
        {
            style.font_size_ = st.font_size_ ;
        }
        else if ( flag == Style::FontStyleState )
        {
            style.font_style_ = st.font_style_ ;
        }
        else if ( flag == Style::FontWeightState )
        {
            style.font_weight_ = st.font_weight_ ;
        }
        else if ( flag == Style::TextDecorationState )
        {
            style.text_decoration_ = st.text_decoration_ ;
        }
        else if ( flag == Style::TextAnchorState )
        {
            style.text_anchor_ = st.text_anchor_ ;
        }
        else if ( flag == Style::TextRenderingState )
        {
            style.text_rendering_quality_ = st.text_rendering_quality_ ;
        }
        else if ( flag == Style::ShapeRenderingState )
        {
            style.shape_rendering_quality_ = st.shape_rendering_quality_ ;
        }
        else if ( flag == Style::ClipPathState )
        {
            style.clip_path_id_ = st.clip_path_id_ ;
        }
    }
    */
}


void RenderingContext::popState() {
    states_.pop_back() ;
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

void RenderVisitor::visit(RectElement &)
{

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
