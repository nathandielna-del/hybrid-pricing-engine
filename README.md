# Modern Hybrid Pricing Engine

A derivatives pricing library written in modern C++ (C++20), exposed to Python through pybind11, and fed with real market data.

## Project Pillars

- **Data & Analytics (Python):** market data retrieval with `yfinance`, cleaning with Pandas, historical volatility estimation.
- **Pricing Engine (C++):** object-oriented pricing library, from Black-Scholes closed-form formulas to Monte Carlo simulation for path-dependent options (Asian, Barrier).
- **Python Bridge (pybind11):** native Python bindings to call the C++ engine directly from Python.

## Tech Stack

C++20 · CMake · GoogleTest · Python · Pandas · pybind11

## Status

🚧 Work in progress. This project is being built step by step as a learning journey in quantitative finance and C++.