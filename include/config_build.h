/* src/config_build.h.  Generated from config_build.h.in by configure.  */
// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
// DESCRIPTION: Verilator: Configure source; system configuration
//
// This file is part of Verilator.
//
// Code available from: http://www.veripool.org/verilator
//
//*************************************************************************
//
// Copyright 2003-2018 by Wilson Snyder.  This program is free software; you can
// redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
//
// Verilator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//*************************************************************************

//**********************************************************************
//**** Version and host name

// Autoconf substitutes this with the strings from AC_INIT.
#define PACKAGE_STRING "Verilator 3.922 2018-03-17"

#define DTVERSION PACKAGE_STRING

//**********************************************************************
//**** Functions

//**********************************************************************
//**** Headers

//**********************************************************************
//**** Default environment

// Set defines to defaults for environment variables
// If set to "", this default is ignored and the user is expected
// to set them at Verilator runtime.

#ifndef DEFENV_SYSTEMC
# define DEFENV_SYSTEMC ""
#endif
#ifndef DEFENV_SYSTEMC_ARCH
# define DEFENV_SYSTEMC_ARCH ""
#endif
#ifndef DEFENV_SYSTEMC_INCLUDE
# define DEFENV_SYSTEMC_INCLUDE ""
#endif
#ifndef DEFENV_SYSTEMC_LIBDIR
# define DEFENV_SYSTEMC_LIBDIR ""
#endif
#ifndef DEFENV_VERILATOR_ROOT
# define DEFENV_VERILATOR_ROOT ""
#endif

//**********************************************************************
//**** Compile options

#include <sys/types.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64) 
#define snprintf _snprintf 
#define vsnprintf _vsnprintf 
#define strcasecmp _stricmp 
#define strncasecmp _strnicmp 
#endif

using namespace std;

//**********************************************************************
//**** Configure-discovered library options

// Define if struct stat has st_mtim.tv_nsec (from configure)
//#define HAVE_STAT_NSEC 1

//**********************************************************************
//**** OS and compiler specifics

#include "verilatedos.h"