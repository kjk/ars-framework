# Copyright: Krzysztof Kowalczyk
# Owner: Andrzej Ciarkowski
#
# Converts the original wiki database in enwiki.cur to the format used
# by iPedia server
#
# Command line parameters:
#  -verbose : if used will print a lot of debugging input to stdout. May slow
#             down conversion process
#  -one term : will force the conversion of one term. Is always verbose
#  -force : if used will force converting, even if entries already exist
#           (I'm not sure if the timestamp-based version of change detection really works anyway)
#  -limit N : limit the number of converter articles to N. This is good for testing
#             changes to the converter. It takes a lot of time to fetch all
#             original articles from enwiki db, this option will limit it to N
#             articles which should be enough to detect major problems with
#             the converter
# -showdups : sets g_fShowDups to True. The idea is that if we convert to an empty ipedia.definitions
#             database, we should never have duplicates. If we have it means that we're loosing
#             some of the original definitions. With this flag we dump dups to stdout.
#             Don't use if ipedia.definitions isn't empty
# -slow     : use the older, slower version of conversion. The code used by default
#             is optimized for bulk conversion to an empty ipedia database
#             but doesn't support -ts argument and has less reporting
# -ts timestamp : limit the rows we update to those that have timestamp greater than the one give
# -fromsql fileName : convert directly from sql file, no need for enwiki.cur database
# -usepsyco : if used, will use psyco for speeding up, false by default

import MySQLdb, sys, datetime, time
import wikipediasql,articleconvert
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
g_fShowDups      = False

# set by -force command line argument
g_fForceConvert  = False

# if True use the older, slower version
g_fSlowVersion   = False
g_connOne        = None
g_connTwo        = None
# set by -ts command line argument
g_timestamp      = None

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

# definition is cur_text from the table
# ts is cur_timestamp from the table
# this is a newer, faster versio but doesn't support some stuff from the old one
def convertTerm(cur_title,definition,ts):
    global g_fVerbose, g_fShowDups, g_connTwo

    ipedia_write_cur = getNamedCursor(g_connTwo, "ipedia_write_cur")

    term=cur_title.replace('_', ' ')
    timestamp=datetime.datetime(int(ts[0:4]), int(ts[4:6]), int(ts[6:8]), int(ts[8:10]), int(ts[10:12]), int(ts[12:14]))
    if g_fVerbose:
        log_txt = "term: %s " % term

    newDef = articleconvert.convertArticle(definition)
    #newDef = definition       # uncomment to insert unconverted definition

    try:
        ipedia_write_cur.execute("""INSERT INTO definitions (term, definition, last_modified) VALUES ('%s', '%s', '%s')""" % (dbEscape(term), dbEscape(newDef), dbEscape(str(timestamp))))
        if g_fVerbose:
            log_txt += "*New record"
    except:
        # assuming that the exception happend because of
        if g_fShowDups:
            print "dup: " + cur_title
        if g_fVerbose:
            log_txt += "Update existing record"
        #print "exception happened for %s" % term
        ipedia_write_cur.execute("""UPDATE definitions SET definition='%s', last_modified='%s' WHERE term='%s'""" % (dbEscape(newDef), dbEscape(str(timestamp)), dbEscape(term)))
    if g_fVerbose:
        print log_txt

# definition is cur_text from the table
# ts is cur_timestamp from the table
# this is the older, slower version
def convertTermSlow(cur_title,definition,ts):
    global g_fVerbose, g_fShowDups, g_connTwo, g_fForceConvert

    ipedia_write_cur = getNamedCursor(g_connTwo, "ipedia_write_cur")

    term=cur_title.replace('_', ' ')
    timestamp=datetime.datetime(int(ts[0:4]), int(ts[4:6]), int(ts[6:8]), int(ts[8:10]), int(ts[10:12]), int(ts[12:14]))
    if g_fVerbose:
        log_txt = "term: %s " % term

    check_cur = getNamedCursor(g_connTwo, "check_cur")
    check_cur.execute("""SELECT id, last_modified FROM definitions WHERE term='%s'""" % dbEscape(term))
    outRow=check_cur.fetchone()
    if outRow:
        if g_fShowDups:
            print "dup: " + cur_title
        if g_fVerbose:
            log_txt +=  "cur:" + outRow[1] + " new: " + str(timestamp)
        if g_fForceConvert or str(outRow[1])<str(timestamp):
            if g_fVerbose:
                log_txt += " Update existing record id: " + str(outRow[0])
            termId = outRow[0]
            newDef = convertDefinition(definition)
            ipedia_write_cur.execute("""UPDATE definitions SET definition='%s', last_modified='%s' WHERE id=%d""" % (dbEscape(newDef), dbEscape(str(timestamp)), termId))
        else:
            if g_fVerbose:
                log_txt += " Skipped record, newer version exists"
    else:
        if g_fVerbose:
            log_txt += "*New record"
        newDef = convertDefinition(definition)
        ipedia_write_cur.execute("""INSERT INTO definitions (term, definition, last_modified) VALUES ('%s', '%s', '%s')""" % (dbEscape(term), dbEscape(newDef), dbEscape(str(timestamp))))

    if g_fVerbose:
        print log_txt

def convertAll(articleLimit):
    global g_connOne, g_fSlowVersion

    rowCount = 0
    rowsLeft = getEnwikiRowCount()
    rowsPerQuery = 500
    curOffset = 0
    cursor = getNamedCursor(g_connOne,"all_cur_ids")
    while True:
        query="""SELECT cur_title,cur_text,cur_timestamp FROM cur WHERE cur_namespace=0"""
        if g_timestamp:
            query+=""" AND cur_timestamp>'%s'""" % dbEscape(sys.argv[1])
        query+=""" LIMIT %d,%d""" % (curOffset, rowsPerQuery)
        #print query
        cursor.execute(query)
        processedInOneFetch = 0
        for row in cursor.fetchall():
            if g_fSlowVersion:
                convertTermSlow(row[0],row[1],row[2])
            else:
                convertTerm(row[0],row[1],row[2])
            rowCount += 1
            rowsLeft -= 1
            processedInOneFetch += 1
            if 0 == rowCount % 500:
                sys.stderr.write("processed %d rows, left %d, last term=%s\n" % (rowCount,rowsLeft,row[0]))
            if articleLimit and rowCount >= articleLimit:
                return
        if 0 == processedInOneFetch:
            break
        curOffset += rowsPerQuery

NS_MAIN = 0
def convertAllFromSQL(fileName,articleLimit):
    count = 0
    for article in wikipediasql.iterWikipediaArticles(fileName):
        # we only convert article from the main namespace
        if article.getNs() != NS_MAIN
            continue
        title = article.getTitle()
        txt = article.getText()
        timestamp = article.getNs()
        convertTerm(title,txt,timestamp)
        count += 1
        if 0 == count % 500:
            sys.stderr.write("processed %d rows, last term=%s\n" % (count,title))
        if articleLimit and count >= articleLimit:
            return

def convertOneTerm(term):
    global g_connOne
    query="""SELECT cur_title,cur_text,cur_timestamp FROM cur WHERE cur_title='%s' AND cur_namespace=0""" % dbEscape(term)
    cursor=g_connOne.cursor()
    cursor.execute(query)
    row=cursor.fetchone()
    cursor.close()
    if not row:
        print "Didn't find a row in enwiki.cur with cur_title='%s'" % dbEscape(term)
    else:
        termId = row[0]
        convertTerm(row[0],row[1],row[2])

def fDetectRemoveCmdFlag(flag):
    fFlagPresent = False
    try:
        pos = sys.argv.index(flag)
        fFlagPresent = True
        sys.argv[pos:pos+1] = []
    except:
        pass
    return fFlagPresent

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

g_startTime = None
g_endTime = None

def startTiming():
    global g_startTime
    g_startTime = time.clock()

def endTiming():
    global g_endTime
    g_endTime = time.clock()

def dumpTimingInfo():
    global g_startTime, g_endTime
    dur = g_endTime - g_startTime
    str = "duration %f seconds\n" % dur
    sys.stderr.write(str)

if __name__=="__main__":

    fUsePsyco = fDetectRemoveCmdFlag("-usepsyco")
    if g_fPsycoAvailable and fUsePsyco:
        print "using psyco"
        psyco.full()

    initDatabase()

    g_fForceConvert = fDetectRemoveCmdFlag( "-force" )
    g_fVerbose = fDetectRemoveCmdFlag( "-verbose" )
    g_fShowDups = fDetectRemoveCmdFlag( "-showdups" )
    g_timestamp = getRemoveCmdArg( "-ts" )
    g_fSlowVersion = fDetectRemoveCmdFlag( "-slow" )

    if g_timestamp and not g_fSlowVersion:
        print "-ts only supported if -slow is used"
        sys.exit(0)

    #print "g_fShowDups=%d" % g_fShowDups
    articleLimit = getRemoveCmdArg("-limit")
    if articleLimit:
        articleLimit = int(articleLimit)

    fromSql = getRemoveCmdArg("-fromsql")
    termToConvert = getRemoveCmdArg("-one")
    startTiming()
    if None!=termToConvert:
        g_fVerbose = True
        convertOneTerm(termToConvert)
    elif None != fromSql:
        convertAllFromSQL(fromSql,articleLimit)
    else:
        convertAll(articleLimit)
    endTiming()
    deinitDatabase()
    dumpTimingInfo()

