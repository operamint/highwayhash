
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <iomanip>
#include "sip_hash.h"

int main()
{
    std::vector<double> vec { 1.23, 1.43, 78.0, 232.23, 90.0, 23.12, 44.8 };
    std::array<double, 7> arr = { 1.23, 1.43, 78.0, 232.23, 90.0, 23.12, 44.8 };
    std::wstring wstr = L"This is a test for hashing wide strings";
    
    freewayhash::SipHashState<> hasher(freewayhash::SipKey::FromStr("0123456789ABCDEF"));
    hasher.Update(vec);
    hasher.Update(arr);
    hasher.Update(wstr);
    std::cout << hasher.Finalize() << std::endl;
    freewayhash::SipKey k1 {0xdeadbeefdeaf10cc, 0xdeadbeefdeaf10cc};
    const uint64_t k2[2] = {0xdeadbeefdeaf10cc, 0xdeadbeefdeaf10cc};
    std::cout << freewayhash::SipHash(k1, vec) << std::endl;
    std::cout << freewayhash::SipHash(k2, arr) << std::endl;
    
    uint64_t d1[2] = {0x0706050403020100, 0x0f0e0d0c0b0a0908};
    std::cout << std::hex << freewayhash::SipHash(d1, d1, 15) << std::endl;
}