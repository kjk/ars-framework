#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

# Copyright: Krzysztof Kowalczyk
# Owner: Andrzej Ciarkowski
#
# Purpose: server component for iPedia
#
# Usage:
#   -silent : will supress most of the messages logged to stdout. TODO: in the
#             future will be replaced with -verbose flag (i.e. we'll be silent
#             by default)
#   -usepsyco : will use psyco, if available

import sys, re, random, datetime, MySQLdb, _mysql_exceptions
import arsutils,iPediaDatabase
from twisted.internet import protocol, reactor
from twisted.protocols import basic
try:
    import psyco
    g_fPsycoAvailable = True
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"
    g_fPsycoAvailable = False

g_articleCount = 0

# if True we'll print debugging info
g_fVerbose = None

lineSeparator =     "\n"
fieldSeparator =    ": "

transactionIdField =    "Transaction-ID"
getCookieField =        "Get-Cookie"
cookieField =           "Cookie"
getDefinitionField =    "Get-Definition"
getRandomField =        "Get-Random-Definition"
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

definitionFormatVersion = 1
protocolVersion = 1

requestLinesCountLimit = 20

class iPediaServerError:
    serverFailure=1
    unsupportedDevice=2
    invalidAuthorization=3
    malformedRequest=4

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
        self.getRandom=None
        self.linesCount=0
        self.searchExpression=None
        
    def getDatabase(self):
        if not self.db:
            self.db=self.factory.createConnection()
        return self.db

    def outputField(self, name, value=None):
        global g_fVerbose
        field=name
        if value:
            field+=fieldSeparator+value
        field+=lineSeparator
        self.transport.write(field)
        if g_fVerbose:
            print field
        
    def outputPayloadField(self, name, payload):
        global g_fVerbose
        self.outputField(name, str(len(payload)))
        self.transport.write(payload)
        self.transport.write(lineSeparator)
        if g_fVerbose:
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
        global g_fVerbose
        if self.error:
            self.outputField(errorField, str(self.error))
        self.transport.loseConnection()
        if self.db:
            self.logRequest()
            self.db.close()
            self.db=None

        if g_fVerbose:
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
        
    def preprocessDefinition(self, db, cursor, definition):
#        definition=iPediaDatabase.validateInternalLinks(db, cursor, definition)
        definition=definition.replace("{{CURRENTMONTH}}", str(datetime.date.today().month));
        definition=definition.replace("{{CURRENTMONTHNAME}}", datetime.date.today().strftime("%B"))
        definition=definition.replace("{{CURRENTMONTHNAMEGEN}}", datetime.date.today().strftime("%B"))
        definition=definition.replace("{{CURRENTDAY}}", str(datetime.date.today().day))
        definition=definition.replace("{{CURRENTDAYNAME}}", datetime.date.today().strftime("%A"))
        definition=definition.replace("{{CURRENTYEAR}}", str(datetime.date.today().year))
        definition=definition.replace("{{CURRENTTIME}}", datetime.date.today().strftime("%X"))
        definition=definition.replace("{{NUMBEROFARTICLES}}", str(self.factory.articleCount))
        return definition
        
    def handleDefinitionRequest(self):
        sys.stderr.write( "'%s' returned from handleDefinitionRequest()\n" % self.requestedTerm )
        cursor=None
        definition=None
        try:
            db=self.getDatabase()
            cursor=db.cursor()
            idTermDef=iPediaDatabase.findExactDefinition(db, cursor, self.requestedTerm)
            if idTermDef:
                self.definitionId, self.term, definition=idTermDef
            if definition:
                self.outputDefinition(self.preprocessDefinition(db, cursor, definition))
            else:
                self.termList=iPediaDatabase.findFullTextMatches(db, cursor, self.requestedTerm)
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
            return False
        return True
        
    def handleSearchRequest(self):
        cursor=None
        try:
            db=self.getDatabase()
            cursor=db.cursor()
            self.termList=iPediaDatabase.findFullTextMatches(db, cursor, self.searchExpression)
            if self.termList:
                self.outputField(resultsForField, self.searchExpression)
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
            return False
        return True

    def handleGetRandomRequest(self):
        cursor=None
        definition=None
        try:
            db=self.getDatabase()
            cursor=db.cursor()
            idTermDef=None
            while not idTermDef:
                idTermDef=iPediaDatabase.findRandomDefinition(db, cursor)
            self.definitionId, self.term, definition=idTermDef
            sys.stderr.write( "'%s' returned from handleGetRandomRequest()\n" % self.term )

            self.outputDefinition(self.preprocessDefinition(db, cursor, definition))
            cursor.close()
            
        except _mysql_exceptions.Error, ex:
            print ex
            if cursor:
                cursor.close()
            self.error=iPediaServerError.serverFailure
            return False
        return True

    def answer(self):
        global g_fVerbose

        if g_fVerbose:
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
            
        if self.searchExpression and not self.handleSearchRequest():
            return self.finish();

        if self.getRandom and not self.handleGetRandomRequest():
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
            if g_fVerbose:
                print request
            
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

            elif request.startswith(getRandomField):
                self.getRandom = True
                
            elif request.startswith(searchField):
                self.searchExpression=self.extractFieldValue(request)        
            else:
                self.error=iPediaServerError.malformedRequest

class iPediaFactory(protocol.ServerFactory):

    def createConnection(self):
        return MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='ipedia')
        
    def __init__(self):
        global g_articleCount
        db=self.createConnection()
        cursor=db.cursor()
        cursor.execute("""select count(*), min(id), max(id) from definitions""")
        row=cursor.fetchone()
        self.articleCount=row[0]
        self.minDefinitionId=row[1]
        self.maxDefinitionId=row[2]
        g_articleCount=row[0]
        print "Number of Wikipedia articles: ", self.articleCount
        cursor.close()
        db.close()

    protocol = iPediaProtocol

def main():
    global g_fVerbose, g_fPsycoAvailable
    g_fVerbose, iPediaDatabase.g_fVerbose = True, True
    if arsutils.fDetectRemoveCmdFlag( "-silent" ):
        g_fVerbose, iPediaDatabase.g_fVerbose = False, False

    fUsePsyco = arsutils.fDetectRemoveCmdFlag("-usepsyco")
    if g_fPsycoAvailable and fUsePsyco:
        print "using psyco"
        psyco.full()

    reactor.listenTCP(9000, iPediaFactory())
    reactor.run()

if __name__ == "__main__":
    main()

