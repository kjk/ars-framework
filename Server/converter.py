import MySQLdb, sys, datetime, re

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

def convertDefinition(text):
    text=text.replace('\r','')
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
    text=text.strip()
    text+='\n'
    return text

query=query="""select cur_title, cur_text, cur_timestamp from enwiki.cur where cur_namespace=0  """

if len(sys.argv)>1:
    query+=""" and cur_timestamp>'%s'""" % db.escape_string(sys.argv[1])

srcCursor=db.cursor()
targetCursor=db.cursor()
srcCursor.execute(query)
row=srcCursor.fetchone()
rowCount = 0
while row:
    term=row[0].replace('_', ' ')
    definition=row[1]
    ts=row[2]
    timestamp=datetime.datetime(int(ts[0:4]), int(ts[4:6]), int(ts[6:8]), int(ts[8:10]), int(ts[10:12]), int(ts[12:14]))
    #print "Converting term: ", term
    
    targetCursor.execute("""select id, last_modified from definitions where term='%s'""" % db.escape_string(term))
    outRow=targetCursor.fetchone()
    if outRow:
        #print "Existing date: ", outRow[1],"; new date: ", timestamp
        if str(outRow[1])<str(timestamp):
            #print "Updating existing record id: ", outRow[0]
            targetCursor.execute("""update definitions set definition='%s', last_modified='%s' where id=%d""" % (db.escape_string(convertDefinition(definition)), db.escape_string(str(timestamp)), outRow[0]))
        else:
            #print "Skipping record, newer version exists."
            pass
    else:
        #print "Creating new record." 
        targetCursor.execute("""insert into definitions (term, definition, last_modified) values ('%s', '%s', '%s')""" % (db.escape_string(term), db.escape_string(convertDefinition(definition)), db.escape_string(str(timestamp))))
    
    row=srcCursor.fetchone()
    rowCount += 1
    if 0 == rowCount % 100:
        print "processed %d rows" % rowCount

srcCursor.close()
targetCursor.close()

db.close()