#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-
# This is only a stub...
from twisted.internet import protocol, reactor
from twisted.protocols import basic
import MySQLdb

lineSeparator =		"\n"
fieldSeparator =	": "

transactionIdField =	"Transaction-ID"
getCookieField = 		"Get-Cookie"
cookieField =				"Cookie"
getDefinitionField =	"Get-Definition"
formatVersionField = 	"Format-Version"
definitionField = 		"Definition"
definitionForField = 	"Definition-For"
definitionNotFoundField = "Definition-Not-Found"

redirectCommand = "#REDIRECT"
termStartDelimiter = "[["
termEndDelimiter = "]]"

def startsWithIgnoreCase(s1, substr):
	if len(s1)<len(substr):
		return False
	if s1[:len(substr)].upper()==substr.upper():
		return True
	return False

def escapeSQL(phrase):
	return phrase.replace("'", "''").replace("%", "\\%").replace("_","\\_")

class iPediaProtocol(basic.LineReceiver):

	def __init__(self):
		self.delimiter='\n'
		self.lines = []
		self.transactionId=False
		self.cookieRequested=False
		self.definitionRequested=False
		self.db=MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='enwiki')
		
	def outputField(self, name, value=None):
		field=name
		if value:
			field+=fieldSeparator+value
		field+=lineSeparator
		self.transport.write(field)
		

	def findDefinition(self):
		finished = False
		while not finished:
			cursor=self.db.cursor()
			query="""select cur_title, cur_text from cur where cur_title like '%s' order by cur_timestamp desc limit 1""" % escapeSQL(self.term.replace(' ', '_'))
			print query
			cursor.execute(query)
			row=cursor.fetchone()
			result=None
			if row!=None:
				self.term=row[0]
				result=row[1]
				print result
				if startsWithIgnoreCase(result, redirectCommand):
					termStart=result.find(termStartDelimiter)+2
					termEnd=result.find(termEndDelimiter)
					self.term=result[termStart:termEnd]
				else:
					result=result.replace('\r', '').replace("<b>","\'\'\'").replace("</b>","\'\'\'").replace("<i>","\'\'").replace("</i>","\'\'").replace("<em>","\'\'").replace("</em>","\'\'")
					finished=True
			else: 
				finished = True
			cursor.close()
		return result
		
	def answer(self):
		if (self.transactionId):
			self.outputField(transactionIdField, self.transactionId)

		if (self.cookieRequested):
			self.outputField(cookieField, "0123456789")
	
		if (self.definitionRequested):
			definition=self.findDefinition()
			if definition!=None:
				self.outputField(formatVersionField, "1")
				self.outputField(definitionForField, self.term)
				self.outputField(definitionField, str(len(definition)))
				self.transport.write(definition)
				self.transport.write(lineSeparator)
			else: self.outputField(definitionNotFoundField)
								

		self.transport.loseConnection()
		del self.db
		print "--------------------------------------------------------------------------------"
		print ""

	def extractFieldValue(self, line):
		index=line.find(fieldSeparator)
		if (index!=-1):
			return line[index+2:]
		else:
			return ""

	def lineReceived(self, request):
		self.lines.append(request)
		if (request == ""):
			self.answer()
		else:
			print request
			if (request.startswith(transactionIdField)):
				self.transactionId=self.extractFieldValue(request)
			
			if (request.startswith(getCookieField)):
				self.cookieRequested=True
		
			if (request.startswith(getDefinitionField)):
				self.definitionRequested=True
				self.term=self.extractFieldValue(request)

		
class iPediaFactory(protocol.ServerFactory):
	protocol = iPediaProtocol
	
reactor.listenTCP(9000, iPediaFactory())
reactor.run()

