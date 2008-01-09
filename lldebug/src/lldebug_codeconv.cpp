//---------------------------------------------------------------------------
//
// Name:        Project1App.cpp
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#include "lldebug_prec.h"
#include "lldebug_codeconv.h"

#include "babel.h"

namespace lldebug {

struct initializer {
	initializer() {
		babel::init_babel();
	}
};

std::string ConvToUTF8(const std::string &str) {
	static initializer s_init;
	babel::analyze_result enc = babel::analyze_base_encoding(str);
	if (enc.get_strict_result() == babel::base_encoding::unknown) {
		assert(0 && "Couldn't identify the string encoding.");
		return str;
	}

	typedef babel::manual_translate_engine<std::string> engine;
	return engine::ignite(str, enc.get_strict_result(), babel::base_encoding::utf8);
}

std::string ConvToUTF8From(const std::string &str, int fromEncoding) {
	static initializer s_init;
	typedef babel::manual_translate_engine<std::string> engine;
	return engine::ignite(str, fromEncoding, babel::base_encoding::utf8);
}

std::string ConvFromUTF8(const std::string &str, int toEncoding) {
	static initializer s_init;
	typedef babel::manual_translate_engine<std::string> engine;
	return engine::ignite(str, babel::base_encoding::utf8, toEncoding);
}

int GetEncoding(const std::string &str, unsigned int maxcount) {
	static initializer s_init;
	babel::analyze_result enc = babel::analyze_base_encoding(str, maxcount);
	return enc.get_strict_result();
}

}
