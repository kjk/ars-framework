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
#   -limit n : only process first N rows
#   -nopsyco : if used, will NOT use psyco
#   -juststats : if used, doesn't write processed data, just calculates
#                and prints the statistics
#   fileName : which file to process

import sys,os,string,arsutils,wikipediasql
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

def usageAndExit():
    print "Usage: dumpSqlToTxt.py [-nopsyco] [-limit n] [-juststats] sqlDump"
    sys.exit(0)

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

        ns = article.getNamespace()
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

def convertFile(inName,limit,fJustStats=False):
    # if a limit is given we don't use cache (to avoid creating partial cache)
    fUseCache = True
    if limit or fJustStats:
        fUseCache = False

    print "fUseCache: %d" % fUseCache
    stats = WikipediaStats()
    count = 0
    timer = arsutils.Timer(fStart=True)
    for article in wikipediasql.iterWikipediaArticles(inName,limit=limit,fUseCache=fUseCache):
        #print sqlArgs
        stats.addStats(article)
        title = article.getTitle().strip()
        viewCount = article.getViewCount()
        #if article.fRedirect():
        #    txt = article.getText()
        #    print title
        #    print txt.strip()
        registerMostViewed(title,viewCount)
        count += 1
        if 0 == count % 2000:
            print "processed %d items" % count

    timer.stop()
    stats.dumpStats()
    dumpMostViewed()
    timer.dumpInfo()

if __name__=="__main__":
    limit = arsutils.getRemoveCmdArgInt("-limit")
    if limit:
        print "limit=%d" % limit
    fJustStats = arsutils.fDetectRemoveCmdFlag("-juststats")
    fNoPsyco = arsutils.fDetectRemoveCmdFlag("-nopsyco")
    if g_fPsycoAvailable and not fNoPsyco:
        print "using psyco"
        psyco.full()

    # now we should only have file name
    if len(sys.argv) != 2:
        usageAndExit()

    sqlDump = sys.argv[1]
    convertFile(sqlDump,limit,fJustStats)
