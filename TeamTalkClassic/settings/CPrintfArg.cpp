///////////////////////////////////////////////////////////////////////////////
// CPrintfArg.cpp
// Argument class for typesafe printf substitute
// (C) Stefan Woerthmueller  Berlin  2005
//     StefanWoe@compuserve.de
///////////////////////////////////////////////////////////////////////////////
#include "CPrintfArg.h"
#include "StringPrintf.h"
#include <stdlib.h>
#include <string.h>
#undef  snprintf
#undef _snprintf

#ifdef WIN32
#define snprintf _snprintf
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
using namespace std;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static const int ARG_BUFFER_SIZE   = 512; // Size of temporary Buffer for numeric arguments output
// And additional space for every string argument
static const int MAX_FMT_LENGTH    =  16; // Maximum length of a printf format string.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern void StringPrintfErrorMessage(const string &s);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CPrintfArg::~CPrintfArg()

{
    if(mDeleteMode == DELETE_PARM)
        switch(mTyp)
    {
        case TYP_STD_STRING_NONCONST: delete    v.mString; break;
        case TYP_C_STRING_NONCONST:   delete [] v.mCharp ; break;
    }

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const string    CPrintfArg::Out(const string &fmt) const
{
    char tx[ARG_BUFFER_SIZE];    
    char outFmt[MAX_FMT_LENGTH];

    // Copy Format into a c-buffer
    strncpy(outFmt, 
        fmt.substr(0, fmt.length() - 1).c_str(), 
        MAX_FMT_LENGTH);
    outFmt[MAX_FMT_LENGTH - 2] = 0;

    // Get Incomming Format specifier
    string fmtTyp = fmt.substr(fmt.length() - 1);
    const char *inTyp = fmtTyp.c_str(); 

    int fmtMismatch = 0;
    char *fmtTypeUsed = "";
    switch(mTyp)
    {
    case TYP_INT:
        fmtTypeUsed = "TYP_INT";
        // legal conversions
        if( inTyp  &&
            *inTyp == 'd' || 
            *inTyp == 'c' || 
            *inTyp == 'x' || 
            *inTyp == 'X' || 
            *inTyp == 'u') 
            strcat(outFmt, inTyp);
        else
        {
            strcat(outFmt, "d");
            fmtMismatch = 1;
        }
        snprintf(tx, ARG_BUFFER_SIZE, outFmt, v.miVal);
        break;
    case TYP_UINT:
        fmtTypeUsed = "TYP_UINT";
        // legal conversions
        if( inTyp  &&
            *inTyp == 'd' || 
            *inTyp == 'c' || 
            *inTyp == 'x' || 
            *inTyp == 'X' || 
            *inTyp == 'u') 
            strcat(outFmt, inTyp);
        else
        {
            strcat(outFmt, "u");
            fmtMismatch = 1;
        }
        snprintf(tx, ARG_BUFFER_SIZE, outFmt, v.miVal);
        break;
#ifdef WIN32
    case TYP_INT64:
        fmtTypeUsed = "TYP_INT64";
        strcpy(outFmt, "%I64u");
        snprintf(tx, ARG_BUFFER_SIZE, outFmt, v.mi64Val);
        break;
#endif
    case TYP_FLOAT:
        fmtTypeUsed = "TYP_FLOAT";
        if( inTyp  &&
            *inTyp == 'f' || 
            *inTyp == 'e' || 
            *inTyp == 'g') 
            strcat(outFmt, inTyp);
        else
        {
            strcat(outFmt, "f");
            fmtMismatch = 1;
        }
        snprintf(tx, ARG_BUFFER_SIZE, outFmt, (float)v.mfVal);
        break;
    case TYP_CHAR:
        fmtTypeUsed = "TYP_CHAR";
        // legal conversions
        if( inTyp  &&
            *inTyp == 'c' || 
            *inTyp == 'd' || 
            *inTyp == 'u') 
            strcat(outFmt, inTyp);
        else
        {
            strcat(outFmt, "c");
            fmtMismatch = 1;
        }
        snprintf(tx, ARG_BUFFER_SIZE, outFmt, v.mChar);
        break;
    case TYP_STD_STRING:
    case TYP_C_STRING:
    case TYP_STD_STRING_NONCONST:
    case TYP_C_STRING_NONCONST:
        {
            const char *cString = NULL;
            if(mTyp == TYP_STD_STRING)
            {
                fmtTypeUsed = "TYP_STD_STRING";
                cString = v.mConstString->c_str();
            }
            else if(mTyp == TYP_STD_STRING_NONCONST)
            {
                fmtTypeUsed = "TYP_STD_STRING_NONCONST";
                cString = v.mString->c_str();
            }
            else if(mTyp == TYP_C_STRING)
            {
                fmtTypeUsed = "TYP_C_STRING";
                cString = v.mConstCharp;
            }
            else 
            {
                fmtTypeUsed = "TYP_C_STRING_NONCONST";
                cString = v.mCharp;
            }
            if(! inTyp  ||
                *inTyp != 's')
                fmtMismatch = 1;

            if(cString != NULL)
            {
                strcat(outFmt, "s");
                size_t argLen = strlen(cString) + ARG_BUFFER_SIZE;
                char *cp = new char [argLen];
                if(cp)
                {
                    snprintf(cp, argLen, outFmt, cString);
                    cp[argLen - 1] = 0;
                    string result(cp);
                    delete [] cp;
                    return result;
                }
            }
            snprintf(tx, ARG_BUFFER_SIZE, "NULL");
            break;
        }
        break;
    }
    tx[ARG_BUFFER_SIZE - 1] = 0;

#ifdef DEBUG
    if(fmtMismatch)
    {
        string s;
        StringPrintf(s, "WARNING: format string %s is illegal for type %s\n", fmt.c_str(), fmtTypeUsed); 
        StringPrintfErrorMessage(s);
    }
#endif


    return string(tx);
}
