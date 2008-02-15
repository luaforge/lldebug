/*
 * Copyright (c) 2005-2008  cielacanth <cielacanth AT s60.xrea.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "lldebug_prec.h"
#include "lldebug_codeconv.h"

//#include "babel.h"
#include "wx/strconv.h"

namespace lldebug {
#if 0
inline std::string wxConvToUTF8(const wxString &str) {
	return std::string(wxConvUTF8.cWX2MB(str.c_str()));
}

inline wxString wxConvFromUTF8(const std::string &str) {
	return wxString(wxConvUTF8.cMB2WX(str.c_str()));
}

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
#endif

}
