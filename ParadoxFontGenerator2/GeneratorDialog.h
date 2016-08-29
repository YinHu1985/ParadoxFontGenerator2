#ifndef GENERATOR_DIALOG_H
#define GENERATOR_DIALOG_H

namespace GeneratorDialog
{
	INT_PTR CALLBACK DialogMain(HWND, UINT, WPARAM, LPARAM);
    INT_PTR CALLBACK ProgressBar(HWND, UINT, WPARAM, LPARAM);
	void Initialize (HINSTANCE hInstance, HWND hParent);
	HWND GetHandle();
	void SetHide(bool hide);
} // namespace GeneratorDialog 

#endif //GENERATOR_DIALOG_H
