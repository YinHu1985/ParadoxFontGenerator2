#include "stdafx.h"
#include "Fonts.h"
#include <vector>
#include <set>
#include "d3d9.h"
#include <d3dx9.h>
#include "gdiplus.h"

#define EXTRASPACE 2

using namespace Gdiplus;

namespace
{
struct CharEntry
{
    unsigned int id;
    int x;
    int y;
    int Height;
    int Width;
    int xoffset;
    int yoffset;
    int xadvance;
    int page;
};

class CharTable
{
public:
    CharTable()
    {
        memset(m_table, 0, sizeof(CharEntry*)*65536);
    }
    ~CharTable()
    {
        for (int i = 0; i < 65536; ++ i)
        {
            if (m_table[i] != nullptr)
            {
                delete m_table[i];
                m_table[i] = nullptr;
            }
        }
    }
    void addChar(const CharEntry& input)
    {
        if (input.id >= 65536)
            return;
        if (m_table[input.id])
            *m_table[input.id] = input;
        else
            m_table[input.id] = new CharEntry(input);
    }
    CharEntry* getChar(unsigned int id)
    {
        if (id >= 65536)
            return nullptr;
        return m_table[id];
    }
    CharEntry* getCharUCS2(unsigned int id, unsigned int codepage)
    {
        return getChar(getIdUCS2(id, codepage));
    }
    int getIdUCS2(unsigned int id, unsigned int codepage)
    {
        if (id >= 65536)
            return id;
        wchar_t c = id;
        char buf[4] = { 0 };
        BOOL flag = TRUE;
        int r = WideCharToMultiByte(codepage, 0, &c, 1, buf, 4, 0, &flag);
        if (r && flag == FALSE)
        {
            int newid = *((unsigned short*)buf);
            return newid;
        }
        else
        {
            return 65536;
        }      
    }

private:
    CharEntry* m_table[65536];
};

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }    
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap color format, width, and height. 
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        MessageBox(NULL, L"GetObject",L"Error",MB_OK);
    //        errhandler("GetObject", hwnd); 

    // Convert the color format to a count of bits. 
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure 
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
    // data structures.) 

    if (cClrBits != 24) 
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
        sizeof(BITMAPINFOHEADER) + 
        sizeof(RGBQUAD) * (1<< cClrBits)); 

    // There is no RGBQUAD array for the 24-bit-per-pixel format. 

    else 
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
        sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure. 

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag. 
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color 
    // indices and store the result in biSizeImage. 
    // For Windows NT, the width must be DWORD aligned unless 
    // the bitmap is RLE compressed. This example shows this. 
    // For Windows 95/98/Me, the width must be WORD aligned unless the 
    // bitmap is RLE compressed.
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
        * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the 
    // device colors are important. 
    pbmi->bmiHeader.biClrImportant = 0; 
    return pbmi; 
} 

void CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, 
                   HBITMAP hBMP, HDC hDC) 
{ 
    HANDLE hf;                 // file handle 
    BITMAPFILEHEADER hdr;       // bitmap file-header 
    PBITMAPINFOHEADER pbih;     // bitmap info-header 
    LPBYTE lpBits;              // memory pointer 
    DWORD dwTotal;              // total count of bytes 
    DWORD cb;                   // incremental count of bytes 
    BYTE *hp;                   // byte pointer 
    DWORD dwTmp; 

    pbih = (PBITMAPINFOHEADER) pbi; 
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits) 
        MessageBox(NULL, L"GlobalAlloc",L"Error",MB_OK);
    //         errhandler("GlobalAlloc", hwnd); 

    // Retrieve the color table (RGBQUAD array) and the bits 
    // (array of palette indices) from the DIB. 
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS)) 
    {
        MessageBox(NULL, L"GetDIBits",L"Error",MB_OK);

        //        errhandler("GetDIBits", hwnd); 
    }

    // Create the .BMP file. 
    hf = CreateFile(pszFile, 
        GENERIC_READ | GENERIC_WRITE, 
        (DWORD) 0, 
        NULL, 
        CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) 
        MessageBox(NULL, L"CreateFile",L"Error",MB_OK);
    //        errhandler("CreateFile", hwnd); 
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
    // Compute the size of the entire file. 
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
        pbih->biSize + pbih->biClrUsed 
        * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices. 
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
        pbih->biSize + pbih->biClrUsed 
        * sizeof (RGBQUAD); 

    // Copy the BITMAPFILEHEADER into the .BMP file. 
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
        (LPDWORD) &dwTmp,  NULL)) 
    {
        MessageBox(NULL, L"WriteFile",L"Error",MB_OK);
        //       errhandler("WriteFile", hwnd); 
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
        + pbih->biClrUsed * sizeof (RGBQUAD), 
        (LPDWORD) &dwTmp, ( NULL))) 
        MessageBox(NULL, L"WriteFile",L"Error",MB_OK);
    ////        errhandler("WriteFile", hwnd); 

    // Copy the array of color indices into the .BMP file. 
    dwTotal = cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL))
        MessageBox(NULL, L"WriteFile",L"Error",MB_OK);
    //           errhandler("WriteFile", hwnd); 

    // Close the .BMP file. 
    if (!CloseHandle(hf)) MessageBox(NULL, L"CloseHandle",L"Error",MB_OK);
    //           errhandler("CloseHandle", hwnd); 

    // Free memory. 
    GlobalFree((HGLOBAL)lpBits);
}

bool Convert(unsigned short int index, unsigned int codePage, bool singleByte = false, wchar_t* out = nullptr)
{
    wchar_t s[2] = {0,0};

    if (index < 32)
        return false;
    unsigned char byte1,byte2;
    byte1 = ((unsigned char*)(&index))[0];
    byte2 = ((unsigned char*)(&index))[1];
    
    if (!singleByte && (byte1 < 0x81 || byte2 < 0x40))
        return false;

    int len = MultiByteToWideChar(codePage, MB_PRECOMPOSED, (char*)&index, singleByte ? 1 : 2, 0, 0);
    if (len == 1)
    {
        MultiByteToWideChar(codePage, MB_PRECOMPOSED, (char*)&index, 2, s, 1);
        if (out)
        {
            *out = s[0];          
        }
        return true;
    }
    return false;

}

HRESULT AddCharacters(const FontOptions& option, std::set<unsigned int>& ids)
{
    bool useGBK = option.useGBK;
    bool excludeSymbols = option.excludeSymbols;
    for(unsigned int i = 32; i < 256; ++ i)
    {
        ids.insert(i);
    }
    if (option.onlyASCII || option.onlyExtraChar)
        return S_OK;
    if (option.padding)
        ids.insert(0xf3a1);
    for(unsigned int i = 256; i < 65536; ++ i)
    {
        unsigned char byte1,byte2;
        byte1 = ((unsigned char*)(&i))[0];
        byte2 = ((unsigned char*)(&i))[1];
        if (!useGBK)
        {
            if(byte1 != 0 && (byte1 < 0xa1 || byte1 > 0xf7 || byte2 < 0xa1||byte2 > 0xfe
                || (byte1 > 0xa4 && byte1 < 0xb0)))
            {
                continue;
            }
        }
        if (excludeSymbols)
        {
            if(byte1 == 0xa2 || byte1 == 0xa4)
            {
                continue;
            }
        }
        if (Convert(i, option.chnCodePage))
            ids.insert(i);
    }
    return S_OK;
}

std::wstring GetFileName(const wchar_t* fontName, const wchar_t* ext, int size, bool rotate, bool black, int count)
{
    std::wstring fileName = fontName;
    fileName += std::to_wstring(size);
    if (rotate)
        fileName += L"ROT";
    if (black)
        fileName += L"black";
    if (count >= 0)
        fileName += L"_" + std::to_wstring(count);
    fileName += L".";
    fileName += ext;
    return fileName;
}

HRESULT ReadExtraChar(const std::wstring& path, std::set<unsigned int>& ids)
{
    FILE* fp = NULL;
    _wfopen_s(&fp, path.c_str(), L"r");
    if (!fp)
        return S_OK;
    unsigned char byte[2] = {0,0};
    while (!feof(fp))
    {
        int c = fgetc(fp);
        if (c == EOF)
            break;
        
        byte[1] = (unsigned char)c;
        if (byte[0] != 0 && Convert(*(unsigned short*)byte, 936))
        {
            ids.insert(*(unsigned short*)byte);
            *(unsigned short*)byte = 0;
        }
        else
        {
            byte[0] = byte[1];
            byte[1] = 0;
        }
    }
    fclose(fp);
    return S_OK;
}

int DrawTexts(HDC bitmapDC, const FontOptions& option, const std::set<unsigned int>& ids, 
    int pageWidth, bool getSizeOnly, CharTable& charTable, int offset, int* end, int maxHeight)
{
    HFONT ChnFont;
    HFONT EngFont;

    ChnFont = CreateFont(0-option.chnFontSize, 0, option.rotate ? 900 : 0, 0, option.chnBold ? FW_BOLD : FW_REGULAR, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, option.chnFontName.c_str());
    EngFont = CreateFont(0-option.engFontSize, 0, 0, 0, option.engBold ? FW_BOLD : FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, option.engFontName.c_str());

    SelectObject(bitmapDC, EngFont);
    wchar_t buf[2] = {0, 0};
    SIZE charSize;
    int posx = option.engFontSize/2;
    int posy = option.engFontSize/2;
    for(unsigned int i = offset; i < 256; ++ i)
    {
        if (!Convert(i, option.engCodePage, true, buf))
            continue;
        GetTextExtentPoint32(bitmapDC, buf, 1, &charSize);
        if (!getSizeOnly)
        {
            TextOut(bitmapDC, posx, posy + option.engYOffsetDraw, buf, 1);
            CharEntry entry;
            entry.id = i;
            entry.Height = charSize.cy;
            entry.Width = charSize.cx;
            entry.x = posx;
            entry.y = posy;
            entry.xadvance = charSize.cx;
            entry.xoffset = 0;
            entry.yoffset = option.engYOffsetFontFile;
            entry.page = 0;
            if (i == 32)
                entry.Height = 0;
            if (i == 46 && option.hideDecimalPoint)
            {
                CharEntry* spaceChar = charTable.getChar(32);
                entry.x = spaceChar->x;
                entry.y = spaceChar->y;
            }
            if (option.extraLineSpace)
            {
                entry.x -= EXTRASPACE;
                entry.y -= EXTRASPACE;
               entry.xadvance += 2*EXTRASPACE;
                entry.Height += 2*EXTRASPACE;
                entry.Width += 2*EXTRASPACE;
            }
            charTable.addChar(entry);
        }
        posx += charSize.cx;
        // extra space for english characters
        posx += option.engFontSize/2; 
        posx += option.extraLineSpace ? (2*EXTRASPACE) : 0;
        if (posx > pageWidth - option.engFontSize)
        {
            // new line
            posx = option.engFontSize/2;
            posy += option.engFontSize;
            // extra space for english characters
            posy += option.engFontSize;
        }
    }
    // new line and some extra space
    posx = option.chnFontSize/2;
    posy += 3 * option.chnFontSize;
    SelectObject(bitmapDC, ChnFont); 
    for(unsigned int i = (offset > 256 ? offset : 256); i < 65536; ++ i)
    {
        if (end)
            *end = i + 1;
        // write in reversed order to make the font file looks nicer
        unsigned short index;
        ((unsigned char*)(&index))[0] = ((unsigned char*)(&i))[1];
        ((unsigned char*)(&index))[1] = ((unsigned char*)(&i))[0];
        if (ids.find(index) == ids.end())
            continue;
        if (!Convert(index, option.chnCodePage, false, buf))
            continue;

        GetTextExtentPoint32(bitmapDC, buf, 1, &charSize);
        if (!getSizeOnly)
        {
            if(!option.rotate)
            {
                TextOut(bitmapDC, posx, posy + option.chnYOffsetDraw, buf, 1);
            }
            else 
            {
                TextOut(bitmapDC, posx + option.chnYOffsetDraw, posy, buf, 1);
            }
            CharEntry entry;
            entry.id = index;
            entry.Height = option.chnFontSize;
            entry.Width = charSize.cx;
            entry.x = posx;
            entry.y = posy;
            if(option.rotate)
            {
                entry.y -= option.chnFontSize;
            }
            entry.xadvance = charSize.cx;
            entry.xoffset = 0;
            entry.yoffset = option.chnYOffsetFontFile;
            entry.page = 0;
            if (option.extraLineSpace)
            {
                entry.x -= EXTRASPACE;
                entry.y -= EXTRASPACE;
                entry.xadvance += 2*EXTRASPACE;
                entry.Height += 2*EXTRASPACE;
                entry.Width += 2*EXTRASPACE;
            }
            charTable.addChar(entry);
        }
        posx += charSize.cx;
        posx += option.extraLineSpace ? (2*EXTRASPACE) : 0;
        
        if (posx > pageWidth - option.chnFontSize)
        {
            // new line
            posx = option.chnFontSize/2;
            posy += option.chnFontSize;
            
            if (option.extraLineSpace)
                posy += (2 * EXTRASPACE);
            else
                posy += EXTRASPACE; // always add some extra line space
            if (end && posy + option.chnFontSize > maxHeight)
            {      
                return posy;
            }
        }
    }
    DeleteObject(ChnFont);
    DeleteObject(EngFont);
    return posy + option.chnFontSize;
}

HRESULT WriteFNT(const FontOptions& option, CharTable& charTable, int width, int height, int count, int start, int end)
{
    FILE * fp;
    std::wstring path = GetFileName(option.chnFontName.c_str(), L"fnt", option.chnFontSize, option.rotate, option.blackMode, count);
    _wfopen_s(&fp, path.c_str(), L"w");
    if (fp == NULL)
    {
        return E_INVALIDARG;
    }
    if (option.ucs2mode)
    {
        fprintf_s(fp,
            "info face=\"dummy\" size=%d bold=0 italic=0 charset=\"\" unicode=1 stretchH=100 smooth=1 aa=1 padding=0,0,0,0 "
            "spacing=0,0\ncommon lineHeight=%d base=%d scaleW=%d scaleH=%d pages=1\n",
            option.chnFontSize, option.chnFontSize + option.chnFontSize / 6, option.chnFontSize, width, height);
        for (int i = 32; i <65536; ++i)
        {
            int id = charTable.getIdUCS2(i, option.chnCodePage);
            unsigned short index;
            ((unsigned char*)(&index))[0] = ((unsigned char*)(&id))[1];
            ((unsigned char*)(&index))[1] = ((unsigned char*)(&id))[0];

            if (index >= start && index < end)
            {
                CharEntry* entryPtr = charTable.getChar(id);
                if (entryPtr && ((entryPtr->Height > 0) || i == 32))
                {
                    fprintf_s(fp, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d\n",
                        i, entryPtr->x, entryPtr->y,
                        entryPtr->Width, entryPtr->Height, entryPtr->xoffset,
                        entryPtr->yoffset, entryPtr->xadvance, entryPtr->page);
                }
                else if (option.padding && Convert(id, option.chnCodePage) != 0)
                {
                    CharEntry* dummy = charTable.getChar(0xf3a1);
                    fprintf_s(fp, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d\n",
                        i, dummy->x, dummy->y,
                        dummy->Width, dummy->Height, dummy->xoffset,
                        dummy->yoffset, dummy->xadvance, dummy->page);
                }
            }
        }
    }
    else
    {
        fprintf_s(fp,
            "info face=\"dummy\" size=%d bold=0 italic=0 charset=\"GB2312\" stretchH=100 smooth=1 aa=1 padding=0,0,0,0 "
            "spacing=0,0\ncommon lineHeight=%d base=%d scaleW=%d scaleH=%d pages=1\n",
            option.chnFontSize, option.chnFontSize + option.chnFontSize / 6, option.chnFontSize, width, height);
        for (int i = start; i <end; ++i)
        {
            CharEntry* entryPtr = charTable.getChar(i);
            if (entryPtr && ((entryPtr->Height > 0) || i == 32))
            {
                fprintf_s(fp, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d\n",
                    i, entryPtr->x, entryPtr->y,
                    entryPtr->Width, entryPtr->Height, entryPtr->xoffset,
                    entryPtr->yoffset, entryPtr->xadvance, entryPtr->page);
            }
            else if (option.padding && Convert(i, option.chnCodePage) != 0)
            {
                CharEntry* dummy = charTable.getChar(0xf3a1);
                fprintf_s(fp, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d\n",
                    i, dummy->x, dummy->y,
                    dummy->Width, dummy->Height, dummy->xoffset,
                    dummy->yoffset, dummy->xadvance, dummy->page);
            }
        }
    }
    
    fclose(fp);
    return S_OK;
}

HRESULT GenerateBMP(const FontOptions& option, const std::set<unsigned int>& ids, CharTable& charTable, int maxSize, bool* overSize, int* count)
{
    bool multipage = false;
    if (count)
        multipage = true;
    
    
    if (overSize)
        *overSize = false;
    if (maxSize != 2048 && maxSize != 4096)
        return E_INVALIDARG;
    if (!multipage)
    {
        HDC bitmapDC;
        bitmapDC = CreateCompatibleDC(NULL);
        int testWidth = 256;
        if (ids.size() > 256)
            testWidth = 1024;
        int height = DrawTexts(bitmapDC, option, ids, testWidth, true, charTable, 0, nullptr, maxSize);
        while (height >= maxSize)
        {
            if (testWidth >= 2048)
            {
                if (overSize)
                    *overSize = true;
                return S_OK; // give up
            }
            testWidth *= 2;
            height = DrawTexts(bitmapDC, option, ids, testWidth, true, charTable, 0, nullptr, maxSize);
        }
        int finalWidth = testWidth;
        int finalHeight = finalWidth;
        while (height > finalHeight)
            finalHeight *= 2;

        HDC hdc = GetDC(NULL);
        HBITMAP hbitmap;
        hbitmap = CreateCompatibleBitmap(hdc, finalWidth, finalHeight);
        DeleteDC(hdc);
        SelectObject(bitmapDC, hbitmap);
        SetBkMode(bitmapDC, TRANSPARENT);
        if (option.blackMode)
        {
            SetTextColor(bitmapDC, RGB(0, 0, 0));
            FloodFill(bitmapDC, 0, 0, RGB(255, 255, 255));
            SetBkColor(bitmapDC, 0xffffffff);
        }
        else
        {
            SetTextColor(bitmapDC, RGB(255, 255, 255));
            SetBkColor(bitmapDC, 0x00000000);
        }
        DrawTexts(bitmapDC, option, ids, finalWidth, false, charTable, 0, nullptr, maxSize);

        std::wstring path = GetFileName(option.chnFontName.c_str(), L"bmp", option.chnFontSize, option.rotate, option.blackMode, -1);
        PBITMAPINFO pBitmapinfo;
        pBitmapinfo = CreateBitmapInfoStruct(hbitmap);
        CreateBMPFile((LPTSTR)path.c_str(), pBitmapinfo, hbitmap, bitmapDC);

        DeleteDC(bitmapDC);
        DeleteObject(hbitmap);
        LocalFree(pBitmapinfo);

        HRESULT hr = WriteFNT(option, charTable, finalWidth, finalHeight, -1, 32, 65536);
        return hr;
    }
    else
    {
        int offset = 32;
        *count = 0;
        while (offset < 65536)
        {
            HDC bitmapDC;
            bitmapDC = CreateCompatibleDC(NULL);
            int nextoffset;
            int height = DrawTexts(bitmapDC, option, ids, 2048, true, charTable, offset, &nextoffset, maxSize);
            int finalWidth = 2048;
            int finalHeight = finalWidth;
            while (height > finalHeight)
                finalHeight *= 2;

            HDC hdc = GetDC(NULL);
            HBITMAP hbitmap;
            hbitmap = CreateCompatibleBitmap(hdc, finalWidth, finalHeight);
            DeleteDC(hdc);
            SelectObject(bitmapDC, hbitmap);
            SetBkMode(bitmapDC, TRANSPARENT);
            if (option.blackMode)
            {
                SetTextColor(bitmapDC, RGB(0, 0, 0));
                FloodFill(bitmapDC, 0, 0, RGB(255, 255, 255));
                SetBkColor(bitmapDC, 0xffffffff);
            }
            else
            {
                SetTextColor(bitmapDC, RGB(255, 255, 255));
                SetBkColor(bitmapDC, 0x00000000);
            }
            DrawTexts(bitmapDC, option, ids, finalWidth, false, charTable, offset, &nextoffset, maxSize);

            std::wstring path = GetFileName(option.chnFontName.c_str(), L"bmp", option.chnFontSize, option.rotate, option.blackMode, *count);
            PBITMAPINFO pBitmapinfo;
            pBitmapinfo = CreateBitmapInfoStruct(hbitmap);
            CreateBMPFile((LPTSTR)path.c_str(), pBitmapinfo, hbitmap, bitmapDC);

            DeleteDC(bitmapDC);
            DeleteObject(hbitmap);
            LocalFree(pBitmapinfo);

            HRESULT hr = WriteFNT(option, charTable, finalWidth, finalHeight, *count, offset, nextoffset);
            ++(*count);
            offset = nextoffset;
            if (FAILED(hr))
                return hr;
        }
        return S_OK;
    }
}

namespace
{
// helper
void AddShiningEffect(Bitmap* argbbitmap, Bitmap* refbitmap, int x, int y, int bmpw, int bmph, int alpha)
{
    int maxDist = 2;

   
    for (int j = y - maxDist; j < y + maxDist + 1; ++ j)
    {
        if (j < 0 || j >= bmph)
            continue;
        for (int i = x - maxDist; i < x + maxDist + 1; ++ i)
        {
            if (i < 0 || i >= bmpw)
                continue;
            Color pcolor;
            refbitmap->GetPixel(i,j,&pcolor);
            unsigned int pixelColor = pcolor.GetValue();
            if (pixelColor == 0x00000000)
            {
                int dist = abs(j - y) + abs(i - x);
                int targetAlpha = int(((float)alpha/255.0) * (127 - (dist - 1) * 64));

                Color pcolor2;
                argbbitmap->GetPixel(i,j,&pcolor2);
                unsigned int pixelColor2 = pcolor2.GetValue();
                if ((unsigned char)(LOBYTE((pixelColor2)>>24)) < targetAlpha)
                {
                    argbbitmap->SetPixel(i,j,((targetAlpha << 24) | 0x00ffffff));
                }
            }
        }
    }
}

} // namespace

HRESULT BMPToPNG(const FontOptions& option, int count)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    std::wstring path = GetFileName(option.chnFontName.c_str(), L"bmp", option.chnFontSize, option.rotate, option.blackMode, count);
    Bitmap* argbbitmap;
    int bmpw,bmph;
    argbbitmap = new Bitmap(path.c_str());
    
    bmpw = argbbitmap->GetWidth();
    bmph = argbbitmap->GetHeight();

    Color pcolor;
    unsigned char colorR = (unsigned char)GetRValue(option.color);
    unsigned char colorG = (unsigned char)GetGValue(option.color);
    unsigned char colorB = (unsigned char)GetBValue(option.color);
    unsigned char colorA = (unsigned char)(LOBYTE((option.color)>>24));
    bool lightcolor = ((0.299 *colorR + 0.587 *colorG + 0.114*colorB) > 128);
    for (int j = 0; j < bmph; ++ j) // process by row for better efficiency
    {
        for (int i = 0; i < bmpw; ++ i)
        {
            argbbitmap->GetPixel(i,j,&pcolor);
            unsigned int pixelColor;

            //	tpcolor = GetPixel(bitmapdc,i,j);
            pixelColor = pcolor.GetValue();
            if ((option.blackMode && pixelColor == 0xffffffff) || (!option.blackMode && pixelColor == 0xff000000))
            {
                pcolor.SetValue(0x00000000);
            }
            else
            {
                double alphadown;
                alphadown = (0.299 *(unsigned char)GetRValue(pixelColor)+0.587 *(unsigned char)GetGValue(pixelColor) +0.114*(unsigned char)GetBValue(pixelColor))/255.0;
                if (option.blackMode)
                {
                    // special case: white ''black font''
                    alphadown = 1 - alphadown;
                    if (!lightcolor)
                        alphadown = (alphadown + 0.2 > 1) ? 1 : (alphadown + 0.2);
                }
                if (option.shining)
                {
                    double r,g,b,a;
                    r = (double)colorR * alphadown + (1 - alphadown)*255;
                    g = (double)colorG * alphadown + (1 - alphadown)*255;
                    b = (double)colorB * alphadown + (1 - alphadown)*255;
                    a = ((double)255 * alphadown + (1 - alphadown)*127) * colorA / 255;
                    pcolor.SetValue(pcolor.MakeARGB((int)a,(int)r,(int)g,(int)b));
                }
                else
                {
                    pcolor.SetValue(pcolor.MakeARGB((unsigned char)(alphadown*colorA),colorR,colorG,colorB));
                }
            }
            argbbitmap->SetPixel(i,j,pcolor);
        }
    }
    if (option.shining)
    {
        Bitmap* refbitmap = argbbitmap->Clone(Rect(0, 0, bmpw, bmph), PixelFormatDontCare);
        for (int j = 0; j < bmph; ++ j) // process by row for better efficiency
        {
            for (int i = 0; i < bmpw; ++ i)
            {
                refbitmap->GetPixel(i,j,&pcolor);
                unsigned int pixelColor;
                //	tpcolor = GetPixel(bitmapdc,i,j);
                pixelColor = pcolor.GetValue();
                if (pixelColor != 0x00000000)
                {
                    double ratio;

                    ratio  = (1.0 - (0.299 *(unsigned char)GetRValue(pixelColor)+0.587 *(unsigned char)GetGValue(pixelColor) +0.114*(unsigned char)GetBValue(pixelColor))/255)/
                                (1.0 - (0.299 *(unsigned char)colorR+0.587 *(unsigned char)colorG +0.114*(unsigned char)colorB)/255);

                    AddShiningEffect(argbbitmap, refbitmap, i, j, bmpw, bmph, (int)(ratio*colorA));
                }
            }
        }
        delete refbitmap;
    }
    path = GetFileName(option.chnFontName.c_str(), L"png", option.chnFontSize, option.rotate, option.blackMode, count);
    CLSID pngClsid;
    GetEncoderClsid(L"image/png", &pngClsid);
    argbbitmap->Save(path.c_str(), &pngClsid, NULL);
    delete argbbitmap;
    
    GdiplusShutdown(gdiplusToken);
    return S_OK;
}

HRESULT PNGToDDS(const FontOptions& option, int count)
{
    LPDIRECT3D9 pd3d9 = NULL;
    HRESULT hr = S_OK;
    if(NULL == (pd3d9 = Direct3DCreate9(D3D_SDK_VERSION)))
        return E_UNEXPECTED;
    else
    {

        LPDIRECT3DDEVICE9 pDevice = NULL;
        LPDIRECT3DTEXTURE9 pTex = NULL;
        //LPDIRECT3DSURFACE9 pSur = NULL;
        D3DPRESENT_PARAMETERS d3dpp; 
        ZeroMemory( &d3dpp, sizeof(d3dpp) );
        d3dpp.Windowed   = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;

        if(FAILED( pd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetActiveWindow(),
            D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &pDevice )))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            std::wstring path = GetFileName(option.chnFontName.c_str(), L"png", option.chnFontSize, option.rotate, option.blackMode, count);
            HRESULT hRet;
            hRet = D3DXCreateTextureFromFileExW(
                pDevice, path.c_str(), 0, 0, 1, 0,
                (D3DFORMAT)MAKEFOURCC('D', 'X', 'T', '3'), D3DPOOL_MANAGED,
                D3DX_DEFAULT, D3DX_DEFAULT,
                0, NULL, NULL, &pTex
                );
            if(hRet != D3D_OK)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                //pTex->GetSurfaceLevel(0,&pSur);
                path = GetFileName(option.chnFontName.c_str(), L"dds", option.chnFontSize, option.rotate, option.blackMode, count);
                hRet = ::D3DXSaveTextureToFile(path.c_str(),D3DXIFF_DDS,pTex,NULL);
                if(hRet != D3D_OK)
                {
                    hr = E_UNEXPECTED;
                }
                /*if(FAILED(::D3DXSaveSurfaceToFile(tps,D3DXIFF_DDS,pSur,NULL,NULL)))MessageBox(mainwnd,L"Error in Creating DDS",L"Error",MB_OK);
                else MessageBox(mainwnd,L"Test Assert",L"Assert",MB_OK);*/
                pTex->Release();
            }
            pDevice->Release();
        }
        pd3d9->Release();
    }
    return hr;
}

} // namespace

HRESULT GenerateFont(const FontOptions& option, int maxSize, bool ignoreOverSize, bool* overSize)
{
    if (!ValidateOption(option))
        return E_INVALIDARG;
    CharTable charTable;
    HRESULT hr = S_OK;
    bool localOverSize = false;
    std::set<unsigned int> ids;
    ReadExtraChar(option.ExtraCharPath, ids);
    AddCharacters(option, ids);
    if (ids.empty())
        return E_INVALIDARG;
    int count = 0;

    hr = GenerateBMP(option, ids, charTable, maxSize, &localOverSize, option.allowMultiFile?&count:nullptr);
    if (FAILED(hr))
        return hr;
    if (overSize)
        *overSize = localOverSize;
    if (!ignoreOverSize && localOverSize)
        return E_OUTOFMEMORY;
    if (!option.allowMultiFile)
    {
        hr = BMPToPNG(option, option.allowMultiFile ? count : -1);
        if (FAILED(hr))
            return hr;
        hr = PNGToDDS(option, option.allowMultiFile ? count : -1);
        return hr;
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            hr = BMPToPNG(option, i);
            if (FAILED(hr))
                return hr;
            hr = PNGToDDS(option, i);
            if (FAILED(hr))
                return hr;
        }
    }
}

void PreviewFont(HDC hdc, const FontOptions& option, int x, int y, const RECT& BoundBox, const std::wstring& text, bool chinese)
{
    if (!ValidateOption(option))
        return;

    HFONT Font;
    RECT innerBound = BoundBox;
    if (chinese)
    {
        Font = CreateFont(0-option.chnFontSize, 0, option.rotate ? 900 : 0, 0, option.chnBold ? FW_BOLD : FW_REGULAR, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH, option.chnFontName.c_str());

        innerBound.top = y + option.chnYOffsetFontFile;
    }
    else
    {
        Font = CreateFont(0-option.engFontSize, 0, 0, 0, option.engBold ? FW_BOLD : FW_MEDIUM, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH, option.engFontName.c_str());

        innerBound.top = y + option.engYOffsetFontFile; 
    }
   
    SelectObject(hdc, Font);
    SetBkMode(hdc, TRANSPARENT); 
    SetTextColor(hdc,option.color & 0xffffff);
    wchar_t buf[2] = {0, 0};
    int realy = y + (chinese ? option.chnYOffsetFontFile : option.engYOffsetFontFile) + (chinese ? option.chnYOffsetDraw : option.engYOffsetDraw);
    for (wchar_t c : text)
    {
        buf[0] = c;
        SIZE charSize;
        GetTextExtentPoint32(hdc, buf, 1, &charSize);
        innerBound.bottom = innerBound.top + (chinese ? option.chnFontSize : charSize.cy);
        ExtTextOut(hdc, x, realy,
                ETO_CLIPPED, &innerBound, buf, 1, NULL);
        x += charSize.cx;
    }
    DeleteObject(Font);
}

bool ValidateOption(const FontOptions& option, std::wstring* errorMsg)
{
    std::wstring error = L"No Errors.";





    if (errorMsg)
        *errorMsg = error;
    return true;
}
