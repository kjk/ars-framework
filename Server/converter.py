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

import MySQLdb, sys, datetime, re, unicodedata

#if True, we'll print a lot of debug text to stdout
g_fVerbose       = False

g_connOne        = None
g_connTwo        = None

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
    print "rows in enwiki: %d" % enwikiRows
    print "rows in ipedia: %d" % ipediaRows

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

def stripUnconsistentBlocks(text, startPattern, endPattern):
    opened=0
    spanStart=-1
    spans=[]    
    pattern=r"(%s)|(%s)" % (startPattern, endPattern)
    blockRe=re.compile(pattern, re.I+re.S)
    for match in blockRe.finditer(text):
        if match.lastindex==1: # This means it's a start tag
            if not opened:
                spanStart=match.start(match.lastindex)
            opened+=1
        else:                       # This is end tag
            if opened==1:
                spans.append((spanStart, match.end(match.lastindex)))
            opened-=1;
            if opened<0:
                opened=0
    if opened:
        spans.append((spanStart, len(text)))
    spans.reverse()             # Iterate from end so that text indices remain valid when we slice and concatenate text
    for span in spans:
        start, end=span
        text=text[:start]+text[end:]
    return text

def stripBlocks(text, blockElem):
    return stripUnconsistentBlocks(text, '<%s.*?>' % blockElem, '</%s>' % blockElem)
    
def replaceRegExp(text, regExp, repl):
    match=regExp.search(text)
    while match:
        #print "Replacing: ", text[match.start():match.end()], " with: ", repl
        text=text[0:match.start()]+repl+text[match.end():]
        match=regExp.search(text)
    return text
    
def replaceTagList(text, tagList, repl):
    for tag in tagList:
        text=replaceRegExp(text, re.compile(r'<(/)?%s(\s+.*?)?>' % tag, re.I), repl)
    return text
    
commentRe=re.compile("<!--.*?-->", re.S)
divRe=re.compile("<div.*?</div>", re.I+re.S)
tableRe=re.compile("<table.*?</table>", re.I+re.S)
captionRe=re.compile("<caption.*?</caption>", re.I+re.S)
tbodyRe=re.compile("<tbody.*?</tbody>", re.I+re.S)
theadRe=re.compile("<thead.*?</thead>", re.I+re.S)
trRe=re.compile("<tr.*?</tr>", re.I+re.S)
tdRe=re.compile("<td.*?</td>", re.I+re.S)
scriptRe=re.compile("<script.*?</script>", re.I+re.S)
badLinkRe=re.compile(r"\[\[((\w\w\w?(-\w\w)?)|(simple)|(image)|(media)):.*?\]\]", re.I+re.S)
numEntityRe=re.compile(r'&#(\d+);')

multipleLinesRe=re.compile("\n{3,100}")
# replace multiple (1+) empty lines with just one empty line.
def stripMultipleNewLines(txt):
    txt=replaceRegExp(txt,multipleLinesRe,"\n\n")
    return txt

def convertEntities(text):
    matches=[]
    for iter in numEntityRe.finditer(text):
        matches.append(iter)
    matches.reverse()
    for match in matches:
        num=int(text[match.start(1):match.end(1)])
        if num>255:
            char=unichr(num)
            decomposed=unicodedata.normalize('NFKD', char)
            valid=''
            for char in decomposed:
                if ord(char)<256:
                    valid+=chr(ord(char))
            if len(valid):
                text=text[:match.start()]+valid+text[match.end():]
    return text

def convertDefinition(text):
    text=text.replace('\r','')
    text=text.replace('&minus;', '-') # no idea why would someone use &minus; but it happens e.g. in "electron"
    text=replaceRegExp(text, commentRe, '')     # This should be safe, as it's illegal in html to nest comments
    
    text=stripBlocks(text, 'div')
    text=stripBlocks(text, 'table')
    text=stripUnconsistentBlocks(text, r'\{\|', r'\|\}')

    text=replaceRegExp(text, scriptRe, '')
    
    text=replaceTagList(text, ['b', 'strong'], "'''")
    text=replaceTagList(text, ['em', 'i', 'cite'], "''")
    text=replaceTagList(text, ['hr'], '----')
    text=replaceTagList(text, ['dfn', 'code', 'samp', 'kbd', 'var', 'abbr', 'acronym', 'blockquote', 'q', 'p', 'pre', 'ins', 'del', 'dir', 'menu', 'img', 'object', 'big', 'span', 'applet', 'font', 'basefont', 'tr', 'td', 'table', 'center', 'div'], '')
    text=replaceRegExp(text, badLinkRe, '')
    text=convertEntities(text)
    text=stripMultipleNewLines(text)
    text=text.strip()
    text+='\n'
    return text

def convertTermWithId(term_id,fForceConvert=False):
    global g_fVerbose, g_connTwo

    cur_data_cur = getNamedCursor(g_connTwo, "cur_data_cur")
    ipedia_write_cur = getNamedCursor(g_connTwo, "ipedia_write_cur")
    cur_data_cur.execute("""SELECT cur_title,cur_text,cur_timestamp FROM enwiki.cur WHERE cur_namespace=0 AND cur_id='%s'""" % term_id)
    defRow=cur_data_cur.fetchone()
    term=defRow[0].replace('_', ' ')
    definition=defRow[1]
    ts=defRow[2]
    timestamp=datetime.datetime(int(ts[0:4]), int(ts[4:6]), int(ts[6:8]), int(ts[8:10]), int(ts[10:12]), int(ts[12:14]))
    if g_fVerbose:
        log_txt = "term: %s " % term

    check_cur = getNamedCursor(g_connTwo, "check_cur")
    check_cur.execute("""SELECT id, last_modified FROM definitions WHERE term='%s'""" % dbEscape(term))
    outRow=check_cur.fetchone()
    if outRow:
        if g_fVerbose: 
            log_txt +=  "cur:" + outRow[1] + " new: " + str(timestamp)
        if fForceConvert or str(outRow[1])<str(timestamp):
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
    return term

def convertAll(articleLimit,fForceConvert):
    global g_connOne
    query="""SELECT cur_id FROM cur WHERE cur_namespace=0"""
    if len(sys.argv)>1:
        print sys.argv
        query+=""" AND cur_timestamp>'%s'""" % dbEscape(sys.argv[1])

    if articleLimit != -1:
        query+=""" LIMIT %d""" % articleLimit

    cursor = getNamedCursor(g_connOne,"all_cur_ids")
    cursor.execute(query)
    print "cursor.execute() done"
    rowCount = 0
    rowsLeft = getEnwikiRowCount()

    while True:
        row=cursor.fetchone()
        if not row:
            break
        term = convertTermWithId(row[0],fForceConvert)
        rowCount += 1
        rowsLeft -= 1
        if 0 == rowCount % 100:
            print "processed %d rows, left %d, last term=%s" % (rowCount,rowsLeft,term)

#    for row in cursor.fetchall():
#        term = convertTermWithId(row[0],fForceConvert)
#        rowCount += 1
#        rowsLeft -= 1
#        if 0 == rowCount % 100:
#            print "processed %d rows, left %d, last term=%s" % (rowCount,rowsLeftterm)

def convertOneTerm(term):
    global g_connTwo
    query="""SELECT cur_id FROM enwiki.cur WHERE cur_namespace=0 AND cur_title='%s'""" % dbEscape(term)
    cursor=g_connTwo.cursor()
    cursor.execute(query)
    row=cursor.fetchone()
    if not row:
        print "Didn't find a row in enwiki.cur with cur_title='%s'" % dbEscape(term)
    else:
        termId = row[0]
        convertTermWithId(termId,True)
    cursor.close()

if __name__=="__main__":

    initDatabase()
    fForceConvert = False
    try:
        pos = sys.argv.index("-force")
        fForceConvert = True
        sys.argv[pos] = []
    except:
        pass

    try:
        pos = sys.argv.index("-verbose")
        g_fVerbose = True
        print sys.argv
        sys.argv[pos:pos+1] = []
        print sys.argv
    except:
        pass

    articleLimit = -1
    pos = -1
    try:
        pos = sys.argv.index("-limit")
    except:
        pass
    if pos != -1:
        articleLimit = int(sys.argv[pos+1])
        #print sys.argv
        sys.argv[pos:pos+2] = []
        #print sys.argv

    pos = -1
    try:
        pos = sys.argv.index("-one")
    except:
        pass
    if pos != -1:
        termToConvert = sys.argv[pos+1]
        g_fVerbose = True
        convertOneTerm(termToConvert)
    else:
        convertAll(articleLimit, fForceConvert)
    deinitDatabase()

