#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 10
#define PRODUCER_SLEEP_TIME_MS 500
#define CONSUMER_SLEEP_TIME_MS 2000

LONG Buffer[BUFFER_SIZE];
LONG LastItemProduced;
ULONG QueueSize;
ULONG QueueStartOffset;

ULONG TotalItemsProduced;
ULONG TotalItemsConsumed;

CONDITION_VARIABLE BufferNotEmpty;
CONDITION_VARIABLE BufferNotFull;
CRITICAL_SECTION   BufferLock;

BOOL StopRequested;

DWORD WINAPI ProducerThreadProc(PVOID p)
{
	ULONG ProducerId = (ULONG)(ULONG_PTR)p;

	while (true)
	{
		Sleep(rand() % PRODUCER_SLEEP_TIME_MS);
		ULONG Item = InterlockedIncrement(&LastItemProduced);
		EnterCriticalSection(&BufferLock);
		while (QueueSize == BUFFER_SIZE && StopRequested == FALSE)
		{
			SleepConditionVariableCS(&BufferNotFull, &BufferLock, INFINITE);
		}
		if (StopRequested == TRUE)
		{
			LeaveCriticalSection(&BufferLock);
			break;
		}
		Buffer[(QueueStartOffset + QueueSize) % BUFFER_SIZE] = Item;
		QueueSize++;
		TotalItemsProduced++;
		printf("Producer %u: item %2d, queue size %2u\r\n", ProducerId, Item, QueueSize);
		LeaveCriticalSection(&BufferLock);
		WakeConditionVariable(&BufferNotEmpty);
	}
	printf("Producer %u exiting\r\n", ProducerId);
	return 0;
}

DWORD WINAPI ConsumerThreadProc(PVOID p)
{
	ULONG ConsumerId = (ULONG)(ULONG_PTR)p;
	while (true)
	{
		EnterCriticalSection(&BufferLock);
		while (QueueSize == 0 && StopRequested == FALSE)
		{
			SleepConditionVariableCS(&BufferNotEmpty, &BufferLock, INFINITE);
		}
		if (StopRequested == TRUE && QueueSize == 0)
		{
			LeaveCriticalSection(&BufferLock);
			break;
		}
		LONG Item = Buffer[QueueStartOffset];
		QueueSize--;
		QueueStartOffset++;
		TotalItemsConsumed++;
		if (QueueStartOffset == BUFFER_SIZE)
		{
			QueueStartOffset = 0;
		}
		printf("Consumer %u: item %2d, queue size %2u\r\n",
			ConsumerId, Item, QueueSize);
		LeaveCriticalSection(&BufferLock);
		WakeConditionVariable(&BufferNotFull);
		Sleep(rand() % CONSUMER_SLEEP_TIME_MS);
	}
	printf("Consumer %u exiting\r\n", ConsumerId);
	return 0;
}

int main(void)
{
	InitializeConditionVariable(&BufferNotEmpty);
	InitializeConditionVariable(&BufferNotFull);
	InitializeCriticalSection(&BufferLock);
	DWORD id;
	HANDLE hProducer1 = CreateThread(NULL, 0, ProducerThreadProc, (PVOID)1, 0, &id);
	HANDLE hConsumer1 = CreateThread(NULL, 0, ConsumerThreadProc, (PVOID)1, 0, &id);
	HANDLE hConsumer2 = CreateThread(NULL, 0, ConsumerThreadProc, (PVOID)2, 0, &id);
	puts("Press enter to stop...");
	getchar();
	EnterCriticalSection(&BufferLock);
	StopRequested = TRUE;
	LeaveCriticalSection(&BufferLock);
	WakeAllConditionVariable(&BufferNotFull);
	WakeAllConditionVariable(&BufferNotEmpty);
	WaitForSingleObject(hProducer1, INFINITE);
	WaitForSingleObject(hConsumer1, INFINITE);
	WaitForSingleObject(hConsumer2, INFINITE);
	printf("TotalItemsProduced: %u, TotalItemsConsumed: %u\r\n",
		TotalItemsProduced, TotalItemsConsumed);
	return 0;
}