#include "test_support.hpp"

#include <array>
#include <iostream>

void run_candidate_tests();
void run_candidate_text_tests();
void run_document_tests();
void run_matcher_tests();
void run_output_tests();
void run_ranking_tests();
void run_renderer_tests();
void run_selector_tests();

int main()
{
    const std::array tests{
        test_case{"candidate", run_candidate_tests},
        test_case{"candidate_text", run_candidate_text_tests},
        test_case{"document", run_document_tests},
        test_case{"matcher", run_matcher_tests},
        test_case{"output", run_output_tests},
        test_case{"ranking", run_ranking_tests},
        test_case{"renderer", run_renderer_tests},
        test_case{"selector", run_selector_tests},
    };

    for (const auto& test : tests) {
        test.run();
        std::cout << "ok: " << test.name << '\n';
    }

    return 0;
}
