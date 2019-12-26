#pragma once

#include <string>
#include <cstring>

class Message
{
public:
	int sender;
	int code;
	std::string data;

	Message(int pSender, int pCode, std::string pData) {
		sender = pSender;
		code = pCode;
		data = pData;
	}
	Message(void* buffer) {
		char* p = reinterpret_cast<char*>(buffer);
		std::memcpy(&sender, p, sizeof(int));
		p += sizeof(int);
		std::memcpy(&code, p, sizeof(int));
		p += sizeof(int);
		int len;
		std::memcpy(&len, p, sizeof(int));
		p += sizeof(int);
		char* adata = new char[len];
		std::memcpy(adata, p, len);
		data = std::string(adata);
	}
	~Message() {}
	void writeTo(void* buffer) {
		char* p = reinterpret_cast<char*>(buffer);
		memcpy(p, &sender, sizeof(int)); p += sizeof(int);
		memcpy(p, &code, sizeof(int)); p += sizeof(int);
		int len = data.length() + 1;
		memcpy(p, &len, sizeof(int)); p += sizeof(int);
		const char* adata = data.c_str();
		memcpy(p, adata, len);
	}
};

