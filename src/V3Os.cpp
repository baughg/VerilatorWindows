// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
// DESCRIPTION: Verilator: Os-specific function wrapper
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

#include "config_build.h"
#include "verilatedos.h"
#include <cstdarg>
#include <sys/stat.h>
// #include <sys/time.h>
/* PORT*/
#include <time.h>
#include <windows.h> 

const __int64 DELTA_EPOCH_IN_MICROSECS = 11644473600000000;

/* IN UNIX the use of the timezone struct is obsolete;
I don't know why you use it. See http://linux.about.com/od/commands/l/blcmdl2_gettime.htm
But if you want to use this structure to know about GMT(UTC) diffrence from your local time
it will be next: tz_minuteswest is the real diffrence in minutes from GMT(UTC) and a tz_dsttime is a flag
indicates whether daylight is now in use
*/
struct timezone2
{
  __int32  tz_minuteswest; /* minutes W of Greenwich */
  bool  tz_dsttime;     /* type of dst correction */
};

struct timeval2 {
  __int32    tv_sec;         /* seconds */
  __int32    tv_usec;        /* microseconds */
};

int gettimeofday(struct timeval2 *tv/*in*/, struct timezone2 *tz/*in*/)
{
  FILETIME ft;
  __int64 tmpres = 0;
  TIME_ZONE_INFORMATION tz_winapi;
  int rez = 0;

  ZeroMemory(&ft, sizeof(ft));
  ZeroMemory(&tz_winapi, sizeof(tz_winapi));

  GetSystemTimeAsFileTime(&ft);

  tmpres = ft.dwHighDateTime;
  tmpres <<= 32;
  tmpres |= ft.dwLowDateTime;

  /*converting file time to unix epoch*/
  tmpres /= 10;  /*convert into microseconds*/
  tmpres -= DELTA_EPOCH_IN_MICROSECS;
  tv->tv_sec = (__int32)(tmpres*0.000001);
  tv->tv_usec = (tmpres % 1000000);


  //_tzset(),don't work properly, so we use GetTimeZoneInformation
  rez = GetTimeZoneInformation(&tz_winapi);
  tz->tz_dsttime = (rez == 2) ? true : false;
  tz->tz_minuteswest = tz_winapi.Bias + ((rez == 2) ? tz_winapi.DaylightBias : 0);

  return 0;
}

/*PORT*/
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <climits>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>
#include <iomanip>
#include <memory>

#if defined(WIN32) || defined(__MINGW32__)
# include <direct.h>  // mkdir
#endif

#include "V3Global.h"
#include "V3String.h"
#include "V3Os.h"

//######################################################################
// Environment

string V3Os::getenvStr(const string& envvar, const string& defaultValue) {
    if (const char* envvalue = getenv(envvar.c_str())) {
	return envvalue;
    } else {
	return defaultValue;
    }
}

void V3Os::setenvStr(const string& envvar, const string& value, const string& why) {
    if (why != "") {
	UINFO(1,"export "<<envvar<<"="<<value<<" # "<<why<<endl);
    } else {
	UINFO(1,"export "<<envvar<<"="<<value<<endl);
    }
#if !defined(__MINGW32__) && (defined(_BSD_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L))
    setenv(envvar.c_str(),value.c_str(),true);
#else
    //setenv() replaced by putenv() in MinGW/Solaris environment. Prototype is different
    //putenv() requires NAME=VALUE format
    string vareq = envvar + "=" + value;
    putenv(const_cast<char*>(vareq.c_str()));
#endif
}

//######################################################################
// Generic filename utilities

string V3Os::filenameFromDirBase (const string& dir, const string& basename) {
    // Don't return ./{filename} because if filename was absolute, that makes it relative
    if (dir == ".") return basename;
    else return dir+"/"+basename;
}

string V3Os::filenameDir (const string& filename) {
    string::size_type pos;
    if ((pos = filename.rfind("/")) != string::npos) {
	return filename.substr(0,pos);
    } else {
	return ".";
    }
}

string V3Os::filenameNonDir (const string& filename) {
    string::size_type pos;
    if ((pos = filename.rfind("/")) != string::npos) {
	return filename.substr(pos+1);
    } else {
	return filename;
    }
}

string V3Os::filenameNonExt (const string& filename) {
    string base = filenameNonDir(filename);
    string::size_type pos;
    if ((pos = base.find(".")) != string::npos) {
	base.erase(pos);
    }
    return base;
}

string V3Os::filenameSubstitute (const string& filename) {
    string out;
    enum { NONE, PAREN, CURLY } brackets = NONE;
    for (string::size_type pos = 0; pos < filename.length(); ++pos) {
        if ((filename[pos] == '$') && (pos+1 < filename.length())) {
	    switch (filename[pos+1]) {
	        case '{': brackets = CURLY; break;
	        case '(': brackets = PAREN; break;
	        default: brackets = NONE; break;
	    }
	    if (brackets != NONE) pos = pos+1;
	    string::size_type endpos = pos+1;
	    while (((endpos+1) < filename.length()) &&
		   (((brackets==NONE) && (isalnum(filename[endpos+1]) || filename[endpos+1]=='_')) ||
		    ((brackets==CURLY) && (filename[endpos+1]!='}')) ||
		    ((brackets==PAREN) && (filename[endpos+1]!=')'))))
		++endpos;
	    // Catch bracket errors
	    if (((brackets==CURLY) && (filename[endpos+1]!='}')) ||
		((brackets==PAREN) && (filename[endpos+1]!=')'))) {
	      v3fatal("Unmatched brackets in variable substitution in file: "+filename);
	    }
	    string envvar = filename.substr(pos+1,endpos-pos);
	    const char* envvalue = NULL;
	    if (envvar != "") envvalue = getenv(envvar.c_str());
	    if (envvalue) {
		out += envvalue;
		if (brackets==NONE) pos = endpos;
		else pos = endpos+1;
	    } else {
		out += filename[pos];  // *pos == '$'
	    }
	} else {
	    out += filename[pos];
	}
    }
    return out;

}

string V3Os::filenameRealPath(const string& filename) {
    // Get rid of all the ../ behavior in the middle of the paths.
    // If there is a ../ that goes down from the 'root' of this path it is preserved.
    char retpath[PATH_MAX];
    if (
#if defined( _MSC_VER ) || defined( __MINGW32__ )
	::_fullpath(retpath,filename.c_str(),PATH_MAX)
#else
	realpath(filename.c_str(), retpath)
#endif
	) {
	return string(retpath);
    } else {
	return filename;
    }
}

bool V3Os::filenameIsRel(const string& filename) {
    return (filename.length()>0 && filename[0] != '/');
}

//######################################################################
// Directory utilities

void V3Os::createDir(const string& dirname) {
#if defined(_WIN32) || defined(__MINGW32__)
    mkdir(dirname.c_str());
#else
    mkdir(dirname.c_str(), 0777);
#endif
}

void V3Os::unlinkRegexp(const string& dir, const string& regexp) {
    if (DIR* dirp = opendir(dir.c_str())) {
	while (struct dirent* direntp = readdir(dirp)) {
	    if (VString::wildmatch(direntp->d_name, regexp.c_str())) {
		string fullname = dir + "/" + string(direntp->d_name);
		unlink (fullname.c_str());
	    }
	}
	closedir(dirp);
    }
}

//######################################################################
// METHODS (performance)

uint64_t V3Os::timeUsecs() {
#if defined(_WIN32) || defined(__MINGW32__)
    return 0;
#else
    timeval tv;
    if (gettimeofday(&tv, NULL) < 0) return 0;
    return static_cast<uint64_t>(tv.tv_sec)*1000000 + tv.tv_usec;
#endif
}

uint64_t V3Os::memUsageBytes() {
#if defined(_WIN32) || defined(__MINGW32__)
    return 0;
#else
    // Highly unportable. Sorry
    const char* const statmFilename = "/proc/self/statm";
    FILE* fp = fopen(statmFilename,"r");
    if (!fp) {
	return 0;
    }
    uint64_t size, resident, share, text, lib, data, dt;  // All in pages
    if (7 != fscanf(fp, "%" VL_PRI64 "d %" VL_PRI64 "d %" VL_PRI64 "d %"
		    VL_PRI64 "d %" VL_PRI64 "d %" VL_PRI64 "d %" VL_PRI64 "d",
		    &size, &resident, &share, &text, &lib, &data, &dt)) {
	fclose(fp);
	return 0;
    }
    fclose(fp);
    return (text + data) * getpagesize();
#endif
}
