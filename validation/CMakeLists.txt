# Random number validation
add_subdirectory(random)

# Radioactivity validation
add_subdirectory(radioactivity)

# Validation build target
add_custom_target(validation DEPENDS ggems_random_stream_generator
                                     ggems_validate_f18_spectrum)
