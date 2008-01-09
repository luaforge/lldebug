//---------------------------------------------------------------------------
//
// Name:        Project1App.h
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_CODECONV_H__
#define __LLDEBUG_CODECONV_H__

namespace lldebug {

std::string ConvToUTF8(const std::string &str);
std::string ConvToUTF8From(const std::string &str, int fromEncoding);
std::string ConvFromUTF8(const std::string &str, int toEncoding);
int GetEncoding(const std::string &str, unsigned int maxcount = 4069);

}

#endif
