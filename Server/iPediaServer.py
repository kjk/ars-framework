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
#    -db name : use database name

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
getArticleCountField=   "Get-Article-Count"
articleCountField=      "Article-Count"

definitionFormatVersion = 1
protocolVersion = 1

requestLinesCountLimit = 20

#code from http://www.noah.org/python/daemonize.py
'''This module is used to fork the current process into a daemon.

Almost none of this is necessary (or advisable) if your daemon 
is being started by inetd. In that case, stdin, stdout and stderr are 
all set up for you to refer to the network connection, and the fork()s 
and session manipulation should not be done (to avoid confusing inetd). 
Only the chdir() and umask() steps remain as useful. 

References:
    UNIX Programming FAQ
        1.7 How do I get my program to act like a daemon?
        http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC16
   
    Advanced Programming in the Unix Environment
        W. Richard Stevens, 1992, Addison-Wesley, ISBN 0-201-56317-7.
'''

def daemonize (stdin='/dev/null', stdout='/dev/null', stderr='/dev/null'):
    '''This forks the current process into a daemon.
    The stdin, stdout, and stderr arguments are file names that
    will be opened and be used to replace the standard file descriptors
    in sys.stdin, sys.stdout, and sys.stderr.
    These arguments are optional and default to /dev/null.
    Note that stderr is opened unbuffered, so
    if it shares a file with stdout then interleaved output
    may not appear in the order that you expect.
    '''

    # Do first fork.
    try: 
        pid = os.fork() 
        if pid > 0:
            sys.exit(0)   # Exit first parent.
    except OSError, e: 
        sys.stderr.write ("fork #1 failed: (%d) %s\n" % (e.errno, e.strerror) )
        sys.exit(1)

    # Decouple from parent environment.
    os.chdir("/") 
    os.umask(0) 
    os.setsid() 

    # Do second fork.
    try: 
        pid = os.fork() 
        if pid > 0:
            sys.exit(0)   # Exit second parent.
    except OSError, e: 
        sys.stderr.write ("fork #2 failed: (%d) %s\n" % (e.errno, e.strerror) )
        sys.exit(1)

    # Now I am a daemon!
    
    # Redirect standard file descriptors.
    si = open(stdin, 'r')
    so = open(stdout, 'a+')
    se = open(stderr, 'a+', 0)
    os.dup2(si.fileno(), sys.stdin.fileno())
    os.dup2(so.fileno(), sys.stdout.fileno())
    os.dup2(se.fileno(), sys.stderr.fileno())

def mainDeamonTest():
    '''This is an example main function run by the daemon.
    This prints a count and timestamp once per second.
    '''
    import time
    sys.stdout.write ('Daemon started with pid %d\n' % os.getpid() )
    sys.stdout.write ('Daemon stdout output\n')
    sys.stderr.write ('Daemon stderr output\n')
    c = 0
    while 1:
        sys.stdout.write ('%d: %s\n' % (c, time.ctime(time.time())) )
        sys.stdout.flush()
        c = c + 1
        time.sleep(1)

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
        self.getArticleCount=False
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
            
        if self.getArticleCount:
            self.outputField(articleCountField, str(self.factory.articleCount))

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
                
            elif request.startswith(getArticleCountField):
                self.getArticleCount=True
            else:
                self.error=iPediaServerError.malformedRequest

class iPediaFactory(protocol.ServerFactory):

    def createConnection(self):
        return MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db=self.dbName)
        
    def __init__(self, dbName):
        global g_articleCount
        self.dbName=dbName
        db=self.createConnection()
        cursor=db.cursor()
        cursor.execute("""select count(*), min(id), max(id) from definitions""")
        row=cursor.fetchone()
        g_articleCount=self.articleCount=row[0]
        self.minDefinitionId=row[1]
        self.maxDefinitionId=row[2]
        print "Number of Wikipedia articles: ", self.articleCount
        cursor.close()
        db.close()

    def changeDatabase(self, newDb):
        self.dbName=newDb
        
    protocol = iPediaProtocol
    
class iPediaTelnetProtocol(basic.LineReceiver):

    listRe=re.compile(r'\s*list\s*', re.I)
    useDbRe=re.compile(r'\s*use\s+(\w+)\s*', re.I)
    
    def listDatabases(self):
        db=None
        cursor=None
        try:
            db=self.factory.iPediaFactory.createConnection()
            cursor=db.cursor()
            cursor.execute("show databases like 'ipedia%'")
            row=cursor.fetchone()
            while row:
                self.transport.write(row[0]+'\r\n')
                row=cursor.fetchone()
        except _mysql_exceptions.Error, ex:
            print ex
            self.transport.write("exception\r\n")
        if cursor:            
            cursor.close()
        if db:            
            db.close()

    def useDatabase(self, dbName):
        self.factory.iPediaFactory.changeDatabase(dbName)
                    
    def lineReceived(self, request):
        if iPediaTelnetProtocol.listRe.match(request):
            self.listDatabases()
        else:
            match=iPediaTelnetProtocol.useDbRe.match(request)
            if match:
                self.useDatabase(match.group(1))
            else:
                self.transport.loseConnection()
                
    
class iPediaTelnetFactory(protocol.ServerFactory):

    def __init__(self, otherFactory):
        self.iPediaFactory=otherFactory 
    
    protocol=iPediaTelnetProtocol
        

def main():
    global g_fVerbose, g_fPsycoAvailable
    g_fVerbose=iPediaDatabase.g_fVerbose = True
    if arsutils.fDetectRemoveCmdFlag( "-silent" ):
        g_fVerbose, iPediaDatabase.g_fVerbose = False, False

    fUsePsyco = arsutils.fDetectRemoveCmdFlag("-usepsyco")
    if g_fPsycoAvailable and fUsePsyco:
        print "using psyco"
        psyco.full()

    dbName=arsutils.fDetectRemoveCmdFlag("-db")
    if not dbName: 
        dbName='ipedia'
                
    factory=iPediaFactory(dbName)
    reactor.listenTCP(9000, factory)
    reactor.listenTCP(9001, iPediaTelnetFactory(factory))
    reactor.run()

if __name__ == "__main__":
    #TODO: only for unix
    #daemonize('/dev/null','/tmp/daemon.log','/tmp/daemon.log')
    main()

