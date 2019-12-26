#pragma once

#include "Message.h"
#include "Code.h"
#include <Windows.h>

class ChannelEstaf
{
private:
	HANDLE semE;
	HANDLE semR;
	HANDLE semW;
	HANDLE semP;
	HANDLE fileMem;
	void* buffer;
	int numR, numW, waitR, waitW;

public:
	ChannelEstaf(std::wstring name) {
		numR = numW = waitR = waitW = 0;
		std::wstring chName = L"ch" + name;
		std::wstring semEName = chName + L"SemE";
		std::wstring semRName = chName + L"SemR";
		std::wstring semWName = chName + L"SemW";
		std::wstring semPName = chName + L"SemP";
		std::wstring fileName = chName + L"File";
		semE = OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, (LPCWSTR)semEName.c_str());
		if (semE == NULL) {
			semE = CreateSemaphore(NULL, 1, 1, (LPCWSTR)semEName.c_str());
		}
		semR = OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, (LPCWSTR)semRName.c_str());
		if (semR == NULL) {
			semR = CreateSemaphore(NULL, 0, 1, (LPCWSTR)semRName.c_str());
		}
		semW = OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, (LPCWSTR)semWName.c_str());
		if (semW == NULL) {
			semW = CreateSemaphore(NULL, 0, 1, (LPCWSTR)semWName.c_str());
		}
		semP = OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, (LPCWSTR)semPName.c_str());
		if (semP == NULL) {
			semP = CreateSemaphore(NULL, 1, 1, (LPCWSTR)semPName.c_str());
		}
		bool creating = false;
		fileMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, fileName.c_str());
		if (fileMem == NULL) {
			fileMem = CreateFileMapping(
				INVALID_HANDLE_VALUE,
				NULL,
				PAGE_READWRITE,
				0, 4096,
				(LPCWSTR)fileName.c_str());
			creating = true;
		}
		if (fileMem != NULL) {
			buffer = MapViewOfFile(fileMem, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
		}
		else {
			DWORD debugCode = GetLastError();
			//std::cerr << "error: FILE_MAP" << std::endl;
			buffer = nullptr;
		}
		if (creating) {
			writeState();
			char* p = reinterpret_cast<char*>(buffer);
			Message* message = new Message(Code::None, Code::WRITER, "__________");
			message->writeTo(p + 20);
			delete message;
		}
		else {
			readState();
		}
	}
	~ChannelEstaf() {
		CloseHandle(semE);
		CloseHandle(semR);
		CloseHandle(semW);
		CloseHandle(semP);
		CloseHandle(fileMem);
	}

	void write(Message* message) {
		WaitForSingleObject(semE, INFINITE);
		readState();
		if (numR > 0 || numW > 0) {
			waitR++;
			writeState();
			ReleaseSemaphore(semE, 1, NULL);
			WaitForSingleObject(semW, INFINITE);
		}
		readState();
		numW++;
		//writeState();
		estafeta();
		char* p = reinterpret_cast<char*>(buffer);
		message->writeTo(p + 20);
		WaitForSingleObject(semE, INFINITE);
		readState();
		numW--;
		estafeta();
	}

	Message* read() {
		Message* message;
		WaitForSingleObject(semE, INFINITE);
		readState();
		if (numW > 0) {
			waitR++;
			writeState();
			ReleaseSemaphore(semE, 1, NULL);
			WaitForSingleObject(semR, INFINITE);
		}
		readState();
		numR++;
		//writeState();
		estafeta();
		char* p = reinterpret_cast<char*>(buffer);
		message = new Message(p + 20);
		WaitForSingleObject(semE, INFINITE);
		readState();
		numR--;
		//writeState();
		estafeta();
		return message;
	}

private:
	void estafeta() {
		readState();
		if ((numR == 0) && (numW == 0) && (waitW > 0)) {
			waitW--;
			writeState();
			ReleaseSemaphore(semW, 1, NULL);
		}
		else if ((numW == 0) && (waitW == 0) && (waitR > 0))
		{
			waitR--;
			writeState();
			ReleaseSemaphore(semR, 1, NULL);
		}
		else {
			writeState();
			ReleaseSemaphore(semE, 1, NULL);
		}
	}

	void readState() {
		WaitForSingleObject(semP, INFINITE);
		char* p = reinterpret_cast<char*>(buffer);
		memcpy(&numR, p + 0, 4);
		memcpy(&numW, p + 4, 4);
		memcpy(&waitR, p + 8, 4);
		memcpy(&waitW, p + 12, 4);
		ReleaseSemaphore(semP, 1, NULL);
	}

	void writeState() {
		WaitForSingleObject(semP, INFINITE);
		char* p = reinterpret_cast<char*>(buffer);
		memcpy(p + 0, &numR, 4);
		memcpy(p + 4, &numW, 4);
		memcpy(p + 8, &waitR, 4);
		memcpy(p + 12, &waitW, 4);
		ReleaseSemaphore(semP, 1, NULL);
	}
};

