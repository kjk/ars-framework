#ifndef __ARSLEXIS_HASHTABLE_HPP__
#define __ARSLEXIS_HASHTABLE_HPP__

#include "BaseTypes.hpp"

namespace ArsLexis {
    class Hashtable
    {
        private:
            char_t **entity_;
            char_t **value_;
            int counter_;
            int max_;

        public:
            Hashtable();
            void put(String entity,String value);
            String get(String code);
            //Use it to free all private memory of Hashtable
            void Dispose();
    };
}
#endif
