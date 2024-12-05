// SimplePlayer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SimplePlayer.h"
#include <vector>
#include "SongPlayer.h"
#include "filedeal.h"
#include <shlobj.h>
#include <commdlg.h>  // For other dialog boxes
#include <thread>
#include <mutex>
#include <string>
#include <codecvt>

#define MAX_LOADSTRING 100
#define FORMAT_SONG_DISPLAY(song, artist, targetWidth) \
    (song + std::string(targetWidth - song.length(), ' ') + artist)
#define RIGHTPAD(str, totalLength, padChar) \
    ((str).length() >= (totalLength) ? (str) : (str) + std::string((totalLength) - (str).length(), (padChar)))

// Global variables:
HINSTANCE hInst;                                // Current instance
WCHAR szTitle[MAX_LOADSTRING];                  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // Main window class name
HWND hSongLryic; // Handle for the lyrics text box
HWND hSongBox; // Handle for the song list box
HICON playIcon;
HICON pauseIcon;
HICON nextIcon;
HICON prevIcon;
HWND playButton;
HWND prevButton;
HWND nextButton;
static int currentSelection = -1;
std::wstring filepath;


std::vector<SongInfo> songLists = {};
std::vector<SongInfo> filterList = {};
// Forward declarations of functions contained in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

std::vector <SongInfo> LoadSongsFromCSV(const std::wstring& filename) {
	std::vector <SongInfo> songs;
	std::ifstream file(filename);

	if (!file.is_open()) {
		MessageBoxA(NULL, "Failed to open lyrics.csv!", "Error", MB_OK |
			MB_ICONERROR);
		return songs;
	}

	std::string line;

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string title, artist, lyrics;

		std::getline(ss, title, ',');
		std::getline(ss, artist, ',');
		std::getline(ss, lyrics, ',');

		songs.push_back({ title, artist, lyrics });
	}

	file.close();
	return songs;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place your code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SIMPLEPLAYER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLEPLAYER));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
//  Function: MyRegisterClass()
//
//  Purpose: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;


	playIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLAY_SONG));
	pauseIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAUSE_SONG));
	nextIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NEXT_SONG));
	prevIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PREV_SONG));

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEPLAYER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SIMPLEPLAYER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SIMPLEPLAYER));

	return RegisterClassExW(&wcex);
}

//
//   Function: InitInstance(HINSTANCE, int)
//
//   Purpose: Saves the instance handle and creates the main window
//
//   Notes:
//
//        In this function, we save the instance handle in a global variable and create and
//        display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store the instance handle in a global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}





//
//  Function: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Purpose: Handles the messages for the main window.
//
//  WM_COMMAND  - Handles the application menu
//  WM_PAINT    - Draws the main window
//  WM_DESTROY  - Sends a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)

	{
	case WM_SIZE: {
		// Get the width and height of the window
		RECT rect;
		GetClientRect(hWnd, &rect);
		int windowWidth = rect.right - rect.left - 300;
		int windowHeight = rect.bottom - rect.top;
		MoveWindow(hSongLryic, 320, 20, windowWidth - 30, windowHeight - 40, TRUE);
		break;
	}
	case WM_CREATE:{
		std::wstring csvFile = L"lyrics.csv";
		vector<SongInfo> songLists = LoadSongsFromCSV(csvFile);
		if (!songLists.empty()) {
			filterList = songLists;

			// Populate the song list box
			for (const auto& song : songLists) {
				const int col1_width = 25; // First column width
				const int col2_width = 280 - col1_width; // Second column width
				std::string displayText = align_string(song.song, col1_width); // First column alignment
				displayText += align_string(song.artist, col2_width); // Second column alignment
				SendMessageA(hSongBox, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
			}
		}
		else {
			MessageBoxA(NULL, "No songs found in lyrics.csv!", "Info", MB_OK |
				MB_ICONINFORMATION);
		}
		// Search box
		CreateWindowEx(0, TEXT("EDIT"), TEXT("Input lyric or artist"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			20, 20, 200, 25, hWnd, (HMENU)IDC_SEARCH_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		// Search
		CreateWindowEx(0, TEXT("BUTTON"), TEXT("Search"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			230, 20, 80, 25, hWnd, (HMENU)IDC_SEARCH_BUTTON, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		// Songs
		hSongBox = CreateWindowEx(0, TEXT("LISTBOX"), TEXT(""),
			WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | LBS_STANDARD | LBS_SORT,
			20, 70, 280, 400, hWnd, (HMENU)IDC_SONG_LIST, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

		// Song Lists
		CreateWindowEx(0, TEXT("EDIT"), TEXT("Song Lists:"),
			WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
			20, 50, 200, 20, hWnd, (HMENU)IDC_SONG_BOX, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		// Create a text box to display lyrics on the right side
		hSongLryic = CreateWindowEx(0, TEXT("EDIT"), TEXT(""),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY |
			ES_CENTER,
			320, 60, 300, 100, hWnd, (HMENU)IDC_LYRICS_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

		// Play
		playButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Play"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_ICON,
			120, 470, 40, 40, hWnd, (HMENU)IDC_PLAY_BUTTON, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		SendMessage(playButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)playIcon);
		// Prev
		prevButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Prev"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_ICON,
			40, 470, 40, 40, hWnd, (HMENU)IDC_PREV_BUTTON, ((LPCREATESTRUCT)lParam)->hInstance, NULL);// Search

		SendMessage(prevButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)prevIcon);

		// Next
		nextButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Next"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_ICON,
			200, 470, 40, 40, hWnd, (HMENU)IDC_NEXT_BUTTON, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

		SendMessage(nextButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)nextIcon);


		EnableWindow(GetDlgItem(hWnd, IDC_PLAY_BUTTON), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_PREV_BUTTON), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_NEXT_BUTTON), FALSE);
		break;
	}
	
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Analyze menu selection:
		switch (wmId)
		{
		case IDC_PREV_BUTTON:
			if (currentSelection > 0) {
				//  If it's not the first item, select the previous song
				currentSelection--;
				SendMessage(hSongBox, LB_SETCURSEL, currentSelection, 0);

				SongInfo selectedSong = filterList.at(currentSelection);
				// Set the lyrics textbox content
				SetWindowTextA(GetDlgItem(hWnd, IDC_LYRICS_EDIT), selectedSong.lyric.c_str());

				if (isPlaying)
				{
					stopAudio();
					closeAudio();
					isPlaying = false;
					isStopping = false;
				}

				SongInfo info = filterList[currentSelection];
				std::wstring songname = string2wstring(info.song);
				std::wstring palyfile = filepath + L"\\" + songname + L".mp3";
				if (isStopping || open(palyfile))
				{
					SendMessage(playButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pauseIcon);
					isPlaying = true;
					playAudio();
				}
			}
			break;

		case IDC_NEXT_BUTTON:
			if (currentSelection < SendMessage(hSongBox, LB_GETCOUNT, 0, 0) - 1) {
				// If it's not the last item, select the next song
				currentSelection++;
				SendMessage(hSongBox, LB_SETCURSEL, currentSelection, 0);
				if (isPlaying)
				{
					stopAudio();
					closeAudio();
					isPlaying = false;
					isStopping = false;
				}
				SongInfo selectedSong = filterList.at(currentSelection);
				// Set the lyrics textbox content
				SetWindowTextA(GetDlgItem(hWnd, IDC_LYRICS_EDIT), selectedSong.lyric.c_str());
				SongInfo info = filterList[currentSelection];
				std::wstring songname = string2wstring(info.song);
				std::wstring palyfile = filepath + L"\\" + songname + L".mp3";
				if (isStopping || open(palyfile))
				{
					SendMessage(playButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pauseIcon);
					isPlaying = true;
					playAudio();
				}
			}
			break;
		case IDC_PLAY_BUTTON:
		{
			if (isPlaying) {
				stopAudio();
				isPlaying = false;
				isStopping = true;
				SendMessage(playButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)playIcon);
			}
			else {
				int selectIdx = SendMessage(GetDlgItem(hWnd, IDC_SONG_LIST), LB_GETCURSEL, 0, 0);
				SongInfo info = filterList[selectIdx];
				std::wstring songname = string2wstring(info.song);
				std::wstring palyfile = filepath + L"\\" + songname + L".mp3";
				if (isStopping || open(palyfile))
				{
					SendMessage(playButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pauseIcon);
					isPlaying = true;
					playAudio();
				}
			}
		}
		break;
		case IDC_SEARCH_BUTTON: {
			char szSearchText[256];
			GetWindowTextA(GetDlgItem(hWnd, IDC_SEARCH_EDIT), szSearchText, sizeof(szSearchText));
			RECT rect;
			GetClientRect(hSongBox, &rect);
			// Clear the list box
			SendMessageA(GetDlgItem(hWnd, IDC_SONG_LIST), LB_RESETCONTENT, 0, 0);
			filterList.clear();
			for (auto& song : songLists)
			{
				KMPSearch(song.lyric, szSearchText);
				if (KMPSearch(song.lyric, szSearchText) != -1) {
					filterList.push_back(song);
					const int col1_width = 25;  // First column width
					const int col2_width = rect.right - rect.left - col1_width;  // Second column width
					std::string displayText = align_string(song.song, col1_width); // First column alignment
					displayText += align_string(song.artist, col2_width); // Second column alignment
					SendMessageA(GetDlgItem(hWnd, IDC_SONG_LIST), LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
				}
			}
			currentSelection = filterList.size() > 0 ? 0 : -1;


			int selectedIndex = -1;

			UpdateSelect(hWnd, selectedIndex);
			break;
		}
		case IDC_SONG_LIST: {
			// Get the current selected song index
			int selectedIndex = -1;

			UpdateSelect(hWnd, selectedIndex);
			if (selectedIndex > LB_ERR) {
				// Get the selected song information
				SongInfo selectedSong = filterList.at(selectedIndex);
				currentSelection = selectedIndex;
				// Set the lyrics textbox content
				SetWindowTextA(GetDlgItem(hWnd, IDC_LYRICS_EDIT), selectedSong.lyric.c_str());
			}
			break;
		}
		case IDM_IMPORT_SONG:
		{
			SendMessage(hSongBox, LB_RESETCONTENT, 0, 0);
			// COM Initialization
			CoInitialize(NULL);

			// BROWSEINFO structure initialization
			BROWSEINFO bi = { 0 };
			bi.lpszTitle = L"Select a Folder";  // Use wide-character string
			bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_USENEWUI | BIF_RETURNONLYFSDIRS;  // Only select folders

			// Get the folder selected by the user
			PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
			if (pidl != NULL) {
				wchar_t path[MAX_PATH];  // Use wide-character buffer
				if (SHGetPathFromIDList(pidl, path)) {
					filepath = path;
					filterList.clear();
					songLists = LoadSongInfo(path);
					for (auto& song : songLists)
					{
						filterList.push_back(song);
						const int col1_width = 25;  // First column width
						const int col2_width = 280 - col1_width;  // Second column width
						std::string displayText = align_string(song.song, col1_width); // First column alignment
						displayText += align_string(song.artist, col2_width); // Second column alignment
						SendMessageA(GetDlgItem(hWnd, IDC_SONG_LIST), LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
					}
				}
				// Release PIDL
				CoTaskMemFree(pidl);
			}

			// COM Cleanup
			CoUninitialize();
		}
		break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void UpdateSelect(HWND hWnd, int& selectIdx) {

	selectIdx = SendMessage(GetDlgItem(hWnd, IDC_SONG_LIST), LB_GETCURSEL, 0, 0);
	EnableWindow(GetDlgItem(hWnd, IDC_PLAY_BUTTON), selectIdx > LB_ERR);
	EnableWindow(GetDlgItem(hWnd, IDC_PREV_BUTTON), selectIdx > LB_ERR);
	EnableWindow(GetDlgItem(hWnd, IDC_NEXT_BUTTON), selectIdx > LB_ERR);
}
