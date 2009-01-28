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
#include <memory.h>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/empty.hpp>

namespace lldebug {

/**
 * @brief MD2 hash generator.
 * 
 * MD2 is a old hash function, but it's enough for this project and
 * calculation is easy.
 *
 * @see http://www.ietf.org/rfc/rfc1319.txt
 */
class MD2Generator {
public:
	explicit MD2Generator();
	~MD2Generator();

	/// Get the MD2 digest.
	void GetDigest(unsigned char digest[16]);

	/// Get the string of MD2 digest.
	std::string GetDigestString();

	/// MD2 block update operation.
	/** Continues an MD2 message-digest operation, processing another
	 * message block, and updating the context.
	 */
	void Update(const unsigned char *input, size_t inputLen);

	/// Ends an MD2 message-digest operation, writing the message digest.
	void Final();

private:
	void Transform(const unsigned char block[16]);

private:
	unsigned char m_state[16]; ///< state
	unsigned char m_checksum[16]; ///< checksum
	unsigned int m_count; ///< number of bytes, modulo 16
	unsigned char m_buffer[16]; ///< input buffer
};

/// Generate MD2 message-digest from a iterator.
template<class It>
std::string GenerateMD2(It begin, It end) {
	MD2Generator md2;
	
	for (; begin != end; ++begin) {
		const unsigned char *data =
			reinterpret_cast<const unsigned char *>(&*begin);

		md2.Update(data, sizeof(*begin));
	}
	
	md2.Final();
	return md2.GetDigestString();
}

/// Generate MD2 message-digest from a container object.
inline std::string GenerateMD2(const std::string &obj) {
	if (obj.empty()) {
		MD2Generator md2;
		md2.Final();
		return md2.GetDigestString();
	}
	else {
		return GenerateMD2(obj.begin(), obj.end());
	}
}

/// Generate MD2 message-digest from a container object.
inline std::string GenerateMD2(const char *obj) {
	if (obj == NULL) {
		MD2Generator md2;
		md2.Final();
		return md2.GetDigestString();
	}
	else {
		return GenerateMD2(&obj[0], &obj[strlen(obj)]);
	}
}

} // end of namespace lldebug
