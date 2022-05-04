# fmt
fmt::printf as extensible C++ replacement for std::printf

## Summary

This header-only C++11 package provides fmt::printf which
is intended as a type-safe and extensible drop-in replacement
for std::printf. The principal idea is to replace

```C++
#include <cstdio>
// ...
int count = std::printf("%d\n", val);
count = std::fprintf(stderr, "%s\n", errmsg);
count = std::snprintf(buf, sizeof buf, "%d", val);
count = std::wprintf(L"%d\n", val);
```

by

```C++
#include "printf.hpp"
// ...
int count = fmt::printf("%d\n", val);
count = fmt::printf(std::cerr, "%s\n", errmsg);
count = fmt::snprintf(buf, sizeof buf, "%d\n", val);
count = fmt::printf(L"%d\n", val); // goes to std::wcout
```

where the behaviour is expected to be identical with the exception that
fmt::printf prints to extensions of basic_ostream instead of `FILE*`
and that the locale of the output stream is used instead of the global
C locale.

Thanks to the variadic templates of C++11, fmt::printf provides the
functionality of std::printf in a typesafe way.  Consequently, it no
longer matters for fmt::printf whether you use `"%f"`, `"%lf"`, or
`"%Lf"` as format as the associated operand type is well known.
fmt::printf is extensible as all operand types are supported
for which an <<-operator exists. Example for std::complex:

```C++
std::complex c = /* ... */;
fmt::printf("c = %20.4g\n", c);
```

fmt::printf uses C++ I/O format flags but makes sure that the previous
state of the output stream is restored to its original state after its
invocation. Any previous state is ignored, i.e. `fmt::printf("%x", val)`
will print _val_ in hex even if `std::cout << std::oct` has been used
before, and the previous octal conversion preference will stay in
effect for <<-operators after the invocation of fmt::printf.

This implementation aims to support faithfully all features of
std::printf according to ISO 9899:2011 and IEEE Std 1003.1, 2013
(POSIX). Other important points were to keep it small and
header-only. These goals have been more important than performance.

This implementation is based on the C++ standard
library and its existing conversions. Some work has
been spent to work around incompatibilities between
C und C++ in regard to formatted printing. There
exists, however, one known exception where fmt::printf
diverts from standard behaviour of std::printf. The problem
is the combination of `"%a"` with a precision, e.g. `"%.2a"`.
This odd combination is not supported by C++11 (see 22.4.2.2.2
in ISO 14882:2011) but supported by std::printf (see 7.21.6.1
in ISO 9899:2011). But it may be questioned how common
or useful it is to combine this.

In particular, fmt::printf supports

* the return value, i.e. the number of bytes written,
* the precision is honored by %s and integer conversions
  (in contrast to std::setprecision which is honored
  by floating point conversions only)
* conversions that take the n-th argument, e.g.
  `print("%2$s, %1$s\n", "world", "Hello");`, and
* `%n` conversions that take an `int*` where the
  number of bytes written so far is stored.

Note that fmt::printf, much like std::printf, is
locale-dependent. But std::printf is based on the
global C locale whereas the behaviour of fmt::printf
depends on the locale of the output stream. One
notable exception is grouping. This is supported
by the C++ locale system but not by C. Instead
std::printf provides the apostrophe format flag
that asks for a grouping with thousands. To
conform to std::printf behaviour, the grouping
by fmt::printf depends solely on the use of the
apostrophe flag, not on the locale.

## Extensions
fmt::printf allows to print pointers
using `%d` or `%o` as decimal or octal values,
respectively. `%u` allows to print pointers
as an unsigned decimal value.

Output operators for pointers are supported.
The format `%s` should be used to enforce
the use of the output operator. Otherwise,
formats like `%d`, `%o`, or `%p` cause the
pointer value to be printed and the output
operator to be ignored.

In general, `%s` should be prefered for
types where an output operator is available.
There are few exceptions like `std::complex`
where formats like `%g` are supported. fmt::printf
configures all `std::ios_base` formatting flags
in these cases but leaves their interpretation to
the matching output operator.

## License

This package is available under the terms of
the [MIT License](https://opensource.org/licenses/MIT).

## Files

To use fmt::printf, you will need just to drop
[printf.hpp](https://github.com/afborchert/fmt/blob/master/printf.hpp)
within your project and `#include` it.

The source file `test_suite.cpp` is a test suite
testing fmt::printf against std::printf and
the Makefile helps to compile it.

## Alternatives

This is not the first attempt to provide printf look
and feel in a type-safe way for C++. There exist
numerous other implementations. I want to name
a few:

 * In 1994, Cay S. Horstmann
   [published an article](http://horstmann.com/cpp/iostreams.html)
   about extending the iostreams library in _C++ Report_ where he
   proposed _setformat_ which takes one format specification
   and configures the stream accordingly like other manipulators.
   Example from his paper:

	cout << "(" << setformat("%8.2f") << x << ","
	    << setformat("8.2f") << y << ")" << endl;

 * The [Boost Format library offers an
   approach](http://www.boost.org/doc/libs/1_59_0/libs/format/doc/format.html)
   that permits the grouping of multiple conversions.
   As the %-operator is used, it does not depend on
   variadic templates. Example:

	std::cout << boost::format("(x, y) = (%4f, %4f)\n" % x % y;

 * Zhihao Yuan [proposed a C++ standard extension](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3506.html)
   providing a printf-like interface for the C++ streams library that is
   [likewise available at Github](https://github.com/lichray/formatxx).
   Example:

	std::cout << std::putf("(x, y) = (%4f, %4f)\n", x, y);

 * [C++ Format](http://cppformat.github.io/latest/index.html)
   by Victor Zverovich provides a library which is no longer
   header-only with two APIs, one of them offering format strings
   as in Python. Example:

	fmt::print("I'd rather be {1} than {0}.", "right", "happy");
