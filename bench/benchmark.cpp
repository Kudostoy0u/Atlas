#include <chrono>
#include <iostream>

int main() {
  const auto started = std::chrono::steady_clock::now();
  const auto elapsed = std::chrono::steady_clock::now() - started;
  std::cout << "atlas benchmark harness ready in "
            << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()
            << "us\n";
  return 0;
}
