#pragma once

class Code {
public:
	static const int
		Client = 1,
		Developer = 2,
		Manager = 3,
		Server = 4,
		Machine = 5;

	static const int
		CODE_TIMEOUT = -1,
		STATE_NEW = 1,
		STATE_ACCEPT = 2,
		STATE_REJECT = 3,
		STATE_DEVELOPED = 4,
		STATE_SUCCESS = 5,
		REQ_CLAIM = 6,
		REQ_GET_RESULT = 7,
		MACHINE_OFF = 8;
};