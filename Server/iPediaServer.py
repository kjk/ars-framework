#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

from twisted.internet import protocol, reactor
from twisted.protocols import basic
import MySQLdb, random, _mysql_exceptions, datetime, re

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

lineSeparator =     "\n"
fieldSeparator =    ": "

transactionIdField =    "Transaction-ID"
getCookieField =        "Get-Cookie"
cookieField =           "Cookie"
getDefinitionField =    "Get-Definition"
formatVersionField =    "Format-Version"
definitionField =       "Definition"
resultsForField =       "Results-For"
notFoundField =         "Not-Found"
errorField =            "Error"
registerField =         "Register"
protocolVersionField =  "Protocol-Version"
clientVersionField =    "Client-Version"
searchField =           "Search"
searchResultsField =    "Search-Results"

redirectCommand =    "#REDIRECT"
termStartDelimiter = "[["
termEndDelimiter =   "]]"

definitionFormatVersion = 1
protocolVersion = 1

listLengthLimit = 200

requestLinesCountLimit = 20

class iPediaServerError:
    serverFailure=1
    unsupportedDevice=2
    invalidAuthorization=3
    malformedRequest=4

def startsWithIgnoreCase(s1, substr):
    if len(s1)<len(substr):
        return False
    if s1[:len(substr)].upper()==substr.upper():
        return True
    return False

class iPediaProtocol(basic.LineReceiver):

    def __init__(self):
        self.delimiter='\n'
        self.transactionId=None
        self.deviceInfoToken=None
        self.db=None
        self.error=0
        self.clientVersion=None
        self.protocolVersion=None
        self.requestedTerm=None
        self.term=None
        self.cookie=None
        self.cookieId=None
        self.userId=None
        self.serialNumber=None
        self.definitionId=None
        self.linesCount=0
        
    def getDatabase(self):
        if not self.db:
            self.db=self.factory.createConnection()
        return self.db

    def outputField(self, name, value=None):
        field=name
        if value:
            field+=fieldSeparator+value
        field+=lineSeparator
        self.transport.write(field)
        print field
        
    def outputPayloadField(self, name, payload):
        self.outputField(name, str(len(payload)))
        self.transport.write(payload)
        self.transport.write(lineSeparator)
        print payload
        
    def logRequest(self):
        cursor=None
        try:
            db=self.getDatabase()
            trIdStr='0'
            if self.transactionId:
                trIdStr=str(int(self.transactionId, 16))
            hasGetCookie=0
            if self.deviceInfoToken:
                hasGetCookie=1
            cookieIdStr='NULL'
            if self.cookieId:
                cookieIdStr=str(self.cookieId)
            hasRegister=0
            if self.serialNumber:
                hasRegister=1
            reqTerm='NULL'
            if self.requestedTerm:
                reqTerm='\''+db.escape_string(self.requestedTerm)+'\''
            defIdStr='NULL'
            if self.definitionId:
                defIdStr=str(self.definitionId)
            cursor=db.cursor()
            clientIp=0
            query=("""insert into requests (client_ip, transaction_id, has_get_cookie_field, cookie_id, has_register_field, requested_term, error, definition_id, request_date) """
                                        """values (%d, %s, %d, %s, %d, %s, %d, %s, now())""" % (clientIp, trIdStr, hasGetCookie, cookieIdStr, hasRegister, reqTerm, self.error, defIdStr))
            cursor.execute(query)
            cursor.close()
        except _mysql_exceptions.Error, ex:
            print ex
            if cursor:
                cursor.close()
        

    def finish(self):
        if self.error:
            self.outputField(errorField, str(self.error))
        self.transport.loseConnection()
        if self.db:
            self.logRequest()
            self.db.close()
            self.db=None
                                
        print ""
        print "--------------------------------------------------------------------------------"
        print ""

            
    def validateDeviceInfo(self):
        return True
        
    def createCookie(self, cursor):
        finished=False
        randMax=2**32-1
        result=None
        while not finished:
            result=""
            for i in range(4):
                val=random.randint(0, randMax)
                hexVal=hex(val)[2:10]
                result+=hexVal
            cursor.execute("""select id from cookies where cookie='%s'""" % result)
            row=cursor.fetchone()
            if not row:
                finished=True
        self.cookie=result
        
    def handleGetCookieRequest(self):

        if not self.validateDeviceInfo():
            self.error=iPediaServerError.unsupportedDevice
            return False
            
        cursor=None
        try:
            db=self.getDatabase()
            cursor=db.cursor()
            cursor.execute("""select id, cookie from cookies where device_info_token='%s'""" % db.escape_string(self.deviceInfoToken))
            row=cursor.fetchone()
            if row:
                self.cookieId=row[0]
                self.cookie=row[1]
                cursor.execute("""select id from registered_users where cookie_id=%d""" % self.cookieId)
                row=cursor.fetchone()
                if row:
                    self.userId=row[0]
            else:
                self.createCookie(cursor)
                cursor.execute("""insert into cookies (cookie, device_info_token, issue_date) values ('%s', '%s', now())""" % (self.cookie, db.escape_string(self.deviceInfoToken)))
                self.cookieId=cursor.lastrowid
            cursor.close()                                      
            self.outputField(cookieField, str(self.cookie))
            return True
            
        except _mysql_exceptions.Error, ex:
            print ex
            if cursor:
                cursor.close()
            self.error=iPediaServerError.serverFailure
            return False;
            
    def handleRegisterRequest(self):
        if not self.cookieId:
            self.error=iPediaServerError.malformedRequest
            return False

        cursor=None
        try:
            db=self.getDatabase()
            cursor=db.cursor()
            cursor.execute("""select id, cookie_id from registered_users where serial_number='%s'""" % db.escape_string(self.serialNumber))
            row=cursor.fetchone()
            if row:
                currentCookieId=row[1]
                if currentCookieId==None:
                    self.userId=row[0]
                    cursor.execute("""update registered_users set cookie_id=%d where id=%d""" % (self.cookieId, self.userId))
                elif currentCookieId!=self.cookieId:
                    self.error=iPediaServerError.invalidAuthorization
                    cursor.close()
                    return False
                else:
                    self.userId=row[0]
            else:
                self.error=iPediaServerError.invalidAuthorization
                cursor.close()
                return False
            cursor.close()
            return True
        except _mysql_exceptions.Error, ex:
            print ex
            if cursor:
                cursor.close()
            self.error=iPediaServerError.serverFailure
            return False;
    
    def handleCookieRequest(self):
        if not self.cookieId:
            cursor=None
            try:
                db=self.getDatabase()
                cursor=db.cursor()
                cursor.execute("""select id from cookies where cookie='%s'""" % db.escape_string(self.cookie))
                row=cursor.fetchone()
                if row:
                    self.cookieId=row[0]
                    cursor.close()
                    return True
                else:
                    self.error=iPediaServerError.invalidAuthorization
                    cursor.close()
                    return False
            except _mysql_exceptions.Error, ex:
                print ex
                if cursor:
                    cursor.close()
                self.error=iPediaServerError.serverFailure
                return False;
        else:
            return True
            
    def outputDefinition(self, definition):
        self.outputField(formatVersionField, str(definitionFormatVersion))
        self.outputField(resultsForField, self.term)
        self.outputPayloadField(definitionField, definition)
        
    def findTarget(self, db, cursor, idTermDef):
        defId, term, definition=idTermDef
        history=[]
        while True:
            if not startsWithIgnoreCase(definition, redirectCommand):
                return defId, term, definition
            history.append(defId)
            termStart=definition.find(termStartDelimiter)+2
            termEnd=definition.find(termEndDelimiter)
            term=definition[termStart:termEnd].replace('_', ' ')
            print "Resolving term: ", term
            query="""select id, term, definition from definitions where term='%s' """ % db.escape_string(term)
            cursor.execute(query)
            row=cursor.fetchone()
            if not row:
                return None

            defId, term, definition=row[0], row[1], row[2]
            if defId in history:
                print "--------------------------------------------------------------------------------"
                print "WARNING! Circular reference: ", term
                print "--------------------------------------------------------------------------------"
                logCircular(defId,term)
                return None
        
    def findExactDefinition(self, db, cursor, term):
        query="""select id, term, definition from definitions where term='%s' """ % db.escape_string(term)
        cursor.execute(query)
        row=cursor.fetchone()
        if row:
            definition=None
            idTermDef=self.findTarget(db, cursor, (row[0], row[1], row[2]))
            return idTermDef
        else:
            return None
        
    internalLinkRe=re.compile(r'\[\[(.*?)(\|.*?)?\]\]', re.S)
                
    def validateInternalLinks(self, db, cursor, definition):
        matches=[]
        for iter in iPediaProtocol.internalLinkRe.finditer(definition):
            matches.append(iter)
        matches.reverse()
        tested=[]
        valid=[]
        for match in matches:
            term=match.group(1)
            print "Verifiying: ", term
            if len(term) and (term not in tested):
                idTermDef=self.findExactDefinition(db, cursor, term)
                if idTermDef:
                    print "Validated..."
                    valid.append(term)
            if term not in valid:
                name=match.group(match.lastindex).lstrip('| ').rstrip().replace('_', ' ')
                print "Replacing with: ", name
                definition=definition[:match.start()]+name+definition[match.end():]
        return definition

    def preprocessDefinition(self, db, cursor, definition):
        definition=self.validateInternalLinks(db, cursor, definition)
        definition=definition.replace("__NOTOC__", '')
        definition=definition.replace("{{CURRENTMONTH}}", str(datetime.date.today().month));
        definition=definition.replace("{{CURRENTMONTHNAME}}", datetime.date.today().strftime("%B"))
        definition=definition.replace("{{CURRENTMONTHNAMEGEN}}", datetime.date.today().strftime("%B"))
        definition=definition.replace("{{CURRENTDAY}}", str(datetime.date.today().day))
        definition=definition.replace("{{CURRENTDAYNAME}}", datetime.date.today().strftime("%A"))
        definition=definition.replace("{{CURRENTYEAR}}", str(datetime.date.today().year))
        definition=definition.replace("{{CURRENTTIME}}", datetime.date.today().strftime("%X"))
        definition=definition.replace("{{NUMBEROFARTICLES}}", str(self.factory.articleCount))
        return definition
        
    def findFullTextMatches(self, db, cursor, reqTerm):
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
            resTuple=self.findTarget(db, cursor, idTermDef)
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
        
    def handleDefinitionRequest(self):
        cursor=None
        definition=None
        try:
            db=self.getDatabase()
            cursor=db.cursor()
            idTermDef=self.findExactDefinition(db, cursor, self.requestedTerm)
            if idTermDef:
                self.definitionId, self.term, definition=idTermDef
#               definition=self.findExactDefinition(db, cursor)
            if definition:
                self.outputDefinition(self.preprocessDefinition(db, cursor, definition))
            else:
                self.termList=self.findFullTextMatches(db, cursor, self.requestedTerm)
                if self.termList:
                    self.outputField(resultsForField, self.requestedTerm)
                    joinedList=""
                    for term in self.termList:
                        joinedList+=(term+'\n')
                    self.outputPayloadField(searchResultsField, joinedList)
                else:
                    self.outputField(notFoundField)
            cursor.close()
        except _mysql_exceptions.Error, ex:
            print ex
            if cursor:
                cursor.close()
            self.error=iPediaServerError.serverFailure
            return False;
        return True
        
    def answer(self):
        
        print ""
        print "--------------------------------------------------------------------------------"
        print ""
        
        if self.transactionId:
                self.outputField(transactionIdField, self.transactionId)
        else:
            return self.finish()
        
        if self.deviceInfoToken and not self.handleGetCookieRequest():
            return self.finish()
    
        if self.cookie:
            if not self.handleCookieRequest():
                return self.finish()
        else:
            self.error=iPediaServerError.malformedRequest
            return self.finish()
            
        if self.serialNumber and not self.handleRegisterRequest():
            return self.finish()
        
        if self.term and not self.handleDefinitionRequest():
            return self.finish()                                

        self.finish()

    def extractFieldValue(self, line):
        index=line.find(fieldSeparator)
        if (index!=-1):
            return line[index+2:]
        else:
            return None
            
    def lineReceived(self, request):
        ++self.linesCount
        
        if requestLinesCountLimit==self.linesCount:
            self.error=iPediaServerError.malformedRequest
            
        if request == ""  or self.error:
            self.answer()
        else:
            print request
            print ""
            
            if request.startswith(transactionIdField):
                self.transactionId=self.extractFieldValue(request)
                
            elif request.startswith(protocolVersionField):
                self.protocolVersion=self.extractFieldValue(request)
                
            elif request.startswith(clientVersionField):
                self.clientVersion=self.extractFieldValue(request)

            elif request.startswith(getCookieField):
                self.deviceInfoToken=self.extractFieldValue(request)
                
            elif request.startswith(cookieField):
                self.cookie=self.extractFieldValue(request)
        
            elif request.startswith(getDefinitionField):
                self.requestedTerm=self.term=self.extractFieldValue(request)
            
            elif request.startswith(registerField):
                self.serialNumber=self.extractFieldValue(request)
            else:
                self.error=iPediaServerError.malformedRequest

        
class iPediaFactory(protocol.ServerFactory):

    def createConnection(self):
        return MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='ipedia')
        

    def __init__(self):
        db=self.createConnection()
        cursor=db.cursor()
        cursor.execute("""select count(*) from definitions""")
        row=cursor.fetchone()
        self.articleCount=row[0]
        print "Number of Wikipedia articles: ", self.articleCount
        cursor.close()
        db.close()

    protocol = iPediaProtocol

def main():
    reactor.listenTCP(9000, iPediaFactory())
    reactor.run()

if __name__ == "__main__":
    main()

