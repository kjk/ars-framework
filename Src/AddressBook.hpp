#ifndef __ARSLEXIS_ADDRESS_BOOK_HPP__
#define __ARSLEXIS_ADDRESS_BOOK_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

bool newAddressBookPerson(const char* inFirstName,
                          const char* inLastName,
                          const char* inAddress,
                          const char* inCity,
                          const char* inState,
                          const char* inZipCode,
                          const char* inPhone);

bool newAddressBookBusiness(const char* inName,
                            const char* inAddress,
                            const char* inCity,
                            const char* inState,
                            const char* inZipCode,
                            const char* inPhone);

#endif