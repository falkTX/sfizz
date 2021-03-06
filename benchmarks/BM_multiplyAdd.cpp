// Copyright (c) 2019, Paul Ferrand
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <benchmark/benchmark.h>
#include <random>
#include <numeric>
#include <vector>
#include <cmath>
#include <iostream>
#include "../sfizz/SIMDHelpers.h"

class MultiplyAdd : public benchmark::Fixture {
public:
  void SetUp(const ::benchmark::State& state) {
    std::random_device rd { };
    std::mt19937 gen { rd() };
    std::uniform_real_distribution<float> dist { 0, 1 };
    input = std::vector<float>(state.range(0));
    output = std::vector<float>(state.range(0));
    gain = std::vector<float>(state.range(0));
    std::fill(output.begin(), output.end(), 1.0f );
    std::generate(gain.begin(), gain.end(), [&]() { return dist(gen); });
    std::generate(input.begin(), input.end(), [&]() { return dist(gen); });
  }

  void TearDown(const ::benchmark::State& state [[maybe_unused]]) {

  }

  std::vector<float> gain;
  std::vector<float> input;
  std::vector<float> output;
};

BENCHMARK_DEFINE_F(MultiplyAdd, Straight)(benchmark::State& state) {
    for (auto _ : state)
    {
        for (int i = 0; i < state.range(0); ++i)
            output[i] += gain[i] * input[i];
    }
}

BENCHMARK_DEFINE_F(MultiplyAdd, Scalar)(benchmark::State& state) {
    for (auto _ : state)
    {
        multiplyAdd<float, false>(gain, input, absl::MakeSpan(output));
    }
}

BENCHMARK_DEFINE_F(MultiplyAdd, SIMD)(benchmark::State& state) {
    for (auto _ : state)
    {
        multiplyAdd<float, true>(gain, input, absl::MakeSpan(output));
    }
}

BENCHMARK_DEFINE_F(MultiplyAdd, Scalar_Unaligned)(benchmark::State& state) {
    for (auto _ : state)
    {
        multiplyAdd<float, false>(absl::MakeSpan(gain).subspan(1), absl::MakeSpan(input).subspan(1), absl::MakeSpan(output).subspan(1));
    }
}

BENCHMARK_DEFINE_F(MultiplyAdd, SIMD_Unaligned)(benchmark::State& state) {
    for (auto _ : state)
    {
        multiplyAdd<float, true>(absl::MakeSpan(gain).subspan(1), absl::MakeSpan(input).subspan(1), absl::MakeSpan(output).subspan(1));
    }
}

BENCHMARK_REGISTER_F(MultiplyAdd, Straight)->RangeMultiplier(4)->Range(1 << 2, 1 << 12);
BENCHMARK_REGISTER_F(MultiplyAdd, Scalar)->RangeMultiplier(4)->Range(1 << 2, 1 << 12);
BENCHMARK_REGISTER_F(MultiplyAdd, SIMD)->RangeMultiplier(4)->Range(1 << 2, 1 << 12);
BENCHMARK_REGISTER_F(MultiplyAdd, Scalar_Unaligned)->RangeMultiplier(4)->Range(1 << 2, 1 << 12);
BENCHMARK_REGISTER_F(MultiplyAdd, SIMD_Unaligned)->RangeMultiplier(4)->Range(1 << 2, 1 << 12);
BENCHMARK_MAIN();