add_executable(spagyrist_fixed_client
  fixed_client.cpp
)

target_link_libraries(spagyrist_fixed_client
  PRIVATE
    spagyrist::spagyrist
)

target_compile_features(spagyrist_fixed_client PRIVATE cxx_std_20)

target_compile_options(spagyrist_fixed_client
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)
