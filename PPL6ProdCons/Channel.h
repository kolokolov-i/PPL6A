#pragma once

#include "Message.h"

#include <Windows.h>
#include <string>
#include <iostream>

class Channel
{
private:
	HANDLE semFree;
	HANDLE semEmpty;
	HANDLE fileMem;
	void* buffer;
public:
	void put(Message* data) {
		WaitForSingleObject(semFree, INFINITE);
		data->writeTo(buffer);
		ReleaseSemaphore(semEmpty, 1, NULL);
	}
	bool putT(Message* data, int timeout) {
		DWORD r = WaitForSingleObject(semFree, timeout);
		if (r == WAIT_TIMEOUT) {
			return false;
		}
		data->writeTo(buffer);
		ReleaseSemaphore(semEmpty, 1, NULL);
	}
	bool canPut() {
		DWORD r = WaitForSingleObject(semFree, 0);
		return r != WAIT_TIMEOUT;
	}
	Message* get() {
		Message* pMessage;
		WaitForSingleObject(semEmpty, INFINITE);
		pMessage = new Message(buffer);
		ReleaseSemaphore(semFree, 1, NULL);
		return pMessage;
	}
	Message* get(int timeout) {
		Message* pMessage;
		DWORD r = WaitForSingleObject(semEmpty, timeout);
		if (r == WAIT_TIMEOUT) {
			return nullptr;
		}
		pMessage = new Message(buffer);
		ReleaseSemaphore(semFree, 1, NULL);
		return pMessage;
	}
	Channel(std::wstring name) {
		std::wstring chName = L"ch" + name;
		std::wstring semFName = chName + L"SemF";
		std::wstring semEName = chName + L"SemE";
		std::wstring fileName = chName + L"File";
		semFree = OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, (LPCWSTR)semFName.c_str());
		if (semFree == NULL) {
			semFree = CreateSemaphore(NULL, 1, 1, (LPCWSTR)semFName.c_str());
		}
		semEmpty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, (LPCWSTR)semEName.c_str());
		if (semEmpty == NULL) {
			semEmpty = CreateSemaphore(NULL, 0, 1, (LPCWSTR)semEName.c_str());
		}
		fileMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, fileName.c_str());
		if (fileMem == NULL) {
			fileMem = CreateFileMapping(
				INVALID_HANDLE_VALUE,
				NULL,
				PAGE_READWRITE,
				0, 4096,
				(LPCWSTR)fileName.c_str());
		}
		if (fileMem != NULL) {
			buffer = MapViewOfFile(fileMem, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
		}
		else {
			DWORD debugCode = GetLastError();
			std::cerr << "error: FILE_MAP" << std::endl;
			buffer = nullptr;
		}
	}
	~Channel() {
		CloseHandle(semFree);
		CloseHandle(semEmpty);
		CloseHandle(fileMem);
	}
};

