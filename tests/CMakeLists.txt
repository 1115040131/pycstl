# 添加依赖库
if(NOT TARGET GTest)
    find_package(GTest CONFIG REQUIRED)
endif()

set(LIBS
    ${PROJECT_NAME}
    GTest::gtest
    GTest::gtest_main
)

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# #########################################
# "make test_XYZ"
# #########################################
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE ${LIBS})

    # 添加测试
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach(test_source ${SYLAR_TEST_SOURCES})
