# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Convert the *.sql to *.txt flat files in the format:
#   ${name}_idx.txt:
#     title
#     $namespace,$body_offset,$body_len,$md5_hash
#
#   ${name}_body.txt:
#     body 
#
#   ${name}_redirects.txt:
#     title
#     title_to_which_redirects (must be in ${name}_idx.txt file
#
# Usage:
#   -limit N : only process first N rows
#   -usepsyco : if used, will use psyco for speeding up, false by default
#   -juststats : if used, doesn't write processed data, just calculates
#                and prints the statistics
#   -force : recreate the files even if they exist
#   fileName : which file to process

import sys,os,os.path,string,time,md5,bz2,wikipediasql
try:
    import psyco
    g_fPsycoAvailable = True
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"
    g_fPsycoAvailable = False

MOST_VIEWED_COUNT = 25

g_mostViewed = {}
g_mostViewedSmallest = None

def getSmallestCount(dictViewed):
    smallest = None
    for count in dictViewed.values():
        if smallest == None:
            smallest = count
        else:
            if count < smallest:
                smallest = count
    return smallest

def removeSmallest(dictViewed):
    smallestKey = None
    smallestVal = None
    for (key,val) in dictViewed.items():
        if smallestKey == None:
            smallestKey = key
            smallestVal = val
        elif val < smallestVal:
            smallestKey = key
            smallestVal = val
    del dictViewed[smallestKey]

def registerMostViewed(title,viewCount):
    global g_mostViewedSmallest, g_mostViewed
    if len(g_mostViewed) < MOST_VIEWED_COUNT:
        g_mostViewed[title] = viewCount
        g_mostViewedSmallest = getSmallestCount(g_mostViewed)
        return
    if viewCount <= g_mostViewedSmallest:
        return
    removeSmallest(g_mostViewed)
    assert len(g_mostViewed)==MOST_VIEWED_COUNT-1
    g_mostViewed[title] = viewCount

def cmp_kv(a, b):
    return cmp(b[1], a[1])

def dumpMostViewed():
    global g_mostViewed
    l = g_mostViewed.items()
    l.sort(cmp_kv)
    print "Most viewed articles:"
    for (key,val) in l:
        print "%9d %s" % (val,key)

def fFileExists(filePath):
    try:
        st = os.stat(filePath)
    except OSError:
        # TODO: should check that Errno is 2
        return False
    return True

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
    print "Usage: dumpSqlToTxt.py [-limit N] [-usepsyco] [-juststats] [-force] fileName"
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
class WikipediaStats:
    def __init__(self):
        self.nsStats = []
        for t in range(NS_COUNT):
            self.nsStats.append(getNsInfo())

    def addStats(self,article):

        ns = article.getNs()
        txt = article.getText()

        self.nsStats[ns][NS_ARTICLES_COUNT] += 1
        self.nsStats[ns][NS_TOTAL_ARTICLES_SIZE] += len(txt)
        if article.fRedirect():
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

def convertFile(inName,limit,fJustStats=False,fSkipIfExists=False):
    if not fJustStats:
        txtName = getBodyFileName(inName)
        idxFileName = getIdxFileName(inName)
        redirectsFileName = getRedirectsFileName(inName)

        if fFileExists(txtName) and fFileExists(idxFileName) and fFileExists(redirectsFileName):
            if fSkipIfExists:
                print "files exist and fSkipIfExists==True so skipping creation"
                return
            else:
                print "files exist bug fSkipIfExists==False so create anyway"

        foIdx = open(idxFileName,"wb")
        foTxt = open(txtName, "wb")
        foRedirects = open(redirectsFileName, "wb")
    stats = WikipediaStats()
    count = 0
    startTiming()
    curPos = 0
    for article in wikipediasql.iterWikipediaArticles(inName):
        #print sqlArgs
        stats.addStats(article)
        title = article.getTitle().strip()
        viewCount = article.getViewCount()
        registerMostViewed(title,viewCount)
        if not fJustStats:
            ns = article.getNs()
            txt = article.getText()

            if article.fRedirect():
                foRedirects.write("%s\n" % title)
                foRedirects.write("%s\n" % txt)
            else:
                if 0==txt.find("#REDIRECT [["):
                    print "%s is a REDIRECT without being marked as such" % txt
                txtLen = len(txt)
                md5Hash = md5.new(txt)

                foIdx.write("%s\n" % title)
                foIdx.write("%d,%d,%d,%s\n" % (ns, curPos, txtLen, md5Hash.hexdigest()))
                curPos += txtLen
                foTxt.write(txt)

        count += 1
        if 0 == count % 2000:
            print "processed %d items" % count
        if count>=limit:
            break
    if not fJustStats:
        foTxt.close()
        foIdx.close()
        foRedirects.close()
    endTiming()
    stats.dumpStats()
    dumpMostViewed()
    dumpTiming()

if __name__=="__main__":
    limit = getRemoveCmdArg("-limit")
    if None == limit:
        limit = 9999999 # very big number
    else:
        limit = int(limit)
    #print "limit=%d" % limit
    fJustStats = fDetectRemoveCmdFlag("-juststats")

    fForce = fDetectRemoveCmdFlag("-force")
    fSkipIfExists = not fForce

    fUsePsyco = fDetectRemoveCmdFlag("-usepsyco")
    if g_fPsycoAvailable and fUsePsyco:
        print "using psyco"
        psyco.full()

    # now we should only have file name
    if len(sys.argv) != 2:
        usageAndExit()

    fileName = sys.argv[1]
    convertFile(fileName,limit,fJustStats,fSkipIfExists)
