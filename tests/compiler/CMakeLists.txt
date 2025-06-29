# Compiler Tests CMakeLists.txt
# Tests for compiler command-line interface and options

# PIE flag tests
add_asthra_test(compiler test_pie_flags.c)