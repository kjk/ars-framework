# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Dumps the original enwiki database content to stdout
# Only dump the title (prepended with ^ for easy browsing in
# text editor) and the text
#
# Usage:
#  -limit N : limits the number of articles to convert
#
#  If no more arguments given, will dump the data to stdout
#
#  -orig fielName : name of the file to which write the original data
#  -converted fileName : name of the file to which write converted data
#  -split N : if given, write to N files, naming them $BASE-nn.$EXT e.g.
#             if "-orig foo.txt" and N is 3, write to 3 files "foo-01.txt",
#             "foo-02.txt" and "foo-03.txt".
#             The idea is that we want to compare those files using windiff or
#             some other diffing program but they don't handle the files as big
#             as we produce (>300 MB)
import sys, string, re, datetime, unicodedata, arsutils, MySQLdb, converter

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
    rowsPerQuery = 2500
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
            if 0 == rowCount % 1000:
                sys.stderr.write("processed %d rows, left %d, last term=%s\n" % (rowCount,rowsLeft,row[0]))
            if articleLimit and rowCount >= articleLimit:
                return
        if 0 == processedInOneFetch:
            break
        curOffset += rowsPerQuery

def genSplitFileName(nameBase, num):
    if None == num:
        return nameBase
    (name,ext) = string.split(nameBase,".")
    result = "%s-%02d.%s" % (name,num,ext)
    #print "result: %s" % result
    return result

def testGenSplitFileName():
    assert genSplitFileName("test.txt",None) == "test.txt"
    assert genSplitFileName("test.txt",1) == "test-01.txt"
    assert genSplitFileName("foo.bar",11) == "foo-11.bar"

g_foOrig = None
g_foConverted = None
def createFiles(fileOrig,fileConverted,num):
    global g_foOrig, g_foConverted
    g_foOrig = open( genSplitFileName(fileOrig,num), "wb")
    g_foConverted = open( genSplitFileName(fileConverted,num), "wb")

def closeFiles():
    global g_foOrig, g_foConverted
    g_foOrig.close()
    g_foConverted.close()


# dump all articles to a file. if articleLimit != None
# limit the number of articles to dump
# fileOrig and fileConverted are the names of files where to write original
# and converted content of the enwiki database
# if splitPartsCount is give, write to that many files
def dumpAllWithConverted(articleLimit,fileOrig,fileConverted,splitPartsCount):
    global g_conn, g_foOrig, g_foConverted

    assert fileOrig
    assert fileConverted

    rowCount = 0
    rowsLeft = getEnwikiRowCount()
    if articleLimit and rowsLeft > articleLimit:
        rowsLeft = articleLimit
    curPartNo = None
    splitAfter = None
    if splitPartsCount:
        splitAfter = rowsLeft / splitPartsCount
        curPartNo = 1

    createFiles(fileOrig, fileConverted, curPartNo)
    rowsPerQuery = 2500
    curOffset = 0
    cursor = getNamedCursor(g_conn,"all_cur_ids")
    while True:
        query="""SELECT cur_title,cur_text,cur_timestamp FROM cur WHERE cur_namespace=0"""
        query+=""" LIMIT %d,%d""" % (curOffset, rowsPerQuery)
        #print query
        cursor.execute(query)
        processedInOneFetch = 0
        for row in cursor.fetchall():
            title = "^" + row[0] + "\n"
            cur_txt = row[1]
            g_foOrig.write(title)
            g_foConverted.write(title)

            g_foOrig.write(cur_txt)
            g_foOrig.write("\n")
            converted = converter.convertDefinition(cur_txt)
            g_foConverted.write(converted)
            g_foConverted.write("\n")
            rowCount += 1
            rowsLeft -= 1
            processedInOneFetch += 1
            if 0 == rowCount % 1000:
                sys.stderr.write("processed %d rows, left %d, last term=%s\n" % (rowCount,rowsLeft,row[0]))
            if articleLimit and rowCount >= articleLimit:
                processedInOneFetch # a hack so that we exit the loop but not by doing return
                break
            if splitAfter and (0 == rowCount % splitAfter):
                closeFiles()
                curPartNo += 1
                createFiles(fileOrig,fileConverted,curPartNo)
        if 0 == processedInOneFetch:
            break
        curOffset += rowsPerQuery
    closeFiles()

if __name__=="__main__":

    testGenSplitFileName()
    initDatabase()
    limit = arsutils.getRemoveCmdArgInt("-limit")
    origFile = arsutils.getRemoveCmdArg("-orig")
    convertedFile = arsutils.getRemoveCmdArg("-converted")
    splitPartsCount = arsutils.getRemoveCmdArgInt("-split")
    if splitPartsCount:
        if not origFile or not convertedFile:
            print "Usage: if -split given, must also give -orig and -converted"
            sys.exit(0)
    if len(sys.argv) > 1:
        print "Usage: dumpEnfikiToFile.py [-limit N] [-orig fileName] [-converted fileName] [-split N]"
        sys.exit(0)
    if origFile and convertedFile:
        dumpAllWithConverted(limit,origFile,convertedFile,splitPartsCount)
    else:
         dumpAll(limit)
    deinitDatabase()
