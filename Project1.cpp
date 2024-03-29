// Project1.cpp
// written by Johannes Wawra under use of Microsoft Visual Studio 2017 Windows Applet Default

#include "stdafx.h"
#include <vector>
#include <array>
#include "Project1.h"
#include <iostream>
#include <string>
#include <fstream>
#include "atlstr.h"
#include <algorithm>

using namespace std;
//FileClass: Represent the files on the disc
class file {
private:
	int size = 512; //file class has size and type only, for oth
	char type = 'F';

public:
	int getSize() {
		return size;
	}

	void setSize(int s) {
		size = s;
	}

	void setType(char t) {
		type = t;
	}

	long createSubdir(char n[9]) {
		return -1;
	}

	char getType() {
		return type;
	}
};

//emptyFile is filler for filesystem
class emptyFile : public file {
public:
	emptyFile() {
		setSize(0);
		setType('0');
	}
};

//stores userData
class userData : public file {
private:
	long back = -1; //previous File in multifile
	long frwd = -1; //next File in mutlifile
	wchar_t* data; //userdata as wchar_t for better in and output

public:
	userData() {
		setType('U');
	}

	void setBack(long b) {
		back = b;
	}

	void setFrwd(long f) {
		frwd = f;
	}

	long getBack() {
		return back;
	}

	long getFrwd() {
		return frwd;
	}

	void addUserData(wchar_t* newData) {
		data = newData;

	}

	TCHAR* getTdata() {
		return data;
	}
};

//subclass from directory to store filenames
class dir {
private:
	char type = 'F'; // F = free, D = directory, U = user data
	char name[9] = "        "; //file name, left justified, blank filled
	long link = 0; //number of first block of file
	int size = 0;
public:
	dir(long l) { //to create a free file just give block number
		link = l;
	}

	dir() {
	}

	char getType() {
		return 'D';
	}

	//creates a new subdir
	BOOLEAN createDir(char n[9]) {
		for (int i = 0; i < 9; i++) {//set name
			name[i] = n[i];
		};
		type = 'D';//set type
		return TRUE;
	}

	//creates a new UserFile
	BOOLEAN createUserFile(char n[9]) {
		for (int i = 0; i < 9; i++) {//set name
			name[i] = n[i];
		};
		type = 'U';//set type
		return TRUE;
	}

	long getLink() {
		return link;
	}

	TCHAR* getTchar() { //get Name as TCHAR
		const char* namec = name;
		size_t length = strlen(namec) + 1;
		wchar_t* wchar = new wchar_t[length];
		size_t outsize;
		mbstowcs_s(&outsize, wchar, length, namec, length - 1);
		return wchar;
	}

	BOOLEAN isFree(long l) { //check if file at this address is free
		if ((link == l) & (type == 'F')) {
			return TRUE;
		}
		return FALSE;
	}

	long isFree() {//check if this file is free
		if (type == 'F') {
			return link;
		}
		return -1;
	}

	BOOLEAN is(long l) {//check if this file exist at this address
		if (link == l) {
			return TRUE;
		}
		return FALSE;
	}

	BOOLEAN deleteDir(long l) {//delete file
		if (link == l) {
			for (int i = 0; i < 11; i++) {
				name[i] = ' ';
			}
			type = 'F';
			return true;
		}
		return false;
	}

};

//directory can contain files
class directory : public file {
private:
	long back = -1; //block before in multifile
	long frwd = -1; //block beyond in multifile
	long free = 512; // first free block
	char filler[4]; //unused
	vector<dir *> dirEntries; //list of subfilenames and addresses
public:
	directory() {
	}
	directory(long startAddress) {//create directory wih start address
		long address = startAddress;
		free = -512;
		for (unsigned int i = 0; i < dirEntries.size(); i++) { //create free direntries
			address += 512;
			dirEntries[i] = new dir(address);
		}
		setType('D');
	}

	BOOLEAN setFrwd(int f) {
		frwd = f;
		return true;
	}
	BOOLEAN setBack(int f) {
		back = f;
		return true;
	}
	long getFrwd() {
		return frwd;
	}
	long getBack() {
		return back;
	}

	long getDirIndex(long link) { //get index of subfile via address
		for (unsigned long i = 0; i < dirEntries.size(); i++) {
			if (dirEntries[i]->is(link)) {
				return i;
			}
		}
		return -1;
	}

	long getFirstFree() { //get first free index
		long link = free;
		for (unsigned long i = 0; i < dirEntries.size(); i++) {
			link = dirEntries[i]->isFree(); //if dir entrie is free
			if (link != -1) {
				free = link; //set first free address
				return i; //return index of free file
			}
		}
		return -1; //no free dir found
	}

	long getNextDir(long address) { //returns next possible dir address from address
		unsigned int index = getDirIndex(address); //get dir at address
		if ((index < dirEntries.size() - 1) && (index != -1)) { //if index exists and not last dir in directory
			return dirEntries[index + 1]->getLink(); //get address of next
		}
		return address; //if no next dir exists return given address (stay in given dir)
	}

	long getPrevDir(long address) { //returns previous possible dir address from address
		unsigned int index = getDirIndex(address);
		if ((index > 0) && (dirEntries.size() > 0) && (index != -1)) {
			return dirEntries[index - 1]->getLink();
		}
		return address;
	}

	long open() { //gives first dirEntrie link
		if (dirEntries.size() > 0) { //if dir entries exist
			return dirEntries[0]->getLink();
		}
		return 0;
	}

	//creates a subdirectory and returns its address
	long createSubdir(char n[9]) {
		long index = getFirstFree();
		if (index != -1) { //if first free exists
			long link = dirEntries[index]->getLink();//get link of dirEntry
			dirEntries[index]->createDir(n);//create dir
			getFirstFree();//actualize first free
			return link;
		}
		return -1;
	}

	//creates a UserFile and returns its address
	long createUserFile(char n[9]) {
		long index = getFirstFree();
		if (index != -1) {
			long link = dirEntries[index]->getLink();
			dirEntries[index]->createUserFile(n);
			getFirstFree();
			return link;
		}
		return -1;
	}

	//get all sub dirs
	vector<dir*> getSubDirs() {
		return dirEntries;
	}

	//deletes entry
	BOOLEAN deleteFile(long address) {
		int toDelete = getDirIndex(address); // get index
		if ((toDelete != -1) && (dirEntries[toDelete]->deleteDir(address))) {
			free = address;
			return true;
		}
		return false;
	}

	//allocate a new dir
	BOOLEAN allocate(long l) {
		dir* toInsert = new dir(l);//create dir at given address
		if (dirEntries.size() < 32) {//if maximum dir size not reached
			if (free == -1) {
				free = l; //reset free index
			}
			dirEntries.insert(dirEntries.begin(), toInsert); //insert dir
			return true;
		}
		return false;//return false if allocation wasnt possible
	}

	//release dir
	BOOLEAN deAllocate(long l) {
		if (dirEntries.size() > 0) {
			long index = getDirIndex(l);//get dir index of address
			if (index != -1) {
				dirEntries.erase(dirEntries.begin() + index);
				return true;
			}
		}
		return false;
	}
};

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // actual Instanz
WCHAR szTitle[MAX_LOADSTRING];                  // Windowtitle
WCHAR szWindowClass[MAX_LOADSTRING];            // Class of mainwindow
vector<file *> filesystem;
vector<directory *> dirSystem;
vector<userData *> dataSystem;
vector<int> opened;
unsigned long actualIndex;
unsigned long selectedIndex;
unsigned long freeSectors;
BOOLEAN fsInit = false;
BOOLEAN shift = false;
BOOLEAN success = true;
std::streambuf *coutbuf;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	NewFilesys(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	NewUserFile(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	NewUserFileData(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	NewDirectory(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	NewDirs(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DeleteDirectory(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	tooMuchFiles(HWND, UINT, WPARAM, LPARAM);




/*
 * a function for init the Filesystem
 * by Johannes Wawra
 */
void initFS() {
	std::vector<file *> rectangles(100);
	std::vector<directory *> dirs(100);
	std::vector<userData *> ud(100);
	directory* d = new directory(0);
	dirs[0] = d;
	rectangles[0] = d;
	for (unsigned int i = 1; i < rectangles.size(); i++) {
		rectangles[i] = new emptyFile();
	}
	dataSystem = ud;
	dirSystem = dirs;
	filesystem = rectangles;
	actualIndex = 0;
	selectedIndex = 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Global Strings init
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// initialize
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT1));

	MSG msg;

	// Mainloop
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: applys window
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: creates Main window
//

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Instanzenhandle in der globalen Variablen speichern

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	//Code from Johannes Wawra
	if (!fsInit) {
		initFS();
		fsInit = true;
	}

	std::ofstream out("out.txt");
	coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
	//Code Johannes Wawra End

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


/*
 * a function for graphically representing the disc
 * by Johannes Wawra
 */
void PaintRectangles(vector<file *> rectangles, int horizontal, int height, int width, int startX, int startY, HDC hdc)
{
	int length = rectangles.size();
	int vertical = length / horizontal + 1;

	for (int j = 0; j < vertical; j++) {
		for (int i = 0; i < horizontal; i++) {
			int x = width * i + startX;
			int y = height * j + startY;
			if (length > j*horizontal + i)
			{
				if ((i + j * horizontal) == selectedIndex) {
					HPEN pen = CreatePen(PS_SOLID, 1, RGB(250, 0, 0));
					SelectObject(hdc, pen);
				}
				else if (rectangles[(i + j * horizontal)]->getType() == '0') {
					HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 250, 100));
					SelectObject(hdc, pen);
				}
				else if (rectangles[(i + j * horizontal)]->getType() == 'U') {
					HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 100, 0));
					SelectObject(hdc, pen);
				}
				else
				{
					HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
					SelectObject(hdc, pen);
				}
				Rectangle(hdc, x, y,
					width*(i + 1) + startX - 5,
					height*(j + 1) + startY - 5);
			}
		}
	}


	int size = 0;
	for (int i = 0; i < length; i++) {
		size += rectangles[i]->getSize();
	}
	freeSectors = (51200 - size) / 512;
	int possibleFiles = freeSectors - freeSectors / 31;
	TCHAR freeStr[300];
	if (freeSectors < 10) {
		swprintf_s(freeStr, TEXT("Free Sectors:  %d!   Possible Files:    %d"), freeSectors, possibleFiles);
	}
	else {
		swprintf_s(freeStr, TEXT("Free Sectors:%d      Possible Files:%d"), freeSectors, possibleFiles);
	}
	TextOut(hdc, 10, 310, freeStr, _tcsclen(freeStr));
}



int next() {
	long address = -1;
	if (selectedIndex != actualIndex) {
		address = dirSystem[actualIndex]->getNextDir(selectedIndex * 512);
	}
	else {
		address = dirSystem[actualIndex]->getBack();
		if (address != -1) {
			actualIndex = address / 512;
		}
	}
	if (address != -1) {
		return address / 512;
	}
	return selectedIndex;
}

int prev() {
	long address = -1;
	if (selectedIndex != actualIndex) {
		address = dirSystem[actualIndex]->getPrevDir(selectedIndex * 512);
	}
	else {
		address = dirSystem[actualIndex]->getFrwd();
		if (address != -1) {
			actualIndex = address / 512;
		}
	}
	if (address != -1) {
		return address / 512;
	}
	return selectedIndex;
}

int open() {
	long address = dirSystem[actualIndex]->open();
	return address / 512;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: MAIN WINDOW FUNCTIONS REALIZATION
//
//  WM_COMMAND  - Menu
//  WM_PAINT    - Paint Window
//  WM_DESTROY  - Exit Application
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Menu
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_NEWFILESYSTEM:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_RESET_Q), hWnd, NewFilesys);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case ID_NEWDIR:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SET_DIR_NAME), hWnd, NewDirectory);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case ID_DIRECTORY_DELETEDIRECTORY:
			if ((selectedIndex != 0) && (filesystem[selectedIndex]->getSize() != 0)) {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DeleteDir), hWnd, DeleteDirectory);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			}
			break;
		case ID_FILESYSTEM_OPEN:
			if (selectedIndex == actualIndex) {
				selectedIndex = open();
			}
			else {
				opened.push_back(actualIndex);
				actualIndex = selectedIndex;
			}
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case ID_DIRECTORY_CLOSE:
			if (actualIndex != selectedIndex) {
				selectedIndex = actualIndex;
			}
			else if (!opened.empty()) {
				actualIndex = opened.back();
				opened.pop_back();
				selectedIndex = actualIndex;
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 'R':
			DialogBox(hInst, MAKEINTRESOURCE(IDD_RESET_Q), hWnd, NewFilesys);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;

		case 'D':
			if (!shift) {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SET_DIR_NAME), hWnd, NewDirectory);
			}
			else {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SET_DIRs), hWnd, NewDirs);
				shift = false;
			}
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;

		case 'F':
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CreateFilee), hWnd, NewUserFile);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;

		case VK_SHIFT:
			shift = true;
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;

		case VK_SPACE:
			if (selectedIndex == actualIndex) {
				selectedIndex = open();
			}
			else {
				if (filesystem[selectedIndex]->getType() == 'D') {
					opened.push_back(actualIndex);
					actualIndex = selectedIndex;
				}
			}
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;

		case VK_ESCAPE:
			if (actualIndex != selectedIndex) {
				selectedIndex = actualIndex;
			}
			else if (!opened.empty()) {
				actualIndex = opened.back();
				opened.pop_back();
				selectedIndex = actualIndex;
			}

		case VK_RIGHT:
			selectedIndex = prev();
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;

		case VK_LEFT:
			selectedIndex = next();
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;

		case VK_DELETE:
			if ((selectedIndex != 0) && (filesystem[selectedIndex]->getSize() != 0)) {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DeleteDir), hWnd, DeleteDirectory);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			}
			break;


		default:
			break;
		}
	}

	case WM_KEYUP:
		switch (wParam)
		{
		case VK_SHIFT:
			break;
		default:
			shift = false;
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;
		}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		//Code from Johannes Wawra

		int horizontal = 10;
		int height = 30;
		int width = 80;
		int xStart = 5;
		int yStart = 5;
		PaintRectangles(filesystem, horizontal, height, width, xStart, yStart, hdc);

		vector<dir*> actualDirs;
		actualDirs = dirSystem[actualIndex]->getSubDirs();
		//Paint Dir Names
		for (unsigned int i = 0; i < actualDirs.size(); i++) {
			TCHAR* name = actualDirs[i]->getTchar();
			long address = actualDirs[i]->getLink();
			int index = address / 512;
			int x = (index % horizontal) * width + xStart + 5;
			int y = (index / horizontal) * height + yStart + 5;
			TextOut(hdc, x, y, name, _tcsclen(name));
		}

		TCHAR sh[] = _T("SHIFT");
		if (shift) {
			TextOut(hdc, (actualIndex % horizontal) * width + xStart + 5,
				(actualIndex / horizontal) * height + yStart + 5, sh, _tcsclen(sh));
		}

		TCHAR fl[] = _T("File creation failed...");
		TCHAR sc[] = _T("File creation succeeded...");
		if (!success) {
			TextOut(hdc, (horizontal + 1) * width, xStart, fl, _tcsclen(fl));
		}
		else {
			TextOut(hdc, (horizontal + 1) * width, xStart, sc, _tcsclen(sc));
		}


		if (filesystem[selectedIndex]->getType() == 'U') {
			TCHAR* dataC = dataSystem[selectedIndex]->getTdata();
			int x = (horizontal + 2) * width;
			int y = 100;
			TextOut(hdc, x, y, dataC, _tcsclen(dataC));
		}



		//End of Johannes Wawras' Code

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		std::cout.rdbuf(coutbuf); //reset to standard output again

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// About-Message
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Reset Question
INT_PTR CALLBACK NewFilesys(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			initFS();
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


long getFreeAddress() {
	for (unsigned int i = 0; i < filesystem.size(); i++) {
		if (filesystem[i]->getSize() == 0) {
			return i * 512;
		}
	}
	return -1;
}

BOOLEAN allocateNextIndex() {
	long toAllocate = getFreeAddress();
	if (!dirSystem[actualIndex]->allocate(toAllocate)) {
		if (toAllocate != -1) {
			long f = dirSystem[actualIndex]->getFrwd();
			if (f = -1) {
				f = toAllocate;
				directory* d = new directory(f);
				dirSystem[f / 512] = d;
				filesystem[f / 512] = d;
				dirSystem[actualIndex]->setFrwd(f);
				dirSystem[f / 512]->setBack(actualIndex * 512);
			}
			actualIndex = f / 512;
			selectedIndex = f / 512;
			return allocateNextIndex();
		}
		return false;
	}
	return true;
}


// Set Filename and Create New Directory
INT_PTR CALLBACK NewDirectory(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			long toAllocate = getFreeAddress();
			if (!dirSystem[actualIndex]->allocate(toAllocate)) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			char name[9] = "12345678";
			GetDlgItemTextA(hDlg, IDC_DirFileName, name, 9);
			long address = dirSystem[actualIndex]->createSubdir(name);
			if (address != -1) {
				directory* d = new directory(address);
				filesystem[address / 512] = d;
				dirSystem[address / 512] = d;
			}
			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Set Filename and Create New User File
INT_PTR CALLBACK NewUserFile(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {

			wchar_t count[4];
			GetDlgItemText(hDlg, IDC_UFileSectorS, count, 4);
			unsigned int counti = _wtoi(count);

			if (counti > freeSectors) {
				success = false;
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)FALSE;
			}
			long toAllocate = getFreeAddress();
			if (!dirSystem[actualIndex]->allocate(toAllocate)) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			char name[9] = "12345678";
			GetDlgItemTextA(hDlg, IDC_DirFileName, name, 9);
			long address = dirSystem[actualIndex]->createUserFile(name);
			if (address != -1) {
				userData* u = new userData();
				filesystem[address / 512] = u;
				dataSystem[address / 512] = u;
			}

			for (unsigned int i = 1; (i < counti) && (i < filesystem.size()); i++) {
				long nextFile = getFreeAddress();
				if (nextFile != -1) {
					userData* u = new userData();
					filesystem[nextFile / 512] = u;
					dataSystem[nextFile / 512] = u;
					dataSystem[nextFile / 512]->setBack(address);
					dataSystem[address / 512]->setFrwd(nextFile);
					address = nextFile;
				}

			}


			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Set Filename and Create New User File
INT_PTR CALLBACK NewUserFileData(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {

			long toAllocate = getFreeAddress();
			if (!dirSystem[actualIndex]->allocate(toAllocate)) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			char name[9] = "12345678";
			GetDlgItemTextA(hDlg, IDC_DirFileName, name, 9);
			long address = dirSystem[actualIndex]->createUserFile(name);
			if (address != -1) {
				userData* u = new userData();
				filesystem[address / 512] = u;
				dataSystem[address / 512] = u;
			}

			wchar_t count[7];
			GetDlgItemText(hDlg, IDC_UFileChars, count, 7);
			signed int countChars = _wtoi(count);

			/*
			if (countChars / 504 + 1 > freeSectors) {
				success = false;
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)FALSE;
			}*/ //not wanted to break before filesystem is filled.

			wchar_t* data = new wchar_t(countChars + 1);
			GetDlgItemText(hDlg, IDC_UserFileData, data, countChars + 1);

			int dataIndex = 0;

			wchar_t* fileData = new wchar_t(min(504, countChars) + 2);
			for (int i = 0; i < min(504, countChars) + 1; i++) {
				fileData[i] = data[i];
			}
			dataSystem[address / 512]->addUserData(fileData);
			countChars -= 504;

			while (countChars > 0) {
				dataIndex += 504;
				long nextFile = getFreeAddress();
				if (nextFile != -1) {
					userData* u = new userData();
					filesystem[nextFile / 512] = u;
					dataSystem[nextFile / 512] = u;
					dataSystem[nextFile / 512]->setBack(address);
					dataSystem[address / 512]->setFrwd(nextFile);
					address = nextFile;

					fileData = new wchar_t(min(504, countChars) + 2);
					for (int i = 0; i < min(504, countChars) + 1; i++) {
						fileData[i] = data[i + dataIndex];
					}
					dataSystem[address / 512]->addUserData(fileData);
					countChars -= 504;
				}
				else {
					countChars = -1;
				}

			}
			countChars = 0;

			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}



BOOLEAN deleteDirectory(int index) {
	if (filesystem[index]->getType() == 'D') {
		long f = dirSystem[index]->getFrwd();
		if (f != -1) {
			deleteDirectory(f / 512);
		}
		vector<dir*> subDirs = dirSystem[index]->getSubDirs();
		for (unsigned int i = 0; i < subDirs.size(); i++) {
			long toDeleteAddress = subDirs[i]->getLink();
			if (toDeleteAddress != -1) {
				int subIndex = toDeleteAddress / 512;
				deleteDirectory(subIndex);
			}
		}
	}
	if (filesystem[index]->getType() == 'U') {
		long f = dataSystem[index]->getFrwd();
		if (f != -1) {
			deleteDirectory(f / 512);
		}
	}
	long toDeleteAddress = index * 512;
	dirSystem[actualIndex]->deleteFile(toDeleteAddress);
	dirSystem[actualIndex]->deAllocate(toDeleteAddress);
	filesystem[index] = new emptyFile();
	selectedIndex = actualIndex;
	return true;
}


INT_PTR CALLBACK DeleteDirectory(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			deleteDirectory(selectedIndex);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Set Filename and Create New Directory
INT_PTR CALLBACK NewDirs(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {


			wchar_t count[4]; //create char for filecount
			GetDlgItemText(hDlg, IDC_DirFileCount, count, 4); //read from input
			unsigned int counti = _wtoi(count); //set countVariable from inputchar

			char name[9] = "        "; //create name
			GetDlgItemTextA(hDlg, IDC_DirFileName, name, 9);//fill name from input
			for (int i = 0; i < 7; i++) {
				if (name[i] == NULL) {
					name[i] = ' '; //unsigned chars are filled with blanks
				}
			}

			if (counti + counti / 31 > freeSectors) { //if not enough sectors to hold files and dirs
				success = false; //end unsuccessfull
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)FALSE;
			}
			for (unsigned int i = 0; (i < counti) && (i < filesystem.size()); i++) { //for all new to create directories
				if (!allocateNextIndex()) { //if out of sectors end
					success = false;
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR)FALSE;
				};
				name[5] = '~'; //set name suffix
				name[7] = '0' + i % 10; //set name suffix numbering
				name[6] = '0' + i / 10;
				long address = dirSystem[actualIndex]->createSubdir(name); //create a subdirectory
				if (address != -1) { //if directory is full
					directory* d = new directory(address);//create new directory
					filesystem[address / 512] = d;
					dirSystem[address / 512] = d;
				}
			}
			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			success = true;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//Dialog for creating too much files
INT_PTR CALLBACK tooMuchFiles(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	//	HWND hwndText = GetDlgItem(hDlg, IDC_toMuchFiles);
	switch (message)
	{
	case WM_INITDIALOG:
		/*		TCHAR tooMuchFilesText[300];
				swprintf_s(tooMuchFilesText, TEXT("You can create maximum %d Files!"), (freeSectors - freeSectors / 31));
				SetWindowText(hwndText, tooMuchFilesText);
				InvalidateRect(hwndText, NULL, true); */
		return (INT_PTR)TRUE;
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}