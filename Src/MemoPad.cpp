#include <MemoPad.hpp>

/**
    TODO:
    - make category "Header" if needed
    - put body in header category

*/
namespace ArsLexis
{

    static void CreateNewMemoRec(DmOpenRef dbMemo, const ArsLexis::String& header, const ArsLexis::String& body)
    {
        UInt16      newRecNo;
        MemHandle   newRecHandle;
        void *      newRecData;
        UInt32      newRecSize;
        char        null = '\0';
    //  UInt16      attr;

        uint_t bodySize = body.length();
        newRecSize = bodySize + 1;
    
        newRecHandle = DmNewRecord(dbMemo,&newRecNo,newRecSize);
        if (NULL == newRecHandle)
            return;
    
        newRecData = MemHandleLock(newRecHandle);
    
        DmWrite(newRecData,0,body.data(),bodySize);
        DmWrite(newRecData,bodySize,&null,1);
        
    //  UInt16 category = 2;
    //  uint_t headerSize = header.length();
        // Change the category of the record.
    //  DmRecordInfo (dbMemo, newRecNo, &attr, NULL, NULL);
    //  attr &= ~dmRecAttrCategoryMask;
    //  attr |= category | dmRecAttrDirty;
    //  DmSetRecordInfo (dbMemo, newRecNo, &attr, NULL);
        
    
        MemHandleUnlock(newRecHandle);
    
        DmReleaseRecord(dbMemo,newRecNo,true);
    }

    void CreateNewMemo(const ArsLexis::String& memoHeader, const ArsLexis::String& memoBody)
    {
        LocalID     id;
        UInt16      cardno;
        UInt32      type,creator;
    
        DmOpenRef   dbMemo = NULL;
    
        bool        weNeedToClose = false;
        // check all the open database and see if memo is currently open
        dbMemo = DmNextOpenDatabase(NULL);
        while (dbMemo)
        {
            DmOpenDatabaseInfo(dbMemo,&id, NULL,NULL,&cardno,NULL);
            DmDatabaseInfo(cardno,id,
                NULL,NULL,NULL, NULL,NULL,NULL,
                NULL,NULL,NULL, &type,&creator);

            if( ('DATA' == type)  && ('memo' == creator) )
                break;
            dbMemo = DmNextOpenDatabase(dbMemo);
        }
        // we either found memo db, in which case dbMemo points to it, or
        // didn't find it, in which case dbTmp is NULL
        if (NULL == dbMemo)
        {    
            dbMemo = DmOpenDatabaseByTypeCreator('DATA','memo',dmModeReadWrite);
            if (NULL != dbMemo)
                weNeedToClose = true;
        }    
        if (NULL != dbMemo)
            CreateNewMemoRec(dbMemo, memoHeader, memoBody);
        if (weNeedToClose)
            DmCloseDatabase(dbMemo);
    } 
}