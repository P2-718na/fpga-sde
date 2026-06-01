// tb_accelerator.cpp

#include <iostream>
#include "example.hpp"

#define TEST_JOBS 2

int main() {

    Job jobs[TEST_JOBS];
    Result results[TEST_JOBS];

    // ------------------------------------------------
    // Generate test inputs
    // ------------------------------------------------
    for (int i = 0; i < TEST_JOBS; i++) {

        for (int j = 0; j < NUM_INPUTS; j++) {

            // Example fixed-point values
            jobs[i].in[j] = (fixed_t)(i + j);
        }
    }

    // ------------------------------------------------
    // Run DUT
    // ------------------------------------------------
    accelerator(jobs, results, TEST_JOBS);

    // ------------------------------------------------
    // Check outputs
    // ------------------------------------------------
    int errors = 0;

    for (int i = 0; i < TEST_JOBS; i++) {

        fixed_t expected_sum = 0;

        for (int j = 0; j < NUM_INPUTS; j++) {
            expected_sum += (fixed_t)(i + j);
        }

        std::cout << "Job " << i << std::endl;

        for (int o = 0; o < NUM_OUTPUTS; o++) {

            fixed_t expected = expected_sum + o;
            fixed_t received = results[i].out[o];

            std::cout
                << "  out[" << o << "] = "
                << received.to_float()
                << std::endl;

            if (received != expected) {

                std::cout
                    << "  ERROR: expected "
                    << expected.to_float()
                    << std::endl;

                errors++;
            }
        }
    }

    // ------------------------------------------------
    // Final result
    // ------------------------------------------------
    std::cout << std::endl;

    if (errors == 0) {

        std::cout
            << "TEST PASSED"
            << std::endl;

        return 0;

    } else {

        std::cout
            << "TEST FAILED: "
            << errors
            << " errors"
            << std::endl;

        return 1;
    }
}