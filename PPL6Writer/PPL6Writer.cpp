#include "ChannelEstaf.h"
#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

string generateMessage();

int main()
{
	srand(time(NULL));
	int writerId = rand() % 100;
	int sleepT = 0;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	ChannelEstaf* channel = new ChannelEstaf(L"General");
	cout << "Writer #" << writerId << " started" << endl;
	bool flag = true;
	while (flag) {
		sleepT = rand() % 500;
		Sleep(sleepT);
		Message* message = new Message(Code::None, Code::WRITER, generateMessage());
		channel->write(message);
		cout << "Writer #" << writerId << ": " << message->data << endl;
		delete message;
	}
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