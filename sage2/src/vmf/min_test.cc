// Copyright 2021 the deepx authors.
// Author: Yafei Zhang (kimmyzhang@tencent.com)
//

#include <sage2/vmf.h>
#include "benchmark.h"
#include "eigen_wrapper.h"
#include "internal_macro.h"

using namespace sage2;  // NOLINT

ATTR_NOINLINE float sage2_min_ps_ref(uint64_t n, const float* x) {
  float min = x[0];
  for (uint64_t i = 1; i < n; ++i) {
    if (x[i] < min) {
      min = x[i];
    }
  }
  return min;
}

ATTR_NOINLINE float eigen_min_ps(uint64_t n, const float* x) {
  auto xv = make_eigen_arrayxf_view(x, n);
  return xv.minCoeff();
}

void Test(int n) {
  std::default_random_engine engine;
  std::vector<float> x(n);
  randn(engine, &x);

  float min1 = sage2_min_ps_ref(n, x.data());

  float min2 = sage2_min_ps(n, x.data());
  CHECK_EQUAL(min1, min2);

  float min3 = eigen_min_ps(n, x.data());
  CHECK_EQUAL(min1, min3);
}

template <class Func>
double Benchmark(Func&& func, int n, int m) {
  std::default_random_engine engine;
  std::vector<float> x(n);
  randn(engine, &x);

  BeginTimer();
  float min = 0;
  for (int i = 0; i < m; ++i) {
    min += func(n, x.data());
  }
  EndTimer();
  REFER(min);
  return GetGFLOPS(1.0 * m * n);
}

void Benchmark(int n) {
  int m = 100000000 / n;
  double gflops[3] = {0};
  gflops[0] = Benchmark(sage2_min_ps_ref, n, m);
  gflops[1] = Benchmark(sage2_min_ps, n, m);
  gflops[2] = Benchmark(eigen_min_ps, n, m);
  PrintContent(n, gflops);
}

int main(int argc, char** argv) {
  int action = 3;
  if (argc > 1) {
    action = std::stoi(argv[1]);
  }

  if (action & 1) {
    for (int i = 1; i < 1000; ++i) {
      Test(i);
    }
  }

  if (action & 2) {
    PrintHeader1(3, "min");
    PrintHeader2(3, "GFLOPS");
    PrintHeader3("ref", "opt", "eigen");
    for (int n : GetLargeN()) {
      Benchmark(n);
    }
  }
  return 0;
}
