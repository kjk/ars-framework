# Copyright: Krzysztof Kowalczyk
# Owner: Andrzej Ciarkowski
#
# Creates iPedia database from wikipedia sql dump
#
# Command line parameters:
#  -verbose : if used will print a lot of debugging input to stdout. May slow
#             down conversion process
#  -limit n : limit the number of converter articles to N. This is good for testing
#             changes to the converter. It takes a lot of time to fetch all
#             original articles from enwiki db, this option will limit it to N
#             articles which should be enough to detect major problems with
#             the converter
# -showdups : sets g_fShowDups to True. The idea is that if we convert to an empty ipedia.definitions
#             database, we should never have duplicates. If we have it means that we're loosing
#             some of the original definitions. With this flag we dump dups to stdout.
#             Don't use if ipedia.definitions isn't empty
# -nopsyco : if used, won't use psyco
# fileName : convert directly from sql file, no need for enwiki.cur database

import sys, MySQLdb, arsutils
import wikipediasql,articleconvert, iPediaDatabase
try:
    import psyco
    g_fPsycoAvailable = True
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"
    g_fPsycoAvailable = False

# if True, we'll print a lot of debug text to stdout
g_fVerbose       = False

# if True, we'll show if we're updating ipedia table twice for the same term
# shouldn't happen if it was empty to begin with
#g_fShowDups      = False

g_connOne        = None
g_connTwo        = None

def usageAndExit():
    print "newConvert.py [-verbose] [-limit n] [-showdups] [-nopsyco] sqlDumpName"
    sys.exit(0)

def getOneResult(conn,query):
    cur = conn.cursor()
    cur.execute(query)
    row = cur.fetchone()
    res = row[0]
    cur.close()
    return res

g_enwikiRowCount = None
def getEnwikiRowCount():
    global g_connOne, g_enwikiRowCount
    if None == g_enwikiRowCount:
        g_enwikiRowCount = getOneResult(g_connOne, """SELECT COUNT(*) FROM enwiki.cur WHERE cur_namespace=0;""")
    return g_enwikiRowCount

g_ipediaRowCount = None
def getIpediaRowCount():
    global g_connOne, g_ipediaRowCount
    if None == g_ipediaRowCount:
        g_ipediaRowCount = getOneResult(g_connOne, """SELECT COUNT(*) FROM ipedia.definitions;""")
    return g_ipediaRowCount

# creates all the global stuff that will be used by other functions
# using globals is ugly but some of it is for performance
# Q: would using server-side cursor speed up things?
# g_connOne = MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='enwiki', cursorclass=MySQLdb.cursors.SSCursor)
def initDatabase():
    global g_connOne, g_connTwo
    # the assumption here is that using two connections will speed up things
    g_connOne = MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='enwiki')
    g_connTwo = MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='ipedia')
    enwikiRows = getEnwikiRowCount()
    ipediaRows = getIpediaRowCount()
    sys.stderr.write("rows in enwiki: %d\n" % enwikiRows)
    sys.stderr.write("rows in ipedia: %d\n" % ipediaRows)

def deinitDatabase():
    global g_connOne, g_connTwo
    closeAllNamedCursors()
    g_connOne.close()
    g_connTwo.close()

g_namedCursors = {}
def getNamedCursor(conn,curName):
    global g_namedCursors
    # a little pooling of cursors based on their names
    # the idea is to save time by not calling conn.cursor()/conn.close()
    if not g_namedCursors.has_key(curName):
        g_namedCursors[curName] = conn.cursor()
    return g_namedCursors[curName]

def closeNamedCursor(curName):
    global g_namedCursors
    if g_namedCursors.has_key(curName):
        cur = g_namedCursors[curName]
        cur.close()
        del g_namedCursors[curName]

def closeAllNamedCursors():
    global g_namedCursors
    for curName in g_namedCursors.keys():
        cur = g_namedCursors[curName]
        cur.close()
    g_namedCursors.clear()

def dbEscape(txt):
    # it's silly that we need connection just for escaping strings
    global g_connTwo
    return g_connTwo.escape_string(txt)

def resolveRedirect(cur_redirect, allRedirects, allArticles):
    # check for (hopefuly frequent) case: this a valid, direct
    # redirect
    if allArticles.has_key(cur_redirect):
        return cur_redirect
    visited = []
    while True:
        if allArticles.has_key(cur_redirect):
            # this points to a valid article, so we resolved the redirect
            return cur_redirect
        if allRedirects.has_key(cur_redirect):
            visited.append(cur_redirect)
            cur_redirect = allRedirects[cur_redirect]
            if cur_redirect in visited:
                print "found circular redirect: %s" % cur_redirect
                return None
        return None

# First pass: go over all articles, either directly from
# sql dump or from cache and gather the following cache
# data:
#   - all redirects
#   - extract link from articles
# Second pass: do the conversion, including veryfing links and put
# converted articles in the database.
def convertArticles(sqlDump,articleLimit):
    count = 0
    redirects = {}
    articlesLinks = {}
    fUseCache = False
    fRecreateCache = True   # mostly for testing
    for article in wikipediasql.iterWikipediaArticles(sqlDump,articleLimit,fUseCache,fRecreateCache):
        # we only convert article from the main namespace
        assert article.getNamespace() == wikipediasql.NS_MAIN
        title = article.getTitle()
        if article.fRedirect():
            redirects[title] = article.getRedirect()
        else:
            txt = article.getText()
            links = articleconvert.articleExtractLinks(txt)
            #articlesLinks[title] = links
            articlesLinks[title] = 1
        count += 1
        if 0 == count % 1000:
            sys.stderr.write("processed %d rows, last term=%s\n" % (count,title))
        if articleLimit and count >= articleLimit:
            break
    # verify redirects
    print "Number of redirects: %d" % len(redirects)
    print "Number of real articles: %d" % len(articlesLinks)
    unresolvedCount = 0
    for (title,redirect) in redirects.items():
        redirectResolved = resolveRedirect(redirect,redirects,articlesLinks)
        if None == redirectResolved:
            unresolvedCount +=1
            #print "redirect '%s' (to '%s') not resolved" % (title,redirect)
    print "Number of unresolved redirects: %d" % unresolvedCount

    #ipedia_write_cur = getNamedCursor(g_connTwo, "ipedia_write_cur")
        
    # go over articles again (hopefully now using the cache),
    # convert them to a destination format (including removing invalid links)
    # and insert into a database
    count = 0
    for article in wikipediasql.iterWikipediaArticles(sqlDump,articleLimit,True,False):
        if article.fRedirect():
            # what do I do now?
            pass
        else:
            title = article.getTitle()
            txt = article.getText()
            converted = articleconvert.convertArticle(title,txt)
            noLinks = articleconvert.removeInvalidLinks(converted,redirects, articlesLinks)
            if noLinks:
                converted = noLinks

        count += 1
        if count % 1000 == 0:
            sys.stderr.write("phase 2 processed %d" % count)
        #term=cur_title.replace('_', ' ')
        #timestamp=datetime.datetime(int(ts[0:4]), int(ts[4:6]), int(ts[6:8]), int(ts[8:10]), int(ts[10:12]), int(ts[12:14]))
        #if g_fVerbose:
        #    log_txt = "term: %s " % term

        #newDef = articleconvert.convertArticle(term, definition)
        #newDef = definition       # uncomment to insert unconverted definition

        #try:
        #    ipedia_write_cur.execute("""INSERT INTO definitions (term, definition, last_modified) VALUES ('%s', '%s', '%s')""" % (dbEscape(term), dbEscape(newDef), dbEscape(str(timestamp))))
        #    if g_fVerbose:
        #        log_txt += "*New record"
        #except:
            # assuming that the exception happend because of
        #    if g_fShowDups:
        #        print "dup: " + cur_title
        #    if g_fVerbose:
        #        log_txt += "Update existing record"
            #print "exception happened for %s" % term
        #    ipedia_write_cur.execute("""UPDATE definitions SET definition='%s', last_modified='%s' WHERE term='%s'""" % (dbEscape(newDef), dbEscape(str(timestamp)), dbEscape(term)))

if __name__=="__main__":

    fNoPsyco = arsutils.fDetectRemoveCmdFlag("-nopsyco")
    if g_fPsycoAvailable and not fNoPsyco:
        print "using psyco"
        psyco.full()

    g_fVerbose = arsutils.fDetectRemoveCmdFlag("-verbose")
    #iPediaDatabase.g_fVerbose=g_fVerbose
    #g_fShowDups = arsutils.fDetectRemoveCmdFlag("-showdups")
    articleLimit = arsutils.getRemoveCmdArgInt("-limit")

    if len(sys.argv) != 2:
        usageAndExit()
    sqlDump = sys.argv[1]

    timer = arsutils.Timer(fStart=True)
    convertArticles(sqlDump,articleLimit)
    timer.stop()
    timer.dumpInfo()

