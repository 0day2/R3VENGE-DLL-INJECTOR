#include<iostream>
#include<windows.h>

using namespace std;

int InjectDLL(DWORD, char*);
int getDLLpath(char*);
int getPID(int*);
int getProc(HANDLE*, DWORD);

int getDLLpatch(char* dll)
{
	cout << "Please enter the patch to your DLL file\n";
	cin >> dll;
	return 1;
}

int getPID(int* PID)
{
	cout << "Please enter the PID (Process ID) to your target process\n";
	cin >> *PID;
	return 1;
}

int getProc(HANDLE* handleToProc, DWORD PID)
{
	*handleToProc = OpenProcess(PROCESS_ALL_ACCESS, false, PID);
	DWORD dwLastError = GetLastError();
	if (*handleToProc == NULL)
	{
		cout << "Unable to open process.\n";
		return -1;
	}
	else
	{
		cout << "Process opened\n";
		return 1;
	}
}

int InjectDLL(DWORD PID,char*dll)
{
	HANDLE handleToProc;
	LPVOID LoadLibAddr;
	LPVOID baseAddr;
	HANDLE remThread;

	int dllLenght = strlen(dll) + 1;

	if(getProc(&handleToProc,PID)<0)
		return -1;

	LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	if (!LoadLibAddr)
		return -1;

	baseAddr = VirtualAllocEx(handleToProc, NULL, dllLenght, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!baseAddr)
		return -1;

	if (!WriteProcessMemory(handleToProc, baseAddr, dll, dllLenght, NULL))
		return -1;

	remThread = CreateRemoteThread(handleToProc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, baseAddr, 0, NULL);

	if (!remThread)
		return -1;

	WaitForSingleObject(remThread, INFINITE);

	VirtualFreeEx(handleToProc, baseAddr, dllLenght, MEM_RELEASE);

	if (CloseHandle(remThread) == 0)
	{
		cout << "Failed to close handle to remote thread";
		return -1;
	}
	if (CloseHandle(handleToProc) == 0)
	{
		cout << "Failed to close handle to target process\n";
		return -1;
	}
}

int main()
{
	SetConsoleTitle("R3VENGE Injector");

	int PID = -1;
	char* dll = new char[255];

	getDLLpatch(dll);
	getPID(&PID);

	InjectDLL(PID, dll);
	system("pause");

	return 0;
}