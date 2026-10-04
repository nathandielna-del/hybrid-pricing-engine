#pragma once

namespace pricer {

[[nodiscard]] double callPayoff(double spot, double strike);
[[nodiscard]] double putPayoff(double spot, double strike);

}  // namespace pricer