/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "SplitTest.h"

SplitTest::SplitTest():
TestTemplate<SplitTest>("SplitTest")
{
	RegisterFunction(this, &SplitTest::SplitByWords, "SplitByWords", NULL);
	RegisterFunction(this, &SplitTest::SplitBySymbols, "SplitBySymbols", NULL);
	RegisterFunction(this, &SplitTest::SplitByWordsWithNewLine, "SplitByWords", NULL);
	RegisterFunction(this, &SplitTest::SplitBySymbolsWithNewLine, "SplitBySymbols", NULL);
}

void SplitTest::LoadResources()
{
    GetBackground()->SetColor(Color::White);
    
    staticText = new UIStaticText();
    staticText->SetRect(Rect(10.f, 10.f, 400.f, 200.f));
    
    font = FTFont::Create("~res:/Fonts/korinna.ttf");
    
    staticText->SetFont(font);
}

void SplitTest::UnloadResources()
{
    SafeRelease(staticText);
    SafeRelease(font);
}

void SplitTest::SplitByWords(PerfFuncData * data)
{
    staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
    Vector<WideString> resultStrings;
    resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY THE DAVA");
    resultStrings.push_back(L"CONSULTING, LLC AND CONTRIBUTORS AS IS AND ANY");
    resultStrings.push_back(L"EXPRESS OR IMPLIED WARRANTIES");
    
    staticText->SetMultiline(true);
    
    Vector<WideString> strings = staticText->GetMultilineStrings();
    bool testSuccess = true;
    
    if(strings.size() != resultStrings.size())
    {
        testSuccess = false;
    }
    else
    {
        Vector<WideString>::iterator it = strings.begin();
        Vector<WideString>::iterator itResult = resultStrings.begin();
        while(it != strings.end() && itResult != resultStrings.end())
        {
            if(*it != *itResult)
                testSuccess = false;
            it++;
            itResult++;
        }
    }
    
    TEST_VERIFY(testSuccess);
}

void SplitTest::SplitBySymbols(PerfFuncData * data)
{
    staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
    Vector<WideString> resultStrings;
    resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTIN");
    resultStrings.push_back(L"G, LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR ");
    resultStrings.push_back(L"IMPLIED WARRANTIES");
    
    staticText->SetMultiline(true, true);
    
    Vector<WideString> strings = staticText->GetMultilineStrings();
    
    bool testSuccess = true;
    
    if(strings.size() != resultStrings.size())
    {
        testSuccess = false;
    }
    else
    {
        Vector<WideString>::iterator it = strings.begin();
        Vector<WideString>::iterator itResult = resultStrings.begin();
        while(it != strings.end() && itResult != resultStrings.end())
        {
            if(*it != *itResult)
                testSuccess = false;
            it++;
            itResult++;
        }
    }
    
    TEST_VERIFY(testSuccess);
}

void SplitTest::SplitByWordsWithNewLine(PerfFuncData * data)
{
    staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY\nTHE DAVA CONSULTING\\nLLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
    Vector<WideString> resultStrings;
    resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY");
    resultStrings.push_back(L"THE DAVA CONSULTING");
    resultStrings.push_back(L"LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR");
    resultStrings.push_back(L"IMPLIED WARRANTIES");
    
    staticText->SetMultiline(true);
    
    Vector<WideString> strings = staticText->GetMultilineStrings();
    
    bool testSuccess = true;
    
    if(strings.size() != resultStrings.size())
    {
        testSuccess = false;
    }
    else
    {
        Vector<WideString>::iterator it = strings.begin();
        Vector<WideString>::iterator itResult = resultStrings.begin();
        while(it != strings.end() && itResult != resultStrings.end())
        {
            if(*it != *itResult)
                testSuccess = false;
            it++;
            itResult++;
        }
    }
    
    TEST_VERIFY(testSuccess);
}

void SplitTest::SplitBySymbolsWithNewLine(PerfFuncData * data)
{
    staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING,\nLLC AND\\nCONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
    Vector<WideString> resultStrings;
    resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTIN");
    resultStrings.push_back(L"G,");
    resultStrings.push_back(L"LLC AND");
    resultStrings.push_back(L"CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED ");
    resultStrings.push_back(L"WARRANTIES");
    
    staticText->SetMultiline(true, true);
    
    Vector<WideString> strings = staticText->GetMultilineStrings();
    
    bool testSuccess = true;
    
    if(strings.size() != resultStrings.size())
    {
        testSuccess = false;
    }
    else
    {
        Vector<WideString>::iterator it = strings.begin();
        Vector<WideString>::iterator itResult = resultStrings.begin();
        while(it != strings.end() && itResult != resultStrings.end())
        {
            if(*it != *itResult)
                testSuccess = false;
            it++;
            itResult++;
        }
    }
    
    TEST_VERIFY(testSuccess);
}