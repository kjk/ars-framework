#ifndef _WINFONT_H_
#define _WINFONT_H_

#include <windows.h>

class WinFont  
{
    HFONT       handle_;
	mutable uint_t* refCount_;

	void release();

public:

    WinFont(HFONT handle = NULL);

	bool createIndirect(const LOGFONT& f);

	bool valid() const {return NULL != handle_;}

	WinFont(const WinFont& other);

	WinFont& operator=(const WinFont& other);

	~WinFont() {release();}
	
    HFONT handle() const {return handle_;}

private:
	
	void attach(HFONT handle, bool nonDesctuctible);

	friend class Graphics;
};

#endif
