# Cmake file for less
# by Pedro Oliva Rodrigues

cmake_minimum_required(VERSION 3.8)
project("gwsw-less")
set(PROJECT_HOMEPAGE_URL "http://www.greenwoodsoftware.com/less")

if(WIN32)
    set(EDIT_PGM_ "notepad")
    set(EDIT_PROTO_ "%E %g")
    if(NOT SYSDIR)
        set(SYSDIR "c:")
    endif()
else()
    set(EDIT_PGM_ "vi")
    set(EDIT_PROTO_ "%E ?lm+%lm. %g")
endif()

if(MINGW)
    add_compile_definitions(MINGW)
endif()


# User preferences
option(SECURE_COMPILE "If enabled, disables a bunch of features in order to be safe to run by unprivileged users." OFF)
option(CMD_HISTORY "Enable if you wish to allow keys to cycle through previous commands at prompts." ON)
option(HILITE_SEARCH "Enable if you wish to have search targets to be displayed in standout mode." ON)
set(EDIT_PGM ${EDIT_PGM_} CACHE STRING "Name of the default to be invoked by the 'v' command if EDITOR env. var. is not set")
set(EDIT_PROTO ${EDIT_PROTO_} CACHE STRING "The default editor prototype, if LESSEDIT is not set.")
option(GNU_OPTIONS "Enable if you wish to support the GNU-style command line options --help and --version." ON)
option(ONLY_RETURN "Enable if you want RETURN to be the only input which will continue past an error message. Otherwise, any key will continue past an error message." OFF)
set(LESSKEYFILE ".less" CACHE STRING "Filename of the default lesskey output file (in the HOME directory).")
set(LESSKEYFILE_SYS "sysless" CACHE STRING "Filename of the system-wide lesskey output file.")
set(DEF_LESSKEYINFILE ".lesskey" CACHE STRING "Filename of the default lesskey input (in the HOME directory).")
set(LESSHISTFILE ".lesshst" CACHE STRING "Filename of the history file (in the HOME directory).")
set(REGEX_LIBRARY regcomp-local CACHE STRING "Select what regular expression library to use ([regcomp-local],posix,pcre2,off).")
option(USE_PYTHON "Use python instead of perl to generate files" OFF)

set(SECURE ${SECURE_COMPILE})

find_package(Perl)
find_package(Python3)

if(PERL_FOUND AND NOT USE_PYTHON)
    set(generator ${PERL_EXECUTABLE})
    set(USE_PERL true)
elseif(Python3_FOUND)
    set(generator ${Python3_EXECUTABLE})
else()
    set(generator generator-NOTFOUND)
endif()

message("Generator: ${generator}")

function(generate_file_cmd  outputfile basedir COMMAND exe script)
    set(location ${basedir}/${outputfile})
    if(NOT EXISTS location)
        set(genfile ${basedir}/${outputfile})
        add_custom_command(OUTPUT ${genfile}
            DEPENDS ${exe} ${script}
            COMMAND ${exe} ${script} ${ARGN}
            WORKING_DIRECTORY ${basedir}
            COMMENT "Generating ${genfile} ..."
        )
    else()
        set(genfile ${location})
    endif()
endfunction()

if(USE_PERL)
    set(script mkhelp.pl)
else()
    set(script mkhelp.py)
endif()

generate_file_cmd(help.c ${CMAKE_SOURCE_DIR} COMMAND ${generator} ${script} < "less.hlp" > help.c)


set(LESS_SRC main.c screen.c brac.c ch.c charset.c cmdbuf.c
    "command.c" cvt.c decode.c edit.c filename.c forwback.c
    ifile.c input.c jump.c line.c linenum.c
    lsystem.c mark.c optfunc.c option.c opttbl.c os.c
    output.c pattern.c position.c prompt.c search.c signal.c
    tags.c ttyin.c)

add_executable(less ${LESS_SRC} help.c)

add_executable(lesskey lesskey.c)
add_executable(lessecho lessecho.c)

# lib for common files and settings
add_library(common OBJECT version.c shared.c)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_BINARY_DIR}) # for defines.h

target_link_libraries(less PRIVATE common)
target_link_libraries(lesskey PRIVATE common)
target_link_libraries(lessecho PRIVATE common)

# dependency detection
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckStructHasMember)

# checks
check_include_files("termios.h" HAVE_TERMIOS_H)
check_include_files("termio.h"  HAVE_TERMIO_H)
check_include_files("termcap.h" HAVE_TERMCAP_H)
check_include_files("langinfo.h"   HAVE_LANGINFO)
check_include_files("sys/ioctl.h"  HAVE_SYS_IOCTL_H)
check_include_files("sys/ptem.h"   HAVE_SYS_PTEM_H)
check_include_files("sys/stream.h" HAVE_SYS_STREAM_H)
list(APPEND CMAKE_EXTRA_INCLUDE_FILES "signal.h")
check_type_size("(sigset_t*)0" SIGSET_T)
check_symbol_exists(sigemptyset "signal.h" HAVE_SIGEMPTYSET)
check_include_files("regex.h" HAVE_POSIX_REGCOMP)
check_symbol_exists(realpath "stdlib.h" HAVE_REALPATH)
if(UNIX)
    check_struct_has_member("struct stat" st_ino "sys/stat.h" HAVE_STAT_INO)
endif()

# find terminal libraries
find_package(Curses) # curses or ncurses
find_library(TINFO_LIB NAMES tinfo libtinfo.so libtinfo.so.5)
find_library(TERMCAP_LIB NAMES termcap libtermcap.so)

if(WIN32)
    set(TERMINAL_LIBRARY WindowsConsole)
elseif(TINFO_LIB)
    set(TERMINAL_LIBRARY tinfo)
    target_link_libraries(less PRIVATE ${TINFO_LIB})
elseif(CURSES_FOUND)
    target_include_directories(less PRIVATE ${CURSES_INCLUDE_DIRS})
    set(TERMINAL_LIBRARY curses)
    target_link_libraries(less PRIVATE ${CURSES_LIBRARIES})
elseif(TERMCAP_LIB)
    set(TERMINAL_LIBRARY termcap)
    target_link_libraries(less PRIVATE ${TERMCAP_LIB})
else()
    set(TERMINAL_LIBRARY NOTFOUND)
endif()

message(STATUS "terminal lib: ${TERMINAL_LIBRARY}")

if(NOT TERMINAL_LIBRARY)
    message(SEND_ERROR "Cannot find terminal libraries")
endif()

# find regex library
if(REGEX_LIBRARY MATCHES regcomp-local)
    target_sources(less PRIVATE ${CMAKE_SOURCE_DIR}/regexp.c)
    set(HAVE_V8_REGCOMP true)
    set(HAVE_REGEXEC2 true)
    set(found yes)
elseif(REGEX_LIBRARY MATCHES posix)
    set(found HAVE_POSIX_REGCOMP)
elseif(REGEX_LIBRARY MATCHES pcre2)
    find_library(pcre2_lib NAMES pcre2 libpcre2-8.so libpcre2-8.so.0)
    if(pcre2_lib)
        set(HAVE_PCRE2 true)
        target_link_libraries(less PRIVATE ${pcre2_lib})
        set(found yes)
    endif()
elseif(NOT REGEX_LIBRARY)
    set(NO_REGEX true)
    set(found yes)
endif()

message(STATUS "regex lib: ${REGEX_LIBRARY}")

if(NOT found)
    message("Cannot find regex lib")
    set(REGEX_LIBRARY NOTFOUND)
endif()

configure_file(defines.cmake.in defines.h)

install(TARGETS "less" lesskey lessecho)