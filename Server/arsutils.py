# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Collect routines frequently used in other places

import sys,time

def fIsBzipFile(inFileName):
    if len(inFileName)>4 and ".bz2" == inFileName[-4:]:
        return True
    return False

def fDetectRemoveCmdFlag(flag):
    fFlagPresent = False
    try:
        pos = sys.argv.index(flag)
        fFlagPresent = True
        sys.argv[pos:pos+1] = []
    except:
        pass
    return fFlagPresent

# given argument name in argName, tries to return argument value
# in command line args and removes those entries from sys.argv
# return None if not found
def getRemoveCmdArg(argName):
    argVal = None
    try:
        pos = sys.argv.index(argName)
        argVal = sys.argv[pos+1]
        sys.argv[pos:pos+2] = []
    except:
        pass
    return argVal

def getRemoveCmdArgInt(argName):
    argVal = getRemoveCmdArg(argName)
    if argVal:
        argVal = int(argVal)
    return argVal

class Timer:
    def __init__(self,fStart=False):
        self.startTime = None
        self.endTime = None
        if fStart:
            self.start()
    def start(self):
        self.startTime = time.clock()
    def stop(self):
        self.endTime = time.clock()
    def dumpInfo(self):
        dur = self.endTime - self.startTime
        txt = "duration %f seconds\n" % dur
        sys.stderr.write(txt)

def fFileExists(filePath):
    try:
        st = os.stat(filePath)
    except OSError:
        # TODO: should check that Errno is 2
        return False
    return True

