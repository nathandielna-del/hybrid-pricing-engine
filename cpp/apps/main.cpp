#include "pricer/core/Payoff.h"

#include <iostream>

int main() {
    const double strike = 100.0;
    const double spotDown = 90.0;
    const double spotUp = 110.0;

    std::cout << "Call payoff (spot = " << spotDown << "): " << pricer::callPayoff(spotDown, strike)
              << '\n';
    std::cout << "Put payoff  (spot = " << spotDown << "): " << pricer::putPayoff(spotDown, strike)
              << '\n';
    std::cout << '\n';
    std::cout << "Call payoff (spot = " << spotUp << "): " << pricer::callPayoff(spotUp, strike)
              << '\n';
    std::cout << "Put payoff  (spot = " << spotUp << "): " << pricer::putPayoff(spotUp, strike)
              << '\n';

    return 0;
}