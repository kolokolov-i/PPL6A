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
	int readerId = rand() % 100;
	int sleepT = 0;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	ChannelEstaf* channel = new ChannelEstaf(L"General");
	cout << "Reader #" << readerId << " started" << endl;
	bool flag = true;
	while (flag) {
		sleepT = rand() % 500;
		Sleep(sleepT);
		Message* message = channel->read();
		cout << "Reader #" << readerId << ": @" << message->code << " -> " << message->data << endl;
		delete message;
	}
}
