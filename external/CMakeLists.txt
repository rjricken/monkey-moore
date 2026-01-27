file(GLOB XML_SRCS "*.cpp")

add_library(monkey-xml STATIC ${XML_SRCS})

target_include_directories(monkey-xml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(monkey-xml PUBLIC _UNICODE)