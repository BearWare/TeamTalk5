///////////////////////////////////////////////////////////////////////////////
// StringPrintf.h
// Typesafe Printf substitute for STL-Strings
// (C) Stefan Woerthmueller  Berlin  2005
//     StefanWoe@compuserve.de
///////////////////////////////////////////////////////////////////////////////

#ifndef __STRING_PRINTF__
#define __STRING_PRINTF__
#include "CPrintfArg.h"
#include <cstdio>
///////////////////////////////////////////////////////////////////////////////
// StringPrintf-versions for std::string
///////////////////////////////////////////////////////////////////////////////
size_t StringPrintf(std::string &out, const std::string &fmt);
size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&);
size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&);

size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(std::string &out, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
///////////////////////////////////////////////////////////////////////////////
// StringPrintf-versions for C-strings
///////////////////////////////////////////////////////////////////////////////
size_t StringPrintf(char *, const std::string &fmt);
size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&);
size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&);

size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
///////////////////////////////////////////////////////////////////////////////
// StringPrintf-versions for C-strings with length-parameter (snprintf-like)
///////////////////////////////////////////////////////////////////////////////
size_t StringPrintf(char *, size_t,  const std::string &fmt);
size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&);
size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&);

size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(char *, size_t,  const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
///////////////////////////////////////////////////////////////////////////////
// StringPrintfCout-versions for Console output (printf-like)
///////////////////////////////////////////////////////////////////////////////
size_t StringPrintfCout(const std::string &fmt);
size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&);
size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&);

size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&);

size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintfCout(const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
///////////////////////////////////////////////////////////////////////////////
// StringPrintfCout-versions for C-File output (fprintf-like)
///////////////////////////////////////////////////////////////////////////////
size_t StringPrintf(FILE *, const std::string &fmt);
size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&);
size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                const CPrintfArg&);

size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);

size_t StringPrintf(FILE *, const std::string &fmt, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&,
                                                                    const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&, const CPrintfArg&);
///////////////////////////////////////////////////////////////////////////////
// Macros to redefine C-library calls
///////////////////////////////////////////////////////////////////////////////

//#ifndef sprintf
//#define   sprintf StringPrintf
//#endif
//#ifndef snprintf
//    #define  snprintf StringPrintf
//#endif
//#ifndef _snprintf
//    #define _snprintf StringPrintf
//#endif
//#ifndef printf
//    #define    printf StringPrintfCout
//#endif
//#ifndef fprintf
//    #define    fprintf StringPrintf
//#endif


#endif 
