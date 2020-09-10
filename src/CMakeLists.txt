add_library(matrix_market_io matrix_market_io.c)

target_compile_features(matrix_market_io PUBLIC c_std_11)
target_include_directories(matrix_market_io PUBLIC ../include)
