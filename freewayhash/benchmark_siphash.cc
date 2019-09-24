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
#include "highwayhash/sip_hash.h"
#include "siphash.h" 
#include "highwayhash/highwayhash.h"


#ifndef SIPTEST
#define SIPTEST 1
#endif

void benchmark()
{
    const double N = 500000000;

    uint64_t sipkey[2], sum_a = 0, sum_b = 0, sum_c = 0;
    std::clock_t start;
    double time_a, time_b, time_c;

    uint8_t* k_str = reinterpret_cast<uint8_t *>(sipkey);
    for (uint8_t i = 0; i < 16; ++i)
        k_str[i] = i;

    std::vector<int> input_lengths { 7, 8, 31, 32, 44, 63, 64, 1024, 1024 * 1024 };
    std::vector<uint8_t> data(input_lengths.back() * 2);
    //srand(0);
    for (uint8_t& m: data) m = rand() % 256;
    const char* in = reinterpret_cast<const char *>(data.data());
    
    int pos = 0;
    const uint64_t hhkey[4] = {1, 2, 3, 4};
    cout << setprecision(2) << fixed;
         
    // SipHash 2-4
    cout << endl << setw(10) << "input len" << setw(15) << "hh::SipHash"
         << setw(15) << "fh::SipHash" << setw(10) << "speedup"
         << setw(19) << "hh::HighwayHash" << setw(10) << "speedup" << endl;
    for (int len: input_lengths) {
        size_t n = size_t(N / len);
        
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
            sum_a += highwayhash::SipHash(sipkey, &in[pos], len);  // used in Python impl.
        }
        time_a = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        cout << setw(10) << len << setw(14) << time_a << "s";
        
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
#if   SIPTEST == 1
            sum_b += freewayhash::SipHash(sipkey, &in[pos], len);
#elif SIPTEST == 2
            sum_b += siphash_hash(sipkey, &in[pos], len);
#elif SIPTEST == 3 // splitting Update() in two and break 8 bytes alignment.
            freewayhash::SipHashState<> hasher(sipkey); // remove <> if C++ >= 17
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
            highwayhash::HHStateT<HH_TARGET> state(hhkey);
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
            sum_a += highwayhash::SipHash13(sipkey, &in[pos], len);
        }
        time_a = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        cout << setw(10) << len << setw(14) << time_a << "s";
        
        start = std::clock();
        for (size_t i = 0; i < n; ++i) {
#if   SIPTEST == 1
            sum_b += freewayhash::SipHash13(sipkey, &in[pos], len);
#elif SIPTEST == 2
            sum_b += siphash_hash13(sipkey, &in[pos], len);
#elif SIPTEST == 3
            freewayhash::SipHash13State hasher(sipkey);
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
