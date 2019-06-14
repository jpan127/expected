
clang++                                 \
    -std=c++17                          \
    -Xclang -flto-visibility-public-std \
    -I.                                 \
    -Itests                             \
    -Imodules/catch2                    \
    main.cpp                            \
    tests/test_expected.cpp             \
    -o test
