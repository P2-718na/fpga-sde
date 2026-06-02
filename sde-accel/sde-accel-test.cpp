#include "accelerator.hpp"

#include <iostream>

static JobPacked make_job(int v)
{
    JobPacked p = 0;

    for(int i=0;i<NUM_INPUTS;i++) {
        p.range(32*i+31,32*i) = v;
    }

    return p;
}

static void print_result(ResultPacked p)
{
    for(int i=0;i<NUM_OUTPUTS;i++) {

        int32_t v =
            p.range(32*i+31,32*i).to_int();

        std::cout << v << " ";
    }

    std::cout << std::endl;
}

int main()
{
    const int N = 16;

    JobPacked jobs[N];
    ResultPacked results[N];

    for(int i=0;i<N;i++) {
        jobs[i] = make_job(i);
    }

    accelerator(
        jobs,
        results,
        N
    );

    for(int i=0;i<N;i++) {

        std::cout
            << "Job "
            << i
            << ": ";

        print_result(results[i]);
    }

    return 0;
}