# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Convert the *.sql to *.txt flat file in the format:
#   ^title
#   namespace
#   text
#   And some other things.
#
# Usage:
#   -limit N : only process first N rows
#   -usepsyco : if used, will use psyco for speeding up, false by default
#   -juststats : if used, doesn't write processed data, just calculates
#                and prints the statistics
#   fileName - which file to process

import sys,os,os.path,string,time,md5,bz2
try:
    import psyco
    g_fPsycoAvailable = True
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"
    g_fPsycoAvailable = False

CUR_ID = 0
CUR_NAMESPACE = 1
CUR_TITLE = 2
CUR_TEXT = 3
CUR_COMMENT = 4
CUR_USER = 5
CUR_USER_TEXT = 6
CUR_TIMESTAMP = 7
CUR_RESTRICTIONS = 8
CUR_COUNTER = 9
CUR_IS_REDIRECT = 10
CUR_MINOR_EDIT = 11
CUR_IS_NEW = 12
CUR_RANDOM = 13
INVERSE_TIMESTAMP = 14
CUR_TOUCHED = 15
EXPECTED_ARGS_COUNT = 16

BEG_TXT = "INSERT INTO cur VALUES "

TOKEN_NONE = 0
TOKEN_LEFT_BRACE = 1
TOKEN_RIGHT_BRACE = 2
TOKEN_STR = 3
TOKEN_NUM = 4
TOKEN_COMMA = 5
TOKEN_FLOAT = 6
TOKEN_SEP = 7

g_fJustStats = False

def getTokenName(token):
    tokStr = ["TOKEN_NONE", "TOKEN_LEFT_BRACE", "TOKEN_RIGHT_BRACE", "TOKEN_STR", "TOKEN_NUM", "TOKEN_COMMA", "TOKEN_FLOAT", "TOKEN_SEP"]
    return tokStr[token]

def fTokenVal(token):
    if token == TOKEN_STR or token == TOKEN_NUM or token == TOKEN_FLOAT:
        return True
    return False

# given argument name in argName, tries to return argument value
# in command line args and removes those entries from sys.argv
# return None if not found
def getRemoveCmdArg(argName):
    argVal = None
    try:
        pos = sys.argv.index(argName)
        argVal = sys.argv[pos+1]
        sys.argv[pos:pos+2] = []
    except:
        pass
    return argVal

def fDetectRemoveCmdFlag(flag):
    fFlagPresent = False
    try:
        pos = sys.argv.index(flag)
        fFlagPresent = True
        sys.argv[pos:pos+1] = []
    except:
        pass
    return fFlagPresent

g_startTime = None
g_endTime = None

def startTiming():
    global g_startTime
    g_startTime = time.clock()

def endTiming():
    global g_endTime
    g_endTime = time.clock()

def dumpTiming():
    global g_startTime, g_endTime
    dur = g_endTime - g_startTime
    str = "duration %f seconds\n" % dur
    sys.stderr.write(str)

def usageAndExit():
    print "Usage: sql_to_txt.py [-limit N] [-usepsyco] [-juststats] fileName"
    sys.exit(0)

def fIsBzipFile(inFileName):
    if len(inFileName)>4 and ".bz2" == inFileName[-4:]:
        return True
    return False

def getBaseFileName(fileName):
    suf = ".bz2"
    sufLen = len(suf)
    if len(fileName)>sufLen and suf == fileName[-sufLen:]:
        fileName = fileName[:-sufLen]
        #print "new file name is %s" % fileName

    suf = ".sql"
    sufLen = len(suf)
    if len(fileName)>sufLen and suf == fileName[-sufLen:]:
        fileName = fileName[:-sufLen]
        #print "new file name is %s" % fileName
    else:
        print "%s is not a valid input file. Must be a *.sql or *.sql.bz2 file"
        sys.exit(0)
    return fileName

def genBaseAndSuffix(inFileName,suffix):
    return getBaseFileName(inFileName) + suffix
    
def getTxtFileName(inFileName):
    return genBaseAndSuffix(inFileName,".txt")

def getRedirectsFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_redirects.txt")

def getHashFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_hash.txt")

BUF_SIZE = 32768
class BigStr:
    def __init__(self,fileName):
        if fIsBzipFile(fileName):
            #print "Using bzip file"
            self.fo = bz2.BZ2File(fileName,"rb", 9)
        else:
            self.fo = open(fileName)
        self.buf = None
        self.curPos = 0
        self.bufLen = 0
        self.curVal = None

    def getTokenVal(self):
        ret = self.curVal
        self.curVal = "VALUE_TAKEN"
        return ret
    
    def getBuf(self):
        return self.buf

    def dumpBeforeAfterBuf(self,beforeLen=20,afterLen=20):
        pos = self.curPos
        if pos > beforeLen:
            startPos = pos-beforeLen
        else:
            startPos = 0
        if pos+afterLen > self.bufLen:
            endPos = self.bufLen-1
        else:
            endPos = pos + afterLen
        print "BEFORE: %s" % self.buf[startPos:self.curPos]
        print "AFTER: %s" % self.buf[self.curPos:endPos]

    def getChar(self):
        if self.curPos==self.bufLen:
            self.buf = self.fo.read(BUF_SIZE)
            if len(self.buf)==0:
                self.close()
                return None
            self.curPos = 0
            self.bufLen = len(self.buf)
        c = self.buf[self.curPos]
        self.curPos += 1
        # sys.stdout.write(c)
        return c

    def ungetChar(self):
        assert self.curPos != 0
        self.curPos -= 1

    def close(self):
        self.fo.close()

    def fSkipUntilTxt(self,txt):
        #print "skipping until %s" % txt
        curPosInTxt = 0
        totalLen = len(txt)
        while True:
            c = self.getChar()
            if c == None:
                return False
            if c==txt[curPosInTxt]:
                curPosInTxt += 1
                if curPosInTxt==totalLen:
                    return True
            else:
                curPosInTxt =0

    def getToken(self):
        c = self.getChar()
        token = TOKEN_NONE
        if c=='(':
            token = TOKEN_LEFT_BRACE
        elif c==')':
            token = TOKEN_RIGHT_BRACE
        elif c==',':
            token = TOKEN_COMMA
        elif c==';':
            token = TOKEN_SEP
        elif c=='\'':
            # this is a beginning of a string
            txt = ""
            while True:
                c = self.getChar()
                assert c != None
                if c =='\\':
                    c = self.getChar()
                    if c=='n':
                        txt += '\n'
                    elif c=='r':
                        txt += '\r'
                    elif c=='\\' or c=='\'' or c=='\"':
                        txt += c
                    elif c=='0':
                        # not really sure what to do with escaped 0, should it really be 0x0?
                        # happens for 'Irish_building'
                        txt += '0'
                    else:
                        # catch all the cases we didn't predict of escaped characters
                        print "got %s escaped" % c
                        self.dumpBeforeAfterBuf()
                        assert 0
                    continue
                elif c == '\'':
                    # this is the end of the string
                    self.curVal = txt
                    token = TOKEN_STR
                    break
                else:
                    txt += c
        elif (c>='0' and c<='9') or c=='.' or c=='-':
            # this must be a number
            fFloat = False
            if c=='.':
                fFloat = True
            num = c
            while True:
                c = self.getChar()
                if (c>='0' and c<='9') or c=='.' or c=='e' or c=='-':
                    num += c
                    if c=='.':
                        fFloat = True
                else:
                    if c!=',':
                        print "got %s, num=%s" % (c,num)
                        self.dumpBeforeAfterBuf()
                    assert c==','
                    self.ungetChar()
                    token = TOKEN_NUM
                    if fFloat:
                        token = TOKEN_FLOAT
                    self.curVal = num
                    break
        #if fTokenVal(token):
        #    print "%s(%s)" % (getTokenName(token),self.curVal)
        #else:
        #    print "%s" % getTokenName(token)            
        if token == TOKEN_NONE:
            self.dumpBeforeAfterBuf()
        assert token != TOKEN_NONE
        return token

ST_NONE = 0
ST_AFTER_LEFT_BRACE = 1
ST_AFTER_VALUE = 2
ST_AFTER_COMMA = 3
ST_AFTER_ARGS = 4

# return a tuple containing sql insert into args
def genINSERTArgs(sqlFileName):
    st = BigStr(sqlFileName)
    fSkipped = st.fSkipUntilTxt(BEG_TXT)
    assert fSkipped
    curState = ST_NONE
    args = []
    while True:
        if curState == ST_NONE:
            token = st.getToken()
            assert token == TOKEN_LEFT_BRACE
            curState = ST_AFTER_LEFT_BRACE
        elif curState == ST_AFTER_LEFT_BRACE:
            token = st.getToken()
            assert token == TOKEN_NUM
            args.append(st.getTokenVal())
            curState = ST_AFTER_VALUE
        elif curState == ST_AFTER_VALUE:
            token = st.getToken()
            if token == TOKEN_COMMA:
                curState = ST_AFTER_COMMA
            elif token == TOKEN_RIGHT_BRACE:
                if len(args) != EXPECTED_ARGS_COUNT:
                    print "expect len(args)=%d and is %d" % (EXPECTED_ARGS_COUNT,len(args))
                    print args
                assert len(args) == EXPECTED_ARGS_COUNT
                curState = ST_AFTER_ARGS
                yield args
                args = []
            else:
                print "got token %s(%d), expected fTokenValue() or TOKEN_RIGHT_BRACE or TOKEN_COMMA" % (getTokenName(token),token)
                st.dumpBeforeAfterBuf()
                assert 0
        elif curState == ST_AFTER_COMMA:
            token = st.getToken()
            if fTokenVal(token):
                args.append(st.getTokenVal())
            else:
                print "got token %s(%d), expected fTokenVal()" % (getTokenName(token),token)
                st.dumpBeforeAfterBuf()
                print args
                assert 0
            curState = ST_AFTER_VALUE
        elif curState == ST_AFTER_ARGS:
            # expecting ',' and '(' or ';'
            token = st.getToken()
            if token == TOKEN_SEP:
                fSkipped = st.fSkipUntilTxt(BEG_TXT)
                if not fSkipped:
                    return
                curState = ST_NONE
            elif token == TOKEN_COMMA:
                token = st.getToken()
                assert token == TOKEN_LEFT_BRACE
                curState = ST_AFTER_LEFT_BRACE
            else:
                print "got token %s, expected TOKEN_SEP or TOKEN_COMMA" % getTokenName(token)
                st.dumpBeforeAfterBuf()
                assert 0

        else:
            # this shouldn't happen
            print "Unknown state %d" % curState
            st.dumpBeforeAfterBuf()
            assert 0

# those come from namespace.php in wikipedia code
wiki_namespaces = ["NS_MAIN", "NS_TALK", "NS_USER", "NS_USER_TALK", "NS_WP", "NS_WIKIPEDIA", "NS_WP_TALK", "NS_WIKIPEDIA_TALK", "NS_IMAGE", "NS_IMAGE_TALK", "NS_MEDIAWIKI", "NS_MEDIAWIKI_TALK", "NS_TEMPLATE", "NS_TEMPLATE_TALK", "NS_HELP", "NS_HELP_TALK" ]
NS_COUNT = len(wiki_namespaces)
def getNsName(ns):
    return wiki_namespaces[ns]

NS_ARTICLES_COUNT = 0
NS_TOTAL_ARTICLES_SIZE = 1
NS_REDIRECTS = 2
NS_MAX_ITEMS = 3
def getNsInfo():
    info = []
    for n in range(NS_MAX_ITEMS):
        info.append(0)
    return info

# TODO:
#   which articles are most frequently redirected
#   which articles are most frequently linked to
#   how many articles are linked but do not exist
#   detect #REDIRECTS which are not marked as redirects in SQL
class WikipediaStats:
    def __init__(self):
        self.nsStats = []
        for t in range(NS_COUNT):
            self.nsStats.append(getNsInfo())

    def addStats(self,sqlArgs):
        
        ns = int(sqlArgs[CUR_NAMESPACE])
        txt = sqlArgs[CUR_TEXT]
        fRedirect = int(sqlArgs[CUR_IS_REDIRECT])

        self.nsStats[ns][NS_ARTICLES_COUNT] += 1
        self.nsStats[ns][NS_TOTAL_ARTICLES_SIZE] += len(txt)
        if fRedirect==1:
            self.nsStats[ns][NS_REDIRECTS] += 1

    def dumpStats(self):
        for ns in range(NS_COUNT):
            nsInfo = self.nsStats[ns]
            articlesCount = nsInfo[NS_ARTICLES_COUNT]
            if 0 == articlesCount:
                continue
            totalArticlesSize = nsInfo[NS_TOTAL_ARTICLES_SIZE]
            print "%s (%d) stats:" % (getNsName(ns),ns)
            print "  Total articles: %d" % articlesCount
            print "  Total articles size: %d" % totalArticlesSize
            print "  Redirects: %d" % nsInfo[NS_REDIRECTS]
            if articlesCount == 0:
                avgArticleSize = 0
            else:
                avgArticleSize = float(totalArticlesSize)/float(articlesCount)
            print "  Average article size: %.2f" % avgArticleSize

def convertFile(inName,limit):
    if not g_fJustStats:
        outName = getTxtFileName(inName)
        redirectsFileName = getRedirectsFileName(inName)
        hashFileName = getHashFileName(inName)
        foOut = open(outName, "wb")
        foRedirects = open(redirectsFileName, "wb")
        foHash = open(hashFileName,"wb")
    stats = WikipediaStats()
    count = 0
    startTiming()
    for sqlArgs in genINSERTArgs(inName):
        #print sqlArgs
        stats.addStats(sqlArgs)
        if not g_fJustStats:
            title = sqlArgs[CUR_TITLE]
            ns = int(sqlArgs[CUR_NAMESPACE])
            txt = sqlArgs[CUR_TEXT]
            foOut.write("^%s\n" % title)
            foOut.write("%d\n" % ns)
            foOut.write("%s" % txt)
            if len(txt)==0 or (txt[-1] != '\n' and txt[-1] != '\r'):
                foOut.write("\n")

            fRedirect = int(sqlArgs[CUR_IS_REDIRECT])
            if fRedirect:
                foRedirects.write("^%s\n" % title)
                foRedirects.write("%s" % txt)
                if len(txt)==0 or (txt[-1] != '\n' and txt[-1] != '\r'):
                    foRedirects.write("\n")
            else:
                md5Hash = md5.new(txt)
                foHash.write("^%s\n" % title)
                foHash.write("%s\n" % md5Hash.hexdigest())
        count += 1
        if 0 == count % 1000:
            print "processed %d items" % count
        if count>=limit:
            break
    if not g_fJustStats:
        foOut.close()
        foRedirects.close()
        foHash.close()
    endTiming()
    stats.dumpStats()
    dumpTiming()

if __name__=="__main__":
    limit = getRemoveCmdArg("-limit")
    if None == limit:
        limit = 9999999 # very big number
    else:
        limit = int(limit)
    print "limit=%d" % limit
    fUsePsyco = fDetectRemoveCmdFlag("-usepsyco")
    g_fJustStats = fDetectRemoveCmdFlag("-juststats")
    if g_fPsycoAvailable and fUsePsyco:
        print "using psyco"
        psyco.full()

    # now we should only have file name
    if len(sys.argv) != 2:
        usageAndExit()

    fileName = sys.argv[1]
    convertFile(fileName,limit)
