
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
#include <cctype>

using namespace acsvparser;

const bool ACSVParser::ParseFile(const std::string &fileName, 
    const std::streamsize bufferSize)
{
    ResetState();

    bool result = true;
    InputFileStreamType inFile(fileName);
    if ( !inFile )
    {
        _errorState = ERRORSTATE_FAILED_TO_OPEN_FILE;
        return false;
    }
    
    const Encoding encoding = GetEncoding(inFile);

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

            if( !ParseString(pBuffer, sizeRead, parseState, encoding) )
            {
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

        if( !ParseString(strBuffer, encoding) )
        {
            result = false;
        }
    }

    return result;
}

const bool ACSVParser::ParseString(const ACSVParser::StringType &strContent,
                                   const Encoding encoding)
{
    ResetState();
    _vVData.clear();
    if( !ParseString(strContent.c_str(), 
                     strContent.length(), 
                     ParseState(),
                     encoding
                    )
      )
    {
        return false;
    }

    return true;
}

const bool ACSVParser::ParseString(const StringValueType * const pStrContent, 
                                   const std::streamsize bufferSize, 
                                   ParseState& parseState,
                                   const Encoding encoding)
{
    const unsigned int byteSize = GetEncodingByteSize(encoding);

    StringType strData;
    for( StringType::size_type i = 0; i < bufferSize; i += byteSize)
    {
        // Convert the token depending on the encoding.
        StringValueType token = pStrContent[i];
        if( byteSize == 2 )
        {                
            if( encoding == ENC_UTF16LE )
            {
                token = token | (pStrContent[i + 1] << (2 << byteSize));
            }
            else if( encoding == ENC_UTF16BE )
            {
                token = (token << (2 << byteSize)) | pStrContent[i + 1];
            }
        }        

        // Skip carriage return
        if( token == L'\r' )
            continue;

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
            _vVData.back().push_back(TypeData(strData));
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
                _vVData.back().push_back(TypeData(strData));
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
        _vVData.back().push_back(TypeData(strData));
    }

    if( _hasTypeRow )
    {
        if( !ProcessDataTypes() )
        {
            _errorState = ERRORSTATE_FAILED_TO_PROCESS_TYPEDATA;
            return false;
        }
    }

    return true;
}

ACSVParser::TypeData ACSVParser::GetContentForHeaderAt(
    const ACSVParser::StringType &headerStr, 
    const ACSVParser::RowDataSizeType row) const
{
    if( _hasHeaderRow )
    {
        const RowDataSizeType actualRow = row + _rowsToSkip;
        if( _vVData.size() > actualRow)
        {
            RowDataType::const_iterator colIter = _vVData[_headerRow].begin();
            while( colIter != _vVData[_headerRow].end() )
            {
                if( colIter->GetString() == headerStr )
                    break;
                ++colIter;
            }

            if( colIter != _vVData[_headerRow].end() )
            {
                const RowDataSizeType col = 
                    std::distance(_vVData[_headerRow].begin(), colIter);
                return _vVData[actualRow][col];
            }
        }
    }

    return TypeData(L"");
}

ACSVParser::Type ACSVParser::GetTypeAt(const DataSizeType row,
    const RowDataSizeType col) const
{
    if( _hasTypeRow )
    {
        const RowDataSizeType actualRow = row + _rowsToSkip;
        StringType typeStr = _vVData[_typeRow][col].GetString();
        // Convert typeStr to lowercase.
        std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), 
            ::tolower);

        if( typeStr == L"bool" )
            return TYPE_BOOL;
        else if( typeStr == L"wchar" )
            return TYPE_WCHAR;
        else if( typeStr == L"uint" )
            return TYPE_UINT;
        else if( typeStr == L"int" )
            return TYPE_INT;
        else if( typeStr == L"float" )
            return TYPE_FLOAT;
        else if( typeStr == L"double" )
            return TYPE_DOUBLE;
        else if( typeStr == L"string" )
            return TYPE_STRING;
    }

    // Unrecognized types default to string.
    return TYPE_STRING;
}

const bool ACSVParser::ProcessDataTypes()
{
    if( _hasTypeRow )
    {
        const DataSizeType noOfRows = GetRowCount();
        for(DataSizeType i = 0; i < noOfRows; ++i)
        {
            const RowDataSizeType noOfCols = GetColumnCount(i);
            for(RowDataSizeType j = 0; j < noOfCols; ++j)
            {
                TypeData& typeData = _vVData[i + _rowsToSkip][j];   
                if( !typeData.ProcessDataType(GetTypeAt(i, j)) )
                    return false;
            }
        }

        return true;
    }

    return false;
}

const ACSVParser::Encoding ACSVParser::GetEncoding(InputFileStreamType &inFile)
{    
    const std::streampos lastPos = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // Read and skip the BOM.
    const StringValueType bom1 = inFile.get();
    const StringValueType bom2 = inFile.get();    
    const StringValueType bom3 = inFile.get();

    Encoding encoding = ENC_UTF8;
    unsigned int bomToSkip = 0;
    if( bom1 == 0xEF && bom2 == 0xBB && bom3 == 0xBF )
    {
        bomToSkip = 3;
        encoding = ENC_UTF8;
    }
    if( bom1 == 0xFF && bom2 == 0xFE )
    {
       bomToSkip = 2;
       encoding = ENC_UTF16LE;
    }
    else if( bom1 == 0xFE && bom2 == 0xFF )
    {
        bomToSkip = 2;
        encoding = ENC_UTF16BE;
    }

    // Undo the skipping of BOM.
    inFile.seekg(lastPos, std::ios::beg);

    // Now skip the appropriate amount.
    inFile.seekg(bomToSkip, std::ios::cur);

    return encoding;    // Default encoding if no BOM is found.
}

const unsigned int ACSVParser::GetEncodingByteSize(const Encoding encoding)
{
    switch(encoding)
    {
    case ENC_UTF8:
        return 1;
    case ENC_UTF16LE:
        return 2;
    case ENC_UTF16BE:
        return 2;
    }

    return 1;   // Default
}