
#ifndef FILE_OP_H

#include <windows.h>

int Init();
void ShowMessage(char* str);

__declspec(dllexport)
int ReadAt(int fd, void* buf, int len, __int64 offset);

__declspec(dllexport)
int WriteAt(int fd, void* buf, int len, __int64 offset);

__declspec(dllexport)
int Open(const char* fname, int oflag);

__declspec(dllexport)
void Close(int fd);

#endif
