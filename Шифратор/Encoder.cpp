#include "crypter.h"

wchar_t Local_File_Dir;
wchar_t szFileName[MAX_PATH] = { 0 }; // Глобальная переменная для буфера имени файла
/*
TrollingCont. 22.05.2020
*/

#define IMG_ORIGIN_EDIT    100 // .img
#define IMG_ENCRYPTED_EDIT 101
#define ENCRYPT_BUTTON     102
#define Folder_Button      103

bool select_dir = false;
int methodCrypto = 0;

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

// Interface
HWND MainWindow, OpenFolder, OriginIMGEdit, EncryptedIMGEdit, EncryptButton, EncryptKeyEdit;
HFONT MainFont, LabelsFont;
// Interface

// Encryption
BCRYPT_ALG_HANDLE BCRAHandle;
BCRYPT_HASH_HANDLE hHash = NULL;
NTSTATUS Status = 0xC0000001L;
DWORD cbData = 0, cbHash = 0, cbHashObject = 0;
PBYTE pbHashObject = NULL;
PBYTE pbHash = NULL;
// Encryption

WCHAR MsgBoxText[64];

LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Encryption part
	if (!NT_SUCCESS(Status = BCryptOpenAlgorithmProvider(&BCRAHandle, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_HASH_REUSABLE_FLAG)))
	{
		StringCchPrintfW(MsgBoxText, 64, L"BCryptOpenAlgorithmProvider failed.\nCode: %u", Status);
		MessageBoxW(MainWindow, MsgBoxText, L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0001);
	}

	if (!NT_SUCCESS(Status = BCryptGetProperty(BCRAHandle, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbHashObject, sizeof(DWORD), &cbData, 0)))
	{
		StringCchPrintfW(MsgBoxText, 64, L"BCryptGetProperty [1] failed.\nCode: %u", Status);
		MessageBoxW(MainWindow, MsgBoxText, L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0001);
	}

	pbHashObject = (PBYTE)malloc(cbHashObject);
	if (pbHashObject == NULL)
	{
		MessageBoxW(MainWindow, L"malloc failed", L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0000);
	}

	if (!NT_SUCCESS(Status = BCryptCreateHash(BCRAHandle, &hHash, pbHashObject, cbHashObject, NULL, 0, BCRYPT_HASH_REUSABLE_FLAG)))
	{
		StringCchPrintfW(MsgBoxText, 64, L"BCryptCreateHash failed.\nCode: %u", Status);
		MessageBoxW(MainWindow, MsgBoxText, L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0001);
	}
	// Encryption part

	// Interface part
	LPCWSTR WndClassName = L"MainWindow";

	WNDCLASSEX WndClass;
	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.style = CS_HREDRAW;
	WndClass.cbWndExtra = 0;
	WndClass.cbClsExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = CreateSolidBrush(RGB(245, 245, 245));
	WndClass.lpszMenuName = NULL;
	WndClass.hIconSm = LoadIcon(WndClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = WndClassName;
	RegisterClassEx(&WndClass);

	MainFont = CreateFontW(17, 7, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Helvetica");
	LabelsFont = CreateFontW(16, 6, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Helvetica");

	MainWindow = CreateWindowExW(WS_EX_ACCEPTFILES, WndClassName, L"LLIEPLLIEHb Cryptographer", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 725, 285, NULL, NULL, hInstance, NULL);

	OriginIMGEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 150, 40, 325+200, 26, MainWindow, (HMENU)IMG_ORIGIN_EDIT, hInstance, NULL);
	OpenFolder = CreateWindowExW(WS_EX_CLIENTEDGE, L"button", L"Выбрать файл", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 39, 125, 30, MainWindow, (HMENU)Folder_Button, hInstance, NULL);


	EncryptedIMGEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 10, 110, 465+200, 26, MainWindow, (HMENU)IMG_ENCRYPTED_EDIT, hInstance, NULL);

	EncryptButton = CreateWindowExW(WS_EX_CLIENTEDGE, L"button", L"Начать шифрование", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 500, 178+25, 200, 30, MainWindow, (HMENU)ENCRYPT_BUTTON, hInstance, NULL);

	EncryptKeyEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 10, 180, 190, 26, MainWindow, (HMENU)IMG_ENCRYPTED_EDIT, hInstance, NULL);


	const COLORREF backgroundColor = RGB(200, 200, 200);


	SetClassLongPtr(MainWindow, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(CreateSolidBrush(backgroundColor)));


	ShowWindow(MainWindow, nShowCmd);
	UpdateWindow(MainWindow);
	MSG Message;
	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	// Interface part

	return 0;
}

void GenerateIMGKeyHash(PUCHAR EncryptKey, DWORD KeyLength)
{
	if (!NT_SUCCESS(Status = BCryptHashData(hHash, (PUCHAR)EncryptKey, KeyLength * 2, 0)))
	{
		StringCchPrintfW(MsgBoxText, 64, L"BCryptHashData failed.\nCode: %u", Status);
		MessageBoxW(MainWindow, MsgBoxText, L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0001);
	}

	if (!NT_SUCCESS(Status = BCryptGetProperty(BCRAHandle, BCRYPT_HASH_LENGTH, (PUCHAR)&cbHash, sizeof(DWORD), &cbData, 0)))
	{
		StringCchPrintfW(MsgBoxText, 64, L"BCryptGetProperty [2] failed.\nCode: %u", Status);
		MessageBoxW(MainWindow, MsgBoxText, L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0001);
	}

	pbHash = (PBYTE)malloc(cbHash);
	if (pbHash == NULL)
	{
		MessageBoxW(MainWindow, L"malloc failed", L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0000);
	}

	if (!NT_SUCCESS(Status = BCryptFinishHash(hHash, pbHash, cbHash, 0)))
	{
		StringCchPrintfW(MsgBoxText, 64, L"BCryptFinishHash failed.\nCode: %u", Status);
		MessageBoxW(MainWindow, MsgBoxText, L"Error", MB_ICONERROR);
		ExitProcess(0xFACE0001);
	}
}

LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC Writer;
	PAINTSTRUCT Pstr;

	switch (message)
	{
	case WM_DESTROY:
		ExitProcess(0);

		break;
	case WM_CLOSE:
		ExitProcess(0);

		break;
	case WM_DROPFILES:
	{
		DWORD Size = DragQueryFileW((HDROP)wParam, 0, NULL, 0);
		LPWSTR DroppedFileName = (LPWSTR)malloc(Size * 2 + 2);
		DragQueryFileW((HDROP)wParam, 0, DroppedFileName, Size + 1);
		SetWindowTextW(OriginIMGEdit, DroppedFileName);
		free(DroppedFileName);

		return 0;
	}
	break;
	case WM_PAINT:
		SendMessageW(OriginIMGEdit, WM_SETFONT, (WPARAM)MainFont, TRUE);
		SendMessageW(EncryptedIMGEdit, WM_SETFONT, (WPARAM)MainFont, TRUE);
		SendMessageW(EncryptButton, WM_SETFONT, (WPARAM)MainFont, TRUE);
		SendMessageW(EncryptButton, WM_SETFONT, (WPARAM)MainFont, TRUE);
		SendMessageW(EncryptKeyEdit, WM_SETFONT, (WPARAM)MainFont, TRUE);
		SendMessageW(OpenFolder, WM_SETFONT, (WPARAM)MainFont, TRUE);


		Writer = BeginPaint(hWnd, &Pstr);
		SelectObject(Writer, LabelsFont);
		SetBkMode(Writer, TRANSPARENT);
		TextOutW(Writer, 10, 15, L"Исходный файл", 15);
		TextOutW(Writer, 10, 85, L"Путь сохранения", 15);
		TextOutW(Writer, 10, 155, L"Ключ шифрования", 15);
		EndPaint(hWnd, &Pstr);

		return 0;
		break;


	case WM_COMMAND:

		if (wParam == Folder_Button)
		{
			OPENFILENAME ofn;       // Структура для настроек диалогового окна
			//wchar_t szFileName[MAX_PATH] = { 0 };    // Буфер для имени файла

			// Задаем настройки диалогового окна
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFilter = L"IMG files (*.img)\0*.img\0IDE files (*.ide)\0*.ide\0IPL files (*.ipl)\0*.ipl\0BAT files (*.bat)\0*.bat\0Прочую хуйню шифруешь? (*.*)\0*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = L"crypto";
			if (GetOpenFileName(&ofn))
			{
				wchar_t message[MAX_PATH + 50];
				swprintf_s(message, L"Хотите ли вы зашифровать этот файл \"%s\"?", PathFindFileNameW(szFileName));
				if (MessageBox(NULL, message, L"Подтверждение", MB_YESNO) == IDYES)
				{
					wchar_t* fileName = PathFindFileNameW(szFileName);
					SetWindowTextW(OriginIMGEdit, szFileName);
					PathRenameExtensionW(fileName, L".crypto");
					SetWindowTextW(OpenFolder, fileName);
					SetWindowTextW(EncryptedIMGEdit, szFileName);
					Local_File_Dir = (wchar_t)szFileName;
					select_dir = true;
				}
				else
				{
					SendMessage(hWnd, WM_COMMAND, Folder_Button, NULL);
				}
			}
			return 0;
		}


		if (wParam == ENCRYPT_BUTTON)
		{
			// Opening files and validating user input
			DWORD OriginalIMGNameLength = GetWindowTextLengthW(OriginIMGEdit);
			if (select_dir == false)
			{
				MessageBoxW(MainWindow, L"Укажите путь к файлу!", L"Ошибка", MB_ICONERROR);
				return 0;
			}

			DWORD EncryptedIMGNameLength = GetWindowTextLengthW(EncryptedIMGEdit);
			if (EncryptedIMGNameLength == 0)
			{
				MessageBoxW(MainWindow, L"Куда сохранять?!", L"Ошибка", MB_ICONERROR);
				return 0;
			}

			DWORD KeyLength = GetWindowTextLengthW(EncryptKeyEdit);
			if (KeyLength == 0)
			{
				MessageBoxW(MainWindow, L"Введите ключ", L"Ошибка", MB_ICONERROR);
				return 0;
			}

			wchar_t* keyText = new wchar_t[KeyLength + 1];
			GetWindowTextW(EncryptKeyEdit, keyText, KeyLength + 1);

			bool isNumeric = true;
			for (int i = 0; i < KeyLength; i++)
			{
				if (!iswdigit(keyText[i]))
				{
					isNumeric = false;
					break;
				}
			}

			delete[] keyText;

			if (isNumeric)
			{
				methodCrypto = 1;
			}
			else
			{
				methodCrypto = 2;
			}

			LPWSTR OriginalIMGName = (LPWSTR)malloc(OriginalIMGNameLength * 2 + 2);
			if (OriginalIMGName == NULL)
			{
				MessageBoxW(MainWindow, L"malloc failed", L"Error", MB_ICONERROR);
				ExitProcess(0xFACE0000);
			}

			GetWindowTextW(OriginIMGEdit, OriginalIMGName, OriginalIMGNameLength + 1);

			HANDLE OriginalIMG = CreateFileW(OriginalIMGName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (OriginalIMG == INVALID_HANDLE_VALUE)
			{
				free(OriginalIMGName);
				StringCchPrintfW(MsgBoxText, 64, L"Ошибка открытия входного файла.\nКод: %u", GetLastError());
				MessageBoxW(MainWindow, MsgBoxText, L"Ошибка", MB_ICONERROR);
				return 0;
			}

			LPWSTR EncryptedIMGName = (LPWSTR)malloc(EncryptedIMGNameLength * 2 + 2);
			if (EncryptedIMGName == NULL)
			{
				MessageBoxW(MainWindow, L"malloc failed", L"Error", MB_ICONERROR);
				ExitProcess(0xFACE0000);
			}

			GetWindowTextW(EncryptedIMGEdit, EncryptedIMGName, EncryptedIMGNameLength + 1);

			if (PathFileExistsW(EncryptedIMGName) && MessageBoxW(MainWindow, L"Выходной файл по указанному пути уже существует. Перезаписать?", L"Выберите действие", MB_YESNO | MB_ICONQUESTION) != IDYES)
			{
				free(OriginalIMGName);
				free(EncryptedIMGName);
				CloseHandle(OriginalIMG);
				return 0;
			}


			LPWSTR EncryptKey = (LPWSTR)malloc(KeyLength * 2 + 2);
			if (EncryptKey == NULL)
			{
				MessageBoxW(MainWindow, L"malloc failed", L"Error", MB_ICONERROR);
				ExitProcess(0xFACE0000);
			}

			wchar_t KeyBuffer[MAX_PATH];
			GetWindowTextW(EncryptKeyEdit, KeyBuffer, MAX_PATH);

			wchar_t InputBuffer[MAX_PATH];
			GetWindowTextW(OriginIMGEdit, InputBuffer, MAX_PATH);

			wchar_t OutputBuffer[MAX_PATH];
			GetWindowTextW(EncryptedIMGEdit, OutputBuffer, MAX_PATH);


			if (methodCrypto == 0)
			{
				// непредвиденная ошибка
				MessageBoxW(MainWindow, L"Непредвиденная ошибка выбора метода", L"Error", MB_ICONERROR);
			}
			else if (methodCrypto == 1)
			{
				// number
				Metod1(KeyBuffer,InputBuffer,OutputBuffer);
				MessageBoxW(MainWindow, L"Метод шифрования [const int] паролем", L"Внимание", MB_OK);
			}
			else if (methodCrypto == 2)
			{
				// string
				 Metod2(KeyBuffer,InputBuffer,OutputBuffer);
				MessageBoxW(MainWindow, L"Метод шифрования [string] паролем", L"Внимание", MB_OK);
			}


			// Cleanup
			free(pbHash);
			CloseHandle(OriginalIMG);
			//CloseHandle(EncryptedIMG);
			free(OriginalIMGName);
			free(EncryptedIMGName);
			free(EncryptKey);

			return 0;
		}

		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
