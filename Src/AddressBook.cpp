/**
 *  Addess book
 *  Most of this code comes from Address 
 *  catalog:
 *   CodeWarrior9\(CodeWarrior Examples)\Palm OS\Palm OS 5.0 SDK Examples\Address\Src
 *
 *  from files:
 *   AddressDB.h
 *   AddressDB.c
 *   AddrDefines.h
 *
 *  Just copied functions needed to add one new record
 *
 */

#include <AddressBook.hpp>

#define addrNumFields						19
#define numPhoneLabels						8


#define firstAddressField			name
#define firstPhoneField				phone1
#define lastPhoneField				phone5
#define numPhoneFields				(lastPhoneField - firstPhoneField + 1)
#define numPhoneLabelsStoredFirst	numPhoneFields
#define numPhoneLabelsStoredSecond	(numPhoneLabels - numPhoneLabelsStoredFirst)

#define firstRenameableLabel		custom1
#define lastRenameableLabel			custom4
#define lastLabel					(addressFieldsCount + numPhoneLabelsStoredSecond)

#define IsPhoneLookupField(p)		(addrLookupWork <= (p) && (p) <= addrLookupMobile)

#define addrLabelLength				16


typedef enum
{
	name,
	firstName,
	company,
	phone1,
	phone2,
	phone3,
	phone4,
	phone5,
	address,
	city,
	state,
	zipCode,
	country,
	title,
	custom1,
	custom2,
	custom3,
	custom4,
	note,			// This field is assumed to be < 4K
	addressFieldsCount
} AddressFields;

typedef union
{
	struct
	{
		unsigned reserved:8;
		unsigned displayPhoneForList:4;	// The phone displayed for the list view 0 - 4
		unsigned phone5:4;				// Which phone (home, work, car, ...)
		unsigned phone4:4;
		unsigned phone3:4;
		unsigned phone2:4;
		unsigned phone1:4;
	} phones;
	UInt32 phoneBits;
} AddrOptionsType;


typedef struct
{
	AddrOptionsType	options;        // Display by company or by name
	Char *			fields[addressFieldsCount];
} AddrDBRecordType;

typedef AddrDBRecordType *AddrDBRecordPtr;

typedef union
{
	struct
	{
		unsigned reserved	:13;
		unsigned note		:1;	// set if record contains a note handle
		unsigned custom4	:1;	// set if record contains a custom4
		unsigned custom3	:1;	// set if record contains a custom3
		unsigned custom2	:1;	// set if record contains a custom2
		unsigned custom1	:1;	// set if record contains a custom1
		unsigned title		:1;	// set if record contains a title
		unsigned country	:1;	// set if record contains a birthday
		unsigned zipCode	:1;	// set if record contains a birthday
		unsigned state		:1;	// set if record contains a birthday
		unsigned city		:1;	// set if record contains a birthday
		unsigned address	:1;	// set if record contains a address
		unsigned phone5		:1;	// set if record contains a phone5
		unsigned phone4		:1;	// set if record contains a phone4
		unsigned phone3		:1;	// set if record contains a phone3
		unsigned phone2		:1;	// set if record contains a phone2
		unsigned phone1		:1;	// set if record contains a phone1
		unsigned company	:1;	// set if record contains a company
		unsigned firstName	:1;	// set if record contains a firstName
		unsigned name		:1;	// set if record contains a name (bit 0)

	} bits;
	UInt32 allBits;
} AddrDBRecordFlags;

// Max length of a field name found in the FieldNamesStrList string list.
#define maxFieldName		31

#define LocalizedAppInfoStr	1000

// Extract the bit at position index from bitfield.  0 is the high bit.
#define BitAtPosition(pos)                ((UInt32)1 << (pos))
#define GetBitMacro(bitfield, index)      ((bitfield) & BitAtPosition(index))
#define SetBitMacro(bitfield, index)      ((bitfield) |= BitAtPosition(index))
#define RemoveBitMacro(bitfield, index)   ((bitfield) &= ~BitAtPosition(index))

#define sortKeyFieldBits   (BitAtPosition(name) | BitAtPosition(firstName) | BitAtPosition(company))

// Indexes into FieldNamesStrList string list.
enum {
	fieldNameStrListCity = 0,
	fieldNameStrListState,
	fieldNameStrListZip
};

// The following structure doesn't really exist.  The first field
// varies depending on the data present.  However, it is convient
// (and less error prone) to use when accessing the other information.
typedef struct {
	AddrOptionsType		options;        // Display by company or by name
	AddrDBRecordFlags	flags;
	UInt8				companyFieldOffset;   // Offset from firstField
	char				firstField;
} PrvAddrPackedDBRecord;

/************************************************************
 *
 *  FUNCTION: PrvAddrDBUnpackedSize
 *
 *  DESCRIPTION: Return the size of an AddrDBRecordType
 *
 *  PARAMETERS: address record
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/10/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static Int16 PrvAddrDBUnpackedSize(AddrDBRecordPtr r)
{
	Int16 size;
	Int16   index;

	size = sizeof (PrvAddrPackedDBRecord) - sizeof (char);   // correct
	for (index = firstAddressField; index < addressFieldsCount; index++)
	{
		if (r->fields[index] != NULL)
			size += StrLen(r->fields[index]) + 1;
	}
	return size;
}
/************************************************************
 *
 *  FUNCTION: PrvAddrDBPack
 *
 *  DESCRIPTION: Pack an AddrDBRecordType.  Doesn't pack empty strings.
 *
 *  PARAMETERS: address record to pack
 *                address record to pack into
 *
 *  RETURNS: the PrvAddrPackedDBRecord is packed
 *
 *  CREATED: 1/10/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static void PrvAddrDBPack(AddrDBRecordPtr s, void * recordP)
{
	Int32                offset;
	AddrDBRecordFlags    flags;
	Int16                index;
	PrvAddrPackedDBRecord*  d=0;
	Int16                len;
	void *               srcP;
	UInt8                companyFieldOffset;

	flags.allBits = 0;

	DmWrite(recordP, (Int32)&d->options, &s->options, sizeof(s->options));
	offset = (Int32)&d->firstField;

	for (index = firstAddressField; index < addressFieldsCount; index++) {
		if (s->fields[index] != NULL)
			/*         if (s->fields[index][0] == '\0')
			 {
			 // so set the companyFieldOffset or clear it code doesn't fail
			 s->fields[index] = NULL;
			 }
			 else
			 */
		{
			ErrFatalDisplayIf(s->fields[index][0] == '\0' && index != note,
							  "Empty field being added");
			srcP = s->fields[index];
			len = StrLen((const char*) srcP) + 1;
			DmWrite(recordP, offset, srcP, len);
			offset += len;
			SetBitMacro(flags.allBits, index);
		}
	}

	// Set the flags indicating which fields are used
	DmWrite(recordP, (Int32)&d->flags.allBits, &flags.allBits, sizeof(flags.allBits));

	// Set the companyFieldOffset or clear it
	if (s->fields[company] == NULL)
		companyFieldOffset = 0;
	else {
		index = 1;
		if (s->fields[name] != NULL)
			index += StrLen(s->fields[name]) + 1;
		if (s->fields[firstName] != NULL)
			index += StrLen(s->fields[firstName]) + 1;
		companyFieldOffset = (UInt8) index;
	}
	DmWrite(recordP, (Int32)(&d->companyFieldOffset), &companyFieldOffset, sizeof(companyFieldOffset));
}

typedef union
{
	struct
	{
		unsigned reserved	 :10;
		unsigned phone8      :1;	// set if phone8 label is dirty
		unsigned phone7      :1;	// set if phone7 label is dirty
		unsigned phone6      :1;	// set if phone6 label is dirty
		unsigned note        :1;	// set if note label is dirty
		unsigned custom4     :1;	// set if custom4 label is dirty
		unsigned custom3     :1;	// set if custom3 label is dirty
		unsigned custom2     :1;	// set if custom2 label is dirty
		unsigned custom1     :1;	// set if custom1 label is dirty
		unsigned title       :1;	// set if title label is dirty
		unsigned country	 :1;	// set if country label is dirty
		unsigned zipCode	 :1;	// set if zipCode label is dirty
		unsigned state		 :1;	// set if state label is dirty
		unsigned city		 :1;	// set if city label is dirty
		unsigned address     :1;	// set if address label is dirty
		unsigned phone5      :1;	// set if phone5 label is dirty
		unsigned phone4      :1;	// set if phone4 label is dirty
		unsigned phone3      :1;	// set if phone3 label is dirty
		unsigned phone2      :1;	// set if phone2 label is dirty
		unsigned phone1      :1;	// set if phone1 label is dirty
		unsigned company     :1;	// set if company label is dirty
		unsigned firstName   :1;	// set if firstName label is dirty
		unsigned name        :1;	// set if name label is dirty (bit 0)

	} bits;
	UInt32 allBits;
} AddrDBFieldLabelsDirtyFlags;

typedef char addressLabel[addrLabelLength];

typedef struct
{
	unsigned reserved:7;
	unsigned sortByCompany	:1;
} AddrDBMisc;

typedef struct
{
	UInt16				renamedCategories;	// bitfield of categories with a different name
	char 					categoryLabels[dmRecNumCategories][dmCategoryLength];
	UInt8 				categoryUniqIDs[dmRecNumCategories];
	UInt8					lastUniqID;	// Uniq IDs generated by the device are between
	// 0 - 127.  Those from the PC are 128 - 255.
	UInt8					reserved1;	// from the compiler word aligning things
	UInt16				reserved2;
	AddrDBFieldLabelsDirtyFlags dirtyFieldLabels;
	addressLabel 		fieldLabels[addrNumFields + numPhoneLabelsStoredSecond];
	CountryType 		country;		// Country the database (labels) is formatted for
	UInt8 				reserved;
	AddrDBMisc			misc;
} AddrAppInfoType;

typedef AddrAppInfoType *AddrAppInfoPtr;

/************************************************************
 *
 *  FUNCTION: AddrDBAppInfoGetPtr
 *
 *  DESCRIPTION: Return a locked pointer to the AddrAppInfo or NULL
 *
 *  PARAMETERS: dbP - open database pointer
 *
 *  RETURNS: locked ptr to the AddrAppInfo or NULL
 *
 *  CREATED: 6/13/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static AddrAppInfoPtr   AddrDBAppInfoGetPtr(DmOpenRef dbP)
{
	UInt16     cardNo;
	LocalID    dbID;
	LocalID    appInfoID;

	if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
		return NULL;
	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL))
		return NULL;

	if (appInfoID == 0)
		return NULL;
	else
		return (AddrAppInfoPtr) MemLocalIDToLockedPtr(appInfoID, cardNo);

}

/************************************************************
 *
 *  FUNCTION: PrvAddrDBFindKey
 *
 *  DESCRIPTION: Return the next valid key
 *
 *  PARAMETERS: database packed record
 *            <-> key to use (ptr to string or NULL for uniq ID)
 *            <-> which key (incremented for use again, starts at 1)
 *            -> sortByCompany
 *
 *  RETURNS:
 *
 *  CREATED: 1/16/95
 *
 *  BY: Roger Flores
 *
 *   COMMENTS:   Returns the key which is asked for if possible and
 *   advances whichKey.  If the key is not available the key advances
 *   to the next one.  The order of keys is:
 *
 * if sortByCompany:
 *      companyKey, nameKey, firstNameKey, uniq ID
 *
 * if !sortByCompany:
 *      nameKey, firstNameKey, companyKey (if no name or first name), uniq ID
 *
 *
 *************************************************************/
static void PrvAddrDBFindKey(PrvAddrPackedDBRecord *r, char **key, UInt16 *whichKey, Int16 sortByCompany)
{
	AddrDBRecordFlags fieldFlags;

	fieldFlags.allBits = r->flags.allBits;

	ErrFatalDisplayIf(*whichKey == 0 || *whichKey == 5, "Bad addr key");

	if (sortByCompany)
	{
		if (*whichKey == 1 && fieldFlags.bits.company)
		{
			*whichKey = 2;
			goto returnCompanyKey;
		}

		if (*whichKey <= 2 && fieldFlags.bits.name)
		{
			*whichKey = 3;
			goto returnNameKey;
		}

		if (*whichKey <= 3 && fieldFlags.bits.firstName)
		{
			*whichKey = 4;
			goto returnFirstNameKey;
		}
	}
	else
	{
		if (*whichKey == 1 && fieldFlags.bits.name)
		{
			*whichKey = 2;
			goto returnNameKey;
		}

		if (*whichKey <= 2 && fieldFlags.bits.firstName)
		{
			*whichKey = 3;
			goto returnFirstNameKey;
		}

		// For now don't consider company name when sorting by person name
		// unless there isn't a name or firstName
		if (*whichKey <= 3 && fieldFlags.bits.company &&
			!(fieldFlags.bits.name || fieldFlags.bits.firstName))
		{
			*whichKey = 4;
			goto returnCompanyKey;
		}

	}

	// All possible fields have been tried so return NULL so that
	// the uniq ID is compared.
	*whichKey = 5;
	*key = NULL;
	return;



returnCompanyKey:
	*key = (char *) &r->companyFieldOffset + r->companyFieldOffset;
	return;


returnNameKey:
	*key = &r->firstField;
	return;


returnFirstNameKey:
	*key = &r->firstField;
	if (r->flags.bits.name)
	{
		*key += StrLen(*key) + 1;
	}
	return;

}

/************************************************************
 *
 *	FUNCTION:	PrvAddrDBComparePackedRecords
 *
 *	DESCRIPTION: Compare two packed records  key by key until
 *		there is a difference.  Return -1 if r1 is less or 1 if r2
 *		is less.  A zero may be returned if two records seem
 *		identical. NULL fields are considered less than others.
 *
 *	PARAMETERS:	address record 1
 *            address record 2
 *
 *	RETURNS: -1 if record one is less
 *           1 if record two is less
 *
 *	HISTORY:
 *		01/14/95	rsf	Created by Roger Flores.
 *		11/30/00	kwk	Only call StrCompare, not StrCaselessCompare
 *							first and then StrCompare. Also use TxtCompare
 *							instead of StrCompare, to skip a trap call.
 *
 *************************************************************/
static Int16 PrvAddrDBComparePackedRecords(PrvAddrPackedDBRecord *r1,
												PrvAddrPackedDBRecord *r2,
												Int16 sortByCompany,
												SortRecordInfoPtr info1,
												SortRecordInfoPtr info2,
												MemHandle appInfoH)
{
	UInt16 whichKey1, whichKey2;
	char *key1, *key2;
	Int16 result;

	whichKey1 = 1;
	whichKey2 = 1;

	do {
		PrvAddrDBFindKey(r1, &key1, &whichKey1, sortByCompany);
		PrvAddrDBFindKey(r2, &key2, &whichKey2, sortByCompany);

		// A key with NULL loses the StrCompare.
		if (key1 == NULL)
		{
			// If both are NULL then return them as equal
			if (key2 == NULL)
			{
				result = 0;
				return result;
			}
			else
				result = -1;
		}
		else
			if (key2 == NULL)
				result = 1;
			else
			{
				// With Palm OS 4.0, StrCompare will try a caseless
				// comparison first, then a case-sensitive, so we
				// only need to call StrCompare. Also, we can call
				// TxtCompare to avoid one extra trap dispatch.
				
				// result = StrCaselessCompare(key1, key2);
				// if (result == 0)
				//		result = StrCompare(key1, key2);
				
				result = TxtCompare(	key1,		// const Char *s1,
											0xFFFF,	// UInt16 s1Len,
											NULL,		// UInt16 *s1MatchLen,
											key2,		// const Char *s2,
											0xFFFF,	// UInt16 s2Len,
											NULL);	// UInt16 *s2MatchLen
			}

	} while (!result);


	return result;
}

/************************************************************
 *
 *  FUNCTION: PrvAddrDBFindSortPosition
 *
 *  DESCRIPTION: Return where a record is or should be
 *      Useful to find or find where to insert a record.
 *
 *  PARAMETERS: address record
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/11/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static UInt16 PrvAddrDBFindSortPosition(DmOpenRef dbP, PrvAddrPackedDBRecord *newRecord)
{
	Int16 sortByCompany;
	AddrAppInfoPtr appInfoPtr;


	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);
	sortByCompany = appInfoPtr->misc.sortByCompany;
	MemPtrUnlock(appInfoPtr);

	return DmFindSortPosition(dbP, (void *) newRecord, NULL, (DmComparF *)
							  PrvAddrDBComparePackedRecords, (Int16) sortByCompany);
}


/************************************************************
 *
 *  FUNCTION: AddrDBNewRecord
 *
 *  DESCRIPTION: Create a new packed record in sorted position
 *
 *  PARAMETERS: database pointer - open db pointer
 *            address record   - pointer to a record to copy into the DB
 *            record index      - to be set to the new record's index
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *             index set if a new record is created.
 *
 *  CREATED: 1/10/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static Err AddrDBNewRecord(DmOpenRef dbP, AddrDBRecordPtr r, UInt16 *index)
{
	MemHandle               recordH;
	Err                     err;
	PrvAddrPackedDBRecord*   recordP;
	UInt16                   newIndex;


	// 1) and 2) (make a new chunk with the correct size)
	recordH = DmNewHandle(dbP, (Int32) PrvAddrDBUnpackedSize(r));
	if (recordH == NULL)
		return dmErrMemError;


	// 3) Copy the data from the unpacked record to the packed one.
	recordP = (PrvAddrPackedDBRecord*) MemHandleLock(recordH);
	PrvAddrDBPack(r, recordP);

	// Get the index
	newIndex = PrvAddrDBFindSortPosition(dbP, recordP);
	MemPtrUnlock(recordP);


	// 4) attach in place
	err = DmAttachRecord(dbP, &newIndex, recordH, 0);
	if (err)
		MemHandleFree(recordH);
	else
		*index = newIndex;

	return err;
}


/**
 *   Only this is ours
 */
// return true if we need to close database
static bool OpenDataBase(DmOpenRef* dbIn, UInt32 typeIn, UInt32 creatorIn)
{
    LocalID     id;
    UInt16      cardno;
    UInt32      type,creator;
    DmOpenRef   dbAddr = NULL;
    bool        weNeedToClose = false;
    // check all the open database and see if address is currently open
    dbAddr = DmNextOpenDatabase(NULL);
    while (dbAddr)
    {
        DmOpenDatabaseInfo(dbAddr,&id, NULL,NULL,&cardno,NULL);
        DmDatabaseInfo(cardno,id,
            NULL,NULL,NULL, NULL,NULL,NULL,
            NULL,NULL,NULL, &type,&creator);

        if( (typeIn == type)  && (creatorIn == creator) )
            break;
        dbAddr = DmNextOpenDatabase(dbAddr);
    }
    // we either found addr db, in which case dbAddr points to it, or
    // didn't find it, in which case dbAddr is NULL
    if (NULL == dbAddr)
    {    
        dbAddr = DmOpenDatabaseByTypeCreator(typeIn,creatorIn,dmModeReadWrite);
        if (NULL != dbAddr)
            weNeedToClose = true;
    }    
    *dbIn = dbAddr;
    return weNeedToClose;
}
 
//our interfaces 
namespace ArsLexis
{
    // person interface
    bool newAddressBookPerson(const char* inFirstName,
                              const char* inLastName,
                              const char* inAddress,
                              const char* inCity,
                              const char* inState,
                              const char* inZipCode,
                              const char* inPhone)
    {
        AddrDBRecordType rec;
        AddrDBRecordPtr  r = &rec;
        UInt16           newIndex;
        bool             ret = false;
       
        for (int i = name; i< addressFieldsCount; i++)
            r->fields[i] = NULL;
        if (0 < StrLen(inLastName))
            r->fields[name] = (char*) inLastName;
        if (0 < StrLen(inFirstName))
            r->fields[firstName] = (char*) inFirstName;
        if (0 < StrLen(inAddress))
            r->fields[address] = (char*) inAddress;
        if (0 < StrLen(inCity))
            r->fields[city] = (char*) inCity;
        if (0 < StrLen(inState))
            r->fields[state] = (char*) inState;
        if (0 < StrLen(inZipCode))
            r->fields[zipCode] = (char*) inZipCode;
        if (0 < StrLen(inPhone))
            r->fields[phone1] = (char*) inPhone;

        //phone - home
        r->options.phoneBits = 1;

        DmOpenRef dbAddr = NULL;
        bool      weNeedToClose = OpenDataBase(&dbAddr,'DATA','addr');
        if (NULL != dbAddr)
        {
            Err error = AddrDBNewRecord(dbAddr, r, &newIndex);
            if (!error)
                ret = true;
        }
        if (weNeedToClose)
            DmCloseDatabase(dbAddr);
        return ret;
    }


    // business interface
    bool newAddressBookBusiness(const char* inName,
                                const char* inAddress,
                                const char* inCity,
                                const char* inState,
                                const char* inZipCode,
                                const char* inPhone)
    {
        AddrDBRecordType rec;
        AddrDBRecordPtr  r = &rec;
        UInt16           newIndex;
        bool             ret = false;
       
        for (int i = name; i< addressFieldsCount; i++)
            r->fields[i] = NULL;
        if (0 < StrLen(inName))
            r->fields[company] = (char*) inName;
        if (0 < StrLen(inAddress))
            r->fields[address] = (char*) inAddress;
        if (0 < StrLen(inCity))
            r->fields[city] = (char*) inCity;
        if (0 < StrLen(inState))
            r->fields[state] = (char*) inState;
        if (0 < StrLen(inZipCode))
            r->fields[zipCode] = (char*) inZipCode;
        if (0 < StrLen(inPhone))
            r->fields[phone1] = (char*) inPhone;

        //phone - work
        r->options.phoneBits = 0;


        DmOpenRef dbAddr = NULL;
        bool      weNeedToClose = OpenDataBase(&dbAddr,'DATA','addr');
        if (NULL != dbAddr)
        {
            Err error = AddrDBNewRecord(dbAddr, r, &newIndex);
            if (!error)
                ret = true;
        }
        if (weNeedToClose)
            DmCloseDatabase(dbAddr);
        return ret;
    }
}