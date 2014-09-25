///////////////////////////////////////////////////////////////////////////////
// StringPrintf.CPP
// Typesafe printf substitute for STL-Strings
// (C) Stefan Woerthmueller  Berlin  2005
///////////////////////////////////////////////////////////////////////////////
#include "StringPrintf.h"
#include <stdlib.h>
#include <string.h>
#include <iostream> 

#ifdef DEBUG

#if defined(WIN32) && ! defined (_CONSOLE)
#include "Windows.h" // Needed only for MessageBox
#endif
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
using namespace std;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static const char * FMT_BEGIN = "%";                            // Starting character of all printf formats
static const char * FMT_TYPES  = "cCdiouxXeEfgGnpsS"; 
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void StringPrintfErrorMessage(const string &s)

{
    //#if defined(WIN32) && ! defined (_CONSOLE)
    //    MessageBox(NULL, s.c_str(), "printf format string error", MB_OK | MB_ICONERROR);
    //#else
    //    cout << s;    
    //#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t xStringPrintf(string &out, const string &fmt, const CPrintfArg **arg1, int argCount)

{   int lastPos = 0, badFmtErr = 0, i = 0;

out.erase();
for(i = 0; ; i++)
{
    // Find next single format string, beginning with "%"
    int fmtPos = fmt.find(FMT_BEGIN, lastPos);

    // No more formats,
    if(fmtPos == string::npos)
        break;

    // Copy string in between to output string, if existent
    if(fmtPos != lastPos)
        out += fmt.substr(lastPos, fmtPos - lastPos);

    // Check for double "%%" - this should output "%"
    if(fmt.find(FMT_BEGIN, fmtPos + 1) == fmtPos + 1)
    {
        out     += "%";
        lastPos  = fmtPos + 2;            
        i--;
        continue;
    }

    // Find the end of the format string.
    // This must be one of the characters in FMT_TYPES
    const char * start = fmt.c_str() + fmtPos;
    const char *p = strpbrk(start, FMT_TYPES);

    // End of format-string not found. This is an Error in the format string
    // Just copy the remaining format string
    if(p == NULL)
    {
#ifdef DEBUG
        string s;
        StringPrintf(s, "WARNING: Error in format string \"%s\".\n"
            "         Single '%%' found without type character\n", fmt.c_str()); 
        StringPrintfErrorMessage(s);
        badFmtErr = 1;
#endif
        lastPos = fmtPos;
        break;
    }
    // Pass the single format string to CPrintfArg::Out.
    // CPrintfArg::Out will return the value it contains
    // converted to a string according to the format string
    // we add the result to the output string
    int len = p - start;
    if(i < argCount)
    {
        out += arg1[i]->Out(fmt.substr(fmtPos, len + 1));
        lastPos = fmtPos + 1 + len;
    }
    else
    {            
        out += string(start, len);
        lastPos = fmtPos + len;
    }
}
#ifdef DEBUG
if(i != argCount && ! badFmtErr)
{
    string s;
    StringPrintf(s, "WARNING: Invalid argument count for format string \"%s\".\n"
        "         %d arguments expected, %d arguments supplied\n", 
        fmt.c_str(), i, argCount); 
    StringPrintfErrorMessage(s);
}
#endif

// Append remaing format string
out += fmt.substr(lastPos);
return out.size();

};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t xStringPrintf(char *out, const string &fmt, const CPrintfArg **arg1, int argCount)

{   string s;
int result = xStringPrintf(s, fmt, arg1, argCount);

strcpy(out, s.c_str()); // Mhhhhh Dangerous strcpy
return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t xStringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg **arg1, int argCount)

{   string s;
int result = xStringPrintf(s, fmt, arg1, argCount);

strncpy(out, s.c_str(), bufflen); 
return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg **arg1, int argCount)

{   string s;
int result = xStringPrintf(s, fmt, arg1, argCount);

cout << s; 
return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg **arg1, int argCount)

{   string s;
int result = xStringPrintf(s, fmt, arg1, argCount);

int err = fputs(s.c_str(), file);
if(err < 0)
return err;
return result;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt)

{   return xStringPrintf(out, fmt, NULL, 0);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1)

{   CPrintfArg const *args[1];
args[0] = &arg1;

return xStringPrintf(out, fmt, args, 1);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2)

{   
    CPrintfArg const *args[2];
    args[0] = &arg1;
    args[1] = &arg2;

    return xStringPrintf(out, fmt, args, 2);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3)

{   
    CPrintfArg const *args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;

    return xStringPrintf(out, fmt, args, 3);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4)

{   
    CPrintfArg const *args[4];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;

    return xStringPrintf(out, fmt, args, 4);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5)

{   
    CPrintfArg const *args[5];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;

    return xStringPrintf(out, fmt, args, 5);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6)

{   
    CPrintfArg const *args[6];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;

    return xStringPrintf(out, fmt, args, 6);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7)

{   
    CPrintfArg const *args[7];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;

    return xStringPrintf(out, fmt, args, 7);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8)

{   
    CPrintfArg const *args[8];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;

    return xStringPrintf(out, fmt, args, 8);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9)

{   
    CPrintfArg const *args[9];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;

    return xStringPrintf(out, fmt, args, 9);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10)

{   
    CPrintfArg const *args[10];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;
    args[9] = &arg10;

    return xStringPrintf(out, fmt, args, 10);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11)

{   
    CPrintfArg const *args[11];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;

    return xStringPrintf(out, fmt, args, 11);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(string &out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11, const CPrintfArg &arg12)

{   
    CPrintfArg const *args[12];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;
    args[11] = &arg12;

    return xStringPrintf(out, fmt, args, 12);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt)

{   
    return xStringPrintf(out, fmt, NULL, 0);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1)

{   CPrintfArg const *args[1];
args[0] = &arg1;

return xStringPrintf(out, fmt, args, 1);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2)

{   
    CPrintfArg const *args[2];
    args[0] = &arg1;
    args[1] = &arg2;

    return xStringPrintf(out, fmt, args, 2);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3)

{   
    CPrintfArg const *args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;

    return xStringPrintf(out, fmt, args, 3);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4)

{   
    CPrintfArg const *args[4];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;

    return xStringPrintf(out, fmt, args, 4);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5)

{   
    CPrintfArg const *args[5];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;

    return xStringPrintf(out, fmt, args, 5);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6)

{   
    CPrintfArg const *args[6];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;

    return xStringPrintf(out, fmt, args, 6);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7)

{   
    CPrintfArg const *args[7];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;

    return xStringPrintf(out, fmt, args, 7);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8)

{   
    CPrintfArg const *args[8];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;

    return xStringPrintf(out, fmt, args, 8);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9)

{   
    CPrintfArg const *args[9];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;

    return xStringPrintf(out, fmt, args, 9);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10)

{   
    CPrintfArg const *args[10];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;
    args[9] = &arg10;

    return xStringPrintf(out, fmt, args, 10);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11)

{   
    CPrintfArg const *args[11];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;

    return xStringPrintf(out, fmt, args, 11);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11, const CPrintfArg &arg12)

{   
    CPrintfArg const *args[12];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;
    args[11] = &arg12;

    return xStringPrintf(out, fmt, args, 12);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt)

{   
    return xStringPrintf(out, bufflen, fmt, NULL, 0);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1)

{   CPrintfArg const *args[1];
args[0] = &arg1;

return xStringPrintf(out, bufflen, fmt, args, 1);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2)

{   
    CPrintfArg const *args[2];
    args[0] = &arg1;
    args[1] = &arg2;

    return xStringPrintf(out, bufflen, fmt, args, 2);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3)

{   
    CPrintfArg const *args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;

    return xStringPrintf(out, bufflen, fmt, args, 3);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4)

{   
    CPrintfArg const *args[4];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;

    return xStringPrintf(out, bufflen, fmt, args, 4);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5)

{   
    CPrintfArg const *args[5];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;

    return xStringPrintf(out, bufflen, fmt, args, 5);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6)

{   
    CPrintfArg const *args[6];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;

    return xStringPrintf(out, bufflen, fmt, args, 6);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7)

{   
    CPrintfArg const *args[7];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;

    return xStringPrintf(out, bufflen, fmt, args, 7);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8)

{   
    CPrintfArg const *args[8];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;

    return xStringPrintf(out, bufflen, fmt, args, 8);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9)

{   
    CPrintfArg const *args[9];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;

    return xStringPrintf(out, bufflen, fmt, args, 9);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10)

{   
    CPrintfArg const *args[10];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;
    args[9] = &arg10;

    return xStringPrintf(out, bufflen, fmt, args, 10);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11)

{   
    CPrintfArg const *args[11];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;

    return xStringPrintf(out, bufflen, fmt, args, 11);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(char *out, size_t bufflen, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11, const CPrintfArg &arg12)

{   
    CPrintfArg const *args[12];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;
    args[11] = &arg12;

    return xStringPrintf(out, bufflen, fmt, args, 12);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt)

{   
    return StringPrintfCout(fmt, NULL, 0);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1)

{   CPrintfArg const *args[1];
args[0] = &arg1;

return StringPrintfCout(fmt, args, 1);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2)

{   
    CPrintfArg const *args[2];
    args[0] = &arg1;
    args[1] = &arg2;

    return StringPrintfCout(fmt, args, 2);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3)

{   
    CPrintfArg const *args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;

    return StringPrintfCout(fmt, args, 3);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4)

{   
    CPrintfArg const *args[4];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;

    return StringPrintfCout(fmt, args, 4);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5)

{   
    CPrintfArg const *args[5];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;

    return StringPrintfCout(fmt, args, 5);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5, const CPrintfArg &arg6)

{   
    CPrintfArg const *args[6];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;

    return StringPrintfCout(fmt, args, 6);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7)

{   
    CPrintfArg const *args[7];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;

    return StringPrintfCout(fmt, args, 7);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8)

{   
    CPrintfArg const *args[8];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;

    return StringPrintfCout(fmt, args, 8);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9)

{   
    CPrintfArg const *args[9];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;

    return StringPrintfCout(fmt, args, 9);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10)

{   
    CPrintfArg const *args[10];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;
    args[9] = &arg10;

    return StringPrintfCout(fmt, args, 10);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                                const CPrintfArg &arg11)

{   
    CPrintfArg const *args[11];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;

    return StringPrintfCout(fmt, args, 11);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintfCout(const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                                const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                                const CPrintfArg &arg11, const CPrintfArg &arg12)

{   
    CPrintfArg const *args[12];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;
    args[11] = &arg12;

    return StringPrintfCout(fmt, args, 12);
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt)

{   
    return StringPrintf(file, fmt, NULL, 0);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1)

{   CPrintfArg const *args[1];
args[0] = &arg1;

return StringPrintf(file, fmt, args, 1);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2)

{   
    CPrintfArg const *args[2];
    args[0] = &arg1;
    args[1] = &arg2;

    return StringPrintf(file, fmt, args, 2);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3)

{   
    CPrintfArg const *args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;

    return StringPrintf(file, fmt, args, 3);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4)

{   
    CPrintfArg const *args[4];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;

    return StringPrintf(file, fmt, args, 4);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5)

{   
    CPrintfArg const *args[5];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;

    return StringPrintf(file, fmt, args, 5);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6)

{   
    CPrintfArg const *args[6];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;

    return StringPrintf(file, fmt, args, 6);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7)

{   
    CPrintfArg const *args[7];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;

    return StringPrintf(file, fmt, args, 7);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8)

{   
    CPrintfArg const *args[8];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;

    return StringPrintf(file, fmt, args, 8);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9)

{   
    CPrintfArg const *args[9];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;

    return StringPrintf(file, fmt, args, 9);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10)

{   
    CPrintfArg const *args[10];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    args[4] = &arg5;
    args[5] = &arg6;
    args[6] = &arg7;
    args[7] = &arg8;
    args[8] = &arg9;
    args[9] = &arg10;

    return StringPrintf(file, fmt, args, 10);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11)

{   
    CPrintfArg const *args[11];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;

    return StringPrintf(file, fmt, args, 11);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t StringPrintf(FILE *file, const string &fmt, const CPrintfArg &arg1, const CPrintfArg &arg2, const CPrintfArg &arg3, const CPrintfArg &arg4, 
                                        const CPrintfArg &arg5, const CPrintfArg &arg6, const CPrintfArg &arg7, const CPrintfArg &arg8, const CPrintfArg &arg9, const CPrintfArg &arg10,
                                        const CPrintfArg &arg11, const CPrintfArg &arg12)

{   
    CPrintfArg const *args[12];
    args[0]  = &arg1;
    args[1]  = &arg2;
    args[2]  = &arg3;
    args[3]  = &arg4;
    args[4]  = &arg5;
    args[5]  = &arg6;
    args[6]  = &arg7;
    args[7]  = &arg8;
    args[8]  = &arg9;
    args[9]  = &arg10;
    args[10] = &arg11;
    args[11] = &arg12;

    return StringPrintf(file, fmt, args, 12);
}
