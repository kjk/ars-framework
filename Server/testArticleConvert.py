# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Automated unit testing of Wikipedia article conversion routines
# Usage:
#   testFileName : file with tests to use

import sys,os,os.path,string,time,md5,bz2,articleconvert
try:
    import psyco
    psyco.full()
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"

def fFileExists(filePath):
    try:
        st = os.stat(filePath)
    except OSError:
        # TODO: should check that Errno is 2
        return False
    return True

def getRemoveCmdArg(argName):
    argVal = None
    try:
        pos = sys.argv.index(argName)
        argVal = sys.argv[pos+1]
        sys.argv[pos:pos+2] = []
    except:
        pass
    return argVal

def fDetectRemoveCmdFlag(flag):
    fFlagPresent = False
    try:
        pos = sys.argv.index(flag)
        fFlagPresent = True
        sys.argv[pos:pos+1] = []
    except:
        pass
    return fFlagPresent

g_startTime = None
g_endTime = None

def startTiming():
    global g_startTime
    g_startTime = time.clock()

def endTiming():
    global g_endTime
    g_endTime = time.clock()

def dumpTiming():
    global g_startTime, g_endTime
    dur = g_endTime - g_startTime
    str = "duration %f seconds\n" % dur
    sys.stderr.write(str)

def usageAndExit():
    print "Usage: testArticleConvert.py testFileName"
    sys.exit(0)

def convertFile(inName,limit,fJustStats=False,fSkipIfExists=False):
    if not fJustStats:
        txtName = getBodyFileName(inName)
        idxFileName = getIdxFileName(inName)
        redirectsFileName = getRedirectsFileName(inName)

        if fFileExists(txtName) and fFileExists(idxFileName) and fFileExists(redirectsFileName):
            if fSkipIfExists:
                print "files exist and fSkipIfExists==True so skipping creation"
                return
            else:
                print "files exist bug fSkipIfExists==False so create anyway"

        foIdx = open(idxFileName,"wb")
        foTxt = open(txtName, "wb")
        foRedirects = open(redirectsFileName, "wb")
    stats = WikipediaStats()
    count = 0
    startTiming()
    curPos = 0
    for article in wikipediasql.iterWikipediaArticles(inName):
        #print sqlArgs
        stats.addStats(article)
        title = article.getTitle().strip()
        viewCount = article.getViewCount()
        registerMostViewed(title,viewCount)
        if not fJustStats:
            ns = article.getNs()
            txt = article.getText()

            if article.fRedirect():
                foRedirects.write("%s\n" % title)
                foRedirects.write("%s\n" % txt)
            else:
                if 0==txt.find("#REDIRECT [["):
                    print "%s is a REDIRECT without being marked as such" % txt
                txtLen = len(txt)
                md5Hash = md5.new(txt)

                foIdx.write("%s\n" % title)
                foIdx.write("%d,%d,%d,%s\n" % (ns, curPos, txtLen, md5Hash.hexdigest()))
                curPos += txtLen
                foTxt.write(txt)

        count += 1
        if 0 == count % 2000:
            print "processed %d items" % count
        if count>=limit:
            break
    if not fJustStats:
        foTxt.close()
        foIdx.close()
        foRedirects.close()
    endTiming()
    stats.dumpStats()
    dumpMostViewed()
    dumpTiming()

class ConvertTest:
    def __init__(self,name,orig,expected):
        self.name = name
        self.orig = orig
        self.expected = expected

    def setConverted(self,converted):
        self.converted = converted

def iterTests(fileName):
    fo = open(fileName,"rb")
    # skip comments and empty lines at the beginning of the file
    while True:
        line = fo.readline()
        if len(line) == 0:
            print "File with no tests in it!"
            return
        if len(line.strip())==0:
            continue
        if line[0] == '#':
            continue
        break

    while len(line) != 0:
        # we have a line to process from the code above
        if line[0] != '@':
            print "! line should start with '@' (line: '%s')" % line
            assert line[0] == '@'
        line = line.strip()
        name = line[1:]
        # read the text before conversion and expected result of conversion
        orig = ""
        expected = ""
        fReadingOrig = True
        line = fo.readline()
        while True:
            if len(line)==0:
                if fReadingOrig:
                    print "Test with name '%s' interrupted in the middle" % name
                    assert len(line)!=0
                else:
                    # this is the end of file
                    test = ConvertTest(name,orig,expected)
                    yield test
                    fo.close()
                    return
            if line[0] == '@':
                if fReadingOrig:
                    fReadingOrig = False
                else:
                    # this is a beginning of a new test
                    test = ConvertTest(name,orig,expected)
                    yield test
                    break
            else:
                if fReadingOrig:
                    orig += line
                else:
                    expected += line
            line = fo.readline()
    # and end of iterTests

failedList = []
def dumpFailed():
    for test in failedList:
        print "Test with name '%s' failed" % test.name
        print "orig: '%s'" % test.orig.strip()
        print "expe: '%s'" % test.expected.strip()
        print "got : '%s'" % test.converted.strip()

def testTests(fileName):
    for test in iterTests(fileName):
        print "name: '%s'" % test.name
        print "orig: '%s'" % test.orig
        print "expe: '%s'" % test.expected

def runTests(fileName):
    testCount = 0
    failedCount = 0
    for test in iterTests(fileName):
        orig = test.orig
        expected = test.expected
        converted = articleconvert.convertArticle(test.name,orig)
        if converted.strip() != expected.strip():
            failedCount += 1
            test.setConverted(converted)
            failedList.append(test)
            sys.stdout.write("-")
        else:
            sys.stdout.write(".")
        testCount += 1
    print
    print "Total  tests: %d" % testCount
    print "Failed tests: %d" % failedCount
    dumpFailed()


if __name__=="__main__":
    # now we should only have file name
    if len(sys.argv) != 2:
        usageAndExit()
    fileName = sys.argv[1]
    runTests(fileName)
    #testTests(fileName)

