#include <File.hpp>

status_t File::open(const char_t* fileName, ulong_t access, ulong_t shareMode, LPSECURITY_ATTRIBUTES securityAttributes, ulong_t creationDisposition, ulong_t flagsAndAttributes, HANDLE templateFile)
{
	if (isOpen())
		close();
		
	handle_ = CreateFile(fileName, access, shareMode, securityAttributes, creationDisposition, flagsAndAttributes, templateFile);
	if (INVALID_HANDLE_VALUE == handle_)
		return GetLastError();
	
	return errNone;
}

status_t File::close()
{
	if (!isOpen())
		return errNone;

	BOOL res = CloseHandle(handle_);
	handle_ = INVALID_HANDLE_VALUE;
	if (!res)
		return GetLastError();
	return errNone;
}

status_t File::size(Size& val) const
{
	assert(isOpen());
	val = GetFileSize(handle_, NULL);
	if (INVALID_FILE_SIZE == val)
	{
		status_t err = GetLastError();
		if (errNone != err)
			return err;
	}
	return errNone;
}

status_t File::seek(SeekOffset offset, SeekType type)
{
	assert(isOpen());
	DWORD moveMethod = FILE_BEGIN;
	switch (type) {
		case seekFromCurrentPosition: moveMethod = FILE_CURRENT; break;
		case seekFromEnd: moveMethod = FILE_END; break;
	}
	DWORD pos = SetFilePointer(handle_, offset, NULL, moveMethod);
	if (INVALID_SET_FILE_POINTER == pos)
	{
		status_t err = GetLastError();
		if (errNone != err)
			return err;
	}
	return errNone;
}

status_t File::read(void* buffer, Size bytesToRead, Size& bytesRead)
{
	assert(isOpen());
	DWORD read;
	BOOL res = ReadFile(handle_, buffer, bytesToRead, &read, NULL);
	if (!res)
	{
		status_t err = GetLastError();
		if (ERROR_HANDLE_EOF != err)
			return GetLastError();
	}
	bytesRead = Size(read);
	return errNone;
}

status_t File::write(const void* buffer, Size bytesToWrite)
{
	assert(isOpen());
	DWORD written;
	BOOL res = WriteFile(handle_, buffer, bytesToWrite, &written, NULL);
	if (!res)
		return GetLastError();
	assert(written == bytesToWrite);
	return errNone;
}

status_t File::flush()
{
	assert(isOpen());
	if (!FlushFileBuffers(handle_))
		return GetLastError();
	return errNone;
}

status_t File::truncate()
{
	assert(isOpen());
	if (!SetEndOfFile(handle_))
		return GetLastError();
	return errNone;
}
