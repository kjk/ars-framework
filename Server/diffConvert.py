# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Purpose:
#   Shows the result of conversion a given wikipedia article (or a random
#   article) using graphical diff (e.g. windiff)
#
#   The idea is that we can use this script to visually inspect how a given
#   article is being converted by our conversion script.
#
#   Currently supports diffing using
#    - windiff (Microsoft tool that can be freely downloaded and usually comes with VisualStudio)
#    - AraxisMerge : commercial tool
#    - WinMerge : free, http://winmerge.sourceforge.net/
#   Adding support for others is trivial (see diffWithWindiff() etc.)
#
# Usage:
#   -limit N : only process first N rows
#   -random : show a random word
#   -title $title : show diff of a given title
#   -dump : dump before and after to stdout
#   fileName - which sql file to process
#
# TODO:
#  - when using -title and doesn't find the aritcle in main, should also check
#    *_redirects.txt file
#
import sys,os,os.path,string,re,random,time,md5,bz2
import arsutils,wikipediasql,dumpSqlToTxt,articleconvert
try:
    import process
except:
    print "requires process module (http://starship.python.net/crew/tmick/)"
    sys.exit(0)

try:
    import psyco
    psyco.full()
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"

# windiff doesn't do that well with long lines so I break long lines into
# a paragraph. It does make the text uglier but for our purpose we don't
# really care
# if using a better diff (e.g. Araxis Merge) program, this could be set to False
# in which case we don't reformat the text
g_reformatLongLines = True

# which diff tool to use. AraxisMerge is the best, imo (shows character-level
# diffs, not only line-level)
(DIFF_WINDIFF,DIFF_ARAXIS, DIFF_WINMERGE) = range(3)

g_DiffTool = DIFF_WINDIFF
#g_DiffTool = DIFF_ARAXIS
#g_DiffTool = DIFF_WINMERGE

if g_DiffTool == DIFF_ARAXIS:
    g_reformatLongLines = False

def usageAndExit():
    print "Usage: diffConvert.py [-limit N] [-random] [-title foo] fileName"
    sys.exit(0)

def fIsBzipFile(inFileName):
    if len(inFileName)>4 and ".bz2" == inFileName[-4:]:
        return True
    return False

def getBaseFileName(fileName):
    suf = ".bz2"
    sufLen = len(suf)
    if len(fileName)>sufLen and suf == fileName[-sufLen:]:
        fileName = fileName[:-sufLen]
        #print "new file name is %s" % fileName

    suf = ".sql"
    sufLen = len(suf)
    if len(fileName)>sufLen and suf == fileName[-sufLen:]:
        fileName = fileName[:-sufLen]
        #print "new file name is %s" % fileName
    else:
        print "%s is not a valid input file. Must be a *.sql or *.sql.bz2 file"
        sys.exit(0)
    return fileName

def genBaseAndSuffix(inFileName,suffix):
    return getBaseFileName(inFileName) + suffix

def getIdxFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_idx.txt")

def getRedirectsFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_redirects.txt")

def getBodyFileName(inFileName):
    return genBaseAndSuffix(inFileName,"_body.txt")

def getTxt(sqlFileName,txtOff,txtLen):
    fn = getBodyFileName(sqlFileName)
    fo = open(fn,"rb")
    fo.seek(txtOff)
    txt = fo.read(txtLen)
    fo.close()
    return txt

class ArticleInfo:
    def __init__(self,sqlFileName,title,ns,txtOffset,txtLen,md5Hash):
        self.sqlFileName = sqlFileName
        self.title = title
        self.ns = ns
        self.txtOffset = txtOffset
        self.txtLen = txtLen
        self.md5Hash = md5Hash
    def getTitle(self): return self.title
    def getNs(self): return self.ns
    def getHash(self): return self.md5Hash
    def getTxt(self): return getTxt(self.sqlFileName, self.txtOffset, self.txtLen)

NS_MAIN = 0

def iterArticles(sqlFileName,fSkipNonMain=True):
    fileName = getIdxFileName(sqlFileName)
    fo = open(fileName,"rb")
    while True:
        title = fo.readline()
        if len(title)==0:
            break
        #print "Title: %s" % title
        dataTxt = fo.readline()
        #print "data: %s" % dataTxt
        dataTuple = dataTxt.split(",")
        ns = int(dataTuple[0])
        if fSkipNonMain and ns != NS_MAIN:
            continue
        txtOffset = int(dataTuple[1])
        txtLen = int(dataTuple[2])
        md5Hash = dataTuple[3]
        article = ArticleInfo(sqlFileName,title,ns,txtOffset,txtLen,md5Hash)
        yield article
    fo.close()

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

def diffWithWindiff(orig,converted):
    p = process.ProcessOpen(["windiff.exe", orig, converted])
    fFinishProcess(p,True)

def diffWithAraxis(orig,converted):
    p = process.ProcessOpen(["C:\Program Files\Araxis Merge 2001\compare.exe", orig, converted])
    fFinishProcess(p,True)

def diffWithWinMerge(orig,converted):
    p = process.ProcessOpen(["c:\Program Files\WinMerge\WinMergeU.exe", orig, converted])
    fFinishProcess(p,True)

def showDiff(article):
    global g_reformatLongLines, g_DiffTool

    title = article.getTitle().strip() + "\n"
    txt = article.getTxt()
    if g_reformatLongLines:
        txt = fill_paragraphs(txt,80)

    converted = articleconvert.convertArticle(txt)
    origTmpName = "c:\\txtOrig.txt"
    convertedTmpName = "c:\\txtConverted.txt"
    fo = open(origTmpName,"wb")
    fo.write(title)
    fo.write(txt)
    fo.close()
    fo = open(convertedTmpName,"wb")
    fo.write(title)
    fo.write(converted)
    fo.close()
    if g_DiffTool == DIFF_WINDIFF:
        diffWithWindiff(origTmpName,convertedTmpName)
    if g_DiffTool == DIFF_ARAXIS:
        diffWithAraxis(origTmpName,convertedTmpName)
    if g_DiffTool == DIFF_WINMERGE:
        diffWithWinMerge(origTmpName,convertedTmpName)

# algorithm based on http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/59865
# a bit slow since it must read the whole file
def getRandomArticle(fileName):
    articleNum = 0
    randomArticle = None
    for article in iterArticles(fileName):
        articleNum += 1
        # How likely is it that this is the last line of the file ?
        # kjk note: I don't get it
        if random.uniform(0,articleNum)<1:
            randomArticle = article
        if articleNum % 30000 == 0:
            print "processed %d articles" % articleNum
    return randomArticle

def showDiffRandom(fileName):
    # at this point shows the diff of the first article
    article = getRandomArticle(fileName)
    showDiff(article)

def findArticle(fileName,title):
    #title = string.lower(title.replace(" ", "_"))
    #title = string.lower(title.replace(" ", "_"))
    titleLower = title.lower()
    print "looking for article with title %s" % titleLower
    count = 0
    for article in iterArticles(fileName):
        #if article.getTitle().lower() == titleLower:
        title = article.getTitle().lower.strip()
        if 0 == title.find(titleLower):
            return article
        if count % 1000 == 0:
            print "processed %d articles, last title %s" % (count,article.getTitle().lower().strip())
        count += 1
    return None

def iterArticlesMatchingTitle(fileName,title):
    #title = string.lower(title.replace(" ", "_"))
    #title = string.lower(title.replace(" ", "_"))
    titleLower = title.lower()
    print "looking for article with title %s" % titleLower
    count = 0
    for article in iterArticles(fileName):
        title = article.getTitle().lower().strip()
        if -1 != title.find(titleLower):
            yield article
        if count % 1000 == 0:
            print "processed %d articles, last title %s" % (count,article.getTitle().lower().strip())
        count += 1

def iterArticlesExactTitle(fileName,title):
    #title = string.lower(title.replace(" ", "_"))
    #title = string.lower(title.replace(" ", "_"))
    titleLower = title.lower()
    print "looking for article with title %s" % titleLower
    count = 0
    for article in iterArticles(fileName):
        title = article.getTitle().lower().strip()
        #if 0 == title.find(titleLower):
        if title == titleLower:
            yield article
        if count % 1000 == 0:
            print "processed %d articles, last title %s" % (count,article.getTitle().lower().strip())
        count += 1

def showDiffTitle(fileName,title):
    article = findArticle(fileName,title)
    if not article:
        print "couldn't find article with the title %s" % title
        return
    showDiff(article)

def dumpArticle(fileName,title):
    for article in iterArticlesExactTitle(fileName,title):
        if not article:
            print "couldn't find the body of article %s" % title
            return
        title = article.getTitle().strip() + "\n"
        txt = article.getTxt()
        converted = articleconvert.convertArticle(title,txt)
        print "TITLE: %s" % title
        print "ORIGINAL: %s" % txt
        print "CONVERTED: %s" % converted
        return

if __name__=="__main__":
    limit = arsutils.getRemoveCmdArgInt("-limit")
    if None == limit:
        limit = 9999999 # very big number

    fRandom = arsutils.fDetectRemoveCmdFlag("-random")
    fDump = arsutils.fDetectRemoveCmdFlag("-dump")

    if fDump and fRandom:
        print "Can't use -dump and -random at the same time"
        usageAndExit()

    title = arsutils.getRemoveCmdArg("-title")

    if title and fRandom:
        print "Can't use -title and -random at the same time"
        usageAndExit()

    if not title and not fRandom:
        print "Have to provide either -title or -random"
        usageAndExit()

    # now we should only have file name
    if len(sys.argv) != 2:
        print "Have to provide *.sql or *.sql.bz2 file name with wikipedia dump"
        usageAndExit()
    fileName = sys.argv[1]

    dumpSqlToTxt.convertFile(fileName,limit,fJustStats=False,fSkipIfExists=True)

    if fRandom:
        showDiffRandom(fileName)
    else:
        assert title
        if fDump:
            dumpArticle(fileName,title)
        else:
            showDiffTitle(fileName,title)

