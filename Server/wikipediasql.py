# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Process wikipedia SQL dumps (*.sql or *.sql.bz2) and returns a
#   WikipediaArticle object representing articles

import sys,os,string,re,bz2,md5
import arsutils,articleconvert

NS_MAIN = 0

# order of fields in SQL INSERT statement
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

relaxedRe1=re.compile(r'\[\[(.*?)\]', re.S)
relaxedRe2=re.compile(r'\[(.*?)\]', re.S)

def getRelaxedRedirect(txt):
    for match in relaxedRe1.finditer(txt):
        link=match.group(1)
        if None != link:
            return link
    for match in relaxedRe2.finditer(txt):
        link=match.group(1)
        if None != link:
            return link
    return None

def getRedirectFromText(txt):
    if 0!=txt.lower().find("#redirect"):
        return None
    links = articleconvert.articleExtractLinks(txt)
    if None == links:
        return getRelaxedRedirect(txt)
    link = links[0]
    #link.dump()
    return link.getLink().strip()

def fInvalidRedirect(row):
    redirectNum = int(row[CUR_IS_REDIRECT])
    if redirectNum!=0 and redirectNum!=1:
        return True
    if 1==redirectNum:
        txt = row[CUR_TEXT]
        redirect = getRedirectFromText(txt)
        if not redirect:
            return True
    return False

class WikipediaArticleFromSql:
    def __init__(self,row):
        self.row = row
        assert not fInvalidRedirect(row)
        self.md5Hash = None
        txt = row[CUR_TEXT]
        #redirectNum = int(row[CUR_IS_REDIRECT])
        #assert redirectNum==0 or redirectNum==1
        self.redirect = None
        # we pretty much ignore the CUR_IS_REDIRECT setting
        # if it's marked as redirect and is not, we throw it out
        # before coming here
        # if it's not marked as redirect but looks like redirect
        # we treat it as redirect anyway
        redirect = getRedirectFromText(txt)
        if redirect:
            self.redirect = redirect.replace(" ", "_")
            if int(row[CUR_IS_REDIRECT])==0:
                # redirect not marked as such
                print "%s is a redirect but not marked as such" % getTitle()
        else:
            self.row[CUR_TEXT] = txt.strip()
    def getId(self):  return int(self.row[CUR_ID])
    def getNamespace(self): return int(self.row[CUR_NAMESPACE])
    def getTitle(self): return self.row[CUR_TITLE]
    def getText(self): return self.row[CUR_TEXT]
    def fRedirect(self):
        if self.redirect:
            return True
        return False
    def getRedirect(self): return self.redirect
    def getViewCount(self): return int(self.row[CUR_COUNTER])
    def getTimestamp(self): return self.row[CUR_TIMESTAMP]
    def getHash(self):
        if self.md5Hash == None:
            md5Obj = md5.new(self.getText())
            self.md5Hash = md5Obj.hexdigest()
        return self.md5Hash

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

def getIdxFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_idx.txt")

def getBodyFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_body.txt")

def getTxt(sqlFileName,txtOff,txtLen):
    fn = getBodyFileName(sqlFileName)
    fo = open(fn,"rb")
    fo.seek(txtOff)
    txt = fo.read(txtLen)
    fo.close()
    return txt

def getConvertedTxt(sqlFileName,txtOff,txtLen):
    fn = getConvBodyFileName(sqlFileName)
    fo = open(fn,"rb")
    fo.seek(txtOff)
    txt = fo.read(txtLen)
    fo.close()
    return txt

class WikipediaArticleRedirect:
    def __init__(self,title,redirect):
        self.title = title.strip()
        self.redirect = redirect.strip()
    def getTitle(self): return self.title
    def getRedirect(self): return self.redirect        
    def fRedirect(self): return True
    def getNamespace(self): return NS_MAIN

class WikipediaArticleFromCache:
    def __init__(self,sqlFileName,title,ns,txtOffset,txtLen,md5Hash,viewCount):
        self.sqlFileName = sqlFileName
        self.title = title
        self.ns = ns
        self.txtOffset = txtOffset
        self.txtLen = txtLen
        self.md5Hash = md5Hash
        self.viewCount = viewCount
    def getHash(self): return self.md5Hash
    def getNamespace(self): return self.ns
    def getTitle(self): return self.title
    def getText(self): return getTxt(self.sqlFileName, self.txtOffset, self.txtLen)
    def fRedirect(self):  return False
    def getViewCount(self): return self.viewCount

class ConvertedArticleFromCache:
    def __init__(self,sqlFileName,title,ns,txtOffset,txtLen):
        self.sqlFileName = sqlFileName
        self.title = title
        self.ns = ns
        self.txtOffset = txtOffset
        self.txtLen = txtLen
    def getNamespace(self): return self.ns
    def getTitle(self): return self.title
    def getText(self): return getConvertedTxt(self.sqlFileName, self.txtOffset, self.txtLen)
    def fRedirect(self):  return False

REDIRECT_MARK = "^r "

def getConvIdxFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_conv_idx.txt")

def getConvBodyFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_conv_body.txt")

def fConvertedCacheExists(sqlDumpFileName):
    txtName=  getConvBodyFileName(sqlDumpFileName)
    idxFileName = getConvIdxFileName(sqlDumpFileName)
    if arsutils.fFileExists(txtName) and arsutils.fFileExists(idxFileName):
        return True
    return False

class ConvertedArticleCacheWriter:
    def __init__(self,sqlDumpFileName):
        self.fCacheExists = fConvertedCacheExists(sqlDumpFileName)
        self.txtName= getConvBodyFileName(sqlDumpFileName)
        self.idxFileName = getConvIdxFileName(sqlDumpFileName)
        self.foIdx = None
        self.foTxt = None
        self.curTxtOffset = None

    def fCacheExists(self): return self.fCacheExists

    def open(self):
        assert self.foIdx == None
        assert self.foTxt == None
        self.foIdx = open(self.idxFileName,"wb")
        self.foTxt = open(self.txtName, "wb")
        self.curTxtOffset = 0

    def write(self,article):
        title = article.getTitle()
        ns = article.getNamespace()
        assert ns == NS_MAIN
        if article.fRedirect():
            self.foIdx.write("%s%s\n" % (REDIRECT_MARK,title))
            self.foIdx.write("%s\n" % article.getRedirect())
        else:
            txt = article.getText()
            txtLen = len(txt)

            self.foIdx.write("%s\n" % title)
            self.foIdx.write("%d,%d,%d\n" % (ns, self.curTxtOffset, txtLen))
            self.curTxtOffset += txtLen
            self.foTxt.write(txt)

    def close(self):        
        if not self.foTxt:
            return
        self.foTxt.close()
        self.foIdx.close()

def fCacheExists(sqlDumpFileName):
    txtName= getBodyFileName(sqlDumpFileName)
    idxFileName = getIdxFileName(sqlDumpFileName)
    if arsutils.fFileExists(txtName) and arsutils.fFileExists(idxFileName):
        return True
    return False

class ArticleCacheWriter:
    def __init__(self,sqlDumpFileName):
        self.fCacheExists = fCacheExists(sqlDumpFileName)
        self.txtName= getBodyFileName(sqlDumpFileName)
        self.idxFileName = getIdxFileName(sqlDumpFileName)
        self.foIdx = None
        self.foTxt = None
        self.curTxtOffset = None

    def fCacheExists(self): return self.fCacheExists

    def open(self):
        assert self.foIdx == None
        assert self.foTxt == None
        self.foIdx = open(self.idxFileName,"wb")
        self.foTxt = open(self.txtName, "wb")
        self.curTxtOffset = 0

    def write(self,article):
        title = article.getTitle()
        ns = article.getNamespace()
        assert ns == NS_MAIN
        if article.fRedirect():
            self.foIdx.write("%s%s\n" % (REDIRECT_MARK,title))
            self.foIdx.write("%s\n" % article.getRedirect())
        else:
            txt = article.getText()
            txtLen = len(txt)
            md5Hash = article.getHash()
            viewCount = article.getViewCount()

            self.foIdx.write("%s\n" % title)
            self.foIdx.write("%d,%d,%d,%s,%d\n" % (ns, self.curTxtOffset, txtLen, md5Hash,viewCount))
            self.curTxtOffset += txtLen
            self.foTxt.write(txt)

    def close(self):        
        if not self.foTxt:
            return
        self.foTxt.close()
        self.foIdx.close()

BEG_TXT = "INSERT INTO cur VALUES "

TOKEN_NONE = 0
TOKEN_LEFT_BRACE = 1
TOKEN_RIGHT_BRACE = 2
TOKEN_STR = 3
TOKEN_NUM = 4
TOKEN_COMMA = 5
TOKEN_FLOAT = 6
TOKEN_SEP = 7

def getTokenName(token):
    tokStr = ["TOKEN_NONE", "TOKEN_LEFT_BRACE", "TOKEN_RIGHT_BRACE", "TOKEN_STR", "TOKEN_NUM", "TOKEN_COMMA", "TOKEN_FLOAT", "TOKEN_SEP"]
    return tokStr[token]

# is this a token representing a value
def fTokenVal(token):
    if token == TOKEN_STR or token == TOKEN_NUM or token == TOKEN_FLOAT:
        return True
    return False

BUF_SIZE = 32768
class SQLTokenizer:
    def __init__(self,fileName):
        if arsutils.fIsBzipFile(fileName):
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
        # when there's a crash pasing, save the conent of the whole
        # buffer for further inspection
        fo = open("tokenizer_buf.txt", "wb")
        fo.write(self.buf)
        fo.close()

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
                # it's a reverse of mysql_sub_escape_string in libmysqld\libmysql.c
                if c =='\\':
                    c = self.getChar()
                    if c=='n':
                        txt += '\n'
                    elif c=='r':
                        txt += '\r'
                    elif c=='\\' or c=='\'' or c=='\"':
                        txt += c
                    elif c=='Z':
                        txt += chr(26)
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

def fIsRedirectLine(line):
    if len(line)>len(REDIRECT_MARK):
        if 0==line.find(REDIRECT_MARK):
            return True
    return False

def iterConvertedArticles(sqlFileName):
    fileName = getConvIdxFileName(sqlFileName)
    if not fConvertedCacheExists(sqlFileName):
        print "converted article cache '%s' doesn't exists" % fileName
        return
    print "getting articles from cache %s" % fileName
    fo = open(fileName,"rb")
    count = 0
    while True:
        title = fo.readline()
        if len(title)==0:
            break
        if fIsRedirectLine(title):
            redirect = fo.readline()
            if title == REDIRECT_MARK:
                #need this to remove stupid redirecto of 0xa0=>Space_(punctuation)
                print "title after stripping is equal to '%s' (REDIRECT_MARK), so skipping" % REDIRECT_MARK
                continue
            redirect = redirect[:-len(REDIRECT_MARK)]
            title = title.strip()
            if len(title)==0:
                print "title after stripping is empty string, so skipping '%s'" % redirect
                continue
            article = WikipediaArticleRedirect(title,redirect.strip())
        else:
            title = title.strip()
            line = fo.readline()
            if len(title)==0:
                print "title after stripping is empty string, so skipping '%s'" % line.strip()
                continue
            lineParts = line.split(",")
            ns = int(lineParts[0])
            assert ns==NS_MAIN
            txtOffset = int(lineParts[1])
            txtLen = int(lineParts[2])
            article = ConvertedArticleFromCache(sqlFileName,title,ns,txtOffset,txtLen)

        yield article
        count += 1
    fo.close()
    return    


# an iterator that given a *.sql or *.sql.bz2 wikipedia dump file
# returns WikpediaArticle instances representing one wikipedia article
# If fUseCache is True, then uses (if exists) or creates *.txt cache files
#TODO: split this into smaller functions. However, don't know how to do it
#  and be able to use yield as well
def iterWikipediaArticles(sqlFileName,limit=None,fUseCache=False,fRecreateCache=False):
    #if limit:
    #    assert fUseCache==False
    print "fUseCache %d, fRecreateCache=%d" % (fUseCache, fRecreateCache)
    cacheWriter = None
    fReallyUseCache = False
    if fRecreateCache:
        cacheWriter = ArticleCacheWriter(sqlFileName)
        cacheWriter.open()
    else:
        if fUseCache and fCacheExists(sqlFileName):
            fReallyUseCache = True
        else:
            cacheWriter = ArticleCacheWriter(sqlFileName)
            cacheWriter.open()

    if fReallyUseCache:
        fileName = getIdxFileName(sqlFileName)
        print "getting articles from cache %s" % fileName
        fo = open(fileName,"rb")
        count = 0
        while True:
            title = fo.readline()
            if len(title)==0:
                break
            if fIsRedirectLine(title):
                redirect = fo.readline()
                title = title.strip()
                if title == REDIRECT_MARK:
                    #need this to remove stupid redirecto of 0xa0=>Space_(punctuation)
                    print "title after stripping is equal to '%s' (REDIRECT_MARK), so skipping" % REDIRECT_MARK
                    continue
                title = title[len(REDIRECT_MARK):]
                if len(title)==0:
                    print "title after stripping is empty string, so skipping '%s'" % redirect
                    continue
                article = WikipediaArticleRedirect(title,redirect.strip())
            else:
                title = title.strip()
                line = fo.readline()
                if len(title)==0:
                    print "title after stripping is empty string, so skipping '%s'" % line.strip()
                    continue
                lineParts = line.split(",")
                #try:
                ns = int(lineParts[0])
                assert ns==NS_MAIN
                txtOffset = int(lineParts[1])
                txtLen = int(lineParts[2])
                md5Hash = lineParts[3]
                viewCount = int(lineParts[4])
                article = WikipediaArticleFromCache(sqlFileName,title,ns,txtOffset,txtLen,md5Hash,viewCount)
                #except:
                #    print "failed to parse as an article for title '%s'" % title

            yield article
            count += 1
            if limit and count > limit:
                break
        fo.close()
        return    

    st = SQLTokenizer(sqlFileName)
    fSkipped = st.fSkipUntilTxt(BEG_TXT)
    assert fSkipped
    curState = ST_NONE
    args = []
    count = 0
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
                # filter out all namespaces except NS_MAIN
                if NS_MAIN != int(args[CUR_NAMESPACE]):
                    args = []
                    continue
                if fInvalidRedirect(args):
                    print "rejected '%s' as invalid redirect" % args[CUR_TITLE].strip()
                    print args[CUR_TEXT].strip()
                    args = []
                    continue
                args[CUR_TITLE] = args[CUR_TITLE].strip()
                if 0==len(args[CUR_TITLE]):
                    # reject titles consisting only of spaces and newlines
                    print "rejected '%s' as invalid (len==0) title" % args[CUR_TEXT]
                    args = []
                    continue
                article = WikipediaArticleFromSql(args)
                if cacheWriter:
                    cacheWriter.write(article)
                yield article
                count += 1
                if limit and count >= limit:
                    break
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
                    # we found the end of file
                    break
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
    st.close()
    if cacheWriter:
        cacheWriter.close()

