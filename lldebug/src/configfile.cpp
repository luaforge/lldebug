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
#include "sysinfo.h"
#include "configfile.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <fstream>


/// Get the root config dir name.
/**
 * For example, config file 'config.xml' is located on
 * 'Root/ConfigDir/config.xml'.
 */
static std::string LLDebugGetConfigRoot();

/// Get the config dir name like 'lldebug' or '.lldebug'.
static std::string LLDebugGetConfigDir();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <shlobj.h> // use shell32.lib
#pragma comment(lib, "shell32")

static std::string LLDebugGetConfigRoot() {
	char szPath[_MAX_PATH];
	LPITEMIDLIST pidl;
	IMalloc *pMalloc;

	::SHGetMalloc(&pMalloc);

	// Get the app data folder.
	if (FAILED(::SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl))) {
		pMalloc->Release();
		return std::string("");
	}

	::SHGetPathFromIDListA(pidl, szPath);
	pMalloc->Free(pidl);
	pMalloc->Release();
	return szPath;
}

static std::string LLDebugGetConfigDir() {
	return "lldebug";
}

#elif defined(__linux__)
static std::string LLDebugGetConfigRoot() {
	return "~/";
}

static std::string LLDebugGetConfigDir() {
	return ".lldebug";
}

#else
#error Please define the config directory.
#endif

namespace lldebug {

std::string EncodeToFilename(const std::string &filename) {
	const char s_lookupTable[] = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"_-";
	std::string result;
	unsigned int c = 0;
	int bcount = 0;

	// Encode the filename (almost same as the base64 encoding).
	for (std::string::size_type i = 0; i <= filename.size(); ++i) {
		c = (c << 8) | (i == filename.size() ? 0 : filename[i]);
		bcount += 8;

		while (bcount >= 6) {
			const unsigned int shiftcount = bcount - 6;
			const unsigned int mask = ((1 << 6) - 1);
			result += s_lookupTable[(c >> shiftcount) & mask];
			c &= (1 << shiftcount) - 1;
			bcount -= 6;
		}
	}

	return result;
}

/// Get the config dir name.
static boost::filesystem::path GetConfigDir() {
	using namespace boost::filesystem;
	path basePath(LLDebugGetConfigRoot());
	path configPath = basePath / LLDebugGetConfigDir();

	// Check whether the filepath exists.
	if (!exists(configPath)) {
		try {
			if (!create_directory(configPath)) {
				configPath = basePath;
			}
		}
		catch (filesystem_error &) {
			configPath = basePath;
		}
	}

	return configPath;
}

boost::filesystem::path GetConfigFilePath(const std::string &filename) {
	if (filename.empty()) {
		return boost::filesystem::path();
	}
	
	boost::filesystem::path configPath = GetConfigDir();
	configPath /= filename;
	return configPath.normalize();
}

std::string GetConfigFileName(const std::string &filename) {
	return GetConfigFilePath(filename).native_file_string();
}


/*-----------------------------------------------------------------*/
safe_ofstream::safe_ofstream() {
}

safe_ofstream::~safe_ofstream() {
}

bool safe_ofstream::open(const std::string &filename, int mode) {
	boost::filesystem::path filePath = filename;
	boost::filesystem::path tmpPath = filename + ".tmp";

	filePath = boost::filesystem::complete(filePath).normalize();
	tmpPath = boost::filesystem::complete(tmpPath).normalize();

	// Open the temporary file.
	m_stream.open(tmpPath.native_file_string().c_str(), mode);
	if (!m_stream.is_open()) {
		return false;
	}

	m_filePath = filePath;
	m_tmpPath = tmpPath;
	return true;
}

bool safe_ofstream::is_open() const {
	return m_stream.is_open();
}

void safe_ofstream::close() {
	m_stream.close();

	if (m_filePath.empty() || m_tmpPath.empty()) {
		return;
	}

	// Does the temporary file exist ?
	if (!boost::filesystem::exists(m_tmpPath)) {
		return;
	}
	
	boost::filesystem::remove(m_filePath);
	boost::filesystem::rename(m_tmpPath, m_filePath);
}

} // end of namespace lldebug
