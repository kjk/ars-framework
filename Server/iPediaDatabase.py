import re, random, sys

redirectCommand =    "#REDIRECT"
termStartDelimiter = "[["
termEndDelimiter =   "]]"

listLengthLimit = 200

g_fVerbose=False

# if true we'll log terms that redirect to themselves to a file
g_fLogCircularReferences = True
g_circularReferencesLogName = "circular.log"

class CircularDetector(object):
    def __init__(self,logFileName):
        self.logFileName = logFileName
        self.loadLog()
    def loadLog(self):
        self.circulars = {}
        try:
            fo = open(self.logFileName, "rb")
            for lines in fo.readlines():
                (defId,term) = line.split(":")
                self.circulars[defId] = term
            fo.close()
        except:
            pass #it's ok if file doesn't exist
    def log(self,defId,term):
        if not self.circulars.has_key(defId):
            self.circulars[defId] = term
    def saveLog(self):
         fo = open(self.logFileName,"wb")
         for defId in self.circulars.keys():
            term = self.circulars[defId]
            toWrite = defId + ":" + term + "\n"
            fo.write(toWrite)
         fo.close()

g_circularDetector = None
def logCircular(defId,term):
    global g_fLogCircularReferences, g_circularDetector, g_circularReferencesLogName
    if not g_fLogCircularReferences:
        return
    if not g_circularDetector:
        g_circularDetector = CircularDetector(g_circularReferencesLogName)
    g_circularDetector.log(defId,term)

def saveCircular():
    global g_fLogCircularReferences, g_circularDetector
    if not g_fLogCircularReferences:
        return
    if g_circularDetector:
        g_circularDetector.saveLog()

def testCircular():
    logCircular("me", "him")
    logCircular("me", "him2")
    logCircular("one", "two")
    logCircular("three", "four")
    saveCircular()

def startsWithIgnoreCase(s1, substr):
    if len(s1)<len(substr):
        return False
    if s1[:len(substr)].upper()==substr.upper():
        return True
    return False

def findTarget(db, cursor, idTermDef):
    global g_fVerbose
    defId, term, definition=idTermDef
    history=[]
    while True:
        if not startsWithIgnoreCase(definition, redirectCommand):
            return defId, term, definition
        history.append(defId)
        termStart=definition.find(termStartDelimiter)+2
        termEnd=definition.find(termEndDelimiter)
        term=definition[termStart:termEnd].replace('_', ' ')
        #print "Resolving term: ", term
        query="""SELECT id, term, definition FROM definitions WHERE term='%s' """ % db.escape_string(term)
        cursor.execute(query)
        row=cursor.fetchone()
        if not row:
            return None

        defId, term, definition=row[0], row[1], row[2]
        if defId in history:
            if g_fVerbose:
                print "--------------------------------------------------------------------------------"
                print "WARNING! Circular reference: ", term
                print "--------------------------------------------------------------------------------"
            logCircular(defId,term)
            return None
        
def findExactDefinition(db, cursor, term):
    query="""SELECT id, term, definition FROM definitions WHERE term='%s';""" % db.escape_string(term)
    cursor.execute(query)
    row=cursor.fetchone()
    if row:
        definition=None
        idTermDef=findTarget(db, cursor, (row[0], row[1], row[2]))
        return idTermDef
    else:
        return None

def findRandomDefinition(db, cursor):
    cursor.execute("""select min(id), max(id) from definitions""")
    row=cursor.fetchone()
    minId, maxId=row[0], row[1]
    term_id = random.randint(minId, maxId)
    #query="""SELECT id, term, definition FROM definitions  ORDER BY RAND() LIMIT 0,1;"""
    #query="""SELECT id, term, definition FROM definitions WHERE id=%d;""" % term_id
    query="""SELECT id, term, definition FROM definitions WHERE id=%d;""" % term_id
    cursor.execute(query)
    row=cursor.fetchone()
    if row:
        definition=None
        idTermDef=findTarget(db, cursor, (row[0], row[1], row[2]))
        return idTermDef
    else:
        return None
        
internalLinkRe=re.compile(r'\[\[(.*?)(\|.*?)?\]\]', re.S)
testedLinks = {}
validLinks = {}

def validateInternalLinks(db, cursor, definition):
    global g_fVerbose, validLinks, testedLinks

    if g_fVerbose:
        sys.stdout.write("* Validaiting internal links: ")
    matches=[]
    for iter in internalLinkRe.finditer(definition):
        matches.append(iter)
    matches.reverse()
    for match in matches:
        term=match.group(1)
        if len(term)!=0 and not testedLinks.has_key(term):
            testedLinks[term] = 1
            idTermDef=findExactDefinition(db, cursor, term)
            if idTermDef:
                if g_fVerbose:
                    sys.stdout.write("'%s' [ok], " % term)
                validLinks[term] = 1
        if not validLinks.has_key(term):
            name=match.group(match.lastindex).lstrip('| ').rstrip().replace('_', ' ')
            if g_fVerbose:
                sys.stdout.write("'%s' => '%s', " % (term,name))
            definition=definition[:match.start()]+name+definition[match.end():]
    return definition

def findFullTextMatches(db, cursor, reqTerm):
    print "Performing full text search..."
    words=reqTerm.split()
    queryStr=''
    for word in words:
        queryStr+=(' +'+word)
    query="""select id, term, definition, match(term, definition) against('%s') as relevance from definitions where match(term, definition) against('%s' in boolean mode) order by relevance desc limit %d""" % (db.escape_string(reqTerm), db.escape_string(queryStr), listLengthLimit)
    cursor.execute(query)
    row=cursor.fetchone()
    if not row:
        print "Performing non-boolean mode search..."
        query="""select id, term, definition, match(term, definition) against('%s') as relevance from definitions where match(term, definition) against('%s') order by relevance desc limit %d""" % (db.escape_string(reqTerm), db.escape_string(reqTerm), listLengthLimit)
        cursor.execute(query)

    tupleList=[]
                    
    while row:
        tupleList.append((row[0], row[1], row[2]))
        print row[1], row[3]
        row=cursor.fetchone()
        
    if not len(tupleList):
        return None

    validatedIds=[]
    termList=[]
    for idTermDef in tupleList:
        defId, term, definition=idTermDef
        print "Checking term: ", term
        resTuple=findTarget(db, cursor, idTermDef)
        if resTuple:
            defId, term, definition=resTuple
            if defId not in validatedIds:
                validatedIds.append(defId)
                termList.append(term)
            else:
                print "Discarding duplicate term: ", term
        else:
            print "Discarding failed redirection..."
    
    if not len(termList):
        return None
    else:
        return termList
        
