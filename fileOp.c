#include "fileOp.h"
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "quicklz.h"

#define STAND_BLOCK_SIZE (32*1024)
#define _FILE_OFFSET_BITS 64

void ShowMessage(char* str)
{
	printf("%s\n", str);
	MessageBoxA(NULL, str, "", MB_OK);
}

typedef struct _TransferBlockHead 
{
	__int64 isCompressed;
	__int64 offset;
	__int64 tag;
}TransferBlockHead ;

#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 1000000

qlz_state_compress state_compress;


int ReadAt(int fd, void* buf, int len, __int64 offset){
	TransferBlockHead* tb = malloc(sizeof(TransferBlockHead) + len +  4096);	//big enough
	BYTE* outdata = (BYTE*)tb + sizeof(TransferBlockHead);
	BYTE* indata = (BYTE*)buf + sizeof(TransferBlockHead);

	int afterCompress = 0;
	__int64 n = 0; 
	if (offset > 0)
	{
		__int64 cnt  = offset / (STAND_BLOCK_SIZE + sizeof(TransferBlockHead));
		offset = cnt * STAND_BLOCK_SIZE;
	}

	assert(len > sizeof(TransferBlockHead));

	n = _lseeki64(fd, offset, SEEK_SET);
	if (n == -1)
	{
		printf("_lseeki64 errno %d, offset %I64d\n", errno, offset);
		ShowMessage("oops _lseeki64 failed");
	}

	//printf("ReadAt %I64d, len = %d\n", offset,  len - sizeof(TransferBlockHead));
	n = _read(fd, indata, len - sizeof(TransferBlockHead));

	//printf("ReadAt %I64d\n", offset);

	if (n <= 0)
	{
		printf("errno %d\n", errno);
		ShowMessage("oops");
	}

	tb->isCompressed = FALSE;
	tb->offset = offset;
	tb->tag = 0x123456789;
	afterCompress = qlz_compress(indata, outdata, n, &state_compress);
	
	if (afterCompress < n)
	{
		tb->isCompressed = TRUE;
		n = sizeof(TransferBlockHead) + afterCompress;
		memcpy(buf, tb,  n);
	}else
	{
		n += sizeof(TransferBlockHead);
		memcpy(buf, tb,  sizeof(TransferBlockHead));
	}

	free(tb);

	return n;

	//return readAtFn(fd, buf, len, offset);
}


int WriteAt(int fd, void* buf, int len, __int64 offset){
	BYTE out[128*1024];
	int decompressSize = 0;

	TransferBlockHead* tb = (TransferBlockHead*)buf;	//big enough
	BYTE* data = (BYTE*)tb + sizeof(TransferBlockHead);
	int dataLen = len - sizeof(TransferBlockHead);

	int n = _lseeki64(fd, tb->offset, SEEK_SET);
	if (n == -1)
	{
		printf("_lseeki64 errno %d\n", errno);
		ShowMessage("oops");
	}

	if ( tb->tag != 0x123456789){
		ShowMessage("error tag");
	}

	//printf("WriteAt %I64d, real %I64d, len = %d\n", offset,  tb->offset, dataLen);

	if (tb->isCompressed)
	{
		decompressSize = qlz_decompress(data, out, &state_compress);
		if ( decompressSize > 0 ){
			printf("len %d decompressSize %d\n", dataLen, decompressSize);
			n = _write(fd, out, decompressSize);
			if (n <= 0)
			{
				printf("errno %d\n", errno);
				ShowMessage("oops");
			}
		}else{
			ShowMessage("decompress failed");
			exit(-1);
		}
	}
	else{
		n = _write(fd, data, dataLen);
		if (n != dataLen)
		{
			//printf("n = %d, dataLen = %d\n", n, dataLen);
			ShowMessage("write failed");
		}
	}

	return len;

	//return writeAtFn(fd, buf, len, offset);
}


int Open(const char* fname, int oflag){

	return _open(fname, oflag);

	//return openFn(fname, oflag);
}


void Close(int fd){

	ShowMessage("close file");
	_close(fd);

	//closeFn(fd);
}

int main(int argc, char** argv){
	return 0;
}
