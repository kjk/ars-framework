
#include <LegacyDataStore.h>
#include <DataStore.hpp>

Err DataStoreInitialize(const char* fileName)
{
    return DataStore::initialize(fileName);
}

DataStoreHandle DataStoreInstance()
{
    return reinterpret_cast<DataStoreHandle>(DataStore::instance());
}

void DataStoreDispose()
{
    DataStore::dispose();
}

Err DataStoreWriteBlob(DataStoreHandle handle, const char* streamName, const void* blob, UInt16 blobSize)
{
    assert(NULL != handle);
    DataStore* ds = reinterpret_cast<DataStore*>(handle);
    DataStoreWriter writer(*ds);
    Err error = writer.open(streamName);
    if (errNone != error)
        return error;
    error = writer.writeRaw(blob, blobSize);
    return error;
}

Err DataStoreReadBlob(DataStoreHandle handle, const char* streamName, void* buffer, UInt16* size)
{
    assert(NULL != handle);
    assert(NULL != size);
    DataStore* ds = reinterpret_cast<DataStore*>(handle);
    DataStoreReader reader(*ds);
    Err error = reader.open(streamName);
    if (errNone != error)
        return error;
    uint_t sz = *size;
    error = reader.readRaw(buffer, sz);
    if (errNone != error)
        return error;
    *size = sz;
    return errNone;
}