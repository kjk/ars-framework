# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk

# Purpose:
# Generate the following statistics about unconverted 
# - entity usage statistics so that we know which international characters 
#   we need to support in the viewer
# - size statistics about articles (because we're curious)
#
# History:
# 2004-03-21 started

# devnotes: using iterators (http://www-106.ibm.com/developerworks/library/l-pycon.html)

#from __future__ import generators 
import MySQLdb, MySQLdb.cursors, sys, datetime, re
import timeit

def replaceRegExp(text, regExp, repl):
    match=regExp.search(text)
    while match:
        print "Replacing: ", text[match.start():match.end()], " with: ", repl
        text=text[0:match.start()]+repl+text[match.end():]
        match=regExp.search(text)
    return text

def replaceTagList(text, tagList, repl):
    for tag in tagList:
        text=replaceRegExp(text, re.compile(r'<(/)?%s(\s+.*?)?>' % tag, re.I), repl)
    return text

def convertDefinition(text):
    text=text.replace('\r','')
    text=replaceRegExp(text, re.compile("<!--.*?-->", re.S), '')
    text=replaceRegExp(text, re.compile("<div.*?</div>", re.I+re.S), '')
    text=replaceRegExp(text, re.compile("<table.*?</table>", re.I+re.S), '')
    text=replaceRegExp(text, re.compile("<caption.*?</caption>", re.I+re.S), '')
    text=replaceRegExp(text, re.compile("<tbody.*?</tbody>", re.I+re.S), '')
    text=replaceRegExp(text, re.compile("<thead.*?</thead>", re.I+re.S), '')
    # The following two lines would seem redundant, but lots of wikipedia articles have screwed tables and it renders really ugly then...
    text=replaceRegExp(text, re.compile("<tr.*?</tr>", re.I+re.S), '') 
    text=replaceRegExp(text, re.compile("<td.*?</td>", re.I+re.S), '') 
    text=replaceRegExp(text, re.compile("<script.*?</script>", re.I+re.S), '')
    text=replaceTagList(text, ['b', 'strong'], "'''")
    text=replaceTagList(text, ['em', 'i', 'cite'], "''")
    text=replaceTagList(text, ['hr'], '----')
    text=replaceTagList(text, ['dfn', 'code', 'samp', 'kbd', 'var', 'abbr', 'acronym', 'blockquote', 'q', 'p', 'pre', 'ins', 'del', 'dir', 'menu', 'img', 'object', 'big', 'span', 'applet', 'font', 'basefont', 'tr', 'td', 'table', 'center'], '')
    text=replaceRegExp(text, re.compile(r"\[\[((\w\w\w?(-\w\w)?)|(simple)|(image)|(media)):.*?\]\]", re.I+re.S), '')
    text=text.strip()
    text+='\n'
    return text

def queryGetOneVal(db,query):
    cursor = db.cursor()
    cursor.execute(query)
    row = cursor.fetchone()
    if row:
        return row[0]
    else:
        return None
    cursor.close()

class getRow2_iter:
    def __init__(self,db):
        print "getRow_iter() __init__"
        query="""select cur_id, cur_title, cur_text from enwiki.cur where cur_namespace=0"""
        self.cursor = db.cursor()
        self.cursor.execute(query)
    def __iter__(self):
        return self
    def next(self):
        print "getRow_iter() next"
        while True:
            row = self.cursor.fetchone()
            if not row:
                break;
            print row
            print row[1]
            yield row
        self.cursor.close()
        raise StopIteration

# an iterator returning a row from enwiki.cur database
# row[0] is cur_id
# row[1] is cur_title
# row[2] is cur_text
def getRow_iter(db):
    query="""select cur_id, cur_title, cur_text from enwiki.cur where cur_namespace=0"""
    cursor = db.cursor()
    cursor.execute(query)
    while True:
        row = cursor.fetchone()
        if not row:
            break;
        yield row
    cursor.close()
    raise StopIteration

def doOne(db):
    rowCount = 0
    for row in getRow_iter(db):
        title = row[1]
        print title
        rowCount += 1
        if rowCount > 9:
            break
        
fDumpTerms = True

#db=MySQLdb.Connect(
#    host='localhost', 
#    user='ipedia', 
#    passwd='ipedia', 
#    db='ipedia')
    #cursorclass=MySQLdb.cursors.SSCursor)

db=MySQLdb.Connect(
    host='localhost', 
    user='ipedia', 
    passwd='ipedia', 
    db='ipedia',
    cursorclass=MySQLdb.cursors.SSCursor)

t = timeit.Timer("doOne(db)")
print t.timeit(number=1)
#doOne(db)

db.close()

