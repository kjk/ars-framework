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

import MySQLdb, sys, datetime, re, unicodedata

#if True, we'll print a lot of debug text to stdout
g_fVerbose = False

db=MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='ipedia')

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

def convertTermWithId(term_id,fForceUpdate=False):
    global g_fVerbose
    cursor=db.cursor()
    cursor.execute("""SELECT cur_title,cur_text,cur_timestamp FROM enwiki.cur WHERE cur_namespace=0 AND cur_id='%s'""" % term_id)
    defRow=cursor.fetchone()
    term=defRow[0].replace('_', ' ')
    definition=defRow[1]
    ts=defRow[2]
    timestamp=datetime.datetime(int(ts[0:4]), int(ts[4:6]), int(ts[6:8]), int(ts[8:10]), int(ts[10:12]), int(ts[12:14]))
    if g_fVerbose:
        print "Converting term: ", term

    cursor.execute("""SELECT id, last_modified FROM definitions WHERE term='%s'""" % db.escape_string(term))
    outRow=cursor.fetchone()
    if outRow:
        if g_fVerbose:
            print "Existing date: ", outRow[1],"; new date: ", timestamp
        if fForceUpdate or str(outRow[1])<str(timestamp):
            if g_fVerbose:
                print "Updating existing record id: ", outRow[0]
            termId = outRow[0]
            newDef = convertDefinition(definition)
            cursor.execute("""UPDATE definitions SET definition='%s', last_modified='%s' WHERE id=%d""" % (db.escape_string(newDef), db.escape_string(str(timestamp)), termId))
        else:
            if g_fVerbose:
                print "Skipping record, newer version exists."
    else:
        if g_fVerbose:
            print "Creating new record." 
        newDef = convertDefinition(definition)
        cursor.execute("""INSERT INTO definitions (term, definition, last_modified) VALUES ('%s', '%s', '%s')""" % (db.escape_string(term), db.escape_string(newDef), db.escape_string(str(timestamp))))
    cursor.close()

def convertAll():
    query="""SELECT cur_id FROM enwiki.cur where cur_namespace=0 """
    if len(sys.argv)>1:
        query+=""" AND cur_timestamp>'%s'""" % db.escape_string(sys.argv[1])

    cursor=db.cursor()
    cursor.execute(query)
    rowCount = 0
    while True:
        row=cursor.fetchone()
        if not row:
            break
        convertTermWithId(row[0])
        rowCount += 1
        if 0 == rowCount % 100:
            print "processed %d rows, last term=%s" % (rowCount,term)
    cursor.close()
    db.close()

def convertOneTerm(term):
    query="""SELECT cur_id FROM enwiki.cur WHERE cur_namespace=0 AND cur_title='%s'""" % db.escape_string(term)
    cursor=db.cursor()
    cursor.execute(query)
    row=cursor.fetchone()
    if not row:
        print "Didn't find a row in enwiki.cur with cur_title='%s'" % db.escape_string(term)
    else:
        termId = row[0]
        convertTermWithId(termId,True)
    cursor.close()
    db.close()

if __name__=="__main__":
    try:
        pos = sys.argv.index("-verbose")
        g_fVerbose = True
        sys.argv[pos] = []
    except:
        pass

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
        convertAll()

