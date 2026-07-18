add_executable(spagyrist_tests
  test_candidate.cpp
  test_candidate_text.cpp
  test_document.cpp
  test_matcher.cpp
  test_output.cpp
  test_ranking.cpp
  test_selector.cpp
  test_renderer.cpp
  test_main.cpp
)

target_link_libraries(spagyrist_tests
  PRIVATE
    spagyrist::spagyrist
)

target_compile_features(spagyrist_tests PRIVATE cxx_std_20)

target_compile_options(spagyrist_tests
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

add_test(NAME spagyrist_tests COMMAND spagyrist_tests)
