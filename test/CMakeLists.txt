file(GLOB XG_TEST_FILES "*.cpp")

foreach(TEST_FILENAME ${XG_TEST_FILES})
    get_filename_component(TEST_TARGET ${TEST_FILENAME} NAME_WE)
    add_executable(${TEST_TARGET} ${TEST_FILENAME})
    target_link_libraries(${TEST_TARGET} xg)
endforeach(TEST_FILENAME)
