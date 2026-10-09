#include "pricer/math/NormalGenerator.h"

namespace pricer {

NormalGenerator::NormalGenerator(std::uint64_t seed) : engine_{seed}, distribution_{0.0, 1.0} {}

double NormalGenerator::next() {
    return distribution_(engine_);
}

}  // namespace pricer