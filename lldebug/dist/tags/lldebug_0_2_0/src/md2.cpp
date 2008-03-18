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
#include "md2.h"

namespace lldebug {

/** Permutation of 0..255 constructed from the digits of pi.
 * It gives a "random" nonlinear byte substitution operation.
 */
static unsigned char PI_SUBST[256] = {
	41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
	19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
	76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
	138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
	245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
	148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
	39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
	181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
	150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
	112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
	96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
	85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
	234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
	129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
	8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
	203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
	166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
	31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

static unsigned char *PADDING[] = {
	(unsigned char *)"",
	(unsigned char *)"\001",
	(unsigned char *)"\002\002",
	(unsigned char *)"\003\003\003",
	(unsigned char *)"\004\004\004\004",
	(unsigned char *)"\005\005\005\005\005",
	(unsigned char *)"\006\006\006\006\006\006",
	(unsigned char *)"\007\007\007\007\007\007\007",
	(unsigned char *)"\010\010\010\010\010\010\010\010",
	(unsigned char *)"\011\011\011\011\011\011\011\011\011",
	(unsigned char *)"\012\012\012\012\012\012\012\012\012\012",
	(unsigned char *)"\013\013\013\013\013\013\013\013\013\013\013",
	(unsigned char *)"\014\014\014\014\014\014\014\014\014\014\014\014",
	(unsigned char *)"\015\015\015\015\015\015\015\015\015\015\015\015\015",
	(unsigned char *)"\016\016\016\016\016\016\016\016\016\016\016\016\016\016",
	(unsigned char *)"\017\017\017\017\017\017\017\017\017\017\017\017\017\017\017",
	(unsigned char *)"\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020",
};

struct TestExecuter {
	explicit TestExecuter() {
		assert(GenerateMD2((const char *)NULL) ==
			"8350e5a3e24c153df2275c9f80692773");
		assert(GenerateMD2("") ==
			"8350e5a3e24c153df2275c9f80692773");
		assert(GenerateMD2("a") ==
			"32ec01ec4a6dac72c0ab96fb34c0b5d1");
		assert(GenerateMD2("abc") ==
			"da853b0d3f88d99b30283a69e6ded6bb");
		assert(GenerateMD2("message digest") ==
			"ab4f496bfb2a530b219ff33031fe06b0");
		assert(GenerateMD2("abcdefghijklmnopqrstuvwxyz") ==
			"4e8ddff3650292ab5a4108c3aa47940b");
		assert(GenerateMD2("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") ==
			"da33def2a42df13975352846c30338cd");
		assert(GenerateMD2("12345678901234567890123456789012345678901234567890123456789012345678901234567890") ==
			"d5976f79d83d3a0dc9806c3c66f3efd8");
	}
};
//static TestExecuter s_tester;


MD2Generator::MD2Generator()
	: m_count(0) {
	memset(m_state, 0, sizeof(m_state));
	memset(m_checksum, 0, sizeof(m_checksum));
}

MD2Generator::~MD2Generator() {
}

/// Get the MD2 digest.
void MD2Generator::GetDigest(unsigned char digest[16]) {
	// Store state in digest.
	memcpy(digest, m_state, 16);
}

/// Get the string of MD2 digest.
std::string MD2Generator::GetDigestString() {
	const static char TO_HEX16[] = "0123456789abcdef";
	std::string result;

	for (int i = 0; i < 16; ++i) {
		result += TO_HEX16[m_state[i] >> 4];
		result += TO_HEX16[m_state[i] & 0x0f];
	}
	
	return result;
}

/// MD2 block update operation.
/** Continues an MD2 message-digest operation, processing another
 * message block, and updating the context.
 */
void MD2Generator::Update(const unsigned char *input, size_t inputLen) {
	if (input == NULL || inputLen == 0) {
		return;
	}

	// Update number of bytes mod 16.
	unsigned int index = m_count;
	m_count = (index + inputLen) & 0x0f;
	
	unsigned int partLen = 16 - index;
	unsigned int i = 0;
	
	// Transform as many times as possible.
	if (inputLen >= partLen) {
		memcpy(&m_buffer[index], &input[0], partLen);
		Transform(m_buffer);
		
		for (i = partLen; i + 15 < inputLen; i += 16) {
			Transform(&input[i]);
		}
		
		index = 0;
	}
	
	// Buffer remaining input.
	memcpy(&m_buffer[index], &input[i], inputLen - i);
}

/// Ends an MD2 message-digest operation, writing the message digest.
void MD2Generator::Final() {
	// Pad out to multiple of 16.
	unsigned int index = m_count;
	unsigned int padLen = 16 - index;
	Update(PADDING[padLen], padLen);
	
	// Extend with checksum.
	Update(m_checksum, 16);
}

/// Transforms state and updates checksum based on block.
void MD2Generator::Transform(const unsigned char block[16]) {
	unsigned char x[48];
	
	// Form encryption block from state, block, state ^ block.
	memcpy(&x[0], m_state, 16);
	memcpy(&x[16], block, 16);
	for (int i = 0; i < 16; ++i) {
		x[i+32] = m_state[i] ^ block[i];
	}
	
	// Encrypt block (18 rounds).
	int t = 0;
	for (int i = 0; i < 18; ++i) {
		for (int j = 0; j < 48; ++j) {
			t = (x[j] ^= PI_SUBST[t]);
		}
		t = (t + i) & 0xff;
	}
	
	// Save new state.
	memcpy(m_state, x, 16);
	
	// Update checksum.
	t = m_checksum[15];
	for (int i = 0; i < 16; ++i) {
		t = (m_checksum[i] ^= PI_SUBST[block[i] ^ t]);
	}
}

} // end of namespace lldebug
