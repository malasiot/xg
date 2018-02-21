SET ( XG_FILES
    ${SRC_ROOT}/path.cpp
    ${SRC_ROOT}/pen.cpp
     ${SRC_ROOT}/color.cpp

    ${SRC_ROOT}/backends/cairo/canvas.cpp
    ${SRC_ROOT}/backends/cairo/scrptrun.cpp
    ${INCLUDE_ROOT}/backends/cairo/canvas.hpp

    ${INCLUDE_ROOT}/font.hpp
    ${INCLUDE_ROOT}/path.hpp
    ${INCLUDE_ROOT}/canvas.hpp
)

add_library(xg SHARED ${XG_FILES})

TARGET_LINK_LIBRARIES(xg
    ${Boost_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${HARFBUZZ_LIBRARIES}
    ${ICU_LIBRARIES}
)
