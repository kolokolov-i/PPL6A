#include "Channel.h"
#include "Code.h"
#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

CONDITION_VARIABLE bufferNotEmpty;
CONDITION_VARIABLE bufferNotFull;
CRITICAL_SECTION bufferLock;

DWORD WINAPI ProducerThreadProc(PVOID p);
DWORD WINAPI ConsumerThreadProc(PVOID p);
string generateMessage();

int main()
{
	InitializeConditionVariable(&bufferNotEmpty);
	InitializeConditionVariable(&bufferNotFull);
	InitializeCriticalSection(&bufferLock);
	Channel* exitChannel = new Channel(L"Exit");
	Channel* generalChannel = new Channel(L"General");
	int prodCount = 2;
	int consCount = 5;
	HANDLE* threads = new HANDLE[prodCount + consCount];
	int i = 0;
	for (; i < prodCount; i++) {
		threads[i] = CreateThread(NULL, 0, ProducerThreadProc, NULL, 0, NULL);
	}
	for (int j = 0; j < consCount; i++, j++) {
		threads[i] = CreateThread(NULL, 0, ConsumerThreadProc, NULL, 0, NULL);
	}
	Sleep(10000);
	exitChannel->put(new Message(Code::Manager, Code::MACHINE_OFF, ""));
	for (int i = 0; i < prodCount + consCount; i++) {
		WaitForSingleObject(threads[i], INFINITE);
	}
	delete[] threads;
	delete exitChannel;
	delete generalChannel;
	DeleteCriticalSection(&bufferLock);
}


DWORD WINAPI ProducerThreadProc(PVOID p) {
	Channel* exitChannel = new Channel(L"Exit");
	Channel* generalChannel = new Channel(L"General");
	srand(time(NULL));
	int producerId = rand() % 100;
	bool flag = true;
	while (flag) {
		Message* msg = exitChannel->get(20);
		if (msg != nullptr) {
			if (msg->code == Code::MACHINE_OFF) {
				flag = false;
				continue;
			}
		}
		Sleep(rand()%100+50);
		EnterCriticalSection(&bufferLock);
		SleepConditionVariableCS(&bufferNotFull, &bufferLock, INFINITE);
		Message* message = new Message(Code::Machine, Code::STATE_SUCCESS, generateMessage());
		generalChannel->put(message);
		cout << "producer #" << producerId << " -> " << message->data << endl;
		LeaveCriticalSection(&bufferLock);
		WakeConditionVariable(&bufferNotEmpty);
	}
	delete exitChannel;
	delete generalChannel;
	return 0;
}

DWORD WINAPI ConsumerThreadProc(PVOID p) {
	Channel* exitChannel = new Channel(L"Exit");
	Channel* generalChannel = new Channel(L"General");
	srand(time(NULL));
	int consumerId = rand() % 100;
	//cout << "consumer #" << consumerId << " started" << endl;
	bool flag = true;
	while (flag) {
		Message* msg = exitChannel->get(20);
		if (msg != nullptr) {
			if (msg->code == Code::MACHINE_OFF) {
				flag = false;
				continue;
			}
		}
		Sleep(rand() % 100 + 50);
		EnterCriticalSection(&bufferLock);
		SleepConditionVariableCS(&bufferNotEmpty, &bufferLock, INFINITE);
		Message* message = generalChannel->get();
		cout << "consumer #" << consumerId << " <- " << message->data << endl;
		LeaveCriticalSection(&bufferLock);
		WakeConditionVariable(&bufferNotFull);
	}
	delete exitChannel;
	delete generalChannel;
	return 0;
}

string generateMessage() {
	char c = 'a' + (rand() % 26);
	int len = 10;
	char* adata = new char[len + 1];
	for (int j = 0; j < len; j++) {
		adata[j] = c;
	}
	adata[len] = 0;
	return string(adata);
}