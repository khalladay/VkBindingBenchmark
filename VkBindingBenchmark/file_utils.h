#pragma once


struct DataBuffer
{
	char* data;
	size_t size;
};

DataBuffer* loadBinaryFile(const char* filepath);

const char* loadTextFile(const char* filepath);

void freeDataBuffer(DataBuffer* buffer);