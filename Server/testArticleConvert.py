# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Automated unit testing of Wikipedia article conversion routines
# Usage:
#   testFileName : file with tests to use
#
import sys,os,os.path,string,time,md5,bz2,articleconvert
try:
    import psyco
    psyco.full()
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"

def usageAndExit():
    print "Usage: testArticleConvert.py testFileName"
    sys.exit(0)

class ConvertTest:
    def __init__(self,name,orig,expected):
        self.name = name
        self.orig = orig
        self.expected = expected

    def setConverted(self,converted):
        self.converted = converted

def fShortTest(line):
    if len(line)>2 and line[:2]=="@s":
        return True
    return False

# short test is in the form:
# @s2 orig*expected
# i.e.: "@s", name of the test (everything up to a space),
# original text and expected text separated by a "*"
def parseShortTest(line):
    assert line[:2] == "@s"
    line = line[2:]
    parts = line.split(" ",1)
    assert len(parts)==2
    name = parts[0]
    txt = parts[1]
    (orig,expected) = txt.split("*")    
    return (name,orig,expected)

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

    # we skip comments (lines beginning with '#') if they appear
    # right after new test mark (line beginning with '@')
    fSkipComments = False
    while len(line) != 0:
        # we have a line to process from the code above
        if line[0] != '@':
            print "! line should start with '@' (line: '%s')" % line
            assert line[0] == '@'
        line = line.strip()
        if fShortTest(line):
            (name,orig,expected) = parseShortTest(line)
            test = ConvertTest(name,orig,expected)
            yield test
            line = fo.readline()
            continue
        name = line[1:]
        # read the text before conversion and expected result of conversion
        orig = ""
        expected = ""
        fReadingOrig = True
        line = fo.readline()
        fSkipComments = True
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
                if fSkipComments and line[0]=='#':
                    #do nothing
                    pass
                else:
                    fSkipComments = False
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
        print "orig    : '%s'" % test.orig.strip()
        print "expected: '%s'" % test.expected.strip()
        print "got     : '%s'" % test.converted.strip()

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

