#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

from twisted.internet import protocol, reactor
from twisted.protocols import basic
import MySQLdb, random, _mysql_exceptions, datetime, re

lineSeparator =     "\n"
fieldSeparator =    ": "

transactionIdField =        "Transaction-ID"
getCookieField =            "Get-Cookie"
cookieField =                   "Cookie"
getDefinitionField =        "Get-Definition"
formatVersionField =        "Format-Version"
definitionField =           "Definition"
definitionForField =        "Definition-For"
definitionNotFoundField = "Definition-Not-Found"
errorField =                    "Error"
registerField =                 "Register"
protocolVersionField =  "Protocol-Version"
clientVersionField =        "Client-Version"

redirectCommand = "#REDIRECT"
termStartDelimiter = "[["
termEndDelimiter = "]]"

definitionFormatVersion = 1
protocolVersion = 1

listLengthLimit = 100

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
            query=("""insert into requests (client_ip, transaction_id, has_get_cookie_field, cookie_id, has_register_field, requested_term, error, definition_id) """
                                        """values (%d, %s, %d, %s, %d, %s, %d, %s)""" % (clientIp, trIdStr, hasGetCookie, cookieIdStr, hasRegister, reqTerm, self.error, defIdStr))
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
        self.outputField(definitionForField, self.term)
        self.outputPayloadField(definitionField, definition)
        
    def findTarget(self, db, cursor, idTermDef):
        defId, term, definition=idTermDef
        history=[]
        while True:
            if not startsWithIgnoreCase(definition, redirectCommand):
                return defId, term, definition
            else:
                history.append(term.lower())
                termStart=definition.find(termStartDelimiter)+2
                termEnd=definition.find(termEndDelimiter)
                term=definition[termStart:termEnd].replace('_', ' ')
                print "Resolving term: ", term
                if term.lower() in history:
                    print "--------------------------------------------------------------------------------"
                    print "WARNING! Circular reference: ", term
                    print "--------------------------------------------------------------------------------"
                    return None
                else:
                    query="""select id, term, definition from definitions where term='%s' """ % db.escape_string(term)
                    cursor.execute(query)
                    row=cursor.fetchone()
                    if row:
                        defId, term, definition=row[0], row[1], row[2]
                    else:
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
        
    internalLinkRe=re.compile(r'\[\[(.*?)(\|(.*?))?\]\]', re.S)
                
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
                name=match.group(match.lastindex).replace('_', ' ')
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
        query="""select id, term, definition from definitions where match(term, definition) against('%s') limit %d""" % (db.escape_string(reqTerm), listLengthLimit)
        cursor.execute(query)
        tupleList=[]
        row=cursor.fetchone()
        while row:
            tupleList.append((row[0], row[1], row[2]))
            print row[1]
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
                    self.outputField(definitionNotFoundField)
                else:
                    self.outputField(definitionNotFoundField)
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
        if (request == ""):
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
    
reactor.listenTCP(9000, iPediaFactory())
reactor.run()

