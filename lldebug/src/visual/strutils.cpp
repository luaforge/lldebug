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
#include "visual/strutils.h"

namespace lldebug {
namespace visual {

static shared_ptr<wxMBConv> s_conv(new wxMBConvUTF8);

int wxSetEncoding(lldebug_Encoding encoding) {
	shared_ptr<wxMBConv> conv;

	switch (encoding) {
	case LLDEBUG_ENCODING_UTF8:
		conv.reset(new wxMBConvUTF8);
		break;
	case LLDEBUG_ENCODING_SJIS:
		conv.reset(new wxCSConv(wxFONTENCODING_CP932));
		break;
	case LLDEBUG_ENCODING_EUC:
		conv.reset(new wxCSConv(wxFONTENCODING_EUC_JP));
		break;
	case LLDEBUG_ENCODING_ISO2022JP:
		break;
	default:
		break;
	}

	if (conv == NULL) {
		return -1;
	}

	s_conv = conv;
	return 0;
}

std::string wxConvToCtxEnc(const wxString &str) {
	return std::string(s_conv->cWX2MB(str.c_str()));
}

wxString wxConvFromCtxEnc(const std::string &str) {
	return wxString(s_conv->cMB2WX(str.c_str()));
}

std::string wxConvToCurrent(const wxString &str) {
	wxMBConv *conv = wxConvCurrent;

	if (conv == NULL) {
		conv = &wxConvUTF8;
	}

	return std::string(conv->cWX2MB(str.c_str()));
}

wxString wxConvFromCurrent(const std::string &str) {
	wxMBConv *conv = wxConvCurrent;

	if (conv == NULL) {
		conv = &wxConvUTF8;
	}

	return wxString(conv->cMB2WX(str.c_str()));
}

} // end of namespace visual
} // end of namespace lldebug
