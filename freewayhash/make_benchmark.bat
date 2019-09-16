:: You may pass arg -DSIPTEST=<n>

:: Old g++ 5.1.0 does not fully support c++17
gcc -c -O3 ruby_siphash.c 
g++ -s -I.. -std=c++14 -O3 -mavx2 -Wall -o benchmark_siphash_gnu.exe %* benchmark_siphash.cc ruby_siphash.o
g++ -s -I.. -std=c++14 -O3 -mavx2 -Wall -o demo_siphash_gnu.exe %* demo_siphash.cc 

:: Visual Studio (add -W4)
cl -c -O2 ruby_siphash.c 
cl -nologo -EHsc -I.. -W3 -std:c++17 -O2 -Oy -Oi -arch:AVX2 -Febenchmark_siphash_vs.exe %* benchmark_siphash.cc ruby_siphash.obj
cl -nologo -EHsc -I.. -W3 -std:c++17 -O2 -Oi -Ot -arch:AVX2 -Fedemo_siphash_vs.exe %* demo_siphash.cc

:: Clang Visual Studio
:: set clang="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\Llvm\8.0.0\bin\clang-cl.exe"
:: %clang% -nologo -EHsc -I.. -std:c++17 -O2 -Oi -arch:AVX2 -Febenchmark_siphash_clang.exe %* benchmark_siphash.cc

:done
del *.obj *.o
