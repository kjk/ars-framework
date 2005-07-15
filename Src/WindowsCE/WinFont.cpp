#include <BaseTypes.hpp>
#include <Debug.hpp>

#include <WindowsCE/WinFont.h>

WinFont::WinFont(HFONT handle):
	handle_(handle),
	refCount_(NULL)
{
	if (NULL != handle_)
		refCount_ = new uint_t(1);
}

void WinFont::release()
{
	if (NULL == handle_)
	{
		assert(NULL == refCount_);
		return;
	}
	assert(0 != *refCount_);
	if (0 == --*refCount_)
	{
		delete refCount_;
		refCount_ = NULL;
		DeleteObject((HGDIOBJ)handle_);
		handle_ = NULL;
	}
}

void WinFont::attach(HFONT handle, bool nonDestructible)
{
	assert(NULL == handle_);
	assert(NULL == refCount_);
	if (NULL == handle)
		return;

	refCount_ = new uint_t(nonDestructible ? 2 : 1);
	handle_ = handle;
}

bool WinFont::createIndirect(const LOGFONT& f)
{
	release();
	attach(CreateFontIndirect(&f), false);
	return (NULL != handle_);
}

bool WinFont::getStockFont(int object)
{
	HGDIOBJ obj = GetStockObject(object);
	if (NULL == obj)
		return false;

	if (OBJ_FONT != GetObjectType(obj))
		return false;

	release();
	attach(HFONT(obj), true);
	return true;
}

void WinFont::getSystemFont()
{
	bool res = getStockFont(SYSTEM_FONT);
	assert(res);
}

WinFont::WinFont(const WinFont& other):
	handle_(other.handle_),
	refCount_(other.refCount_)
{
	if (NULL != handle_)
	{
		assert(NULL != refCount_);
		++*refCount_;
	}
}

WinFont& WinFont::operator=(const WinFont& other)
{
	if (this == &other)
		return *this;

	release();
	handle_ = other.handle_;
	refCount_ = other.refCount_;
	if (NULL != handle_)
	{
		assert(NULL != refCount_);
		++*refCount_;
	}
	return *this;
}
