			
// Copyright (c) 2011 Angelo Rohit Joseph Pulikotil
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "ACSVParser.h"
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cassert>

using namespace acsvparser;

const bool ACSVParser::ParseFile(const std::string &fileName, 
							const std::streamsize bufferSize)
{
	ResetState();

	bool result = true;
	std::ifstream inFile(fileName);
	if ( !inFile )
	{
		_errorState = ERRORSTATE_FAILED_TO_OPEN_FILE;
		return false;
	}

	if( bufferSize != ACSVParser::Slurp )
	{
		ParseState parseState;
		StringValueType * const pBuffer = 
			new StringValueType[static_cast<unsigned int>(bufferSize)];
		if( !pBuffer )
		{
			_errorState = ERRORSTATE_FAILED_TO_ALLOCATE_BUFFER;
			return false;
		}

		_vVData.clear();
		while( !inFile.eof() )
		{
			std::streamsize sizeRead = 
					inFile.read(pBuffer, bufferSize).gcount();

			if( !ParseString(pBuffer, sizeRead, parseState) )
			{
				_errorState = ERRORSTATE_FAILED_TO_PARSE_CONTENT;
				result = false;
				break;
			}
		}

		if( pBuffer )
			delete[] pBuffer;
	}
	else
	{
		// Read entire contents of file into a string.
		StringType strBuffer;
		std::copy(	
			std::istreambuf_iterator<StringValueType>(inFile.rdbuf()),
			std::istreambuf_iterator<StringValueType>(),
			std::back_insert_iterator<StringType>( strBuffer )
			);

		if( !ParseString(strBuffer) )
		{
			_errorState = ERRORSTATE_FAILED_TO_PARSE_CONTENT;
			result = false;
		}
	}

	return result;
}

const bool ACSVParser::ParseString(const ACSVParser::StringType &strContent)
{
	ResetState();
	_vVData.clear();
	if( !ParseString(strContent.c_str(), strContent.length(), ParseState()) )
	{
		_errorState = ERRORSTATE_FAILED_TO_PARSE_CONTENT;
		return false;
	}

	return true;
}

const bool ACSVParser::ParseString(const StringValueType * const pStrContent, 
					const std::streamsize bufferSize, ParseState& parseState)
{
	StringType strData;
	for( StringType::size_type i = 0; i < bufferSize; ++i )
	{
		const StringValueType token = pStrContent[i];
		
		if( token == _textDelim )
		{
			// Skip and record escaped text delimiters.
			if( i < (bufferSize - 1) && pStrContent[i + 1] == _textDelim )
			{
				strData += token;
				++i;
			}

			parseState.bDidBeginTextDelim = !parseState.bDidBeginTextDelim;
		}
		else if( token == _separator && !parseState.bDidBeginTextDelim )
		{
			if( _vVData.empty() )
			{
				_vVData.push_back(RowDataType());
			}
			_vVData.back().push_back(strData);
			strData.clear();
		}
		else if( token == _recordSeparator && 
			!(_shouldAcceptEmbeddedNewlines && parseState.bDidBeginTextDelim))
		{				
			if( !strData.empty() )
			{
				if( _vVData.empty() )
				{
					_vVData.push_back(RowDataType());
				}
				_vVData.back().push_back(strData);
				strData.clear();
			}

			_vVData.push_back(RowDataType());
		}
		else
		{
			strData += token;
		}
	}

	if( !strData.empty() )
	{
		if( _vVData.empty() )
		{
			_vVData.push_back(RowDataType());
		}
		_vVData.back().push_back(strData);
	}

	return true;
}

ACSVParser::StringType ACSVParser::GetContentForHeaderAt(
							const ACSVParser::StringType &headerStr, 
							const ACSVParser::RowDataSizeType row) const
{
	if( _hasHeaderRow )
	{
		const RowDataSizeType actualRow = row + _rowsToSkip;
		if( _vVData.size() > actualRow)
		{
			// TODO: Can be optimized by sorting the 
			// header content into another vector.
			// Then do a string binary search instead of the current linear 
			// search and find the corresponding column for the header.
			// This comes at the cost of extra storage for 
			// the header information.

			RowDataType::const_iterator colIter = 
				std::find(_vVData[_headerRow].begin(), _vVData[_headerRow].end(), headerStr);
			if( colIter != _vVData[_headerRow].end() )
			{
				const RowDataSizeType col = 
					std::distance(_vVData[_headerRow].begin(), colIter);
				return _vVData[actualRow][col];
			}
		}
	}

	return StringType();
}

ACSVParser::Type ACSVParser::GetTypeAt(const DataSizeType row,
								const RowDataSizeType col) const
{
	if( _hasTypeRow )
	{
		// TODO: Can be optimized by saving the
		// type content into another vector when parsing itself
		// or when the type row is set.

		const RowDataSizeType actualRow = row + _rowsToSkip;
		const std::string typeStr = _vVData[_typeRow][col];
		// TODO: Convert typeStr to lowercase.
		if( typeStr == "int" )
			return TYPE_INT;
		else if( typeStr == "char" )
			return TYPE_CHAR;
	}

	// Unrecognized types default to string.
	return TYPE_STRING;
}