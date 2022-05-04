/* 
   Copyright (c) 2015, 2016, 2020 Andreas F. Borchert
   All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
   KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/*
   test suite for "printf.hpp" with test cases where
   std::printf and fmt::printf are expected to deliver
   identical results
*/

#include <cerrno>
#include <clocale>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include "printf.hpp"

#ifdef __INTEL_COMPILER
/* disable warnings of the Intel compiler for out of range values
   as they are intentional */
#pragma warning(disable:265)
#endif

static unsigned int testcases = 0;
static unsigned int successful = 0;
static unsigned int warnings = 0;
static unsigned int skipped = 0;
static unsigned int broken = 0; /* std::printf is broken */
static unsigned int cpp_broken = 0; /* iostreams are broken */

template<typename... Values>
int sprint(char* buffer, std::size_t size,
      const char* format, Values&&... values) {
#if defined(__clang__)
   #pragma clang diagnostic ignored "-Wformat-security"
#endif
   return std::snprintf(buffer, size, format, std::forward<Values>(values)...);
}

template<typename... Values>
int sprint(wchar_t* buffer, std::size_t size,
      const wchar_t* format, Values&&... values) {
   if (buffer && size > 0) {
      return std::swprintf(buffer, size, format,
	 std::forward<Values>(values)...);
   } else {
      /* unfortunately the behaviour of swprintf differs
	 from that of snprintf, i.e. if size == 0, just -1
	 is returned instead of computing the required buffer
	 size */
      wchar_t buf[1024]; // should be sufficient for our tests
      return std::swprintf(buf, sizeof buf/sizeof buf[0], format,
	 std::forward<Values>(values)...);
   }
}

int compare(const char* s1, const char* s2) {
   return std::strcmp(s1, s2);
}

int compare(const wchar_t* s1, const wchar_t* s2) {
   return std::wcscmp(s1, s2);
}

template<typename... Values>
int print(const char* format, Values&&... values) {
   return fmt::printf(std::cout, format, std::forward<Values>(values)...);
}

template<typename... Values>
int print(const wchar_t* format, Values&&... values) {
   return fmt::printf(std::wcout, format, std::forward<Values>(values)...);
}

void diff_analysis(bool implementation_defined,
      const char* format, int count1, int count2,
      const char* buf1, const char* buf2) {
   if (implementation_defined) {
      print("implementation-dependent test for \"%s\" differs:\n", format);
   } else {
      print("test for \"%s\" fails:\n", format);
   }
   if (count1 == count2) {
      print("   fmt delivers: '%s'\n", buf1);
      print("   std delivers: '%s'\n", buf2);
   } else {
      print("   fmt delivers: '%s' (%d)\n", buf1, count1);
      print("   std delivers: '%s' (%d)\n", buf2, count2);
   }
}

void diff_analysis(bool implementation_defined,
      const wchar_t* format, int count1, int count2,
      const wchar_t* buf1, const wchar_t* buf2) {
   if (implementation_defined) {
      print(L"implementation-dependent test for L\"%s\" differs:\n", format);
   } else {
      print(L"test for L\"%s\" fails:\n", format);
   }
   if (count1 == count2) {
      print(L"   fmt delivers: '%s'\n", buf1);
      print(L"   std delivers: '%s'\n", buf2);
   } else {
      print(L"   fmt delivers: '%s' (%d)\n", buf1, count1);
      print(L"   std delivers: '%s' (%d)\n", buf2, count2);
   }
}

void count_mismatch(bool implementation_defined,
      const char* format, int count1, int count2) {
   if (implementation_defined) {
      print("implementation-dependent test for \"%s\" differs,", format);
   } else {
      print("test for \"%s\" fails,", format);
   }
   print(" fmt returns %d, std returns %d\n",
      count1, count2);
}

void count_mismatch(bool implementation_defined,
      const wchar_t* format, int count1, int count2) {
   if (implementation_defined) {
      print(L"implementation-dependent test for \"%s\" differs,", format);
   } else {
      print(L"test for \"%s\" fails,", format);
   }
   print(L" fmt returns %d, std returns %d\n",
      count1, count2);
}

void errno_mismatch(bool implementation_defined,
      const char* format, int err1, int err2) {
   if (implementation_defined) {
      print("implementation-dependent test for \"%s\" differs,", format);
   } else {
      print("test for \"%s\" fails,", format);
   }
   print(" fmt sets errno to %d, std sets errno to %d\n",
      err1, err2);
}

void errno_mismatch(bool implementation_defined,
      const wchar_t* format, int err1, int err2) {
   if (implementation_defined) {
      print(L"implementation-dependent test for \"%s\" differs,", format);
   } else {
      print(L"test for \"%s\" fails,", format);
   }
   print(L" fmt sets errno to %d, std sets errno to %d\n",
      err1, err2);
}

void offset_mismatch(const char* format, int off1, int off2) {
   print("test for \"%s\" fails,", format);
   print(" fmt sets offset to %d, std sets offset to %d\n",
      off1, off2);
}

void offset_mismatch(const wchar_t* format, int off1, int off2) {
   print(L"test for \"%s\" fails,", format);
   print(L" fmt sets offset to %d, std sets offset to %d\n",
      off1, off2);
}

void print_values(int index) {
}

#if __cplusplus < 201703L
/* an output operator for nullptr_t is provided since C++17
   but neither for C++11 nor C++14;
   however clang++ defines this operator also for C++11 and C++14
   beginning from at least clang version 12 */
#if defined(__clang__) && defined(_LIBCPP_VERSION)
   #if _LIBCPP_VERSION < 12000
      #define DEFINE_OUTPUT_OPERATOR_FOR_NULLPTR
   #endif
#else
   #define DEFINE_OUTPUT_OPERATOR_FOR_NULLPTR
#endif

#ifdef DEFINE_OUTPUT_OPERATOR_FOR_NULLPTR
std::ostream& operator<<(std::ostream& out, std::nullptr_t p) {
   out << "nullptr";
   return out;
}
#endif
#endif

template<typename Value, typename... Values>
void print_values(int index, Value value, Values&&... values) {
   std::cout << "   argument #" << index << ": '" << value << "'" << std::endl;
   print_values(index + 1, std::forward<Values>(values)...);
}

template<typename CharT, typename... Values>
bool check_printf(const CharT* expected, const CharT* format,
      Values&&... values) {
   int count = sprint(nullptr, 0, format, std::forward<Values>(values)...);
   bool ok = false;
   if (count >= 0) {
      CharT* buf = new CharT[count + 1];
      sprint(buf, count + 1, format, std::forward<Values>(values)...);
      ok = compare(expected, buf) == 0;
      if (!ok) {
	 fmt::printf(
	    "std::printf deviates from standard for \"%s\": \"%.*s\"\n",
	    format, count, buf);
      }
      delete[] buf;
   } else if (!ok) {
      fmt::printf("std::printf from standard for \"%s\"\n", format);
   }
   return ok;
}

/* older C++ libraries do not support hexfloat
   which is required to support %a etc.  */
bool check_hexfloat() {
   std::ostringstream os;
   os.setf(std::ios_base::scientific | std::ios_base::fixed,
      std::ios_base::floatfield);
   os << std::numeric_limits<double>::max();
   std::string osstr(os.str());
   return osstr == "0x1.fffffffffffffp+1023";
}

template<typename CharT, typename... Values>
bool general_testcase(bool implementation_defined,
      bool with_offset, int& offset,
      const CharT* format, Values&&... values) {
   ++testcases;
   std::basic_ostringstream<CharT> os;
   errno = 0;
   int off1 = 0; int off2 = 0;
   int count1 = fmt::printf(os, format, std::forward<Values>(values)...);
   if (with_offset) off1 = offset;
   int err1 = errno;
   errno = 0;
   int count2 = sprint(nullptr, 0, format, std::forward<Values>(values)...); 
   if (with_offset) off2 = offset;
   int err2 = errno;
   if (count1 < 0 || count2 < 0) {
      if (count1 != count2) {
	 count_mismatch(implementation_defined, format, count1, count2);
	 print_values(1, values...);
	 if (implementation_defined) ++warnings;
	 return false;
      }
      if (err1 != err2) {
	 errno_mismatch(implementation_defined, format, err1, err2);
	 print_values(1, values...);
	 if (implementation_defined) ++warnings;
	 return false;
      }
      ++successful;
      return true;
   }

   /* compare the resulting strings */
   CharT* buf = new CharT[count2 + 1];
   sprint(buf, count2 + 1, format, values...);
   std::basic_string<CharT> osstr(os.str());
   bool ok = compare(buf, osstr.c_str()) == 0;
   if (!ok) {
      diff_analysis(implementation_defined, format, count1, count2,
	 osstr.c_str(), buf);
      if (implementation_defined) ++warnings;
   }
   delete[] buf;
   if (with_offset && off1 != off2) {
      offset_mismatch(format, off1, off2);
      ok = false;
   }
   if (!ok) print_values(1, values...);
   if (ok) ++successful;
   return ok;
}

template<typename CharT, typename... Values>
bool testcase(const CharT* format, Values&&... values) {
   int offset = 0;
   return general_testcase(/* implementation defined = */ false,
      /* with offset = */ false, /* unused */ offset,
      format, std::forward<Values>(values)...);
}

template<typename CharT, typename... Values>
bool implementation_dependent_testcase(const CharT* format,
      Values&&... values) {
   int offset = 0;
   return general_testcase(/* implementation defined = */ true,
      /* with offset = */ false, /* unused */ offset,
      format, std::forward<Values>(values)...);
}

template<typename CharT, typename... Values>
bool testcase_with_offset(int& offset,
      const CharT* format, Values&&... values) {
   return general_testcase(/* implementation defined = */ false,
      /* with offset = */ true, offset,
      format, std::forward<Values>(values)...);
}

template<typename CharT, typename... Values>
bool special_testcase(int expected_count,
      const CharT* expected_string,
      const CharT* format, Values&&... values) {
   ++testcases;
   bool ok = true;
   std::basic_ostringstream<CharT> os;
   auto count = fmt::printf(os, format, std::forward<Values>(values)...);
   if ((count < 0 || expected_count < 0) && count != expected_count) {
      print("special test for \"%s\" fails,", format);
      print(" fmt returns %d but we expected %d\n", count, expected_count);
      print_values(1, values...);
      return false;
   }
   if (count != expected_count) {
      ok = false;
   }
   std::basic_string<CharT> fmt_str(os.str());
   std::basic_string<CharT> expected_str(expected_string);
   if (fmt_str != expected_str) {
      ok = false;
   }
   if (ok) {
      ++successful; return true;
   } else {
      print("special test for \"%s\" fails:\n", format);
      if (count != expected_count) {
	 print("   fmt returns:           %d\n", count);
	 print("   expected return value: %d\n", expected_count);
      }
      print("   fmt delivers:          \"%s\"\n", fmt_str);
      print("   expected:              \"%s\"\n", expected_str);
      print_values(1, values...);
      return false;
   }
}

/* for tests of the support for output operators */
template<typename T>
struct TestObject {
   TestObject() : object() {
   }
   TestObject(T&& val) : object(val) {
   }
   T object;
};
template<typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& out, const TestObject<T>& t) {
   out << t.object << "/TO";
   return out;
}
template<typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& out, const TestObject<T>* tp) {
   out << tp->object << "/TO";
   return out;
}

void run_tests() {
   testcase("Hello world!");
   testcase("%% %% %%%%");

   /* test with many arguments of different types */
   testcase("%d %u %lg %c %s", 27, 13u, 2.3, 'x', "Hello");

   char c_values[] = {'a', 'A', '.', '/', ' ', '\t', '\n', '\0',
      std::numeric_limits<char>::min(), std::numeric_limits<char>::max()};
   for (auto val: c_values) {
      testcase("%c", val);
      testcase("%c%c", val, val);
      testcase("%c %c", val, val);
      testcase("%8c", val);
      testcase("%-8c", val);
   }
   testcase("%c", 65);

   for (bool val: {false, true}) {
      testcase("%d", val);
      testcase("%3d", val);
      testcase("%03d", val);
   }

   signed char sc_values[] = {'a', 'A', '.', '/', ' ', -1,
      std::numeric_limits<signed char>::min(),
      std::numeric_limits<signed char>::max()};
   for (auto val: sc_values) {
      testcase("%hhd", val);
      testcase("%hhd%hhd", val, val);
      testcase("%hhd %hhd", val, val);
      testcase("%8hhd", val);
      testcase("%-8hhd", val);
   }

   unsigned char uc_values[] = {'a', 'A', '.', '/', ' ',
      std::numeric_limits<unsigned char>::min(),
      std::numeric_limits<unsigned char>::max()};
   for (auto val: uc_values) {
      testcase("%hhu", val);
      testcase("%hhu%hhu", val, val);
      testcase("%hhu %hhu", val, val);
      testcase("%8hhu", val);
      testcase("%-8hhu", val);
   }

   short int si_values[] = {0, 1234, -1234,
      std::numeric_limits<short int>::min(),
      std::numeric_limits<short int>::max()};
   for (auto val: si_values) {
      testcase("%hd", val);
      testcase("%8hd", val);
      testcase("%-8hd", val);
      testcase("%+8hd", val);
      testcase(" %hd ", val);
      testcase("%hd%hd", val, val);
   }

   int i_values[] = {0, -12, 42, 117, 1234, -1234,
      std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};
   for (auto val: i_values) {
      testcase("%d", val);
      testcase("%1d", val);
      testcase("%2d", val);
      testcase("%3d", val);
      testcase("%4d", val);
      testcase("%5d", val);
      testcase("%6d", val);
      testcase("%7d", val);
      testcase("%8d", val);
      testcase("%9d", val);
      testcase("%10d", val);
      testcase("%11d", val);
      testcase("%12d", val);
      testcase("%0d", val);
      testcase("%01d", val);
      testcase("%02d", val);
      testcase("%03d", val);
      testcase("%04d", val);
      testcase("%05d", val);
      testcase("%06d", val);
      testcase("%07d", val);
      testcase("%08d", val);
      testcase("%09d", val);
      testcase("%010d", val);
      testcase("%011d", val);
      testcase("%012d", val);
      testcase("%-d", val);
      testcase("%-1d", val);
      testcase("%-2d", val);
      testcase("%-3d", val);
      testcase("%-4d", val);
      testcase("%-5d", val);
      testcase("%-6d", val);
      testcase("%-7d", val);
      testcase("%-8d", val);
      testcase("%-9d", val);
      testcase("%-10d", val);
      testcase("%-11d", val);
      testcase("%-12d", val);
      testcase("%+d", val);
      testcase("%+1d", val);
      testcase("%+2d", val);
      testcase("%+3d", val);
      testcase("%+4d", val);
      testcase("%+5d", val);
      testcase("%+6d", val);
      testcase("%+7d", val);
      testcase("%+8d", val);
      testcase("%+9d", val);
      testcase("%+10d", val);
      testcase("%+11d", val);
      testcase("%+12d", val);
      testcase("%0-8d", val);
      testcase("%0+8d", val);
      testcase("% d", val);
      testcase("% 1d", val);
      testcase("% 2d", val);
      testcase("% 3d", val);
      testcase("% 4d", val);
      testcase("% 5d", val);
      testcase("% 6d", val);
      testcase("% 7d", val);
      testcase("% 8d", val);
      testcase("% 9d", val);
      testcase("% 10d", val);
      testcase("% 11d", val);
      testcase("% 12d", val);
      testcase("%.4d", val);
      testcase("%8.4d", val);
      testcase("% .4d", val);
      testcase("% 8.4d", val);
      testcase("% -4d", val);
      testcase("% -.4d", val);
      testcase("% 8.4d", val);
      testcase("% -8.4d", val);
      testcase("%+.4d", val);
      testcase("%+8.4d", val);
      testcase("%+ .4d", val);
      testcase("%+ 8.4d", val);
      testcase("%+ -4d", val);
      testcase("%+ -.4d", val);
      testcase("%+ 8.4d", val);
      testcase("%+ -8.4d", val);
      testcase(" %d ", val);
      testcase("%d%d", val, val);
   }

   long int li_values[] = {0, 1234, -1234,
      std::numeric_limits<long int>::min(),
      std::numeric_limits<long int>::max()};
   for (auto val: li_values) {
      testcase("%ld", val);
      testcase("%8ld", val);
      testcase("%-8ld", val);
      testcase("%+8ld", val);
      testcase(" %ld ", val);
      testcase("%ld%ld", val, val);
   }

   long long int lli_values[] = {0, 1234, -1234,
      std::numeric_limits<long long int>::min(),
      std::numeric_limits<long long int>::max()};
   for (auto val: lli_values) {
      testcase("%lld", val);
      testcase("%16lld", val);
      testcase("%-16lld", val);
      testcase("%+16lld", val);
      testcase(" %lld ", val);
      testcase("%lld%lld", val, val);
   }

   std::intmax_t im_values[] = {0, 17, -13, 1234, -1234,
      std::numeric_limits<std::intmax_t>::min(),
      std::numeric_limits<std::intmax_t>::max()};
   for (auto val: im_values) {
      testcase("%jd", val);
      testcase("%16jd", val);
      testcase("%-16jd", val);
      testcase("%+16jd", val);
      testcase(" %jd ", val);
      testcase("%jd%jd", val, val);
   }

   using ssize_t = std::make_signed<std::size_t>::type;
   ssize_t ssize_values[] = {0, 17, -13, 1234, -1234,
      sizeof(ssize_t),
      std::numeric_limits<ssize_t>::min(),
      std::numeric_limits<ssize_t>::max()};
   for (auto val: ssize_values) {
      testcase("%zd", val);
      testcase("%16zd", val);
      testcase("%-16zd", val);
      testcase("%+16zd", val);
      testcase(" %zd ", val);
      testcase("%zd%zd", val, val);
   }

   std::ptrdiff_t ptrdiff_values[] = {0, 17, -13, 1234, -1234,
      &ptrdiff_values[4] - &ptrdiff_values[0],
      &ptrdiff_values[0] - &ptrdiff_values[4],
      std::numeric_limits<std::ptrdiff_t>::min(),
      std::numeric_limits<std::ptrdiff_t>::max()};
   for (auto val: ptrdiff_values) {
      testcase("%td", val);
      testcase("%16td", val);
      testcase("%-16td", val);
      testcase("%+16td", val);
      testcase(" %td ", val);
      testcase("%td%td", val, val);
   }

   unsigned int ui_values[] = {1, 42, 117, 1234, 2048,
      std::numeric_limits<unsigned int>::min(),
      std::numeric_limits<unsigned int>::max()};
   for (auto val: ui_values) {
      testcase("%u", val);
      testcase("%8u", val);
      testcase("%-8u", val);
      testcase("%+8u", val);
      testcase("%o", val);
      testcase("%x", val);
      testcase("%X", val);
      testcase(" %u ", val);
      testcase("%u%u", val, val);
      testcase("%#o", val);
      testcase("%#.4o", val);
      testcase("%#x", val);
      testcase("%#.4x", val);
      testcase("%#o", val);
      testcase("%#.4o", val);
      testcase("%#x", val);
      testcase("%#.4x", val);
   }

   unsigned long int uli_values[] = {1234,
      std::numeric_limits<unsigned long int>::min(),
      std::numeric_limits<unsigned long int>::max()};
   for (auto val: uli_values) {
      testcase("%lu", val);
      testcase("%8lu", val);
      testcase("%-8lu", val);
      testcase("%+8lu", val);
      testcase("%lo", val);
      testcase("%lx", val);
      testcase("%lX", val);
      testcase(" %lu ", val);
      testcase("%lu%lu", val, val);
   }

   unsigned long long int ulli_values[] = {1234,
      std::numeric_limits<unsigned long long int>::min(),
      std::numeric_limits<unsigned long long int>::max()};
   for (auto val: ulli_values) {
      testcase("%llu", val);
      testcase("%16llu", val);
      testcase("%-16llu", val);
      testcase("%+16llu", val);
      testcase("%llo", val);
      testcase("%llx", val);
      testcase("%llX", val);
      testcase(" %llu ", val);
      testcase("%llu%llu", val, val);
   }

   std::uintmax_t uim_values[] = {0, 17, 1234,
      std::numeric_limits<std::uintmax_t>::min(),
      std::numeric_limits<std::uintmax_t>::max()};
   for (auto val: uim_values) {
      testcase("%ju", val);
      testcase("%16ju", val);
      testcase("%-16ju", val);
      testcase("%+16ju", val);
      testcase("%jo", val);
      testcase("%jx", val);
      testcase("%jX", val);
      testcase(" %ju ", val);
      testcase("%ju%ju", val, val);
   }

   std::size_t size_values[] = {0, 17, 1234,
      sizeof(uim_values),
      std::numeric_limits<std::size_t>::min(),
      std::numeric_limits<std::size_t>::max()};
   for (auto val: size_values) {
      testcase("%zu", val);
      testcase("%16zu", val);
      testcase("%-16zu", val);
      testcase("%+16zu", val);
      testcase("%zo", val);
      testcase("%zx", val);
      testcase("%zX", val);
      testcase(" %zu ", val);
      testcase("%zu%zu", val, val);
   }

   using uptrdiff_t = std::make_unsigned<std::ptrdiff_t>::type;
   uptrdiff_t uptrdiff_values[] = {0, 17, 1234,
      std::numeric_limits<uptrdiff_t>::min(),
      std::numeric_limits<uptrdiff_t>::max()};
   for (auto val: uptrdiff_values) {
      testcase("%tu", val);
      testcase("%16tu", val);
      testcase("%-16tu", val);
      testcase("%+16tu", val);
      testcase("%to", val);
      testcase("%016to", val);
      testcase("%tx", val);
      testcase("%tX", val);
      testcase(" %tu ", val);
      testcase("%tu%tu", val, val);
   }

   bool nan_works = check_printf(" NAN", "% F", std::nanf("1"));
   if (!nan_works) ++broken;
   bool hexfloat_works = check_hexfloat();
   if (!hexfloat_works) {
      ++cpp_broken;
   }
   bool uppercase_inf_works = check_printf("INF", "%E",
      std::numeric_limits<float>::max() * 2);
   if (!uppercase_inf_works) {
      ++broken;
   }
   float f_values[] = {0, -0.0f, -1, 42, 1234.5678f, 1.25E-10f, 3E+10f,
      std::numeric_limits<float>::min() / 2,
      std::numeric_limits<float>::max() * 2,
      std::numeric_limits<float>::min(),
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::epsilon(),
      std::sqrt(-1.0f), std::nanf("1"), -std::nanf("1")};
   for (auto val: f_values) {
      if (!nan_works && !std::isfinite(val)) continue; 
      testcase("%f", val);
      testcase("%e", val);
      testcase("%g", val);
      if (uppercase_inf_works || std::isfinite(val)) {
	 testcase("%F", val);
	 testcase("%E", val);
	 testcase("%G", val);
      }
      if (hexfloat_works) {
	 testcase("%a", val);
	 // testcase("%.0a", val);
	 // testcase("%.2a", val);
	 if (uppercase_inf_works || std::isfinite(val)) {
	    testcase("%A", val);
	 }
      }
      testcase("%10.2f", val);
      testcase("%10.2e", val);
      testcase("%10.2g", val);
      testcase("%#g", val);
      testcase("%0g", val);
      testcase("%01g", val);
      testcase("%02g", val);
      testcase("%03g", val);
      testcase("%04g", val);
      testcase("%05g", val);
      testcase("%06g", val);
      testcase("%07g", val);
      testcase("%08g", val);
      testcase("%09g", val);
      testcase("%010g", val);
      testcase("%011g", val);
      testcase("%012g", val);
      testcase("%-g", val);
      testcase("%-1g", val);
      testcase("%-2g", val);
      testcase("%-3g", val);
      testcase("%-4g", val);
      testcase("%-5g", val);
      testcase("%-6g", val);
      testcase("%-7g", val);
      testcase("%-8g", val);
      testcase("%-9g", val);
      testcase("%-10g", val);
      testcase("%-11g", val);
      testcase("%-12g", val);
      testcase("%+g", val);
      testcase("%+1g", val);
      testcase("%+2g", val);
      testcase("%+3g", val);
      testcase("%+4g", val);
      testcase("%+5g", val);
      testcase("%+6g", val);
      testcase("%+7g", val);
      testcase("%+8g", val);
      testcase("%+9g", val);
      testcase("%+10g", val);
      testcase("%+11g", val);
      testcase("%+12g", val);
      testcase("%0-8g", val);
      testcase("%0+8g", val);
      testcase("% f", val);
      testcase("% 1f", val);
      testcase("% 2f", val);
      testcase("% 3f", val);
      testcase("% 4f", val);
      testcase("% 5f", val);
      testcase("% 6f", val);
      testcase("% 7f", val);
      testcase("% 8f", val);
      testcase("% 9f", val);
      testcase("% 10f", val);
      testcase("% 11f", val);
      testcase("% 12f", val);
      testcase("% e", val);
      testcase("% 1e", val);
      testcase("% 2e", val);
      testcase("% 3e", val);
      testcase("% 4e", val);
      testcase("% 5e", val);
      testcase("% 6e", val);
      testcase("% 7e", val);
      testcase("% 8e", val);
      testcase("% 9e", val);
      testcase("% 10e", val);
      testcase("% 11e", val);
      testcase("% 12e", val);
      testcase("% g", val);
      testcase("% 1g", val);
      testcase("% 2g", val);
      testcase("% 3g", val);
      testcase("% 4g", val);
      testcase("% 5g", val);
      testcase("% 6g", val);
      testcase("% 7g", val);
      testcase("% 8g", val);
      testcase("% 9g", val);
      testcase("% 10g", val);
      testcase("% 11g", val);
      testcase("% 12g", val);
      testcase("% 8.4f", val);
      testcase("% -8.4f", val);
   }

#ifdef __SUNPRO_CC
   // suppress the harmless warning regarding the intended overflow
   // of std::numeric_limits<double>::max() * 2
   #pragma error_messages (off, mulfovfl)
#endif
   double d_values[] = {-0.0, 1234.5678, 1.25E-10, 3E+10,
      std::numeric_limits<double>::min() / 2,
      std::numeric_limits<double>::max() * 2,
      std::numeric_limits<double>::min(),
      std::numeric_limits<double>::max(),
      std::numeric_limits<double>::lowest(),
      std::numeric_limits<double>::epsilon(),
      std::sqrt(-1.0), std::nan("1"), -std::nan("1")};
   for (auto val: d_values) {
      if (!nan_works && !std::isfinite(val)) continue; 
      testcase("%lf", val);
      testcase("%le", val);
      testcase("%lg", val);
      if (uppercase_inf_works || std::isfinite(val)) {
	 testcase("%lF", val);
	 testcase("%lE", val);
	 testcase("%lG", val);
      }
      if (hexfloat_works) {
	 testcase("%la", val);
	 if (uppercase_inf_works || std::isfinite(val)) {
	    testcase("%lA", val);
	 }
      }
      testcase("%10.2lf", val);
      testcase("%10.2le", val);
      testcase("%10.2lg", val);
      testcase("%#lg", val);
      testcase("% lf", val);
      testcase("% 1lf", val);
      testcase("% 2lf", val);
      testcase("% 3lf", val);
      testcase("% 4lf", val);
      testcase("% 5lf", val);
      testcase("% 6lf", val);
      testcase("% 7lf", val);
      testcase("% 8lf", val);
      testcase("% 9lf", val);
      testcase("% 10lf", val);
      testcase("% 11lf", val);
      testcase("% 12lf", val);
      testcase("% le", val);
      testcase("% 1le", val);
      testcase("% 2le", val);
      testcase("% 3le", val);
      testcase("% 4le", val);
      testcase("% 5le", val);
      testcase("% 6le", val);
      testcase("% 7le", val);
      testcase("% 8le", val);
      testcase("% 9le", val);
      testcase("% 10le", val);
      testcase("% 11le", val);
      testcase("% 12le", val);
      testcase("% lg", val);
      testcase("% 1lg", val);
      testcase("% 2lg", val);
      testcase("% 3lg", val);
      testcase("% 4lg", val);
      testcase("% 5lg", val);
      testcase("% 6lg", val);
      testcase("% 7lg", val);
      testcase("% 8lg", val);
      testcase("% 9lg", val);
      testcase("% 10lg", val);
      testcase("% 11lg", val);
      testcase("% 12lg", val);
   }

   long double ld_values[] = {-0.0L, 1234.5678L, 1.25E-10L, 3E+10L,
      std::numeric_limits<long double>::min() / 2,
      std::numeric_limits<long double>::max() * 2,
      std::numeric_limits<long double>::min(),
      std::numeric_limits<long double>::max(),
      std::numeric_limits<long double>::lowest(),
      std::numeric_limits<long double>::epsilon(),
      std::sqrt(-1.0L), std::nanl("1"), -std::nanl("1")};
   for (auto val: ld_values) {
      if (!nan_works && !std::isfinite(val)) continue; 
      testcase("%Lf", val);
      testcase("%Le", val);
      testcase("%Lg", val);
      if (uppercase_inf_works || std::isfinite(val)) {
	 testcase("%LF", val);
	 testcase("%LE", val);
	 testcase("%LG", val);
      }
      if (hexfloat_works) {
	 testcase("%La", val);
	 if (uppercase_inf_works || std::isfinite(val)) {
	    testcase("%LA", val);
	 }
      }
      testcase("%10.2Lf", val);
      testcase("%10.2Le", val);
      testcase("%10.2Lg", val);
      testcase("%#Lf", val);
      testcase("% Lf", val);
      testcase("% 1Lf", val);
      testcase("% 2Lf", val);
      testcase("% 3Lf", val);
      testcase("% 4Lf", val);
      testcase("% 5Lf", val);
      testcase("% 6Lf", val);
      testcase("% 7Lf", val);
      testcase("% 8Lf", val);
      testcase("% 9Lf", val);
      testcase("% 10Lf", val);
      testcase("% 11Lf", val);
      testcase("% 12Lf", val);
      testcase("% Le", val);
      testcase("% 1Le", val);
      testcase("% 2Le", val);
      testcase("% 3Le", val);
      testcase("% 4Le", val);
      testcase("% 5Le", val);
      testcase("% 6Le", val);
      testcase("% 7Le", val);
      testcase("% 8Le", val);
      testcase("% 9Le", val);
      testcase("% 10Le", val);
      testcase("% 11Le", val);
      testcase("% 12Le", val);
      testcase("% Lg", val);
      testcase("% 1Lg", val);
      testcase("% 2Lg", val);
      testcase("% 3Lg", val);
      testcase("% 4Lg", val);
      testcase("% 5Lg", val);
      testcase("% 6Lg", val);
      testcase("% 7Lg", val);
      testcase("% 8Lg", val);
      testcase("% 9Lg", val);
      testcase("% 10Lg", val);
      testcase("% 11Lg", val);
      testcase("% 12Lg", val);
   }

   const char* s_values[] = {"Hi", "Hallo", "", "Hello world"};
   for (auto val: s_values) {
      testcase("%s", val);
      testcase("%16s", val);
      testcase("%-16s", val);
      implementation_dependent_testcase("%p", val);
   }

   const char* ptr_values[] = {"Hi", nullptr};
   for (auto val: ptr_values) {
      implementation_dependent_testcase("[%p]", val);
   }
   implementation_dependent_testcase("[%p]", nullptr);
   char string_value[] = "Hi";
   implementation_dependent_testcase("[%p]", string_value);
   char* charptr_value = nullptr;
   implementation_dependent_testcase("[%p]", charptr_value);
   wchar_t wstring_value[] = L"Hi";
   implementation_dependent_testcase("[%p]", wstring_value);
   wchar_t* wcharptr_value = nullptr;
   implementation_dependent_testcase("[%p]", wcharptr_value);

   /* dynamic width and/or precision */
   for (int width = 0; width < 20; ++width) {
      testcase("%*s", width, "Hello world");
      testcase("%*lg", width, std::numeric_limits<double>::max());
      testcase("%*lg %d", width, std::numeric_limits<double>::max(), width);
      testcase("%*lg %d %d", width, std::numeric_limits<double>::max(),
	 width, width);
      testcase("%0*d", width, 1234);
      testcase("%*d", width, 1234);
      testcase("%*d", width, 1234);
      testcase("%*d %d", width, 1234, width);
      testcase("%*d %d %d", width, 1234, width, width);
      testcase("%.*lg %d", width, std::numeric_limits<double>::max(), width);
      testcase("%.*d", width, 1234);
      testcase("%.*d %d", width, 1234, width);
      testcase("%.*d %d %d", width, 1234, width, width);
      testcase("%.*lg %d", width, std::numeric_limits<double>::max(), width);
      testcase("%.*lg %d %d", width, std::numeric_limits<double>::max(),
	 width, width);
      for (int precision = 0; precision < 15; ++precision) {
	 testcase("%*.*s", width, precision, "Hello world");
	 testcase("%*.*lg", width, precision,
	    std::numeric_limits<double>::max());
	 testcase("%*.*lg %d %d", width, precision,
	    std::numeric_limits<double>::max(),
	    width, precision);
	 testcase("%*.*d", width, precision, 1234);
	 testcase("%0*.*d", width, precision, 1234);
      }
   }
   for (int precision = 0; precision < 15; ++precision) {
      testcase("%20.*lg", precision, std::numeric_limits<double>::max());
   }

   /* huge width values */
   testcase("%1024d", 42);
   testcase("%2048d", 42);
   testcase("%4095d", 42); /* see ISO 9899:2011 § 7.21.6.1 (15) */

   /* check that grouping flag is ignored when the base != 10 */
   testcase("%'8x", 0x12345678);
   testcase("%'8o", 0x12345678);

   /* test %n */
   int offset;
   testcase_with_offset(offset, "%n", &offset);
   testcase_with_offset(offset, "Hi!%n", &offset);
   testcase_with_offset(offset, "Hello,%n world!", &offset);
   testcase_with_offset(offset, "%s%n%s", "Hello, ", &offset, "world");

   /* tests of POSIX features */
   testcase("%1$s", "Hello world");
   testcase("%2$s, %1$s", "world", "hello");
   testcase("%3$*1$s %2$d", 20, 4711, "Hi!");
   testcase("%1$.*2$f", 1.23456789, 3);
   /* more POSIX feature tests below under a well-defined locale */

   /* wide characters;
      to test this, we change the locale for C, C++,
      and the two output streams
   */
   const char* locale_string = "en_US.UTF-8";
   bool locale_ok = std::setlocale(LC_ALL, locale_string);
   if (locale_ok) {
      /* this is likely to fail for g++ implementations
	 that are not based on glibc as they just
	 support "C" */
      try {
	 auto locale = std::locale(locale_string);
	 std::locale::global(locale);
	 std::cout.imbue(locale); std::wcout.imbue(locale);
      } catch (...) {
	 locale_ok = false;
      }
   }
   if (locale_ok) {
      /* make sure that C++ operates with the same locale */

      testcase(L"Hello world");

      /* note that we skip numeric_limits in case of wchar_t
	 as we could get EILSEQ error codes (happens when Xcode is used) */
      wchar_t wc_values[] = {L'a', L'A', L'.', L'/', L' ',
	 L'\u00fc', /* LATIN SMALL LETTER U WITH DIAERESIS */
	 L'\u017f', /* LATIN SMALL LETTER LONG S */
      };
      for (auto val: wc_values) {
	 testcase(L"%lc", val);
	 testcase(L"%C", val);
	 testcase(L"%lc%lc", val, val);
	 testcase(L"%lc %lc", val, val);
	 testcase(L"%8lc", val);
	 testcase(L"%-8lc", val);
      }
      testcase(L"%lc", 65);

      const wchar_t* ws_values[] = {L"Hi", L"Hallo", L"", L"Hello world",
	 L"\u00fc", /* LATIN SMALL LETTER U WITH DIAERESIS */
	 L"\u017f", /* LATIN SMALL LETTER LONG S */
      };
      for (auto val: ws_values) {
	 testcase(L"%ls", val);
	 testcase(L"%S", val);
	 testcase(L"%16ls", val);
	 testcase(L"%-16ls", val);
      }

      /* printing wide characters to a narrow stream */
      for (auto val: wc_values) {
	 testcase("%lc", val);
	 testcase("%4lc", val);
      }
      for (auto val: ws_values) {
	 testcase("%ls", val);
	 testcase("%4ls", val);
	 testcase("%10ls", val);
	 testcase("%10.2ls", val);
      }

      /* printing narrow characters to a wide stream;
	 we test just ASCII characters here;
	 others might fail with EILSEQ */
      char ac_values[] = {'a', 'A', '.', '/', ' ', '\t', '\n', '\0'};
      for (auto val: ac_values) {
	 testcase(L"%c", val);
	 testcase(L"%4c", val);
      }
      for (auto val: s_values) {
	 testcase(L"%s", val);
	 testcase(L"%4s", val);
	 testcase(L"%10s", val);
	 testcase(L"%10.2s", val);
      }

      /* POSIX extension for thousands under a non-C locale */

      /* check for GNU C library bug that generates an
         empty string for the following case */
      bool empty_bug = !check_printf("0", "%'*.*d", 0, 0, 0);
      if (empty_bug) ++broken;

      for (auto val: i_values) {
	 testcase("%d", val);
	 testcase("%'d", val);
	 for (int width = 0; width < 20; ++width) {
	    testcase("%'*d", width, val);
	    testcase("%d %'*d %d", val, width, val, val);
	    if (!empty_bug) {
	       for (int precision = 0; precision < 15; ++precision) {
		  testcase("%'*.*d", width, precision, val);
	       }
	    }
	 }
      }

      for (auto val: d_values) {
	 testcase("%f", val);
	 testcase("%g", val);
	 testcase("%e", val);
	 testcase("%'f", val);
	 testcase("%'g", val);
	 testcase("%'e", val);
	 for (int width = 0; width < 20; ++width) {
	    testcase("%'*f", width, val);
	    testcase("%'*g", width, val);
	    testcase("%'*e", width, val);
	    testcase("%f %'*f %f", val, width, val, val);
	    for (int precision = 0; precision < 15; ++precision) {
	       testcase("%'*.*f", width, precision, val);
	    }
	 }
      }
   }

   /* run special cases which are not supported by std::printf
      but fmt::printf */
   special_testcase(1, "0", "%d", reinterpret_cast<void*>(0));
   special_testcase(4, "1024", "%d", reinterpret_cast<void*>(1024));
   special_testcase(2, "-1", "%d", reinterpret_cast<void*>(-1));
   special_testcase(4, "2000", "%o", reinterpret_cast<void*>(1024));
   {
      TestObject<int> to{42};
      special_testcase(5, "42/TO", "%s", to);
      special_testcase(7, "  42/TO", "%4d", to);
   }
   {
      TestObject<double> to{1.5};
      special_testcase(6, "1.5/TO", "%s", to);
      special_testcase(8, "  1.5/TO", "%5s", to);
      special_testcase(9, "1.5000/TO", "%.4f", to);
   }
   {
      auto top = new TestObject<int>{64};
      special_testcase(5, "64/TO", "%s", top);
      delete top;
   }

   fmt::printf("%u/%u tests succeeded\n", successful, testcases);
   if (warnings > 0) {
      fmt::printf("%d implementation-dependent tests "
	 "delivered different results\n", warnings);
   }
   if (successful + warnings < testcases) {
      fmt::printf("%d tests failed\n", testcases - successful - warnings);
   }
   if (skipped > 0) {
      fmt::printf("%d tests skipped\n", skipped);
   }
   if (broken > 0) {
      fmt::printf("%d test series skipped where std::printf deviates from "
	 "standard\n", broken);
   }
   if (cpp_broken > 0) {
      fmt::printf("%d test series skipped where the C++ library does not "
	 "conform to the C++11 standard\n", cpp_broken);
   }
   if (!locale_ok) {
      fmt::printf("locale dependent tests skipped\n");
   }
}

int main() {
   run_tests();
}
