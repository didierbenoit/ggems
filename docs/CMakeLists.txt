add_subdirectory(Doxygen)
# add_subdirectory(Sphinx)

add_custom_target(docs)
add_dependencies(docs doxygen)
# add_dependencies(docs doxygen sphinx)
