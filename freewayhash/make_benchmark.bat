:: You may pass arg -DTEST=<n>

:: Old g++ 5.1.0 does not fully support c++17
g++ -s -I.. -std=c++14 -O3 -mavx2 -Wall -o benchmark_siphash_gnu.exe %* benchmark_siphash.cc 

:: Visual Studio (add -W4)
cl -nologo -EHsc -I.. -std:c++17 -O2 -arch:AVX2 -Febenchmark_siphash_vs.exe %* benchmark_siphash.cc

del *.obj *.o
