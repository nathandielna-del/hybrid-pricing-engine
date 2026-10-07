#pragma once

namespace pricer {
[[nodiscard]] double normalPdf(double x) noexcept;
[[nodiscard]] double normalCdf(double x) noexcept;

}  // namespace pricer