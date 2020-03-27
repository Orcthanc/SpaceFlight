#!/bin/bash

mkdir -p build && cd build && cmake -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=DEBUG .. && make -j 9
