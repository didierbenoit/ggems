file(GLOB TOOLS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_library(ggems_tools STATIC ${TOOLS_SOURCES})

target_include_directories(ggems_tools PUBLIC ${PROJECT_SOURCE_DIR}/include)
