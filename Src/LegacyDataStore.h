#ifndef ARSLEXIS_LEGACY_DATA_STORE_H__
#define ARSLEXIS_LEGACY_DATA_STORE_H__

#include <PalmOS.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DataStore_ * DataStoreHandle;

Err DataStoreInitialize(const char* fileName);

DataStoreHandle DataStoreInstance();

void DataStoreDispose();

Err DataStoreWriteBlob(DataStoreHandle handle, const char* streamName, const void* blob, UInt16 blobSize);

Err DataStoreReadBlob(DataStoreHandle handle, const char* streamName, void* buffer, UInt16* size);

#ifdef __cplusplus
}
#endif

#endif