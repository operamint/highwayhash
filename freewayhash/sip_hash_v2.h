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

#ifndef FREEWAYHASH_SIP_HASH_V2_H_
#define FREEWAYHASH_SIP_HASH_V2_H_

// Portable, very fast standalone C++ SipHash implementation.
// freewayhash::v2::SipHash.
// 
// This a branch-free and cleaner implementation. It only has a minor usage
// disadvantage compared to freewayhash::SipHash:
//
// The implementation is streaming-capable, meaning it can incrementally compute
// a hash by multiple calls to Update() before Finalize(). However, all data blocks
// given to Update() must be multipy of 8 bytes (need not be aligned).
// This is adequate for one-shot hashing or for streaming of data, but not for 
// e.g. encoding multiple arbitrary length data blocks into one single hash.
// For general usage it is therefore recommended to use freewayhash::SipHash.

#include <cstdint>
#include <cstring>

#ifdef _MSC_VER
    #define HH_INLINE __forceinline
#else
    #define HH_INLINE inline
#endif

namespace freewayhash { namespace v2 {
    
    using HH_U64 = unsigned long long;
    using std::uint8_t;
    using std::size_t;

    template <int bits> HH_INLINE HH_U64 RotateLeft(HH_U64 x) {
        return (x << bits) | (x >> (64 - bits));
    }
    template <int u, int v> HH_INLINE void HalfRound(uint64_t& a, uint64_t& b, uint64_t& c, uint64_t& d) {
        a += b;
        c += d;
        b = RotateLeft<u>(b) ^ a;
        d = RotateLeft<v>(d) ^ c;
        a = RotateLeft<32>(a);
    }
    HH_U64 Le64ToHost(HH_U64);
}}

#if defined(_WIN32) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    inline freewayhash::v2::HH_U64 freewayhash::v2::Le64ToHost(HH_U64 x) { return x; }
#elif defined(__APPLE__)
    #include <libkern/OSByteOrder.h>
    inline freewayhash::v2::HH_U64 freewayhash::v2::Le64ToHost(HH_U64 x) { return OSSwapLittleToHostInt64(x); }
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    #include <sys/endian.h>
    inline freewayhash::v2::HH_U64 freewayhash::v2::Le64ToHost(HH_U64 x) { return letoh64(x); }
#elif defined(__linux__) || defined(__CYGWIN__) || defined(__GNUC__) || defined(__GNU_LIBRARY__)
    #include <endian.h>
    inline freewayhash::v2::HH_U64 freewayhash::v2::Le64ToHost(HH_U64 x) { return le64toh(x); }
#else
    #error "Unsupported platform.  Cannot determine byte order."    
#endif


namespace freewayhash { namespace v2 {

    // SipHashState API.
    // As of c++17, default SipHashState<> type need no <>.
    template <int kUpdateIters = 2, int kFinalizeIters = 4> class SipHashState {
        size_t mLength;
        HH_U64 v0, v1, v2, v3;

    public:
        using Key = HH_U64[2];

        explicit HH_INLINE SipHashState(const HH_U64 key[2]) :
            mLength(0),
            v0(key[0] ^ 0x736f6d6570736575),
            v1(key[1] ^ 0x646f72616e646f6d),
            v2(key[0] ^ 0x6c7967656e657261),
            v3(key[1] ^ 0x7465646279746573) {
        }
        
        // requirement: n_bytes % 8 == 0
        HH_INLINE const void* Update(const void* byte_octets, size_t n_bytes) {
            const HH_U64* in_u64 = reinterpret_cast<const HH_U64 *>(byte_octets);
            mLength += n_bytes;
            size_t n_words = n_bytes >> 3;
            HH_U64 m;
            
            while (n_words--) {
                std::memcpy(&m, in_u64++, 8);
                Digest<kUpdateIters>(Le64ToHost(m));
            }
            return in_u64;
        }

        HH_INLINE HH_U64 Finalize(const void* bytes, size_t n_bytes) {
            bytes = Update(bytes, n_bytes & ~7);
            return Remaining(bytes, n_bytes & 7);
        }

    private:
       // requirement: n_bytes < 8
       HH_INLINE HH_U64 Remaining(const void* bytes, size_t n_bytes) {
            const uint8_t* in_u8 = reinterpret_cast<const uint8_t *>(bytes);
            HH_U64 m = (mLength += n_bytes) << 56;
            
            switch (n_bytes) {
                case 7: m |= HH_U64(in_u8[6]) << 48;
                case 6: m |= HH_U64(in_u8[5]) << 40;
                case 5: m |= HH_U64(in_u8[4]) << 32;
                case 4: m |= HH_U64(in_u8[3]) << 24;
                case 3: m |= HH_U64(in_u8[2]) << 16;
                case 2: m |= HH_U64(in_u8[1]) << 8;
                case 1: m |= HH_U64(in_u8[0]);
            }
            Digest<kUpdateIters>(m);
            
            v2 ^= 0xff;
            Compress<kFinalizeIters>();
            
            return v0 ^ v1 ^ v2 ^ v3;
        }        

        template <int rounds> HH_INLINE void Compress() {
            for (int i = 0; i < rounds; ++i) {
                HalfRound<13, 16>(v0, v1, v2, v3);
                HalfRound<17, 21>(v2, v1, v0, v3);
            }
        }

        template <int rounds> HH_INLINE void Digest(HH_U64 m) {
            v3 ^= m;
            Compress<rounds>();
            v0 ^= m;
        }
    };

    // highwayhash-like API:

    template <int kUpdateIters = 2, int kFinalizeIters = 4>
    HH_INLINE HH_U64 SipHash(const HH_U64 key[2], const void* bytes, const HH_U64 size) {
        return SipHashState<kUpdateIters, kFinalizeIters>(key).Finalize(bytes, size);
    }

    template <int kUpdateIters = 2, int kFinalizeIters = 4, typename ContiguousContainer>
    HH_INLINE HH_U64 SipHash(const HH_U64 key[2], const ContiguousContainer& container) {
        return SipHashState<kUpdateIters, kFinalizeIters>(key)
               .Finalize(container.data(), container.size() * sizeof(typename ContiguousContainer::value_type));
    }

    using SipHash13State = SipHashState<1, 3>;

    HH_INLINE HH_U64 SipHash13(const HH_U64 key[2], const void* bytes, const HH_U64 size) {
        return SipHash13State(key).Finalize(bytes, size);
    }
}}

#endif
