// Copyright 2019 operamint (github). All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::setw;	
using std::setprecision;
using std::fixed;

#include "freewayhash/sip_hash.h"
#include "freewayhash/sip_hash_v2.h"
#include "highwayhash/sip_hash.h"
#include "highwayhash/highwayhash.h"

#ifndef TEST
#define TEST 1
#endif

void benchmark()
{
    highwayhash::HH_U64 key[2], sum_a = 0, sum_b = 0, sum_c = 0;
    std::clock_t start;
    double time_a, time_b, time_c;

    char* k_str = reinterpret_cast<char *>(key);
    for (int i = 0; i < 16; ++i)
        k_str[i] = char(i);

    std::vector<int> input_lengths { 7, 8, 31, 32, 44, 63, 64, 1024, 1024 * 1024 };
    std::vector<char> data(input_lengths.back() * 2);
    for (char& m: data) m = rand() % 256;
    const char* in = data.data();
    const double N = 2000000000;
    int pos = 0;

    cout << setprecision(2) << fixed;
         
    // SipHash 2-4
    cout << endl << setw(10) << "input len" << setw(15) << "hh::SipHash"
         << setw(15) << "fh::SipHash" << setw(10) << "speedup"
         << setw(19) << "hh::HighwayHash" << setw(10) << "speedup" << endl;
    for (int len: input_lengths) {
        size_t n = size_t(N / len);
        
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
            sum_a += highwayhash::SipHash(key, &in[pos], len);
        }
        time_a = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        cout << setw(10) << len << setw(14) << time_a << "s";
        
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
#if   TEST == 1
            sum_b += freewayhash::SipHash(key, &in[pos], len);
#elif TEST == 2
            sum_b += freewayhash::v2::SipHash(key, &in[pos], len);
#elif TEST == 3
            freewayhash::SipHashState<> hasher(key); // remove <> if C++ >= 17
            hasher.Update(&in[pos], 5);
            hasher.Update(&in[pos + 5], len - 5);
            sum_b += hasher.Finalize();
#endif
        }
        time_b = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        cout << setw(14) << time_b << "s" << setw(9) << (time_a / time_b) << "x";
            
        highwayhash::HHResult64 result;
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
            highwayhash::HHStateT<HH_TARGET> state(key);
            highwayhash::HighwayHashT(&state, &in[pos], len, &result);
            sum_c += result;
        }
        time_c = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        cout << setw(18) << time_c << "s" << setw(9) << (time_b / time_c) << "x" << endl;
    }

    // SipHash 1-3
    cout << endl << setw(10) << "input len" << setw(15) << "hh::SipHash13"
         << setw(15) << "fh::SipHash13" << setw(10) << "speedup" << endl;
    for (int len: input_lengths) {
        size_t n = size_t(N / len);
        
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
            sum_a += highwayhash::SipHash13(key, &in[pos], len);
        }
        time_a = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        cout << setw(10) << len << setw(14) << time_a << "s";
        
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
#if   TEST == 1
            sum_b += freewayhash::SipHash13(key, &in[pos], len);
#elif TEST == 2
            sum_b += freewayhash::v2::SipHash13(key, &in[pos], len);
#elif TEST == 3
            freewayhash::SipHash13State hasher(key);
            hasher.Update(&in[pos], 5);
            hasher.Update(&in[pos + 5], len - 5);
            sum_b += hasher.Finalize();
#endif
        }
        time_b = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        cout << setw(14) << time_b << "s" << setw(9) << (time_a / time_b) << "x" << endl;
    }
    cout << endl;
    cout << "freewayhash::SipHash     checksum: " << sum_b << endl;
    cout << "highwayhash::SipHash     checksum: " << sum_a << endl;
    cout << "highwayhash::HighwayHash checksum: " << sum_c << endl;
}



int main()
{
    benchmark();
}
