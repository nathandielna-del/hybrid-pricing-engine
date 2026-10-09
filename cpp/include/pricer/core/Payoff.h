#pragma once

namespace pricer {

[[nodiscard]] double callPayoff(double spot, double strike);
[[nodiscard]] double putPayoff(double spot, double strike);

class Payoff {
public:
    virtual ~Payoff() = default;

    // Polymorphic base: no copy, no move (prevents slicing).
    Payoff(const Payoff&) = delete;
    Payoff& operator=(const Payoff&) = delete;
    Payoff(Payoff&&) = delete;
    Payoff& operator=(Payoff&&) = delete;

    [[nodiscard]] virtual double operator()(double spotAtMaturity) const = 0;

protected:
    Payoff() = default;  // only derived classes can construct a Payoff
};

class CallPayoff final : public Payoff {
public:
    explicit CallPayoff(double strike);

    [[nodiscard]] double operator()(double spotAtMaturity) const noexcept override;
    [[nodiscard]] double strike() const noexcept;

private:
    double strike_;
};

class PutPayoff final : public Payoff {
public:
    explicit PutPayoff(double strike);

    [[nodiscard]] double operator()(double spotAtMaturity) const noexcept override;
    [[nodiscard]] double strike() const noexcept;

private:
    double strike_;
};

// Pays 1 if the spot at maturity is strictly above the strike, 0 otherwise ("cash-or-nothing").
class DigitalCallPayoff final : public Payoff {
public:
    explicit DigitalCallPayoff(double strike);

    [[nodiscard]] double operator()(double spotAtMaturity) const override;
    [[nodiscard]] double strike() const noexcept;

private:
    double strike_;
};

}  // namespace pricer