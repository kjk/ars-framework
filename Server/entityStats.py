# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk

# Purpose:
# Generate the following statistics about unconverted 
# - entity usage statistics so that we know which international characters 
#   we need to support in the viewer
# - size statistics about articles (because we're curious)
#
# History:
#   2004-03-21 started

# devnotes: using iterators (http://www-106.ibm.com/developerworks/library/l-pycon.html)

import MySQLdb, MySQLdb.cursors, sys, datetime, re
import timeit

def queryGetOneVal(db,query):
    cursor = db.cursor()
    cursor.execute(query)
    row = cursor.fetchone()
    if row:
        return row[0]
    else:
        return None
    cursor.close()

# an iterator returning a row from enwiki.cur database
# row[0] is cur_id
# row[1] is cur_title
# row[2] is cur_text
def getRow_iter(db):
    query="""select cur_id, cur_title, cur_text from enwiki.cur where cur_namespace=0"""
    cursor = db.cursor()
    cursor.execute(query)
    while True:
        row = cursor.fetchone()
        if not row:
            break;
        yield row
    cursor.close()
    raise StopIteration

def entities_iter(txt):
    parts = txt.split("&#")
    for p in parts:
        pos = p.find(";")
        if -1 != pos:
            ent = p[:pos]
            if ent.isdigit():
                yield ent
    raise StopIteration

def entIterTest():
    txt = "hello&#123;test&#34;"
    for entity in getEntities_iter(txt):
        print entity

    txt = "bluff"
    for entity in getEntities_iter(txt):
        print entity

def genEntitiesStats(db):
    entities = {}
    for row in getRow_iter(db):
        title = row[1]
        txt = row[2]
        #print txt
        for entity in entities_iter(txt):
            #print "%s in %s" % (entity,title)
            if entities.has_key(entity):
                entities[entity] = entities[entity] + 1
            else:
                entities[entity] = 1
    return entities

def cmpEnt(e1,e2):
    return cmp(e1[0],e2[0])

def dumpEntities(entities):
    # display entities nicely sorted by frequency
    sorted = []
    for key in entities.keys():
        val = entities[key]
        sorted.append( [val,key] )
    sorted.sort(cmpEnt)
    for ent in sorted:
        print "%d - &#%s;" % (ent[0], ent[1])

# should we dump titles to the file
fDumpTitles = True

db=MySQLdb.Connect(
    host='localhost', 
    user='ipedia', 
    passwd='ipedia', 
    db='ipedia')
    #cursorclass=MySQLdb.cursors.SSCursor)
entities = genEntitiesStats(db)
dumpEntities(entities)
db.close()

