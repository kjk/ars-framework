#include <StringListPayloadHandler.hpp>

namespace ArsLexis {

    StringListPayloadHandler::~StringListPayloadHandler()
    {}
    
    status_t StringListPayloadHandler::handleIncrement(const String& payload, ulong_t& length, bool finish)
    {
        buffer_.append(payload, 0, length);
        if (finish) 
        {
            Strings_t vec;
            String::size_type pos=0;
            while (true) 
            {
                String::size_type next=buffer_.find(_T('\n'), pos);
                vec.push_back(String(buffer_, pos, next-pos));
                if (String::npos==next)
                    break;
                else
                    pos=next+1;
            }
            return listReady(vec);
        }
        else
            return errNone;
    }


}