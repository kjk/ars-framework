# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Shows the result of conversion a given wikipedia article (or a random
#   article) using graphical diff (e.g. windiff)
#
#   The idea is that we can use this script to visually inspect how a given
#   article is being converted by our conversion script.
#
#   Currently supports diffing using
#    - windiff (Microsoft tool that can be freely downloaded and usually comes with VisualStudio)
#    - AraxisMerge : commercial tool
#    - WinMerge : free, http://winmerge.sourceforge.net/
#   Adding support for others is trivial (see diffWithWindiff() etc.)
#
# Usage:
#   -limit N : only process first N rows
#   -random : show a random word
#   -title $title : show diff of a given title
#   -dump : dump before and after to stdout
#   fileName - which sql file to process
#
# TODO:
#  - when using -title and doesn't find the aritcle in main, should also check
#    *_redirects.txt file
#
import sys,os,os.path,string,re,random,time,md5,bz2
import arsutils,wikipediasql,articleconvert
try:
    import psyco
    psyco.full()
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"

def usageAndExit():
    print "Usage: diffConvert.py [-limit N] [-random] [-title foo] fileName"
    sys.exit(0)

# algorithm based on http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/59865
# a bit slow since it must read the whole file
def getRandomArticle(fileName):
    articleNum = 0
    randomArticle = None
    for article in wikipediasql.iterWikipediaArticles(fileName,None,fUseCache=True,fRecreateCache=False):
        if article.fRedirect():
            continue
        articleNum += 1
        # How likely is it that this is the last line of the file ?
        # kjk note: I don't get it
        if random.uniform(0,articleNum)<1:
            randomArticle = article
        if articleNum % 30000 == 0:
            print "processed %d articles" % articleNum
    return randomArticle

def showDiffRandom(fileName):
    # at this point shows the diff of the first article
    article = getRandomArticle(fileName)
    title = article.getTitle()
    convertedArticle = None
    if wikipediasql.fConvertedCacheExists(fileName):
        convertedArticle = findConvertedArticle(fileName,title)

    if not convertedArticle:
        print "didn't find article '%s' in the converted cache" % title
    origTxt = article.getText()
    origTxt = arsutils.normalizeNewlines(origTxt)
    if convertedArticle:
        converted = arsutils.normalizeNewlines(convertedArticle.getText())
        arsutils.showTxtDiff(origTxt, converted)
    else:
        converted = articleconvert.convertArticle(article.getTitle(), article.getText())
        converted = arsutils.normalizeNewlines(converted)
        arsutils.showTxtDiff(origTxt,converted)


# TODO: doesn't handle redirects
def findOrigArticle(fileName,title):
    titleLower = title.lower()
    print "looking for article with title %s" % titleLower
    count = 0
    for article in wikipediasql.iterWikipediaArticles(fileName,None,fUseCache=True,fRecreateCache=False):
        #if article.getTitle().lower() == titleLower:
        title = article.getTitle().lower()
        title = title.replace("_", " ")
        if 0 == title.find(titleLower):
            return article
        if count % 2000 == 0:
            print "processed %d articles, last title %s" % (count,title)
        count += 1
    return None

def findConvertedArticle(fileName,title):
    titleLower = title.lower()
    print "looking for converted article with title '%s'" % titleLower
    count = 0
    for article in wikipediasql.iterConvertedArticles(fileName):
        title = article.getTitle().lower()
        if 0 == title.find(titleLower):
            return article
        if count % 2000 == 0:
            print "processed %d articles, last title %s" % (count,title)
        count += 1
    return None

def iterArticlesExactTitle(fileName,title):
    #title = string.lower(title.replace(" ", "_"))
    #title = string.lower(title.replace(" ", "_"))
    titleLower = title.lower()
    print "looking for article with title %s" % titleLower
    count = 0
    for article in iterArticles(fileName):
        title = article.getTitle().lower().strip()
        #if 0 == title.find(titleLower):
        if title == titleLower:
            yield article
        if count % 1000 == 0:
            print "processed %d articles, last title %s" % (count,article.getTitle().lower().strip())
        count += 1

def showDiffTitle(fileName,title):
    article = findOrigArticle(fileName,title)
    if not article:
        print "couldn't find article with the title %s" % title
        return

    convertedArticle = None
    if wikipediasql.fConvertedCacheExists(fileName):
        convertedArticle = findConvertedArticle(fileName,title)

    if not convertedArticle:
        print "didn't find article '%s' in the converted cache" % title
    origTxt = article.getText()
    origTxt = arsutils.normalizeNewlines(origTxt)
    if convertedArticle:
        converted = arsutils.normalizeNewlines(convertedArticle.getText())
        arsutils.showTxtDiff(origTxt, converted)
    else:
        converted = articleconvert.convertArticle(article.getTitle(), article.getText())
        converted = arsutils.normalizeNewlines(converted)
        arsutils.showTxtDiff(origTxt,converted)

def dumpArticle(fileName,title):
    for article in iterArticlesExactTitle(fileName,title):
        if not article:
            print "couldn't find the body of article %s" % title
            return
        title = article.getTitle().strip() + "\n"
        txt = article.getTxt()
        converted = articleconvert.convertArticle(title,txt)
        print "TITLE: %s" % title
        print "ORIGINAL: %s" % txt
        print "CONVERTED: %s" % converted
        return

if __name__=="__main__":
    limit = arsutils.getRemoveCmdArgInt("-limit")
    if None == limit:
        limit = 9999999 # very big number

    fRandom = arsutils.fDetectRemoveCmdFlag("-random")
    fDump = arsutils.fDetectRemoveCmdFlag("-dump")

    if fDump and fRandom:
        print "Can't use -dump and -random at the same time"
        usageAndExit()

    title = arsutils.getRemoveCmdArg("-title")

    if title and fRandom:
        print "Can't use -title and -random at the same time"
        usageAndExit()

    if not title and not fRandom:
        print "Have to provide either -title or -random"
        usageAndExit()

    # now we should only have file name
    if len(sys.argv) != 2:
        print "Have to provide *.sql or *.sql.bz2 file name with wikipedia dump"
        usageAndExit()
    fileName = sys.argv[1]

    if fRandom:
        showDiffRandom(fileName)
    else:
        assert title
        if fDump:
            dumpArticle(fileName,title)
        else:
            showDiffTitle(fileName,title)

