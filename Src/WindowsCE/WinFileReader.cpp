
#include <FileReader.hpp>
#include <cassert>

namespace ArsLexis {

    FileReader::FileReader():
        handle_(INVALID_HANDLE_VALUE)
    {}

    FileReader::~FileReader()
    {
        if (isOpen())
            close();
    }
    
    status_t FileReader::close()
    {
        assert(isOpen());
        BOOL res=::CloseHandle(handle_);
        handle_=INVALID_HANDLE_VALUE;
		if (!res)
			return ::GetLastError();
		else 
			return errNone;
    }
    
    status_t FileReader::read(int& ret)
    {
        assert(isOpen());
		char chr;
		DWORD result;
		BOOL res=::ReadFile(handle_, &chr, 1, &result, NULL);
		if (res)
		{
			if (0==result)
				ret=npos;
			else
				ret=chr;
			return errNone;
		}
		else
			return ::GetLastError();
    }

    status_t FileReader::read(int& ret, String& dst, int offset, int range)
    {
        assert(isOpen());
		DWORD result;
		BOOL res=::ReadFile(handle_, &dst[offset], range, &result, NULL);
		if (res)
		{
			ret=result;
			return errNone;
		}
		else
			return ::GetLastError();
    }

    status_t FileReader::open(const char_t* fileName, ulong_t access, ulong_t shareMode, LPSECURITY_ATTRIBUTES securityAttributes, 
		ulong_t creationDisposition, ulong_t flagsAndAttributes, HANDLE templateFile)
    {
        if (isOpen())
            close();
		handle_=::CreateFile(fileName, access, shareMode, securityAttributes, creationDisposition, flagsAndAttributes, templateFile);
		if (INVALID_HANDLE_VALUE==handle_)
			return ::GetLastError();
		else 
			return errNone;
	}

}