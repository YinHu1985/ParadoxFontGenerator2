#include "stdafx.h"
#include "GeneratorDialog.h"
#include <exception>
#include "Fonts.h"
#include "resource.h"

namespace
{
HWND hDlg = 0;
FontOptions previewOption;

int CALLBACK AddFaceName(
    ENUMLOGFONTEX *lpelfe,    // logical-font data
    NEWTEXTMETRICEX *lpntme,  // physical-font data
    DWORD FontType,           // type of font
    LPARAM lParam             // application-defined data
    )
{

    HWND hcb;
    WCHAR* fn;
    fn = lpelfe->elfLogFont.lfFaceName;
    static WCHAR lastFN[50] = L"";

    HDC hdc;
    hdc = CreateCompatibleDC(NULL);
    WCHAR testword[2] = L"";

    HFONT ChiFont;
    ChiFont = CreateFont(12,0,0,0, FW_NORMAL ,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH ,fn);

    SelectObject(hdc,ChiFont);
    GetGlyphIndices(hdc,L"个",1,(LPWORD)testword,GGI_MARK_NONEXISTING_GLYPHS);
    DeleteObject(ChiFont);
    DeleteDC(hdc);


    if(lpelfe->elfLogFont.lfFaceName[0] != '@' && lstrcmpi(lpelfe->elfLogFont.lfFaceName, lastFN) != 0)
    {
        hcb = GetDlgItem((*(HWND*)lParam),IDC_COMBO_ENG_FONT);
            SendMessage(hcb ,CB_ADDSTRING,0,(LPARAM)fn);

        if(testword[0] != 0xffff)
        {
            hcb = GetDlgItem((*(HWND*)lParam),IDC_COMBO_CHN_FONT);
            SendMessage(hcb ,CB_ADDSTRING,0,(LPARAM)fn);
        }
        lstrcpy(lastFN,lpelfe->elfLogFont.lfFaceName);
    }
    return true;
}

bool CollectOptions(HWND hDlg, FontOptions& optionOut)
{
    BOOL success = TRUE;
    wchar_t buffer[32];

    SendDlgItemMessage(hDlg, IDC_COMBO_ENG_FONT ,WM_GETTEXT, 32,(LPARAM)buffer); 
    optionOut.engFontName = buffer;
    SendDlgItemMessage(hDlg, IDC_COMBO_CHN_FONT, WM_GETTEXT, 32,(LPARAM)buffer); 
    optionOut.chnFontName = buffer;

    unsigned int colorR = GetDlgItemInt(hDlg, IDC_EDIT_COLORR, &success, FALSE);
    if (success == FALSE)
        return false;
    unsigned int colorG = GetDlgItemInt(hDlg, IDC_EDIT_COLORG, &success, FALSE);
    if (success == FALSE)
        return false;
    unsigned int colorB = GetDlgItemInt(hDlg, IDC_EDIT_COLORB, &success, FALSE);
    if (success == FALSE)
        return false;
    unsigned int colorA = GetDlgItemInt(hDlg, IDC_EDIT_COLORA, &success, FALSE);
    if (success == FALSE)
        return false;

    optionOut.color = (colorA & 0xff)<<24 | (colorB & 0xff)<<16 | (colorG & 0xff)<<8| (colorR & 0xff);

    optionOut.engFontSize = GetDlgItemInt(hDlg, IDC_EDIT_ENG_SIZE, &success, FALSE);
    if (success == FALSE)
        return false;
    optionOut.chnFontSize = GetDlgItemInt(hDlg, IDC_EDIT_CHN_SIZE, &success, FALSE);
    if (success == FALSE)
        return false;
    optionOut.engCodePage = GetDlgItemInt(hDlg, IDC_EDIT_ENG_CODEPAGE, &success, FALSE);
    if (success == FALSE)
        return false;
    optionOut.chnCodePage = GetDlgItemInt(hDlg, IDC_EDIT_CHN_CODEPAGE, &success, FALSE);
    if (success == FALSE)
        return false;
    optionOut.engYOffsetDraw = GetDlgItemInt(hDlg, IDC_EDIT_ENG_YOFFSET_DRAW, &success, TRUE);
    if (success == FALSE)
        return false;
    optionOut.engYOffsetFontFile = GetDlgItemInt(hDlg, IDC_EDIT_ENG_YOFFSET_FILE, &success, TRUE);
    if (success == FALSE)
        return false;
    optionOut.chnYOffsetDraw = GetDlgItemInt(hDlg, IDC_EDIT_CHN_YOFFSET_DRAW, &success, TRUE);
    if (success == FALSE)
        return false;
    optionOut.chnYOffsetFontFile = GetDlgItemInt(hDlg, IDC_EDIT_CHN_YOFFSET_FILE, &success, TRUE);
    if (success == FALSE)
        return false;

    optionOut.engBold = IsDlgButtonChecked(hDlg, IDC_CHECK_ENG_BOLD) == BST_CHECKED;
    optionOut.chnBold = IsDlgButtonChecked(hDlg, IDC_CHECK_CHN_BOLD) == BST_CHECKED;
    optionOut.blackMode = IsDlgButtonChecked(hDlg, IDC_CHECK_BLACK_MODE) == BST_CHECKED;
    optionOut.excludeSymbols = IsDlgButtonChecked(hDlg, IDC_CHECK_EXCLUDE_MORE) == BST_CHECKED;
    optionOut.extraLineSpace = IsDlgButtonChecked(hDlg, IDC_CHECK_LINE_SPACE) == BST_CHECKED;
    optionOut.hideDecimalPoint = IsDlgButtonChecked(hDlg, IDC_CHECK_HIDE_DP) == BST_CHECKED;
    optionOut.onlyASCII = IsDlgButtonChecked(hDlg, IDC_CHECK_ONLY_ASCII) == BST_CHECKED;
    optionOut.onlyExtraChar = IsDlgButtonChecked(hDlg, IDC_CHECK_ONLY_EXTRA) == BST_CHECKED;
    optionOut.padding = IsDlgButtonChecked(hDlg, IDC_CHECK_PADDING) == BST_CHECKED;
    optionOut.rotate = IsDlgButtonChecked(hDlg, IDC_CHECK_ROTATE) == BST_CHECKED;
    optionOut.useGBK = IsDlgButtonChecked(hDlg, IDC_CHECK_USE_GBK) == BST_CHECKED;
    optionOut.shining = IsDlgButtonChecked(hDlg, IDC_CHECK_SHINING_EFFECT) == BST_CHECKED;
    optionOut.ucs2mode = IsDlgButtonChecked(hDlg, IDC_UCS2) == BST_CHECKED;
    optionOut.allowMultiFile = IsDlgButtonChecked(hDlg, IDC_MULTIPAGE) == BST_CHECKED;
    return true;
}

} //namespace



INT_PTR CALLBACK	GeneratorDialog::DialogMain(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Message handler for Main dialog
    UNREFERENCED_PARAMETER(lParam);
    try{
        switch (message)
        {
        case WM_INITDIALOG:
            {
                RECT Rect;
                GetWindowRect(hDlg, &Rect);
                SetWindowPos(hDlg, HWND_TOP, 
                (GetSystemMetrics(SM_CXSCREEN)/2 - ((Rect.right - Rect.left)/2)),   
                (GetSystemMetrics(SM_CYSCREEN)/2 - ((Rect.bottom - Rect.top)/2)),  
                (Rect.right - Rect.left), (Rect.bottom - Rect.top), SWP_SHOWWINDOW);

                HDC hdc;
                LOGFONT lf;
                lf.lfFaceName[0] = '\0';
                lf.lfCharSet = DEFAULT_CHARSET;
                hdc = ::GetDC(hDlg);
                EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROC)AddFaceName,(LPARAM)&hDlg,0);
                SetDlgItemText(hDlg, IDC_COMBO_ENG_FONT, L"Arial");
                SetDlgItemText(hDlg, IDC_COMBO_CHN_FONT, L"宋体");

                SetDlgItemInt(hDlg, IDC_EDIT_COLORA, 255, FALSE);
                SetDlgItemInt(hDlg, IDC_EDIT_COLORR, 255, FALSE);
                SetDlgItemInt(hDlg, IDC_EDIT_COLORG, 255, FALSE);
                SetDlgItemInt(hDlg, IDC_EDIT_COLORB, 255, FALSE);

                SetDlgItemInt(hDlg, IDC_EDIT_ENG_SIZE, 13, FALSE);
                SetDlgItemInt(hDlg, IDC_EDIT_CHN_SIZE, 13, FALSE);
                SetDlgItemInt(hDlg, IDC_EDIT_ENG_CODEPAGE, 1252, FALSE);
                SetDlgItemInt(hDlg, IDC_EDIT_CHN_CODEPAGE, 936, FALSE);

                SetDlgItemInt(hDlg, IDC_EDIT_ENG_YOFFSET_DRAW, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_EDIT_ENG_YOFFSET_FILE, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_EDIT_CHN_YOFFSET_DRAW, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_EDIT_CHN_YOFFSET_FILE, 0, TRUE);
                return (INT_PTR)TRUE;
            }
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                PAINTSTRUCT ps2;
                BeginPaint(hDlg, &ps);
                HWND hWndPicCtrl = GetDlgItem(hDlg, IDC_PIC_PREVIEW); 
                HDC hdc = BeginPaint(hWndPicCtrl, &ps2);
                RECT r;
                GetClientRect(hWndPicCtrl, &r);
                HBRUSH hBrush = (HBRUSH)CreateSolidBrush(0x66aaaa);
                FillRect(hdc,&r,hBrush);
                MoveToEx(hdc, 20, 20, NULL);
                LineTo(hdc, r.right, 20);
                PreviewFont(hdc, previewOption, 20, 20, r, L"中文测试", true);
               
                MoveToEx(hdc, 20, 180, NULL);
                LineTo(hdc, r.right, 180);
                 PreviewFont(hdc, previewOption, 20, 180, r, L"AaBbFfGgWw12345", false);
                EndPaint(hWndPicCtrl, &ps2);
                ReleaseDC(hWndPicCtrl, hdc);
                EndPaint(hDlg, &ps);
                break;
            }
        case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                case IDOK:
                    {
                        FontOptions options;
                        CollectOptions(hDlg, options);
                        std::wstring errorMsg;
                        if (!ValidateOption(options, &errorMsg))
                        {
                            MessageBox(hDlg, L"Error!", errorMsg.c_str(), MB_OK);
                            break;
                        }       
                        bool limitSize = IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT_SIZE) == BST_CHECKED;
                        HRESULT hr = GenerateFont(options, limitSize ? 2048 : 4096);
                        if (SUCCEEDED(hr)) 
                            MessageBox(hDlg, L"SUCCEEDED!", L"Finished", MB_OK);   
                        else
                            MessageBox(hDlg, L"FAILED!", L"Finished", MB_OK);   
                         break;
                    }
                case IDPREVIEW:
                    { 
                        CollectOptions(hDlg, previewOption);
                        std::wstring errorMsg;
                        if (!ValidateOption(previewOption, &errorMsg))
                        {
                            MessageBox(hDlg, L"Error!", errorMsg.c_str(), MB_OK);
                            break;
                        }       

                        RECT r;
                        GetClientRect(hDlg, &r);
                        InvalidateRect(hDlg,&r,true);
                        UpdateWindow(hDlg);
                        break;
                    }
                case IDCANCEL:
                    {
                        DestroyWindow(hDlg);
                        return (INT_PTR)TRUE;
                    }
                case IDC_BUTTON_SEL_COLOR:
                    {
                        CColorDialog colorDialog;
                        if (IDOK == colorDialog.DoModal())
                        {
                            unsigned int color = colorDialog.GetColor();
                            unsigned char colorR = (unsigned char)GetRValue(color);
                            unsigned char colorG = (unsigned char)GetGValue(color);
                            unsigned char colorB = (unsigned char)GetBValue(color);
                            SetDlgItemInt(hDlg, IDC_EDIT_COLORR, colorR, FALSE);
                            SetDlgItemInt(hDlg, IDC_EDIT_COLORG, colorG, FALSE);
                            SetDlgItemInt(hDlg, IDC_EDIT_COLORB, colorB, FALSE);
                        }
                        
                        break;
                    }
                default:
                    {
                        break;
                    }
                }
                break;
            }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        }

    }
    catch(std::exception& e)
    {
        MessageBoxA(NULL, e.what(), "Error!", MB_OK);
        // TODO:record error
    }
    return (INT_PTR)FALSE;
}

void GeneratorDialog::Initialize (HINSTANCE hInstance, HWND hParent)
{
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))  
    {       
        // TODO: change error code to suit your needs              
        // Ignore... we don't really need it.
    } 

    hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_GENERATOR), hParent, GeneratorDialog::DialogMain);

    GeneratorDialog::SetHide(true);
}

HWND GeneratorDialog::GetHandle()
{
    return hDlg;
}

void GeneratorDialog::SetHide(bool hide)
{
    if (hDlg)
    {
        ShowWindow(hDlg, hide ? SW_HIDE : SW_SHOW);
        
        UpdateWindow(hDlg);
    }
}
