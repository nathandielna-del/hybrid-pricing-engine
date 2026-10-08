# --- Librairie de pricing ---
add_library(pricer src/core/Payoff.cpp src/math/NormalDistribution.cpp src/models/BlackScholes.cpp src/models/BlackScholesModel.cpp src/core/VanillaOption.cpp src/engines/AnalyticBlackScholesEngine.cpp src/math/NormalGenerator.cpp src/engines/MonteCarloEngine.cpp)

# PUBLIC : les utilisateurs de pricer (demo, tests, module Python) doivent trouver ses headers
target_include_directories(pricer PUBLIC include)
# PUBLIC : les headers de pricer pourront utiliser du C++20, donc ses utilisateurs aussi
target_compile_features(pricer PUBLIC cxx_std_20)
# PRIVATE : nos exigences de qualité ne concernent que notre propre code
target_compile_options(pricer PRIVATE -Wall -Wextra -Wpedantic)

# --- Exécutable de démonstration ---
add_executable(demo apps/main.cpp)
target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic)
# demo hérite automatiquement de l'include et du C++20 de pricer
target_link_libraries(demo PRIVATE pricer)

# --- Tests unitaires ---
add_subdirectory(tests)