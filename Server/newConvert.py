# Copyright: Krzysztof Kowalczyk
# Owner: Andrzej Ciarkowski
#
# Creates iPedia database from wikipedia sql dump
#
# Command line parameters:
#  -recreatedb : if given, recreate ipedia database
#  -recreatedatadb : if given, recreate ipedia data database
#  -verbose : if used will print a lot of debugging input to stdout. May slow
#             down conversion process
#  -limit n : limit the number of converter articles to N. This is good for testing
#             changes to the converter. It takes a lot of time to fetch all
#             original articles from enwiki db, this option will limit it to N
#             articles which should be enough to detect major problems with
#             the converter
# -showdups : sets g_fShowDups to True. The idea is that if we convert to an empty ipedia.articles
#             database, we should never have duplicates. If we have it means that we're loosing
#             some of the original articles. With this flag we dump dups to stdout.
#             Don't use if ipedia.articles isn't empty
# -nopsyco : if used, won't use psyco
# fileName : convert directly from sql file, no need for enwiki.cur database

import sys, os, datetime, MySQLdb
import  arsutils, wikipediasql,articleconvert
try:
    import psyco
    g_fPsycoAvailable = True
except:
    print "psyco not available. You should consider using it (http://psyco.sourceforge.net/)"
    g_fPsycoAvailable = False

# if True, we'll print a lot of debug text to stdout
g_fVerbose       = False

# if True, we'll show if we're updating ipedia table twice for the same title
# shouldn't happen if it was empty to begin with
g_fShowDups      = False

g_connRoot       = None
g_connIpedia     = None
g_connIpediaDbName = None

g_dbName = None

DATA_DB_NAME = "ipedia_data"

def usageAndExit():
    print "newConvert.py [-verbose] [-limit n] [-showdups] [-nopsyco] sqlDumpName"
    sys.exit(0)

def getOneResult(conn,query):
    cur = conn.cursor()
    cur.execute(query)
    row = cur.fetchone()
    res = row[0]
    cur.close()
    return res

def getRootConnection():
    global g_connRoot
    if g_connRoot:
        return g_connRoot
    g_connRoot = MySQLdb.Connect(host='localhost', user='root', passwd='', db='')
    return g_connRoot

def getIpediaConnection(dbName):
    global g_connIpedia, g_connIpediaDbName
    if dbName==None:
        assert g_connIpedia
        return g_connIpedia
    if g_connIpedia:
        assert dbName == g_connIpediaDbName
    g_connIpedia = MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db=dbName)
    g_connIpediaDbName = dbName
    return g_connIpedia

g_ipediaRowCount = None
def getIpediaRowCount():
    global g_ipediaRowCount
    if None == g_ipediaRowCount:
        conn = getIpediaConnection(None)
        g_ipediaRowCount = getOneResult(conn, """SELECT COUNT(*) FROM ipedia.articles;""")
    return g_ipediaRowCount

def printIpediaRowCount():
    ipediaRows = getIpediaRowCount()
    sys.stderr.write("rows in ipedia: %d\n" % ipediaRows)

def deinitDatabase():
    global g_connIpedia, g_connRoot
    print "deinitDatabase()"
    closeAllNamedCursors()
    if g_connIpedia:
        g_connIpedia.close()
    if g_connRoot:
        g_connRoot.close()

g_namedCursors = {}
def getNamedCursor(conn,curName):
    global g_namedCursors
    # a little pooling of cursors based on their names
    # the idea is to save time by not calling conn.cursor()/conn.close()
    if not g_namedCursors.has_key(curName):
        g_namedCursors[curName] = conn.cursor()
    return g_namedCursors[curName]

def closeNamedCursor(curName):
    global g_namedCursors
    if g_namedCursors.has_key(curName):
        cur = g_namedCursors[curName]
        cur.close()
        del g_namedCursors[curName]

def closeAllNamedCursors():
    global g_namedCursors
    for cur in g_namedCursors.values():
        cur.close()
    g_namedCursors.clear()

def dbEscape(txt):
    # it's silly that we need connection just for escaping strings
    global g_connIpedia
    return g_connIpedia.escape_string(txt)

def resolveRedirect(title,cur_redirect, allRedirects, allArticles):
    # check for (hopefuly frequent) case: this a valid, direct
    # redirect
    if allArticles.has_key(cur_redirect):
        return cur_redirect
    visited = [title]
    visited.append(cur_redirect)
    while True:
        if allArticles.has_key(cur_redirect):
            # this points to a valid article, so we resolved the redirect
            return cur_redirect
        if allRedirects.has_key(cur_redirect):
            # there is some other redirects for this -> keep looking
            cur_redirect = allRedirects[cur_redirect]
            if cur_redirect in visited:
                #print "found circular redirect: %s" % cur_redirect
                break
            visited.append(cur_redirect)
        else:
            # no more redirects -> we couldn't resolve the redirect
            break
    dumpUnresolvedRedirect(visited)
    return None

class UnresolvedRedirectsWriter:
    def __init__(self,sqlDumpName):
        self.fileName = wikipediasql.genBaseAndSuffix(sqlDumpName,"_unres_redirects.txt")
        self.fo = None
    def open(self):
        assert self.fo == None
        self.fo = open(self.fileName,"wb")
    def write(self,visited):
        n = 0
        for txt in visited:
            if n==0:
                # this is title
                self.fo.write(": %s\n" % txt.strip())
            else:
                self.fo.write("  %s\n" % txt.strip())
            n+=1
    def close(self):        
        if self.fo:
            self.fo.close()

g_redirectsWriter = None

def dumpUnresolvedRedirect(visited):
    global g_redirectsWriter
    if g_redirectsWriter:
        g_redirectsWriter.write(visited)

def setUnresolvedRedirectWriter(sqlDump):
    global g_redirectsWriter
    assert None == g_redirectsWriter
    g_redirectsWriter = UnresolvedRedirectsWriter(sqlDump)
    g_redirectsWriter.open()

class ConvertedArticle:
    def __init__(self,ns,title,txt):
        self.ns = ns
        self.title = title
        self.txt = txt
    def getText(self): return self.txt
    def getNamespace(self): return self.ns
    def fRedirect(self): return False
    def getTitle(self): return self.title

class ConvertedArticleRedirect:
    def __init__(self,ns,title,redirect):
        self.ns = ns
        self.title = title
        self.redirect = redirect
    def getTitle(self): return self.title
    def fRedirect(self): return True
    def getRedirect(self): return self.redirect
    def getNamespace(self): return self.ns

def getDbNameFromFileName(sqlFileName):
    base = os.path.basename(sqlFileName)
    txt = wikipediasql.getBaseFileName(base)
    pos = txt.find("_cur_table")
    date = txt[:pos]
    dbName = "ipedia_%s" % date
    return dbName

# First pass: go over all articles, either directly from
# sql dump or from cache and gather the following cache
# data:
#   - all redirects
#   - extract link from articles
# Second pass: do the conversion, including veryfing links and put
# converted articles in the database.
def convertArticles(sqlDump,dbName,articleLimit):
    count = 0
    redirects = {}
    articleTitles = {}
    fTesting = False
    if fTesting:
        fUseCache = False
        fRecreateCache = True
    else:
        fUseCache = True
        fRecreateCache = False
    for article in wikipediasql.iterWikipediaArticles(sqlDump,articleLimit,fUseCache,fRecreateCache):
        # we only convert article from the main namespace
        assert article.getNamespace() == wikipediasql.NS_MAIN
        title = article.getTitle()
        if article.fRedirect():
            redirects[title] = article.getRedirect()
        else:
            txt = article.getText()
            links = articleconvert.articleExtractLinks(txt)
            #articleTitles[title] = links
            articleTitles[title] = 1
        count += 1
        if 0 == count % 1000:
            sys.stderr.write("processed %d rows, last title=%s\n" % (count,title.strip()))
        if articleLimit and count >= articleLimit:
            break
    # verify redirects
    print "Number of redirects: %d" % len(redirects)
    print "Number of real articles: %d" % len(articleTitles)
    unresolvedCount = 0
    setUnresolvedRedirectWriter(sqlDump)
    for (title,redirect) in redirects.items():
        redirectResolved = resolveRedirect(title,redirect,redirects,articleTitles)
        if None == redirectResolved:
            unresolvedCount +=1
            #print "redirect '%s' (to '%s') not resolved" % (title,redirect)
    print "Number of unresolved redirects: %d" % unresolvedCount

    #initDatabase()
    ipedia_write_cur = getNamedCursor(getIpediaConnection(dbName), "ipedia_write_cur")
        
    # go over articles again (hopefully now using the cache),
    # convert them to a destination format (including removing invalid links)
    # and insert into a database
    count = 0
    convWriter = wikipediasql.ConvertedArticleCacheWriter(sqlDump)
    convWriter.open()
    for article in wikipediasql.iterWikipediaArticles(sqlDump,articleLimit,True,False):
        if article.fRedirect():
            convertedArticle = ConvertedArticleRedirect(article.getNamespace(), article.getTitle(), article.getRedirect())
            # what do I do now?
        else:
            title = article.getTitle()
            txt = article.getText()
            converted = articleconvert.convertArticle(title,txt)
            noLinks = articleconvert.removeInvalidLinks(converted,redirects,articleTitles)
            if noLinks:
                converted = noLinks
            convertedArticle = ConvertedArticle(article.getNamespace(), article.getTitle(), converted)

        title = title.replace("_", " ")
        title = title.lower()
        if article.fRedirect():
            # what now?
            pass
        else:
            if g_fVerbose:
                log_txt = "title: %s " % title
            try:
                ipedia_write_cur.execute("""INSERT INTO articles (title, body) VALUES ('%s', '%s')""" % (dbEscape(title), dbEscape(converted)))
                if g_fVerbose:
                    log_txt += "*New record"
            except:
                # assuming that the exception happend because of
                if g_fShowDups:
                    print "dup: " + title
                if g_fVerbose:
                    log_txt += "Update existing record"
                ipedia_write_cur.execute("""UPDATE articles SET body='%s' WHERE title='%s'""" % (dbEscape(converted), dbEscape(title)))
            if g_fVerbose:
                print log_txt
        convWriter.write(convertedArticle)
        count += 1
        if count % 1000 == 0:
            sys.stderr.write("phase 2 processed %d, last title=%s\n" % (count,article.getTitle()))

    convWriter.close()

# return a list of databases on the server
def getDbList(conn):
    cur = conn.cursor()
    cur.execute("SHOW DATABASES;")
    dbs = []
    for row in cur.fetchall():
        dbs.append(row[0])
    cur.close()    
    return dbs    

genSchemaSql = """
CREATE TABLE `articles` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `title` varchar(255) NOT NULL,
  `body` mediumtext NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `title_index` (`title`)
) TYPE=MyISAM; 
"""

genSchema2Sql = """
CREATE TABLE `redirect` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `title` varchar(255) NOT NULL,
  `redirect` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `title_index` (`title`)
) TYPE=MyISAM; 

"""

def delDb(conn,dbName):
    cur = conn.cursor()
    cur.execute("DROP DATABASE %s" % dbName)
    cur.close()
    print "Database '%s' deleted" % dbName

def createDb(conn,dbName):
    cur = conn.cursor()
    cur.execute("CREATE DATABASE %s" % dbName)
    cur.execute("USE %s" % dbName)
    cur.execute(genSchemaSql)
    cur.execute(genSchema2Sql)
    cur.execute("GRANT ALL ON %s.* TO 'ipedia'@'localhost' IDENTIFIED BY 'ipedia';" % dbName)
    cur.close()
    print "Created '%s' database and granted perms to ipedia user" % dbName

cookiesSql = """CREATE TABLE `cookies` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `cookie` varchar(32) binary NOT NULL default '',
  `device_info_token` varchar(255) NOT NULL default '',
  `issue_date` timestamp(14) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `cookie_unique` (`cookie`)
) TYPE=MyISAM;"""


regusersSql = """CREATE TABLE `registered_users` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `cookie_id` int(10) unsigned default '0',
  `user_name` varchar(255) NOT NULL default '',
  `serial_number` varchar(255) binary NOT NULL default '',
  `registration_date` timestamp(14) NOT NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;"""

requestsSql = """CREATE TABLE `requests` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `client_ip` int(10) unsigned NOT NULL default '0',
  `transaction_id` int(10) unsigned NOT NULL default '0',
  `has_get_cookie_field` tinyint(1) NOT NULL default '0',
  `cookie_id` int(10) unsigned default '0',
  `has_register_field` tinyint(1) NOT NULL default '0',
  `requested_term` varchar(255) default '',
  `error` int(10) unsigned NOT NULL default '0',
  `request_date` timestamp(14) NOT NULL,
  `definition_id` int(10) unsigned default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM; """

def delDataDb(conn):
    cur = conn.cursor()
    cur.execute("DROP DATABASE %s" % DATA_DB_NAME)
    cur.close()
    print "Database '%s' deleted" % DATA_DB_NAME

def createDataDb(conn):
    cur = conn.cursor()
    cur.execute("CREATE DATABASE %s" % DATA_DB_NAME)
    cur.execute("USE %s" % DATA_DB_NAME)
    cur.execute(cookiesSql)
    cur.execute(regusersSql)
    cur.execute(requestsSql)
    cur.execute("GRANT ALL ON %s.* TO 'ipedia'@'localhost' IDENTIFIED BY 'ipedia';" % DATA_DB_NAME)
    cur.close()
    print "Created '%s' database and granted perms to ipedia user" % DATA_DB_NAME

def createIpediaDb(sqlDumpName,dbName,fRecreateDb=False,fRecreateDataDb=False):
    connRoot = getRootConnection()
    dbList = getDbList(connRoot)
    fDbExists = False
    fDataDbExists = False
    for db in dbList:
        if db==dbName:
            fDbExists = True
        if db==DATA_DB_NAME:
            fDataDbExists = True
            print "Data database '%s' already exists" % DATA_DB_NAME
    if not fDbExists:
        createDb(connRoot,dbName)
    else:
        if fRecreateDb:
            delDb(connRoot,dbName)
            createDb(connRoot,dbName)
        else:
            print "Database '%s' already exists. Use -recreatedb flag in order to force recreation of the database" % dbName
            sys.exit(0)
    if not fDataDbExists:
        createDataDb(connRoot)
    else:
        if fRecreateDataDb:
            delDataDb(connRoot)
            createDataDb(connRoot)
        else:
            print "Database '%s' exists" % DATA_DB_NAME

def createFtIndex():
    print "starting to create full-text index"
    query = "CREATE FULLTEXT INDEX full_text_index ON articles(title,body);"
    conn = getIpediaConnection(None)
    cur = conn.cursor()
    cur.execute(query)
    cur.close()
    print "finished creating full-text index"

if __name__=="__main__":

    fNoPsyco = arsutils.fDetectRemoveCmdFlag("-nopsyco")
    if g_fPsycoAvailable and not fNoPsyco:
        print "using psyco"
        psyco.full()

    g_fVerbose = arsutils.fDetectRemoveCmdFlag("-verbose")
    g_fShowDups = arsutils.fDetectRemoveCmdFlag("-showdups")
    fRecreateDb = arsutils.fDetectRemoveCmdFlag("-recreatedb")
    fRecreateDataDb = arsutils.fDetectRemoveCmdFlag("-recreatedatadb")
    articleLimit = arsutils.getRemoveCmdArgInt("-limit")

    if len(sys.argv) != 2:
        usageAndExit()
    sqlDump = sys.argv[1]

    dbName = getDbNameFromFileName(sqlDump)
    try:
        createIpediaDb(sqlDump,dbName,fRecreateDb,fRecreateDataDb)
        timer = arsutils.Timer(fStart=True)
        convertArticles(sqlDump,dbName,articleLimit)
        timer.stop()
        timer.dumpInfo()
        createFtIndex()
    finally:
        deinitDatabase()

