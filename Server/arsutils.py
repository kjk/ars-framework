# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Collect routines frequently used in other places

import os,sys,time
try:
    import process
except:
    print "requires process module (http://starship.python.net/crew/tmick/)"
    sys.exit(0)

# which diff tool to use. AraxisMerge is the best, imo (shows character-level
# diffs, not only line-level)
(DIFF_WINDIFF,DIFF_ARAXIS, DIFF_WINMERGE) = range(3)

#g_DiffTool = DIFF_WINDIFF
g_DiffTool = DIFF_ARAXIS
#g_DiffTool = DIFF_WINMERGE

# windiff doesn't do that well with long lines so I break long lines into
# a paragraph. It does make the text uglier but for our purpose we don't
# really care
# if using a better diff (e.g. Araxis Merge) program, this could be set to False
# in which case we don't reformat the text
g_reformatLongLines = True

if g_DiffTool == DIFF_ARAXIS:
    g_reformatLongLines = False

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

def normalizeNewlines(txt):
    txt = txt.strip()  
    crlf = chr(13)+chr(10)
    lf = chr(10)
    return txt.replace(crlf, lf)

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

def fExecutedCorrectly(stderrTxt):
    if -1 == stderrTxt.find("is not recognized"):
        return True
    else:
        return False

def fFinishProcess(proc,fPrintStdout=False):
    res_stdout = proc.stdout.read()
    res_stderr = proc.stderr.read()
    status = proc.wait()
    if fPrintStdout:
        print res_stdout
        print res_stderr
    return fExecutedCorrectly(res_stderr)

def diffWithWindiff(orig,converted):
    p = process.ProcessOpen(["windiff.exe", orig, converted])
    fFinishProcess(p,True)

def diffWithAraxis(orig,converted):
    p = process.ProcessOpen(["C:\Program Files\Araxis Merge 2001\compare.exe", orig, converted])
    fFinishProcess(p,True)

def diffWithWinMerge(orig,converted):
    p = process.ProcessOpen(["c:\Program Files\WinMerge\WinMergeU.exe", orig, converted])
    fFinishProcess(p,True)

# code from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/134571
def justify_line(line, width):
    """Stretch a line to width by filling in spaces at word gaps.

    The gaps are picked randomly one-after-another, before it starts
    over again.

    """
    i = []
    while 1:
        # line not long enough already?
        if len(' '.join(line)) < width:
            if not i:
                # index list is exhausted
                # get list if indices excluding last word
                i = range(max(1, len(line)-1))
                # and shuffle it
                random.shuffle(i)
            # append space to a random word and remove its index
            line[i.pop(0)] += ' '
        else:
            # line has reached specified width or wider
            return ' '.join(line)

def fill_paragraphs(text, width=80, justify=0):
    """Split a text into paragraphs and wrap them to width linelength.

    Optionally justify the paragraphs (i.e. stretch lines to fill width).

    Inter-word space is reduced to one space character and paragraphs are
    always separated by two newlines. Indention is currently also lost.

    """
    # split taxt into paragraphs at occurences of two or more newlines
    paragraphs = re.split(r'\n\n+', text)
    for i in range(len(paragraphs)):
        # split paragraphs into a list of words
        words = paragraphs[i].strip().split()
        line = []; new_par = []
        while 1:
           if words:
               if len(' '.join(line + [words[0]])) > width and line:
                   # the line is already long enough -> add it to paragraph
                   if justify:
                       # stretch line to fill width
                       new_par.append(justify_line(line, width))
                   else:
                       new_par.append(' '.join(line))
                   line = []
               else:
                   # append next word
                   line.append(words.pop(0))
           else:
               # last line in paragraph
               new_par.append(' '.join(line))
               line = []
               break
        # replace paragraph with formatted version
        paragraphs[i] = '\n'.join(new_par)
    # return paragraphs separated by two newlines
    return '\n\n'.join(paragraphs)

def showTxtDiff(txtOne, txtTwo, fReformatLongLines=False):
    txtOneName = "c:\\txtOne.txt"
    txtTwoName = "c:\\txtTwo.txt"

    if fReformatLongLines:
        txtOne = fill_paragraphs(txtOne,80)
        txtTwo = fill_paragraphs(txtTwo,80)

    fo = open(txtOneName,"wb")
    #fo.write(title)
    fo.write(txtOne)
    fo.close()
    fo = open(txtTwoName,"wb")
    #fo.write(title)
    fo.write(txtTwo)
    fo.close()
    if g_DiffTool == DIFF_WINDIFF:
        diffWithWindiff(txtOneName,txtTwoName)
    if g_DiffTool == DIFF_ARAXIS:
        diffWithAraxis(txtOneName,txtTwoName)
    if g_DiffTool == DIFF_WINMERGE:
        diffWithWinMerge(txtOneName,txtTwoName)

def showTxtDiffArray(txtOneArray, txtTwoArray, fReformatLongLines=False):
    txtOneName = "c:\\txtOne.txt"
    txtTwoName = "c:\\txtTwo.txt"

    assert len(txtOneArray)==len(txtTwoArray)

    foOne = open(txtOneName,"wb")
    foTwo = open(txtTwoName,"wb")

    for (txtOne,txtTwo) in zip(txtOneArray,txtTwoArray):
        if fReformatLongLines:
            txtOne = fill_paragraphs(txtOne,80)
            txtTwo = fill_paragraphs(txtTwo,80)
        foOne.write(txtOne)
        foTwo.write(txtTwo)

    foOne.close()
    foTwo.close()

    if g_DiffTool == DIFF_WINDIFF:
        diffWithWindiff(origTmpName,convertedTmpName)
    if g_DiffTool == DIFF_ARAXIS:
        diffWithAraxis(origTmpName,convertedTmpName)
    if g_DiffTool == DIFF_WINMERGE:
        diffWithWinMerge(origTmpName,convertedTmpName)

