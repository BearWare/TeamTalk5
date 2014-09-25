///////////////////////////////////////////////////////////////////////////////
// CPrintfArg.h
// Argument class for typesafe printf substitute
// (C) Stefan Woerthmueller  Berlin  2005
//     StefanWoe@compuserve.de
///////////////////////////////////////////////////////////////////////////////

#ifndef __CPRINTF_ARG__
#define __CPRINTF_ARG__
#include <string>
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG) && ! defined(DEBUG)
#define DEBUG
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CPrintfArg

{
public:
    typedef enum
    {
        TYP_INT,
        TYP_INT64,
        TYP_UINT,
        TYP_FLOAT,
        TYP_CHAR,
        TYP_C_STRING,
        TYP_STD_STRING,
        TYP_C_STRING_NONCONST,
        TYP_STD_STRING_NONCONST,
        TYP_VOIDP,
    }DATA_TYPES;
    typedef enum
    {
        DELETE_PARM,
        NO_DELETE
    }DELETE_MODE;
    CPrintfArg(const int  x)             : mTyp(TYP_INT)       , mDeleteMode(NO_DELETE)
    {                                                        
        v.miVal = x;                                         
    };                                                       
    CPrintfArg(const unsigned int  x)    : mTyp(TYP_UINT)      , mDeleteMode(NO_DELETE)
    {                                                        
        v.muVal = x;                                         
    };                                                       
    CPrintfArg(const void *x)            : mTyp(TYP_UINT)      , mDeleteMode(NO_DELETE)
    {                                                        
        v.muVal = (unsigned long)x;                           
    };                                                       
    CPrintfArg(const long x)             : mTyp(TYP_INT)       , mDeleteMode(NO_DELETE)
    {                                                        
        v.miVal = x;                                         
    };                                                       
    CPrintfArg(const unsigned long x)    : mTyp(TYP_UINT)      , mDeleteMode(NO_DELETE)
    {                                                        
        v.muVal = x;                                         
    };                                                       
#ifdef WIN32                                                 
    CPrintfArg(const __int64 x)          : mTyp(TYP_INT64)     , mDeleteMode(NO_DELETE)
    {                                                        
        v.mi64Val = x;                                       
    };                                                       
#endif                                                       
    CPrintfArg(const float x)            : mTyp(TYP_FLOAT)     , mDeleteMode(NO_DELETE)
    {                                                        
        v.mfVal = (double)x;                                 
    };                                                       
    CPrintfArg(const double x)           : mTyp(TYP_FLOAT)     , mDeleteMode(NO_DELETE)
    {                                                        
        v.mfVal = x;                                         
    };                                                       
    CPrintfArg(const char c)             : mTyp(TYP_CHAR)      , mDeleteMode(NO_DELETE)
    {
        v.mChar = c;
    };
    CPrintfArg(const char           *p)  : mTyp(TYP_C_STRING)  , mDeleteMode(NO_DELETE)
    {                                                                                 
        v.mConstCharp = p;                                                            
    };                                                                                
    CPrintfArg(const unsigned char  *p)  : mTyp(TYP_C_STRING)  , mDeleteMode(NO_DELETE)
    {                                                                                 
        v.mConstCharp = (const char*)p;                                                     
    };                                                                                
    CPrintfArg(const    std::string &s)  : mTyp(TYP_STD_STRING), mDeleteMode(NO_DELETE)
    {
        v.mConstString = &s;
    };


    // Non cost constructors for objects to delete on destruction
    CPrintfArg(char           *p, DELETE_MODE mode = NO_DELETE)  : mTyp(TYP_C_STRING_NONCONST)  , mDeleteMode(mode)
    {
        v.mCharp = p;
    };
    CPrintfArg(unsigned char  *p, DELETE_MODE mode = NO_DELETE)  : mTyp(TYP_C_STRING_NONCONST)  , mDeleteMode(mode)
    {
        v.mCharp = (char*)p;
    };
    CPrintfArg(   std::string &s, DELETE_MODE mode = NO_DELETE)  : mTyp(TYP_STD_STRING_NONCONST), mDeleteMode(mode)
    {
        v.mString = &s;
    };

    ~CPrintfArg();

    const std::string Out(const std::string &fmt) const ;
private:
             DATA_TYPES  mTyp;          
             DELETE_MODE mDeleteMode;
             union
             {
#ifdef WIN32
                             __int64   mi64Val;
#endif
                                long   miVal;
                       unsigned long   muVal;
                                double mfVal;
                                char   mChar;
                                char  *mCharp;
                         std::string  *mString;
                 const          char  *mConstCharp;
                 const   std::string  *mConstString;
             }v;
};
#endif
