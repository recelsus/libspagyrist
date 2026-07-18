#include "test_support.hpp"

#include <array>
#include <iostream>

void run_builtin_state_tests();
void run_builtin_view_tests();
void run_candidate_tests();
void run_candidate_text_tests();
void run_document_tests();
void run_help_tests();
void run_matcher_tests();
void run_output_tests();
void run_ranking_tests();
void run_renderer_tests();
void run_runtime_tests();
void run_selector_tests();
void run_terminal_tests();

int main()
{
    const std::array tests{
        test_case{"builtin_state", run_builtin_state_tests},
        test_case{"builtin_view", run_builtin_view_tests},
        test_case{"candidate", run_candidate_tests},
        test_case{"candidate_text", run_candidate_text_tests},
        test_case{"document", run_document_tests},
        test_case{"help", run_help_tests},
        test_case{"matcher", run_matcher_tests},
        test_case{"output", run_output_tests},
        test_case{"ranking", run_ranking_tests},
        test_case{"renderer", run_renderer_tests},
        test_case{"runtime", run_runtime_tests},
        test_case{"selector", run_selector_tests},
        test_case{"terminal", run_terminal_tests},
    };

    for (const auto& test : tests) {
        test.run();
        std::cout << "ok: " << test.name << '\n';
    }

    return 0;
}
