# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Dumps the original enwiki database content to stdout
# Only dump the title (prepended with ^ for easy browsing in
# text editor) and the text
#
# Todo:
#  * add ability to dump the original and converted to 2 files
#    so that we can use a diffing tool to compare them and see
#    changes (e.g. using -orig orig_dump.txt -converted converted_dump.txt)
#    Those should be good stress tool for diffing tools.
import MySQLdb, sys, datetime, re, unicodedata

g_conn        = None

def getOneResult(conn,query):
    cur = conn.cursor()
    cur.execute(query)
    row = cur.fetchone()
    res = row[0]
    cur.close()
    return res

g_enwikiRowCount = None
def getEnwikiRowCount():
    global g_conn, g_enwikiRowCount
    if None == g_enwikiRowCount:
        g_enwikiRowCount = getOneResult(g_conn, """SELECT COUNT(*) FROM enwiki.cur WHERE cur_namespace=0;""")
    return g_enwikiRowCount

# creates all the global stuff that will be used by other functions
# using globals is ugly but some of it is for performance
# Q: would using server-side cursor speed up things?
# g_connOne = MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='enwiki', cursorclass=MySQLdb.cursors.SSCursor)
def initDatabase():
    global g_conn
    g_conn = MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='enwiki')
    enwikiRows = getEnwikiRowCount()

def deinitDatabase():
    global g_conn
    closeAllNamedCursors()
    g_conn.close()

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
    global g_conn
    return g_conn.escape_string(txt)

# dump all articles to a file. if articleLimit != None
# limit the number of articles to dump
def dumpAll(articleLimit):
    global g_conn

    rowCount = 0
    rowsLeft = getEnwikiRowCount()
    rowsPerQuery = 500
    curOffset = 0
    cursor = getNamedCursor(g_conn,"all_cur_ids")
    while True:
        query="""SELECT cur_title,cur_text,cur_timestamp FROM cur WHERE cur_namespace=0"""
        if len(sys.argv)>1:
            query+=""" AND cur_timestamp>'%s'""" % dbEscape(sys.argv[1])
        query+=""" LIMIT %d,%d""" % (curOffset, rowsPerQuery)
        #print query
        cursor.execute(query)
        processedInOneFetch = 0
        for row in cursor.fetchall():
            print "^" + row[0]
            print row[1]
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

if __name__=="__main__":

    initDatabase()
    limit = getRemoveCmdArg("-limit")
    if limit:
        limit = int(limit)
    dumpAll(limit)
    deinitDatabase()

