
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

#ifndef ACSVPARSER_HEADER
#define ACSVPARSER_HEADER

#include <string>
#include <sstream>
#include <vector>

namespace acsvparser
{ 
    class ACSVParser
    {
    public:
        const static std::streamsize Slurp = 0;

        typedef std::string StringType;
        typedef StringType::value_type StringValueType;

        // If the parser fails at any point,
        // its state can be queried for any of these.
        enum ErrorState
        {
            ERRORSTATE_FAILED_TO_OPEN_FILE          = -3,
            ERRORSTATE_FAILED_TO_ALLOCATE_BUFFER,
            ERRORSTATE_FAILED_TO_PROCESS_TYPEDATA,
            ERRORSTATE_NONE                         = 0
        };

        // Supported field types.
        enum Type
        {
            TYPE_BOOL       = 0,
            TYPE_UCHAR,
            TYPE_CHAR,
            TYPE_UINT,
            TYPE_INT,
            TYPE_FLOAT,
            TYPE_DOUBLE,
            TYPE_STRING,
        };

        // Raw field data.
        class TypeData
        {
        private:
            union RawData
            {
                bool            boolData;
                unsigned char   ucharData;
                char            charData;
                unsigned int    uintData;
                int             intData;
                float           floatData;
                double          doubleData;
            } _rawData;

            Type _type;
            StringType _stringData;

        public:
            explicit TypeData(StringType strData) : 
            _stringData(strData),
                _type(TYPE_STRING)
            {}

            const bool ProcessDataType(const Type type)
            {
                std::istringstream iss(_stringData);
                if( type == TYPE_BOOL )
                {
                    if( !(iss >> _rawData.boolData) )               
                        return false;                   
                }
                else if( type == TYPE_UCHAR )
                {
                    if( !(iss >> _rawData.ucharData) )              
                        return false;                   
                }
                else if( type == TYPE_CHAR )
                {
                    if( !(iss >> _rawData.charData) )               
                        return false;                   
                }
                else if( type == TYPE_UINT )
                {
                    if( !(iss >> _rawData.uintData) )               
                        return false;                   
                }
                else if( type == TYPE_INT )
                {
                    if( !(iss >> _rawData.intData) )                
                        return false;                   
                }
                else if( type == TYPE_FLOAT )
                {
                    if( !(iss >> _rawData.floatData) )              
                        return false;                   
                }
                else if( type == TYPE_DOUBLE )
                {
                    if( !(iss >> _rawData.doubleData) )             
                        return false;                   
                }

                _type = type;
                return true;
            }

        public:
            ACSVParser::Type GetType() const { return _type; }
            StringType GetString() const { return _stringData; }
            bool GetBool() const { return _rawData.boolData; }
            unsigned char GetUChar() const { return _rawData.ucharData; }
            char GetChar() const { return _rawData.charData; }
            unsigned int GetUInt() const { return _rawData.uintData; }
            int GetInt() const { return _rawData.intData; }         
            float GetFloat() const { return _rawData.floatData; }
            double GetDouble() const { return _rawData.doubleData; }
        };

        typedef std::vector<TypeData> RowDataType;
        typedef RowDataType::size_type RowDataSizeType;
        typedef std::vector<RowDataType> DataType;
        typedef DataType::size_type DataSizeType;

        // Data Members
    private:
        StringValueType _separator;
        StringValueType _textDelim;
        StringValueType _recordSeparator;
        bool            _shouldAcceptEmbeddedNewlines;
        ErrorState      _errorState;
        RowDataSizeType _rowsToSkip;
        RowDataSizeType _headerRow;
        RowDataSizeType _typeRow;

        bool            _hasHeaderRow;
        bool            _hasTypeRow;

        DataType        _vVData;

        // Stores state of the parser when parsing buffered content from file.
        // For internal use only.
        struct ParseState
        {
            bool bDidBeginTextDelim;
            ParseState() : bDidBeginTextDelim(false)
            {}
        };

    public:
        // Constructor / Destructor
        explicit ACSVParser() :
        _separator(','),
            _textDelim('\"'),
            _recordSeparator('\n'),
            _shouldAcceptEmbeddedNewlines(true),
            _headerRow(0),
            _typeRow(0),
            _hasHeaderRow(false),
            _hasTypeRow(false),
            _rowsToSkip(0),
            _errorState(ERRORSTATE_NONE)
        {}

        ~ACSVParser() {}

    private:
        // Copy constructor / assignment operator
        ACSVParser(const ACSVParser &);
        ACSVParser& operator =(const ACSVParser &);

        // Functions
    private:
        const bool ParseString(const StringValueType * const pStrContent, 
            const std::streamsize bufferSize, ParseState &parseState);
        ACSVParser::Type GetTypeAt(const DataSizeType row,
            const RowDataSizeType col) const;   
        const bool ProcessDataTypes();

    public:
        // Accessors
        const StringValueType GetSeparator() const 
        { return _separator; }
        const StringValueType GetTextDelimiter() const 
        { return _textDelim; }
        const StringValueType GetRecordSeparator() const 
        { return _recordSeparator; }
        const ErrorState GetErrorState() const
        { return _errorState; }
        const bool HasHeaderRow() const
        { return _hasHeaderRow; }
        const bool HasTypeRow() const
        { return _hasTypeRow; }

        // Setters
        void SetSeparator(const StringValueType value) 
        { _separator = value; }
        void SetTextDelimiter(const StringValueType value) 
        { _textDelim = value; }
        void SetRecordSeparator(const StringValueType value) 
        { _recordSeparator = value; }
        void SetShouldAcceptEmbeddedNewlines(const bool value) 
        { _shouldAcceptEmbeddedNewlines = value; }
        void SetHeaderRow(const RowDataSizeType value) 
        { 
            _headerRow = value; 
            _hasHeaderRow = true;
            if( _rowsToSkip <= _headerRow ) 
                _rowsToSkip = _headerRow + 1; 
        }
        void SetTypeRow(const RowDataSizeType value)
        {
            _typeRow = value;
            _hasTypeRow = true;
            if( _rowsToSkip <= _typeRow )
                _rowsToSkip = _typeRow + 1;
        }
        void SetRowsToSkip(RowDataSizeType rowsToSkip)
        { 
            _rowsToSkip = rowsToSkip; 
            if( _rowsToSkip <= _headerRow  && _hasHeaderRow )
                _rowsToSkip = _headerRow + 1;
            if( _rowsToSkip <= _typeRow && _hasTypeRow )
                _rowsToSkip = _typeRow + 1;
        }

        // Others
        const bool ParseFile(const std::string &fileName, 
            const std::streamsize bufferSize = ACSVParser::Slurp);
        const bool ParseString(const StringType& strContent);

        void ResetState() { _errorState = ERRORSTATE_NONE; }

        DataSizeType GetRowCount() const 
        { 
            return (_vVData.empty() || (_vVData.size() < _rowsToSkip)) ? 
                0 : _vVData.size() - _rowsToSkip;       
        }
        RowDataSizeType GetColumnCount(DataSizeType row) const 
        { 
            return (_vVData.empty() || (_vVData.size() <= row + _rowsToSkip)) ? 
                0 : _vVData[row + _rowsToSkip].size();          
        }

        const TypeData& GetContentAt(const DataSizeType row, 
            const RowDataSizeType col) const
        { return _vVData[row + _rowsToSkip][col]; }             

        TypeData GetContentForHeaderAt(const StringType& headerStr, 
            const RowDataSizeType row) const;

        // Overloaded operators
        const RowDataType& operator[](const DataSizeType row) const
        { return _vVData[row + _rowsToSkip]; }

        friend std::ostream& operator<<(std::ostream& out, 
            const TypeData& typeData) 
        {
            out << typeData.GetString();
            return out;
        }
    };
}   // namespace acsvparser

#endif  // ACSVPARSER_HEADER