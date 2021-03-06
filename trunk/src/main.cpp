
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

#include <iostream>
#include "ACSVParser.h"

using namespace std;
using namespace acsvparser;

int main( int argc, char *argv[] )
{
    ACSVParser csvParser;

    // Set the header and data type rows.
    csvParser.SetHeaderRow(0);
    csvParser.SetTypeRow(1);       
    
    if( !csvParser.ParseFile("sample_utf8.csv", ACSVParser::Slurp) )
    {
        // The cause of the error can be further queried with
        // csvParser.GetErrorState();
        std::cout << "Failed to parse file!" << std::endl;
        return -1;
    }

    // Display parsed content.
    std::cout << "Parsed data:\n\n";
    const ACSVParser::DataSizeType noOfRows = csvParser.GetRowCount();

    for(ACSVParser::DataSizeType i = 0; i < noOfRows; ++i)
    {
        const ACSVParser::RowDataSizeType noOfCols = 
                            csvParser.GetColumnCount(i);
        for(ACSVParser::RowDataSizeType j = 0; j < noOfCols; ++j)
        {
            // Note: wcout does not print wide characters above 255.
            wprintf(L"%s, ", csvParser[i][j].GetString().c_str());
        }
        std::cout << "\n";
    }

    // Get data based on type.
    const int val = csvParser[0][0].GetInt();
    std::cout << "Value of (0, 0) is " << val << "\n";

    // Get data using known header.
    std::cout << "Rating for second metahuman is " 
              << csvParser.GetContentForHeaderAt(L"rating", 1).GetFloat();

    return 0;
}