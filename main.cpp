/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <dedkam2@wp.pl> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

#include <windows.h>
#include <iostream>
#include <commctrl.h>
#include <fstream>
#include <time.h>
#include <tchar.h>
#include <cstdio>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <Wininet.h>
#include <Shlobj.h>
#include <iomanip>
#include <sstream>

#define START 1
#define IDC_PICKPATH 2
#define IDC_SEEKFF 3
#define IDC_CONVERT 4
#define STARTAPP 5
using namespace std;

HANDLE worker;
HWND hwnd, hProgressBar, infoPanel, hWndListBox, PATHBUTTON, FORMATLIST, CONVERT;

BROWSEINFO ofn;
TCHAR szFile[260] = { 0 };
vector <string> tmp, res, res2, formats = {"mp4", "webm", "mkv", "flv", "vob", "ogg", "ogv", "avi", "wmv", "mpg", "mpeg", "ogv", "3gp", "mpv", "m4v", "mov"};
TCHAR path[MAX_PATH];

static const char alphanum[] =
"0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

class MyCallback : public IBindStatusCallback
{
public:
    MyCallback() {}
    ~MyCallback() {}
    STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText)
    {
        if(ulStatusCode != 5)
        {
        	SetWindowText(infoPanel, "Downloading main convert API...");
            return S_OK;
        }
        else
        {
        	int percent;
        	double max = 50675;
        	double prog = ulProgress/1024;
        	percent = prog/max*100;
        	SendMessage(hProgressBar, PBM_SETPOS, (WPARAM) percent, 0);
            return S_OK;
        }
    }
    STDMETHOD(OnStartBinding)( DWORD dwReserved,  IBinding __RPC_FAR *pib)
    { return E_NOTIMPL; }
    STDMETHOD(GetPriority)( LONG __RPC_FAR *pnPriority)
    { return E_NOTIMPL; }
    STDMETHOD(OnLowResource)( DWORD reserved)
    { return E_NOTIMPL; }
    STDMETHOD(OnStopBinding)( HRESULT hresult, LPCWSTR szError)
    { return E_NOTIMPL; }
    STDMETHOD(GetBindInfo)( DWORD __RPC_FAR *grfBINDF, BINDINFO __RPC_FAR *pbindinfo)
    { return E_NOTIMPL; }
    STDMETHOD(OnDataAvailable)( DWORD grfBSCF,  DWORD dwSize,  FORMATETC __RPC_FAR *pformatetc,  STGMEDIUM __RPC_FAR *pstgmed)
    { return E_NOTIMPL; }
    STDMETHOD(OnObjectAvailable)( REFIID riid, IUnknown __RPC_FAR *punk)
    { return E_NOTIMPL; }
    STDMETHOD_(ULONG,AddRef)()
    { return 0; }
    STDMETHOD_(ULONG,Release)()
    { return 0; }
    STDMETHOD(QueryInterface)( REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
    { return E_NOTIMPL; }
};
MyCallback pCallback;

string genRandom() {
	string out = "";
	int n = 15, stringLength = sizeof(alphanum) - 1;
	while (n--) out += alphanum[rand() % stringLength];
	return out;
}

void convertVisible(string s, string p)
{
	EnableWindow(CONVERT, FALSE);
	if (res.size() < 1) {
		SetWindowText(infoPanel, "No files for conversion selected.");
		return;
	}
	string output = genRandom(), XD;
	for (int i = 0; i < res.size(); i++)
	{
		if (res2[i] != s) {
			XD = "Converting: " + res[i];
			SetWindowText(infoPanel, XD.c_str());
			string eh = "/C MKDIR " + output;
			ShellExecute(NULL, "open", "cmd.exe", eh.c_str(), NULL, SW_HIDE);
			string random = genRandom(), command = "/C HandBrakeCLI.exe -i \""+ p + "/" + res[i] + "\" -o \".\\" + output + "\\" + random + "." + s + "\"";
			
			SHELLEXECUTEINFO ShExecInfo = {0};
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
			ShExecInfo.hwnd = NULL;
			ShExecInfo.lpVerb = NULL;
			ShExecInfo.lpFile = "cmd.exe";        
			ShExecInfo.lpParameters = command.c_str();   
			ShExecInfo.lpDirectory = NULL;
			ShExecInfo.nShow = SW_HIDE;
			ShExecInfo.hInstApp = NULL;
			ShellExecuteEx(&ShExecInfo);
			WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
			CloseHandle(ShExecInfo.hProcess);
		} else {
			XD = "Skipped: " + res[i];
			SetWindowText(infoPanel, XD.c_str());
			string command = "/C COPY \"" + p + "\\" + res[i] + "\" \".\\" + output + "\\\"";
			ShellExecute(NULL, "open", "cmd.exe", command.c_str(), NULL, SW_HIDE);
		}
	}
	XD = "Done, output: " + output;
	SetWindowText(infoPanel, XD.c_str());
	XD = ".\\" + output;
	ShellExecute(NULL, "open", XD.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

ifstream::pos_type filesize(const char* filename)
{
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg(); 
}

string trim(string str) {
    int numEndSpaces = 1;
    for (int i = str.length() - 1; i >= 0; i--) {
        if (!isspace(str[i])) break;
        numEndSpaces++;
    }
    return str.substr(0, str.length() - numEndSpaces);
}

void seekForFiles(string y)
{
	if (y == "") {
		SetWindowText(infoPanel, "Select Folder first.");
		return;
	}
	SendMessage(hWndListBox, LB_RESETCONTENT, 0, 0);
	tmp.clear();
	res.clear();
	res2.clear();
	string cmd = "/C DIR \"" + y + "\" /A:-D /B > sdklfsdlifjlsaeilrl.txt";
	ShellExecute(NULL, "open", "cmd.exe", cmd.c_str(), NULL, SW_HIDE);
	SetWindowText(infoPanel, "Checking...");
	Sleep(3000);
	fstream x;
	x.open("sdklfsdlifjlsaeilrl.txt", ios::in);
	string in;
	while (!x.eof() && getline(x, in)) tmp.push_back(in);
	x.close();
	for (int i = 0; i < tmp.size(); i++) {
		string format, tmpf;
		format = "";
		tmpf = tmp[i];
		for (int j = tmpf.length(); j>0; j--) {
			if (tmpf[j] == '.') break;
			else format += tolower(tmpf[j]);
		}
		reverse(format.begin(), format.end());
		format = trim(format);
		if (find(formats.begin(), formats.end(), format) != formats.end()) {
			res.push_back(tmp[i]);
			res2.push_back(format);
		}
	}
	ShellExecute(NULL, "open", "cmd.exe", "/C DEL /Q sdklfsdlifjlsaeilrl.txt", NULL, SW_HIDE);
	if (res.size() == 0) {
		SetWindowText(infoPanel, "There wasn't any files found.");
		return;
	} else {
		string x = "Found: " + to_string(res.size()) + " files for possible conversion.";
		SetWindowText(infoPanel, x.c_str());
		for (int i = 0; i < res.size(); i++) {
			string file = y + "\\" + res[i];
			double fsize = filesize(file.c_str());
			double size = fsize/1024000;
			stringstream streamM;
			streamM << fixed << setprecision(2) << size;
			string s = streamM.str();
			string out = res[i] + " - " + s + "MB";
			SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)out.c_str());
		}
	}
	EnableWindow(CONVERT, TRUE);
}

void runApp() {
	SendMessage(hProgressBar, WM_CLOSE, 0, 0);
	SendMessage(infoPanel, WM_CLOSE, 0, 0);
	SetWindowPos(hwnd, 0, 0, 0, 400, 297, SWP_NOMOVE);
	SetTimer(hwnd, STARTAPP, 1, NULL);
	return;
}

void WriteAppDown() {
	PATHBUTTON = CreateWindow(
		"STATIC",
		"Click there to select folder.",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOTIFY | SS_SUNKEN | SS_ENDELLIPSIS,
		5, 5, 380, 20,
		hwnd, (HMENU)IDC_PICKPATH, NULL, NULL);
	hWndListBox = CreateWindow(
		"LISTBOX",
		NULL,
        WS_VISIBLE | WS_CHILD | LBS_NOSEL | WS_BORDER | WS_VSCROLL,
		5, 30, 380, 180,
		hwnd,NULL,NULL,NULL);
	infoPanel = CreateWindow(
		"STATIC",
		"- - -",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_SUNKEN | SS_CENTER,
		5, 211, 380, 20,
		hwnd, NULL, NULL, NULL);
	CreateWindow(
		"BUTTON",
		"Seek for Files",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_SUNKEN | BS_CENTER,
		5, 234, 125, 25,
		hwnd, (HMENU)IDC_SEEKFF, NULL, NULL);
	CONVERT = CreateWindow(
		"BUTTON",
		"Convert",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_SUNKEN | BS_CENTER,
		132, 234, 125, 25,
		hwnd, (HMENU)IDC_CONVERT, NULL, NULL);
	FORMATLIST = CreateWindow(
		"COMBOBOX",
		"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST | WS_VSCROLL,
		259, 234, 125, 100,
		hwnd, NULL, NULL, NULL);
	for (int i = 0; i < formats.size(); i++) {
		SendMessage(FORMATLIST, CB_ADDSTRING, 0, (LPARAM)TEXT(formats[i].c_str()));
	}
	KillTimer(hwnd, STARTAPP);
	return;
}

void WORKER(){
	InitCommonControls();
	SetWindowText(infoPanel, "Downloading main convert API...");
	while(true) {
		if (URLDownloadToFile(NULL, "https://www35.zippyshare.com/d/Vist2hvH/47642/HandBrakeCLI.exe", "HandBrakeCLI.exe", 0, &pCallback) != S_OK) {
			SetWindowText(infoPanel, "Download Failed! Retrying...");
			Sleep(1000);
		} else {
			SetWindowText(infoPanel, "Download Succed!");
			Sleep(1000);
			break;
		}
	}
	runApp();
	TerminateProcess(worker, WM_CLOSE);
	return;
}

void startDownloadProcedure() {
	hProgressBar = CreateWindowEx(0,PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
		0, 18, 290, 15, hwnd, NULL, NULL, NULL);
	infoPanel = CreateWindowEx(0,"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		0, 0, 290, 18, hwnd, NULL, NULL, NULL);
	worker = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WORKER, NULL, 0, NULL);
	return;
}

void startUp() {
	if (filesize("HandBrakeCLI.exe") < 51891200) {
		startDownloadProcedure();
	} else {
		runApp();
	}
	KillTimer(hwnd, START);
	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {

		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		
		case WM_CREATE: {
			SetTimer(hwnd, START, 1, NULL);
			break;
		}
		
		case WM_TIMER: {
			switch (wParam) 
			{
				case START: {
					startUp();
					break;
				}
				case STARTAPP: {
					WriteAppDown();
					EnableWindow(CONVERT, FALSE);
					break;
				}
			}
			break;
		}
		
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_PICKPATH:
				{
					EnableWindow(CONVERT, FALSE);
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.hwndOwner = hwnd;
					ofn.lpfn = NULL;
					ofn.pidlRoot = NULL;
					ofn.lpszTitle = "Select Folder to seek for files.";
					ofn.lParam = 0;
					ofn.iImage = -1;
					ofn.ulFlags = OFN_HIDEREADONLY | OFN_NOVALIDATE | OFN_PATHMUSTEXIST | BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
					LPITEMIDLIST lpItem = SHBrowseForFolder(&ofn);
					SHGetPathFromIDList(lpItem, path);
					if ((string)(&path[0]) == "") {
						SetWindowText(PATHBUTTON, "Click there to select folder.");
					} else SetWindowText(PATHBUTTON, path);
					break;
				}
				case IDC_SEEKFF: {
					seekForFiles((string)(&path[0]));
					break;
				}
				case IDC_CONVERT: {
					TCHAR format[3] = {0};
					int idx_row = SendMessage(FORMATLIST, CB_GETCURSEL, 0, 0 );
					SendMessage(FORMATLIST, CB_GETLBTEXT, idx_row, (LPARAM)format);
					if ((string)(&format[0]) == "") {
						SetWindowText(infoPanel, "Select Format.");
						break;
					} else if ((string)(&path[0]) == "") {
						SetWindowText(infoPanel, "Select Folder.");
						break;
					}
					convertVisible((string)(&format[0]), (string)(&path[0]));
					break;
				}
			}
			break;
		}

		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = {0};	
	MSG msg;
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);

	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+3);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	int xPos = GetSystemMetrics(SM_CXSCREEN)/2-200;
	int yPos = GetSystemMetrics(SM_CYSCREEN)/2-50;

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"WindowClass",
		"Format Vids Tool",
		WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WM_TIMER,
		xPos,
		yPos,
		300,
		66,
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
