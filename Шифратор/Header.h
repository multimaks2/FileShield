#pragma once
#include <fstream>
#include <Windows.h> // HandlerRoutine
#include <random>
#include <iostream>
#include <string>
#include <Windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <bcrypt.h>
#include "Encoder.h"
#include <locale>
#include <codecvt>
#include <Urlmon.h>
#include <WinINet.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "Urlmon.lib")


#define is_delim(c) (std::isspace((c)) || std::ispunct((c)))


#ifdef WIN32
#ifndef va_copy
#define va_copy(dest, orig) (dest) = (orig)
#endif
#endif

typedef unsigned int   uint;

class SString : public std::string
{
public:
	// Constructors
	SString() : std::string() {}

	SString(const char* szText) : std::string(szText ? szText : "") {}

	explicit SString(const char* szFormat, ...) : std::string()
	{
		if (szFormat)
		{
			va_list vl;

			va_start(vl, szFormat);
			vFormat(szFormat, vl);
			va_end(vl);
		}
	}

	SString(const std::string& strText) : std::string(strText) {}

	SString& Format(const char* szFormat, ...)
	{
		va_list vl;

		va_start(vl, szFormat);
		SString& str = vFormat(szFormat, vl);
		va_end(vl);

		return str;
	}

	SString& vFormat(const char* szFormat, va_list vl);
	void     OnFormatException(const char* szFormat);
	void     OnInvalidParameter(const char* szFormat);

	// Access
	char& operator[](int iOffset) { return std::string::operator[](iOffset); }

	// Operators
	SString operator+(const char* other) const { return std::string(*this) + other; }
	SString operator+(const std::string& other) const { return std::string(*this) + other; }
	SString operator+(const SString& other) const { return std::string(*this) + other; }

	// Assignment
	operator const char* () const { return c_str(); }            // Auto assign to const char* without using c_str()
	const char* operator*() const { return c_str(); }

	// Functions
	void           Split(const SString& strDelim, std::vector<SString>& outResult, unsigned int uiMaxAmount = 0, unsigned int uiMinAmount = 0) const;
	bool           Split(const SString& strDelim, SString* pstrLeft, SString* pstrRight, int iIndex = 1) const;
	SString        SplitLeft(const SString& strDelim, SString* pstrRight = NULL, int iIndex = 1) const;
	SString        SplitRight(const SString& strDelim, SString* pstrLeft = NULL, int iIndex = 1) const;
	SString        Replace(const char* szOld, const char* szNew, bool bSearchJustReplaced = false) const;
	SString        ReplaceI(const char* szOld, const char* szNew, bool bSearchJustReplaced = false) const;
	SString        TrimStart(const char* szOld) const;
	SString        TrimEnd(const char* szOld) const;
	SString        ToLower() const;
	SString        ToUpper() const;
	SString        ConformLineEndings() const;
	bool           Contains(const SString& strOther) const;
	bool           ContainsI(const SString& strOther) const;
	bool           CompareI(const SString& strOther) const;
	bool           EqualsI(const SString& strOther) const { return CompareI(strOther); }
	SString        SubStr(int iPos, int iCount = 0x3fffffff) const;
	SString        Left(int iCount) const;
	SString        Right(int iCount) const;
	bool           EndsWith(const SString& strOther) const;
	bool           EndsWithI(const SString& strOther) const;
	bool           BeginsWith(const SString& strOther) const;
	bool           BeginsWithI(const SString& strOther) const;
	static SString Join(const SString& strDelim, const std::vector<SString>& parts, int iFirst = 0, int iCount = 0x3fffffff);
	void           AssignLeft(const char* szOther, uint uiMaxLength);
};

class SStringX : public SString
{
public:
	SStringX(const char* szText) : SString(std::string(szText ? szText : "")) {}
	SStringX(const char* szText, uint uiLength) : SString(std::string(szText ? szText : "", uiLength)) {}
};

//
// SCharStringRef
//
// String reference - Used for direct access to Lua strings
//
struct SCharStringRef
{
	SCharStringRef() : pData(NULL), uiSize(0) {}
	char* pData;
	size_t uiSize;
};

//
// Faster type of SString::Split
// Uses pointers to a big buffer rather than an array of strings
//
template <class STRING_TYPE, class CHAR_TYPE>
class TSplitString : public std::vector<const CHAR_TYPE*>
{
public:
	TSplitString() {}
	TSplitString(const STRING_TYPE& strInput, const STRING_TYPE& strDelim, unsigned int uiMaxAmount = 0, unsigned int uiMinAmount = 0)
	{
		Split(strInput, strDelim, uiMaxAmount, uiMinAmount);
	}

	void Split(const STRING_TYPE& strInput, const STRING_TYPE& strDelim, unsigned int uiMaxAmount = 0, unsigned int uiMinAmount = 0)
	{
		// Copy string to buffer
		uint iInputLength = strInput.length();
		buffer.resize(iInputLength + 1);
		memcpy(&buffer[0], &strInput[0], (iInputLength + 1) * sizeof(CHAR_TYPE));

		// Prime result list
		this->clear();
		this->reserve(16U < uiMaxAmount ? 16U : uiMaxAmount);

		// Split into pointers
		size_t ulCurrentPoint = 0;
		while (true)
		{
			size_t ulPos = strInput.find(strDelim, ulCurrentPoint);
			if (ulPos == STRING_TYPE::npos || (uiMaxAmount > 0 && uiMaxAmount <= this->size() + 1))
			{
				if (ulCurrentPoint <= strInput.length())
					push_back(&buffer[ulCurrentPoint]);
				break;
			}
			push_back(&buffer[ulCurrentPoint]);
			buffer[ulPos] = 0;
			ulCurrentPoint = ulPos + strDelim.length();
		}
		while (this->size() < uiMinAmount)
			push_back(&buffer[iInputLength]);
	}

protected:
	std::vector<CHAR_TYPE> buffer;
};

typedef TSplitString<std::string, char>     CSplitString;
typedef TSplitString<std::wstring, wchar_t> CSplitStringW;


char* remove_words(char* s, const char* w)
{
	char* t = s;
	if ((s = std::strstr(s, w)) == NULL)
		return t;

	const size_t n = std::strlen(w);
	for (char* p = s; *s; *s = *p) {
		if (!std::strncmp(p, w, n) && (p == t || is_delim(*(p - 1))) && (!*(p + n) || is_delim(*(p + n))))
			p += n;
		else {
			++s;
			++p;
		}
	}
	return t;
}


SString fullFileSoundDir(SString val1, const char* result)
{
	val1 += result;
	SString C = val1.c_str();
	return C;
}

SString getDirFile(const char* searchFile, const char* consolePath)
{
	size_t arrc_size = strlen(consolePath) + 1; // вычисл€ем размер буфера дл€ arrc
	char* arrc = new char[arrc_size]; // выдел€ем пам€ть под динамический массив

	strncpy_s(arrc, arrc_size, consolePath, _TRUNCATE); // копируем consolePath в arrc
	const char* clearDir = remove_words(arrc, "crypto.exe");

	SString result = fullFileSoundDir(clearDir, searchFile);

	delete[] arrc; // освобождаем пам€ть
	return result;
}

char* wchar_to_char(const wchar_t* pwchar)
{
	// get the number of characters in the string.
	int currentCharIndex = 0;
	char currentChar = pwchar[currentCharIndex];

	while (currentChar != '\0')
	{
		currentCharIndex++;
		currentChar = pwchar[currentCharIndex];
	}

	const int charCount = currentCharIndex + 1;

	// allocate a new block of memory size char (1 byte) instead of wide char (2 bytes)
	char* filePathC = (char*)malloc(sizeof(char) * charCount);

	for (int i = 0; i < charCount; i++)
	{
		// convert to char (1 byte)
		char character = pwchar[i];

		*filePathC = character;

		filePathC += sizeof(char);

	}
	filePathC += '\0';

	filePathC -= (sizeof(char) * charCount);

	return filePathC;
}

const char* getConsolePath()
{
	WCHAR buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(buffer[0]));
	auto res = wchar_to_char(buffer);

	return res;
}

bool fileExists(const std::string& filename)
{
	std::ifstream file(filename);
	return file.good();
}