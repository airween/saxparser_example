#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([saxparser], [0.1], [airween@digitalwave.hu])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libxml headers and libs
AM_PATH_XML2([2.9.0])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# check host os
AC_CANONICAL_HOST

build_linux=no
build_mac=no
os_type=unknown

case "${host_os}" in
    linux*)
        build_linux=yes
        os_type=Linux
        ;;
    darwin*)
        build_mac=yes
        os_type=Darwin
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_OUTPUT

echo \
"----------------------------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION} configuration:

 OS Type        ${os_type}
 Prefix         ${prefix}
 Preprocessor   ${CPP} ${CPPFLAGS}
 C Compiler     ${CC} ${CFLAGS}
 LibXML2 ver    $xml_config_major_version.$xml_config_minor_version.$xml_config_micro_version

-----------------------------------------------------------------------"
