#include "../../core/include/core/memory.h"
#pragma once

//���������� ��������� ���������� � ����, �����, ��

//������ ���������� �� ��������� � ������� �������� ��� � ��������� ����
void GetEnvironmentComment( StringBuilder& s );
//����� ���������� � ������ s �������� sz_s, ���������� ����� ���������� ������
int GetEnvironmentComment( char* s, int sz_s );
