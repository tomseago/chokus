AC_INIT([chokus], [0.1], [tom@tomseago.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

PKG_CHECK_MODULES([YAML], [yaml-0.1])
PKG_CHECK_MODULES([ARGTABLE], [argtable2])

# Checks for programs.
#AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h stddef.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memmove memset])



AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile ])
AC_OUTPUT
