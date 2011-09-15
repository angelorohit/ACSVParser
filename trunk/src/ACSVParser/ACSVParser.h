
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
    /// Class that encapsulates a CSV Parser
    class ACSVParser
    {
    public:
        /// Constant used to indicate that data from the file
        /// may be slurped instead of buffered.
        const static std::streamsize Slurp = 0;
    
        typedef std::wstring StringType;
        typedef StringType::value_type StringValueType;
        typedef std::wistringstream InputStringStreamType;
        typedef std::wifstream InputFileStreamType;        

        /// Enumeration of states that indicate the cause of failure
        /// in case of a parser error.
        enum ErrorState
        {
            ERRORSTATE_FAILED_TO_OPEN_FILE          = -3,
            ERRORSTATE_FAILED_TO_ALLOCATE_BUFFER,
            ERRORSTATE_FAILED_TO_PROCESS_TYPEDATA,
            ERRORSTATE_NONE                         = 0
        };

        /// Enumeration of supported field data types.
        enum Type
        {
            TYPE_BOOL       = 0,
            TYPE_WCHAR,
            TYPE_UINT,
            TYPE_INT,
            TYPE_FLOAT,
            TYPE_DOUBLE,
            TYPE_STRING,
        };

        /// Class that encapsulates field content data.
        class TypeData
        {
        private:
            union RawData
            {
                bool            boolData;
                wchar_t         wcharData;
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

            // Others
            /*! \fn const bool ProcessDataType(const Type type) 
             *  \brief Processes raw data based on the type passed in.                        
             *  \param type a Type enum specifying the data type.
             *  \return true on success and false otherwise.
             */
            const bool ProcessDataType(const Type type)
            {
                InputStringStreamType iss(_stringData);
                switch( type )
                {
                case TYPE_BOOL:                
                    if( !(iss >> _rawData.boolData) )               
                        return false;   
                    break;
                case TYPE_WCHAR:
                    if( !(iss >> _rawData.wcharData) )
                        return false;
                    break;
                case TYPE_UINT:                
                    if( !(iss >> _rawData.uintData) )               
                        return false;                   
                    break;
                case TYPE_INT:                
                    if( !(iss >> _rawData.intData) )                
                        return false;                                   
                    break;
                case TYPE_FLOAT:                
                    if( !(iss >> _rawData.floatData) )              
                        return false;                                   
                    break;
                case TYPE_DOUBLE:                
                    if( !(iss >> _rawData.doubleData) )             
                        return false;    
                    break;
                case TYPE_STRING:
                    break;
                default:
                    return false;
                }

                _type = type;
                return true;
            }

        public:
            /// Returns the type of the data as a Type enum.
            /// See the Type for all the supported data types.
            ACSVParser::Type GetType() const { return _type; }

            /// Returns the underlying string data.
            /// This data can always be accessed irrespective of the data type.
            /// For eg; if the data type is TYPE_INT, the user can still call
            /// this routine to get its string representation.
            StringType GetString() const { return _stringData; }

            /// Returns the data as a bool.
            bool GetBool() const { return _rawData.boolData; }

            /// Returns the data as a wide-character.
            wchar_t GetWChar() const { return _rawData.wcharData; }

            /// Returns the data as an unsigned int.
            unsigned int GetUInt() const { return _rawData.uintData; }

            /// Returns the data as an int.
            int GetInt() const { return _rawData.intData; }         

            /// Returns the data as a float.
            float GetFloat() const { return _rawData.floatData; }

            /// Returns the data as a double.
            double GetDouble() const { return _rawData.doubleData; }
        };

        /// The type for a single row of parsed data.
        typedef std::vector<TypeData> RowDataType;

        /// The size type for a single row of data.
        typedef RowDataType::size_type RowDataSizeType;

        /// The type for the entire content of data.
        typedef std::vector<RowDataType> DataType;

        /// The size type for the entire content of data.
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

        /// Stores state of the parser when parsing buffered content from file.
        /// FOR INTERNAL USE ONLY
        struct ParseState
        {
            bool bDidBeginTextDelim;
            ParseState() : bDidBeginTextDelim(false)
            {}
        };

        /// The supported character set encodings.
        enum Encoding
        {
            ENC_UTF8    = 0,
            ENC_UTF16LE,
            ENC_UTF16BE
        };

    public:
        // Constructor / Destructor
        explicit ACSVParser() :
            _separator(L','),
            _textDelim(L'\"'),
            _recordSeparator(L'\n'),
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
                               const std::streamsize bufferSize, 
                               ParseState &parseState,
                               const Encoding encoding);
        ACSVParser::Type GetTypeAt(const DataSizeType row,
            const RowDataSizeType col) const;   
        const bool ProcessDataTypes();
        const Encoding GetEncoding(InputFileStreamType &inFile);
        const unsigned int GetEncodingByteSize(const Encoding encoding);        

    public:
        // Accessors
        /// Returns the separator value used by the parser.
        /// Default value is comma character if not set by user.
        const StringValueType GetSeparator() const 
        { return _separator; }
        /// Returns the text delimiter used by the parser.
        /// Default value is double quote character if not set by user.
        const StringValueType GetTextDelimiter() const 
        { return _textDelim; }
        /// Returns the record separator used by the parser.
        /// Default value is newline character if not set by user.
        const StringValueType GetRecordSeparator() const 
        { return _recordSeparator; }

        /// Returns the error state of the parser.
        /// Can be queried in case of a parsing error.
        const ErrorState GetErrorState() const
        { return _errorState; }

        /// Indicates whether a header row was specified for 
        /// the parser.
        const bool HasHeaderRow() const
        { return _hasHeaderRow; }

        /// Indicates whether a data type row was specified for
        /// the parser.
        const bool HasTypeRow() const
        { return _hasTypeRow; }

        // Setters
        /// Sets the separator value for the parser.
        void SetSeparator(const StringValueType value) 
        { _separator = value; }

        /// Sets the text delimiter for the parser.
        void SetTextDelimiter(const StringValueType value) 
        { _textDelim = value; }

        /// Sets the record separator for the parser.
        void SetRecordSeparator(const StringValueType value) 
        { _recordSeparator = value; }

        /// Sets whether the parser should accept embedded record separators.
        void SetShouldAcceptEmbeddedNewlines(const bool value) 
        { _shouldAcceptEmbeddedNewlines = value; }

        /// Sets the row in the CSV file that contains field headers.
        void SetHeaderRow(const RowDataSizeType value) 
        { 
            _headerRow = value; 
            _hasHeaderRow = true;
            if( _rowsToSkip <= _headerRow ) 
                _rowsToSkip = _headerRow + 1; 
        }

        /// Sets the row in the CSV file that contains data type information
        /// for each field.
        void SetTypeRow(const RowDataSizeType value)
        {
            _typeRow = value;
            _hasTypeRow = true;
            if( _rowsToSkip <= _typeRow )
                _rowsToSkip = _typeRow + 1;
        }

        /// Sets the number of initial rows to be skipped when parsing.
        void SetRowsToSkip(RowDataSizeType rowsToSkip)
        { 
            _rowsToSkip = rowsToSkip; 
            if( _rowsToSkip <= _headerRow  && _hasHeaderRow )
                _rowsToSkip = _headerRow + 1;
            if( _rowsToSkip <= _typeRow && _hasTypeRow )
                _rowsToSkip = _typeRow + 1;
        }

        // Others
        /*! \fn const bool ParseFile(const std::string &fileName, 
                const std::streamsize bufferSize = ACSVParser::Slurp) 
         *  \brief Parses the contents of a CSV file.
         *  \param fileName the name of CSV file.
         *  \param bufferSize the size of the internal buffer to be used
                   when parsing. Default bufferSize is ACSVParser::Slurp which
                   simply slurps the entire CSV file content.
         *  \return true on success and false otherwise.
         */
        const bool ParseFile(const std::string &fileName, 
            const std::streamsize bufferSize = ACSVParser::Slurp);

        /*! \fn const bool ParseString(const StringType& strContent)
         *  \brief Parses a string as CSV content.
         *  \param strContent the string to be parsed.  
         *  \param encoding the character set encoding of the string content.
         *  \return true on success and false otherwise.
         */
        const bool ParseString(const StringType& strContent,
                               const Encoding encoding);

        /// Resets the error state of the parser.
        void ResetState() { _errorState = ERRORSTATE_NONE; }

        /*! \fn DataSizeType GetRowCount() const 
         *  \brief Returns the number of rows in parsed content.        
         *  \return the number of rows.
         */
        DataSizeType GetRowCount() const 
        { 
            return (_vVData.empty() || (_vVData.size() < _rowsToSkip)) ? 
                0 : _vVData.size() - _rowsToSkip;       
        }

        /*! \fn RowDataSizeType GetColumnCount(DataSizeType row) const
         *  \brief Returns the number of columns for a specified row in 
                   parsed content.
         *  \param row the row for which the number of columns
                   are to be found.
         *  \return the number of columns on success and 
                    0 if the row was invalid.
         */
        RowDataSizeType GetColumnCount(DataSizeType row) const 
        { 
            return (_vVData.empty() || (_vVData.size() <= row + _rowsToSkip)) ? 
                0 : _vVData[row + _rowsToSkip].size();          
        }

        /*! \fn const TypeData& GetContentAt(const DataSizeType row, 
                const RowDataSizeType col) const
         *  \brief Retrieves the parsed data content for a specified row and
                   column.
         *  \param row the row.
         *  \param col the column.
         *  \return the content.
         */
        const TypeData& GetContentAt(const DataSizeType row, 
            const RowDataSizeType col) const
        { return _vVData[row + _rowsToSkip][col]; }             

        /*! \fn TypeData GetContentForHeaderAt(const StringType& headerStr, 
                const RowDataSizeType row) const
         *  \brief Retrieves the parsed data content for a specified row with
                   header content.
         *  \param headerStr the header content.
         *  \param row the row.
         *  \return the content.
         */
        TypeData GetContentForHeaderAt(const StringType& headerStr, 
            const RowDataSizeType row) const;

        // Overloaded operators
        /*! \fn const RowDataType& operator[](const DataSizeType row) const
         *  \brief Overloaded operator that can be used in lieu of the 
                   GetContentAt() routine.
                   For eg; instead of saying csvParser.GetContentAt(row, col), 
                   the user can also say csvParser[row][col]
         *  \return the content.
         */
        const RowDataType& operator[](const DataSizeType row) const
        { return _vVData[row + _rowsToSkip]; }

        /*! \fn friend std::ostream& operator<<(std::ostream& out, 
                const TypeData& typeData) 
         *  \brief Overloaded operator for output streaming of parsed content.
         *  \param out the output stream to be put to.
         *  \param typeData the parsed content to be streamed.
         *  \return the output stream.
         */
        friend std::wostream& operator<<(std::wostream& out, 
            const TypeData& typeData) 
        {
            out << typeData.GetString();
            return out;
        }
    };
}   // namespace acsvparser

#endif  // ACSVPARSER_HEADER