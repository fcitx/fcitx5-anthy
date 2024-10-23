add_custom_target(copy-im DEPENDS anthy.conf.in-fmt)
add_custom_command(TARGET copy-im COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/src/anthy.conf ${CMAKE_CURRENT_BINARY_DIR}/anthy.conf)
