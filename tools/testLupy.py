# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Test the speed of indexing/searching of Lupy ft
#
# Usage:
#   -usepsyco : if used, will use psyco for speeding up, false by default
#   -limit N : only index first N articles
#   -index fileName - which file to index
#   -search 
#   -verbose : print a lot of info
import os, sys, time
import cStringIO as StringIO
from lupy.index.term import Term
from lupy.search.indexsearcher import IndexSearcher
from lupy.search.term import TermQuery
from lupy.search.phrase import PhraseQuery
from lupy.search.boolean import BooleanQuery
from lupy.index.indexwriter import IndexWriter
from lupy import document
try:
    import psyco
    g_fPsycoAvailable = True
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"
    g_fPsycoAvailable = False

g_fVerbose = False

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
    print "Usage: testLupy.py [-usepsyco] [-limit N] [-verbose] [-index fileName] [-search term]"
    sys.exit(0)

g_indexPath = "lupy_ind"

ST_NONE = 0
ST_AFTER_TITLE = 1
ST_AFTER_NS = 2
ST_IN_TXT = 3

def iterGetArticle(fileName):
    fo = open(fileName, "rb")

    state = ST_NONE
    title = None
    ns = None
    txt = None
    while True:
        line = fo.readline()
        if len(line)==0:
            break
        if state == ST_NONE:
            assert line[0] == '^'
            title = line[1:]
            state = ST_AFTER_TITLE
        elif state == ST_AFTER_TITLE:
            try:
                ns = int(line)
            except:
                print "Invalid namespace (%s) for title=%s" % (line,title)
            txt = ""
            state = ST_IN_TXT
        elif state == ST_IN_TXT:
            if line[0] == '^':
                # this means we got a new article
                title = line[1:]
                state = ST_AFTER_TITLE
                yield (title,ns,txt)
            else:
                txt += line
        else:
            print "Unkown state (%d)" % state
            assert 0

    assert state == ST_IN_TXT
    yield (title,ns,txt)
    fo.close()

def indexOneArticle(indexer, title, txt):
    # create document
    d = document.Document()
    # add a file field containing the path to this file
    f = document.Keyword('title',title)
    d.add(f)
    # I happen to know that the title is separated
    # from the story by '\n\n\n', so I can easily get the title
    # which we store in the title field
    #fp = open(fname,'rb')
    #s = fp.read().decode("latin-1")
    #title = s.split('\n\n\n')[0]
    #f = document.Text('title',title)
    #d.add(f)
    # Here I pass False as the 3rd arg to ensure that
    # the actual text of s is not stored in the index
    # the following lines using TextWithReader are
    # more typical.   
    f = document.Text('body', txt, False)
    d.add(f)
    # Add text of an open file (fp)
    # This is typically how you add a file to an index
    # f = field.Text('text', fp)
    # d.add(f)    
    #fp.close()
    # add doc to index

    #if g_fVerbose:
    #    sys.stdout.write('indexing %s' % title)
    indexer.addDocument(d)

def index(fileName,limit):
    startTiming()
    # create a new index in a directory
    indexer = IndexWriter(g_indexPath, True)

    count = 0
    failedCount = 0
    for (title,ns,txt) in iterGetArticle(fileName):
        if ns != 0:
            continue
        try:
            # iso-8859-1 is the default encoding used in Wikipedia
            txt = txt.decode("iso-8859-1")
            title = title.decode("iso-8859-1")
            indexOneArticle(indexer,title,txt)
            count += 1
            if count >= limit:
                break
            if count % 500 == 0:
                print "indexed %d articles" % count
        except:
            # for now just ignore possible decoding errors
            if g_fVerbose:
                sys.stdout.write('failed to index %s' % title)
            failedCount += 1

    # Uncomment the following line to optimize the index.
    # Have a look in the index dir before you optimize.
    # You will probably see a dozens of files from
    # several segments. optimize() merges all the segments
    # into one. It can be quite an expensive operation, but
    # it can save space and speed up searches.

    indexer.optimize()        
    indexer.close()
    endTiming()

    print "indexed %d articles" % count
    print "failed to index %d articles" % failedCount
    dumpTiming()

def printHits(hits):
    if len(hits) == 0:
        print 'Nothing found!'
    else:
        for hit in hits:
            print 'Found in document %s' % (hit.get('title'))
        
def termSearch(qStr):
    t = Term('body', qStr)
    q = TermQuery(t)
    return q

def titleSearch(qStr):
    t = Term('title', qStr)
    q = TermQuery(t)
    return q

def phraseSearch(qStr, field='body'):
    """Find all docs containing the phrase C{qStr}."""
    parts = qStr.split()
    q = PhraseQuery()
    for p in parts:
        t = Term(field, p)
        q.add(t)
    return q

def boolSearch(ands=[], ors=[], nots=[]):
    """Build a simple boolean query.
    each word in B{ands} is equiv to +word
    each word in B{ors} is equiv to word
    each word in B{nots} is equiv to -word
    e.g. boolSearch(['spam'], ['eggs'], ['parrot', 'cheese'])
    is equiv to +spam eggs -parrot -cheese in Google/Lucene syntax"""    
    q = BooleanQuery()

    for a in ands:
        t = Term('body', a)
        tq = TermQuery(t)
        q.add(tq, True, False)
    for a in ors:
        t = Term('body', a)
        tq = TermQuery(t)
        q.add(tq, False, False)        
    for a in nots:
        t = Term('body', a)
        tq = TermQuery(t)
        q.add(tq, False, True)
    return q

def runQuery(q, searcher):
    """The run a query through a searcher and return the hits"""
    print 'Query:', q.toString('body')
    hits = searcher.search(q)
    printHits(hits)
    print
    return hits

def search(term):
    startTiming()

    searcher = IndexSearcher(g_indexPath)
    # Note that all queries have to be submitted in lower-case...
    q = termSearch(term)
    runQuery(q, searcher)
    
    #q = phraseSearch('fox and the crow')
    #runQuery(q, searcher)

    # A boolean search equiv to 'x and not y'
    #q = boolSearch(['fox'], [], ['crow', 'lion'])
    #runQuery(q, searcher)

    # Search the title field
    #q = titleSearch('frog')
    #runQuery(q, searcher)

    searcher.close()

    endTiming()
    dumpTiming()

if __name__ == "__main__":
    g_fVerbose = fDetectRemoveCmdFlag("-verbose")
    if g_fVerbose:
        print "verbose output"
    indexFile = getRemoveCmdArg("-index")
    searchTerm = getRemoveCmdArg("-search")
    if indexFile and searchTerm:
        print "can't use -index and -search and the same time"
        usageAndExit()

    limit = getRemoveCmdArg("-limit")
    if None == limit:
        limit = 9999999 # very big number
    else:
        limit = int(limit)
    print "limit=%d" % limit

    fUsePsyco = fDetectRemoveCmdFlag("-usepsyco")
    if g_fPsycoAvailable and fUsePsyco:
        print "using psyco"
        psyco.full()

    if indexFile:
        index(indexFile,limit)
    if searchTerm:
        search(searchTerm)


