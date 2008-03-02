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

#include "precomp.h"
#include "context/codeconv.h"

#if defined(LLDEBUG_USE_ICU)
	#include "unicode/ucnv.h"
#elif defined(LLDEBUG_USE_ICONV)
	#include "iconv.h"
#elif defined(LLDEBUG_USE_BABEL)
	#include "babel.h"
#endif

namespace lldebug {
namespace context {

/// The current encoding.
static lldebug_Encoding s_lldebugEncoding = LLDEBUG_ENCODING_UTF8;

lldebug_Encoding GetEncoding() {
	return s_lldebugEncoding;
}

#if defined(LLDEBUG_USE_ICU) || defined(LLDEBUG_USE_ICONV)
/// Encoding name.
static std::string s_encodingName = "";

int SetEncoding(lldebug_Encoding encoding) {
	switch (encoding) {
	case LLDEBUG_ENCODING_UTF8:
		s_encodingName = "";
		break;
	case LLDEBUG_ENCODING_SJIS:
		s_encodingName = "sjis";
		break;
	case LLDEBUG_ENCODING_EUC:
		s_encodingName = "euc-jp";
		break;
	case LLDEBUG_ENCODING_ISO2022JP:
		s_encodingName = "iso2022jp";
		break;
	default:
		return -1;
	}
	
	s_lldebugEncoding = encoding;
	return 0;
}
#endif

#if defined(LLDEBUG_USE_ICU)
std::string ConvToUTF8(const std::string &input) {
	if (s_encodingName.empty() || input.empty()) {
		return input;
	}
	UErrorCode error = U_ZERO_ERROR;
	std::vector<UChar> buffer;

	{
		// Make input string converter.
		UConverter* inputConverter = ucnv_open(
			s_encodingName.c_str(),
			&error);
		assert(U_SUCCESS(error));

		// Allocate the intermediate buffer.
		size_t bufferLength = input.length() / ucnv_getMinCharSize(inputConverter);
		buffer.resize(bufferLength + 1);

		// Convert 'input' to unicode(UTF16) character.
		const char *inputPtr = input.c_str();
		UChar *bufferPtr = &buffer[0];
		ucnv_toUnicode(
			inputConverter,
			&bufferPtr, &buffer[bufferLength] + 1,
			&inputPtr, &input[input.length() - 1] + 1,
			// &input[input.length()] can't do
			NULL, true, &error);
		assert(U_SUCCESS(error));
		ucnv_close(inputConverter);

		// Create the string object from vector.
		buffer.resize(bufferPtr - &buffer[0]);
	}

	{
		// Make utf8 string converter.
		UConverter* outputConverter = ucnv_open("utf-8", &error);
		assert(U_SUCCESS(error));

		// Allocate the output buffer.
		size_t outputLength = buffer.size() * ucnv_getMaxCharSize(outputConverter);
		std::vector<char> output(outputLength + 1);

		// Convert buffer string to 'output'.
		const UChar *bufferPtr = &buffer[0];
		char *outputPtr = &output[0];
		ucnv_fromUnicode(
			outputConverter,
			&outputPtr, &output[outputLength],
			&bufferPtr, &buffer[buffer.size() - 1] + 1,
			// &buffer[buffer.size()] can't do
			NULL, true, &error);
		assert(U_SUCCESS(error));
		ucnv_close(outputConverter);

		return std::string(&output[0], outputPtr - &output[0]);
	}
}

std::string ConvFromUTF8(const std::string &input) {
	if (s_icuEncoding.empty() || input.empty()) {
		return input;
	}
	UErrorCode error = U_ZERO_ERROR;
	std::vector<UChar> buffer;

	{
		// Make input string converter.
		UConverter* inputConverter = ucnv_open("utf-8", &error);
		assert(U_SUCCESS(error));

		// Allocate the intermediate buffer.
		size_t bufferLength = input.length() / ucnv_getMinCharSize(inputConverter);
		buffer.resize(bufferLength + 1);

		// Convert 'input' to unicode(UTF16) character.
		const char *inputPtr = input.c_str();
		UChar *bufferPtr = &buffer[0];
		ucnv_toUnicode(
			inputConverter,
			&bufferPtr, &buffer[bufferLength],
			&inputPtr, &input[input.length() - 1] + 1,
			// &input[input.length()] can't do
			NULL, true, &error);
		assert(U_SUCCESS(error));
		ucnv_close(inputConverter);

		// Create the string object from vector.
		buffer.resize(bufferPtr - &buffer[0]);
	}

	{
		// Make utf8 string converter.
		UConverter* outputConverter = ucnv_open(
			s_encodingName.c_str(),
			&error);
		assert(U_SUCCESS(error));

		// Allocate the output buffer.
		size_t outputLength = buffer.size() * ucnv_getMaxCharSize(outputConverter);
		std::vector<char> output(outputLength + 1);

		// Convert buffer string to 'output'.
		const UChar *bufferPtr = &buffer[0];
		char *outputPtr = &output[0];
		ucnv_fromUnicode(
			outputConverter,
			&outputPtr, &output[outputLength],
			&bufferPtr, &buffer[buffer.size() - 1] + 1,
			// &buffer[buffer.length()] can't do
			NULL, true, &error);
		assert(U_SUCCESS(error));
		ucnv_close(outputConverter);

		return std::string(&output[0], outputPtr - &output[0]);
	}
}

#elif defined(LLDEBUG_USE_ICONV)
static std::string convert(const std::string &input,
						   const char *fromEncoding,
						   const char *toEncoding) {
	iconv_t cd = iconv_open(toEncoding, fromEncoding);
	if (cd == (iconv_t)-1) {
		return input; // Return the input.
	}

	char *inputPtr = const_cast<char *>(input.c_str());
	size_t inputLen = input.length();
	std::vector<char> output(input.length());

	size_t resultLen;
	do {
		// Set the size twice.
		output.resize(output.size() * 2);
		size_t outputLen = output.size();

		// Do convert.
		resultLen = iconv(cd, &inputPtr, &inputLen, &output[0], &outputLen);
	} while (resultLen == -1 && errno == E2BIG);

	if (resultLen == -1) {
		return input; // Return the input.
	}

	iconv_close(cd);
	return std::string(&output[0], resultLen);
}

std::string ConvToUTF8(const std::string &input) {
	return convert(input, s_encodingName.c_str(), "utf-8");
}

std::string ConvFromUTF8(const std::string &input) {
	return convert(input, "utf-8", s_encodingName.c_str());
}

#elif defined(LLDEBUG_USE_BABEL)
static int s_babelEncoding;

/// Babel initializer
struct babel_initializer {
	explicit babel_initializer() {
		babel::init_babel();
	}
};

int SetEncoding(lldebug_Encoding encoding) {
	switch (encoding) {
	case LLDEBUG_ENCODING_UTF8:
		s_babelEncoding = babel::base_encoding::utf8;
		break;
	case LLDEBUG_ENCODING_SJIS:
		s_babelEncoding = babel::base_encoding::sjis;
		break;
	case LLDEBUG_ENCODING_EUC:
		s_babelEncoding = babel::base_encoding::euc;
		break;
	case LLDEBUG_ENCODING_ISO2022JP:
		s_babelEncoding = babel::base_encoding::iso2022jp;
		break;
	default:
		return -1;
	}

	s_lldebugEncoding = encoding;
	return 0;
}

std::string ConvToUTF8(const std::string &input) {
	static babel_initializer s_init;
	typedef babel::manual_translate_engine<std::string,std::string> engine;
	return engine::ignite(input, s_babelEncoding, babel::base_encoding::utf8);
}

std::string ConvFromUTF8(const std::string &input) {
	static babel_initializer s_init;
	typedef babel::manual_translate_engine<std::string,std::string> engine;
	return engine::ignite(input, babel::base_encoding::utf8, s_babelEncoding);
}

#else
int SetEncoding(lldebug_Encoding encoding) {
	if (encoding == LLDEBUG_ENCODING_UTF8) {
		return 0;
	}
	else {
		return -1;
	}
}

std::string ConvToUTF8(const std::string &input) {
	return input; /* do nothing */
}

std::string ConvFromUTF8(const std::string &input) {
	return input; /* do nothing */
}
#endif

} // end of namespace context
} // end of namespace lldebug
