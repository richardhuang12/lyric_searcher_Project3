#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <locale>
#include <codecvt>
#include <comdef.h>
#include <filesystem>
#include <mmeapi.h>

namespace fs = std::filesystem;
using namespace std;

struct SongInfo {
	std::string song; // song
	std::string artist; // singer
	std::string year; // year
	std::string lyric; // lyric
};

string wstring2string(wstring wstr)
{
	std::string result;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	if (len <= 0)return result;
	char* buffer = new char[len + 1];
	if (buffer == NULL)return result;
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}
wstring string2wstring(string str)
{
	std::wstring result;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if (len < 0)return result;
	wchar_t* buffer = new wchar_t[len + 1];
	if (buffer == NULL)return result;
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}

std::wstring replaceRNRN(const std::wstring& input) {
	std::wstring result = input;
	size_t pos = 0;
	while ((pos = result.find(L"\\r\\n", pos)) != std::wstring::npos) {
		result.replace(pos, 5, L"\r\n");  //move cursor back 
		pos += 2; 
	}
	return result;
}

std::vector<std::wstring> splitWStringByComma(const std::wstring& input) {
	std::wstringstream ss(input);  // formatting data values with commas
	std::wstring token ;
	std::vector<std::wstring> result;

	while (std::getline(ss, token, L',')) {
		std::wstring wstr = replaceRNRN(token);
		result.push_back(wstr);
	}

	return result;
}

std::vector<SongInfo> LoadSongInfo(const std::wstring& folderPath) {
	std::vector<SongInfo> songVect;

	//reads "lyrics.csv" to create a SongInfo object with lyrics, artist, and title
	fs::path path(folderPath);

	for (const auto& entry : fs::directory_iterator(folderPath)) {
		if (entry.is_regular_file() && entry.path().filename() == "lyrics.csv") {
			std::wifstream file(entry.path(), std::ios::in | std::ios::binary); 
			if (!file.is_open()) {
				std::cerr << ": " << entry.path() << std::endl;
				continue;
			}

		
			file.imbue(std::locale("en_US.UTF-8"));

			std::wstring wline;
			bool songNameFound = false, artistFound = false;

			while (std::getline(file, wline)) {
				std::vector<std::wstring> temp = splitWStringByComma(wline);
				if (temp.size() >= 4)
				{
					std::string song = wstring2string(temp[0]);
					std::string artist = wstring2string(temp[1]);
					std::string year = wstring2string(temp[2]);
					std::string lyric = wstring2string(temp[3]);
					SongInfo songInfo = { song,artist,year,lyric };
					songVect.push_back(songInfo);
				}
			}
			return songVect;
		}
	}

	return songVect;
}


std::string align_string(const std::string& str, size_t width, bool left_align = true) {
	std::string result = str;
	if (result.length() < width) {
		size_t pad_length = width - result.length();
		if (left_align) {
			result.append(pad_length, ' ');  //add extra spaces to align when displaying 
		}
		else {
			result.insert(0, pad_length, ' ');  
		}
	}
	return result;
}

/*
Constructing a Partial Matching Table (Next Array)
*/
std::vector<int> buildKMPTable(const std::string& pattern) {
	int m = pattern.size();
	std::vector<int> next(m, 0);  
	int j = 0;  
	for (int i = 1; i < m; ++i) {
		while (j > 0 && pattern[i] != pattern[j]) {
			j = next[j - 1]; 
		}
		if (pattern[i] == pattern[j]) {
			j++; 
		}
		next[i] = j;  
	}
	return next;
}

// KMP str match
int KMPSearch(const std::string& text, const std::string& pattern) {
	int n = text.size();
	int m = pattern.size();
	if (m == 0) return 0;  

	std::vector<int> next = buildKMPTable(pattern);
	int j = 0;  

	for (int i = 0; i < n; ++i) {
		while (j > 0 && text[i] != pattern[j]) {
			j = next[j - 1];
		}
		if (text[i] == pattern[j]) {
			j++;
		}
		if (j == m) {
			return i - m + 1;  
		}
	}
	return -1;  
}