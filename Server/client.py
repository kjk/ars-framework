# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Emulates client (Palm) application by issuing requests to the server.
# For testing the server.
#
# TODO:
#   add cmd line options -get term, -get-random for driving the script from cmd line

import sys, re, socket, random, pickle

# server string must be of form "name:port"
g_serverList = ["localhost:9000", "dict-pc.arslexis.com:9000"]

g_defaultServerNo = 0 # index within g_serverList

g_cookie = None
g_exampleDeviceInfo = "HS50616C6D204F5320456D756C61746F72:OC70616C6D:OD00000000"

TRANSACTION_ID  = "Transaction-ID:"
GET_COOKIE      = "Get-Cookie:"
COOKIE          = "Cookie:"
FORMAT_VER      = "Format-Version:"
RESULTS_FOR     = "Results-For:"
DEFINITION      = "Definition:"
PROTOCOL_VER    = "Protocol-Version:"
CLIENT_VER      = "Client-Version:"
GET_DEF         = "Get-Definition:"
GET_RANDOM      = "Get-Random-Definition:"

g_pickleFileName = "client_pickled_data.dat"
def pickleState():
    global g_pickleFileName,g_cookie
    # save all the variables that we want to persist across session on disk
    fo = open(g_pickleFileName, "wb")
    pickle.dump(g_cookie,fo)
    fo.close()

def unpickleState():
    global g_cookie
    # restores all the variables that we want to persist across session from
    # the disk
    try:
        fo = open(g_pickleFileName, "rb")
    except IOError:
        # it's ok to not have the file
        return
    g_cookie = pickle.load(fo)
    fo.close()

def getGlobalCookie():
    global g_cookie
    return g_cookie

def getServerNamePort():
    srv = g_serverList[g_defaultServerNo]
    (name,port) = srv.split(":")
    port = int(port)
    return (name,port)

def socket_readAll(sock):
    result = ""
    while True:
        data = sock.recv(10)
        if 0 == len(data):
            break
        result += data
    return result

def buildLine( field, value):
    return "%s %s\n" % (field,value)

def buildCommonRequestPart():
    protocolVer = "1"
    clientVer = "0.5"
    transactionId = "%x" % random.randint( 0, 2**32-1)

    req  = buildLine(PROTOCOL_VER, protocolVer)
    req += buildLine(CLIENT_VER, clientVer)
    req += buildLine(TRANSACTION_ID, transactionId)
    return req

def buildGetRandomDefinitionRequest():
    req = buildCommonRequestPart()
    if getGlobalCookie():
        req += buildLine(COOKIE, getGlobalCookie())
    else:
        req += buildLine(GET_COOKIE, g_exampleDeviceInfo)
    req += GET_RANDOM + "\n"
    req += "\n"
    return req

def buildGetDefinitionRequest(term):
    req = buildCommonRequestPart()
    if getGlobalCookie():
        req += buildLine(COOKIE, getGlobalCookie())
    else:
        req += buildLine(GET_COOKIE, g_exampleDeviceInfo)
    req += buildLine(GET_DEF, term)
    req += "\n"
    return req

# parser server response. Returns a dictionary where keys are the
# names of fields e.g. like FORMAT_VER, COOKIE and values their values
# returns None if there was an error parsing (the response didn't follow
# the format we expect)
def parseServerResponse(response):
    result = {}
    defTxt = ""
    defLenLeft = 0
    fWasEmptyLine = False
    for fld in response.split("\n"):
        if 0==len(fld) and 0==defLenLeft:
            #assert not fWasEmptyLine
            fWasEmptyLine = True
            continue
        #print "line: _%s_" % fld
        if defLenLeft > 0:
            # this is a part of DEFINITION part of the response
            defTxt += fld + "\n"
            defLenLeft -= (len(fld) + 1)
            if 0 == defLenLeft:
                result[DEFINITION] = defTxt
            #print "*** defLenLeft=%d" % defLenLeft
            continue
        keyPos = fld.find(":")
        if keyPos == -1:
            print "*** didn't find ':' in " + fld
            return None
        key = fld[:keyPos+1]
        if fld[keyPos+1] != ' ':
            print "'%s' and not ' ' is at pos %d in '%s'" % (fld[keyPos+1], keyPos+1, fld)
            return None
        value = fld[keyPos+2:]
        #print "key: _%s_" % key
        #print "val: _%s_" % value
        if key == DEFINITION:
            defLenLeft = int(value)
            #print "*** defLenLeft=%d" % defLenLeft
        else:
            result[key] = value
    return result

def getDef(term):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    (serverName, serverPort) = getServerNamePort()
    sock.connect((serverName,serverPort))
    #print "Connected to server"

    req = buildGetDefinitionRequest("home")
    #print "Sening:", req
    sock.sendall(req)
    #print "Sent all"
    response = socket_readAll(sock)
    #print "Received:", response
    sock.close()
    return response

def getRandomDef():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    (serverName, serverPort) = getServerNamePort()
    sock.connect((serverName,serverPort))
    #print "Connected to server"

    req = buildGetRandomDefinitionRequest()
    #print "Sening:", req
    sock.sendall(req)
    #print "Sent all"
    response = socket_readAll(sock)
    #print "Received:", response
    sock.close()
    return response

def doGetDef(term):
    global g_cookie
    term = "home"
    defResponse = getDef(term)
    print "term: %s" % term
    print "##### response:"
    print defResponse
    parsedResponse = parseServerResponse(defResponse)
    if not parsedResponse:
        print "FAILURE in parseServerResponse"
        sys.exit(0)
    if not getGlobalCookie() and parsedResponse.has_key(COOKIE):
        print "Found cookie: %s" % parsedResponse[COOKIE]
        g_cookie = parsedResponse[COOKIE]

def doGetRandomDef():
    global g_cookie
    defResponse = getRandomDef()
    print "# response:"
    print defResponse
    parsedResponse = parseServerResponse(defResponse)
    if not parsedResponse:
        print "FAILURE in parseServerResponse"
        sys.exit(0)
    if not getGlobalCookie() and parsedResponse.has_key(COOKIE):
        print "Found cookie: %s" % parsedResponse[COOKIE]
        g_cookie = parsedResponse[COOKIE]

def main():
    #doGetDef("home")
    doGetRandomDef()

if __name__=="__main__":
    try:
        unpickleState()
        main()
    finally:
        # make sure that we pickle the state even if we crash
        pickleState()
 
