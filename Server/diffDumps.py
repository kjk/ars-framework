# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Calculates the diff between two wikipedia dumps. The diff is:
#    * which articles have been added
#    * which articles have been deleted
#    * which articles have changed
#  Creates an *.sql file that we can import directly to
#
# Usage:
#   diffDumps.py dumpOne dumpTwo
#
# TODO:
#  - generate the diff sql file
#  - put the code common with diffConvert.py into a separate file
#
import sys,os,os.path,string,re,random,time,md5,bz2,dumpSqlToTxt
try:
    import psyco
    psyco.full()
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"

def usageAndExit():
    print "Usage: diffDumps.py dumpFileOne dumpFileTwo"
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

def getIdxFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_idx.txt")

def getRedirectsFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_redirects.txt")

def getBodyFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_body.txt")

def getTxt(sqlFileName,txtOff,txtLen):
    fn = getBodyFileName(sqlFileName)
    fo = open(fn,"rb")
    fo.seek(txtOff)
    txt = fo.read(txtLen)
    fo.close()
    return txt

class ArticleInfo:
    def __init__(self,sqlFileName,title,ns,txtOffset,txtLen,md5Hash):
        self.sqlFileName = sqlFileName
        self.title = title
        self.ns = ns
        self.txtOffset = txtOffset
        self.txtLen = txtLen
        self.md5Hash = md5Hash
        self.md5Hash2 = None
    def getTitle(self): return self.title
    def getNs(self): return self.ns
    def getHash(self): return self.md5Hash
    def getTxt(self): return getTxt(self.sqlFileName, self.txtOffset, self.txtLen)
    def setHash2(self,md5Hash2): self.md5Hash2 = md5Hash2
    def getHash2(self): return self.md5Hash2

NS_MAIN = 0

def iterArticles(sqlFileName,fSkipNonMain=True):
    fileName = getIdxFileName(sqlFileName)
    fo = open(fileName,"rb")
    while True:
        title = fo.readline()
        if len(title)==0:
            break
        #print "Title: %s" % title
        dataTxt = fo.readline()
        #print "data: %s" % dataTxt
        dataTuple = dataTxt.split(",")
        ns = int(dataTuple[0])
        if fSkipNonMain and ns != NS_MAIN:
            continue
        txtOffset = int(dataTuple[1])
        txtLen = int(dataTuple[2])
        md5Hash = dataTuple[3]
        article = ArticleInfo(sqlFileName,title,ns,txtOffset,txtLen,md5Hash)
        yield article
    fo.close()

def calcDiff(dumpOne,dumpTwo):
    dumpSqlToTxt.convertFile(dumpOne,9999999,fJustStats=False,fSkipIfExists=True)
    dumpSqlToTxt.convertFile(dumpTwo,9999999,fJustStats=False,fSkipIfExists=True)
    articlesFromDumpOne = {}
    newArticles = {}
    count = 0
    for article in iterArticles(dumpOne):
        articlesFromDumpOne[article.getTitle()] = article
        count += 1
        if count % 5000 == 0:
            print "processed %d articles from dump one" % count

    # now calculate differences
    for article in iterArticles(dumpTwo):
        count += 1
        if count % 5000 == 0:
            print "processed %d articles from dump two" % count
        if not articlesFromDumpOne.has_key(article.getTitle()):
            # this is a new article
            newArticles[article.getTitle()] = article
            continue
        if articlesFromDumpOne[article.getTitle()].md5Hash == article.md5Hash:
            # those are the same, so we don't have to remember it
            del articlesFromDumpOne[article.getTitle()]
            continue
        articlesFromDumpOne[article.getTitle()].setHash2(article.md5Hash)
    newArticlesCount = len(newArticles)
    deletedArticlesCount = 0
    changedArticlesCount = 0
    for article in articlesFromDumpOne.values():
        if None == article.getHash2():
            # this is a deleted article
            deletedArticlesCount += 1
        else:
            changedArticlesCount += 1
    print "new:     %d" % newArticlesCount
    print "deleted: %d" % deletedArticlesCount
    print "changed: %d" % changedArticlesCount

if __name__=="__main__":
    # we should have 2 file names
    if len(sys.argv) != 3:
        usageAndExit()
    dumpOne = sys.argv[1]
    dumpTwo = sys.argv[2]
    if dumpOne > dumpTwo:
        (dumpTwo,dumpOne) = (dumpOne,dumpTwo)
        print "switched names"
        print "dumpOne: %s" % dumpOne
        print "dumpTwo: %s" % dumpTwo

    calcDiff(dumpOne,dumpTwo)

