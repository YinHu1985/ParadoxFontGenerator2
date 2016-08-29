#ifndef FONTS_H
#define FONTS_H

struct FontOptions
{
    std::wstring engFontName;
    std::wstring chnFontName;
    int engFontSize;
    int chnFontSize;
    bool engBold;
    bool chnBold;
    int engYOffsetDraw;
    int chnYOffsetDraw;
    int engYOffsetFontFile;
    int chnYOffsetFontFile;
    bool rotate;
    unsigned int color;

    int chnCodePage;
    int engCodePage;
    bool onlyASCII;
    // GB only options
    bool useGBK;
    bool excludeSymbols;
    // Extra characters
    std::wstring ExtraCharPath;
    bool onlyExtraChar; 
    bool hideDecimalPoint;
    bool padding;
    bool extraLineSpace;

    bool blackMode;

    bool shining;

    bool ucs2mode;

    bool allowMultiFile;

    FontOptions() 
        : engFontName(L"Garamond")
        , chnFontName(L"simsun")
        , engFontSize(13)
        , chnFontSize(13)
        , engBold(false)
        , chnBold(false)
        , engYOffsetDraw(0)
        , chnYOffsetDraw(1)
        , engYOffsetFontFile(-2)
        , chnYOffsetFontFile(0)
        , rotate(false)
        , color(0xFFFFFFFF)
        , chnCodePage(936)
        , engCodePage(1252)
        , onlyASCII(false)
        , useGBK(false)
        , excludeSymbols(false)
        , ExtraCharPath(L"addchar.txt")
        , onlyExtraChar(false)
        , hideDecimalPoint(false)
        , padding(false)
        , extraLineSpace(true)
        , blackMode(false)
        , shining(false)
        , ucs2mode(false)
        , allowMultiFile(false)
            {};
};

bool ValidateOption(const FontOptions& option, std::wstring* errorMsg = nullptr);
HRESULT GenerateFont(const FontOptions& option, int maxSize = 4096, bool ignoreOverSize = false, bool* overSize = nullptr);
void PreviewFont(HDC hdc, const FontOptions& option, int x, int y, const RECT& BoundBox, const std::wstring& text, bool chinese);

#endif // FONTS_H
