#include "wrapper/ping_wrapper.hpp"
#include <iomanip>
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    if (argc != 4) {
      std::cerr << PingWrapper::VERSION << "\n"
                << "Usage: ping_adv <target_ip> <count> <interval_ms>\n"
                << "  count: 2-50\n"
                << "  interval: 0.1-10ms\n";
      return 1;
    }

    const std::string_view ip{argv[1]};
    const int count = std::stoi(argv[2]);
    const auto interval =
        std::chrono::duration<double, std::milli>(std::stod(argv[3]));

    PingWrapper ping{std::string{ip}};
    auto stats = ping.execute(count, interval);

    std::cout << "[" << ip << "] ";
    if (stats.is_valid()) {
      const auto [avg_latency, jitter] =
          std::tuple{stats.average_latency_ms(), stats.jitter_ms()};
      std::cout << std::fixed << std::setprecision(2)
                << "Test Result: Average Latency " << avg_latency
                << "ms, Jitter " << jitter << "ms (" << stats.count()
                << " results)\n";
      return 0;
    } else {
      std::cout << "Test Failed\n";
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
}