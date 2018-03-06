SET ( XG_FILES
    ${SRC_ROOT}/path.cpp
    ${SRC_ROOT}/pen.cpp
    ${SRC_ROOT}/color.cpp
    ${SRC_ROOT}/image.cpp

    ${SRC_ROOT}/backends/cairo/canvas.cpp
    ${SRC_ROOT}/backends/cairo/text_layout.cpp
    ${SRC_ROOT}/backends/cairo/text_layout.hpp
    ${SRC_ROOT}/backends/cairo/font_manager.cpp
    ${SRC_ROOT}/backends/cairo/font_manager.hpp
    ${SRC_ROOT}/backends/cairo/scrptrun.cpp
    ${INCLUDE_ROOT}/backends/cairo/canvas.hpp

    ${INCLUDE_ROOT}/font.hpp
    ${INCLUDE_ROOT}/path.hpp
    ${INCLUDE_ROOT}/canvas.hpp
    ${INCLUDE_ROOT}/image.hpp
    ${INCLUDE_ROOT}/vector.hpp
    ${INCLUDE_ROOT}/rectangle.hpp
    ${INCLUDE_ROOT}/xform.hpp

    ${INCLUDE_ROOT}/util/variant.hpp
    ${INCLUDE_ROOT}/util/dictionary.hpp

    ${SRC_ROOT}/util/dictionary.cpp
    ${SRC_ROOT}/util/strings.cpp

    ${SRC_ROOT}/svg/svg_parser.cpp
    ${SRC_ROOT}/svg/svg_parser.hpp
    ${SRC_ROOT}/svg/svg_href_resolver.cpp
    ${SRC_ROOT}/svg/svg_href_resolver.hpp
    ${SRC_ROOT}/svg/svg_document.cpp
    ${SRC_ROOT}/svg/svg_dom.cpp
    ${SRC_ROOT}/svg/svg_length.cpp
    ${SRC_ROOT}/svg/svg_length.hpp
    ${SRC_ROOT}/svg/svg_style.cpp
    ${SRC_ROOT}/svg/svg_style.hpp
    ${SRC_ROOT}/svg/svg_parse_util.cpp
    ${SRC_ROOT}/svg/svg_parse_util.hpp
    ${SRC_ROOT}/svg/svg_render_context.cpp
    ${SRC_ROOT}/svg/svg_render_context.hpp

    ${INCLUDE_ROOT}/svg_document.hpp
)

add_library(xg SHARED ${XG_FILES})

TARGET_LINK_LIBRARIES(xg
    ${Boost_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${HARFBUZZ_LIBRARIES}
    ${ICU_LIBRARIES}
    ${EXPAT_LIBRARIES}
)
