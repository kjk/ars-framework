# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Process wikipedia SQL dumps (*.sql or *.sql.bz2) and returns a
#   WikipediaArticle object representing articles

import sys,os,string,bz2

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

class WikipediaArticle:
    def __init__(self,row):
        self.row = row
    def getId(self):  return int(self.row[CUR_ID])
    def getNs(self): return int(self.row[CUR_NAMESPACE])
    def getTitle(self): return self.row[CUR_TITLE]
    def getText(self): return self.row[CUR_TEXT]
    def fRedirect(self):  return int(self.row[CUR_IS_REDIRECT])==1
    def getViewCount(self): return int(self.row[CUR_COUNTER])
    def getTimestamp(self): return self.row[CUR_TIMESTAMP]

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

def fIsBzipFile(inFileName):
    if len(inFileName)>4 and ".bz2" == inFileName[-4:]:
        return True
    return False

BUF_SIZE = 32768
class BigStr:
    def __init__(self,fileName):
        if fIsBzipFile(fileName):
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

# an iterator that given a *.sql or *.sql.bz2 wikipedia dump file
# returns WikpediaArticle instances representing one wikipedia article
def iterWikipediaArticles(sqlFileName):
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
                yield WikipediaArticle(args)
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

