#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-
# This is only a stub...
from twisted.internet import protocol, reactor
from twisted.protocols import basic
import MySQLdb
import random

lineSeparator =		"\n"
fieldSeparator =	": "

transactionIdField =		"Transaction-ID"
getCookieField = 			"Get-Cookie"
cookieField =					"Cookie"
getDefinitionField =		"Get-Definition"
formatVersionField = 		"Format-Version"
definitionField = 			"Definition"
definitionForField = 		"Definition-For"
definitionNotFoundField = "Definition-Not-Found"
errorField = 					"Error"
registerField = 				"Register"
protocolVersionField =	"Protocol-Version"
clientVersionField = 		"Client-Version"

redirectCommand = "#REDIRECT"
termStartDelimiter = "[["
termEndDelimiter = "]]"

definitionFormatVersion = 1
protocolVersion = 1

# @todo Provide some more meaningful error messages
serverFailureError = 			"==Error==\n\nServer failure."
fakeCookieError = 			"==Error==\n\nFake cookie."
unsupportedDeviceError = 	"==Error==\n\nUnsupported device."
fakeSerialNumberError = 	"==Error==\n\nFake serial number."
snAlreadyUsedError = 		"==Error==\n\nSerial number already used on other device."
malformedRequestError = 	"==Error==\n\nRequest is malformed."

def startsWithIgnoreCase(s1, substr):
	if len(s1)<len(substr):
		return False
	if s1[:len(substr)].upper()==substr.upper():
		return True
	return False

#def escapeSQL(phrase):
#	return phrase.replace("'", "''").replace("%", "\\%").replace("_","\\_")

class iPediaProtocol(basic.LineReceiver):

	def __init__(self):
		self.delimiter='\n'
		self.transactionId=None
		self.deviceInfoToken=None
		self.db=None
		self.error=None
		self.clientVersion=None
		self.protocolVersion=None
		self.term=None
		self.cookie=None
		self.cookieId=None
		self.userId=None
		self.serialNumber=None
		
	def getDatabase(self):
		if not self.db:
			self.db=MySQLdb.Connect(host='localhost', user='ipedia', passwd='ipedia', db='ipedia')
		return self.db

	def outputField(self, name, value=None):
		field=name
		if value:
			field+=fieldSeparator+value
		field+=lineSeparator
		self.transport.write(field)
		print field
		
	def outputPayloadField(self, name, payload):
		self.outputField(name, str(len(payload)))
		self.transport.write(payload)
		self.transport.write(lineSeparator)
		print payload

	def finish(self):
		if self.error:
			self.outputField(formatVersionField, str(definitionFormatVersion))
			self.outputPayloadField(errorField, self.error)
		self.transport.loseConnection()
		if self.db:
			try:
				self.db.close()
			except:
			    raise
				
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
			self.error=unsupportedDeviceError
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
			
		except:
			raise
			if cursor:
				cursor.close()
			self.error=serverFailureError
			return False;
			
	def handleRegisterRequest(self):
		if 0==self.cookieId:
			self.error=malformedRequestError
			return False

		cursor=None
		try:
			db=self.getDatabase()
			cursor=db.getDatabase()
			cursor.execute("""select id, cookie_id from registered_users where serial_number='%s'""" % db.escape_string(self.serialNumber))
			row=cursor.fetchone()
			if row:
				currentCookieId=row[1]
				if currentCookieId==None:
					self.userId=row[0]
					cursor.execute("""update registered_users set cookie_id=%d where id=%d""" % (self.cookieId, self.userId))
				elif currentCookieId!=self.cookieId:
					self.error=snAlreadyUsedError
					cursor.close()
					return False
				else:
					self.userId=row[0]
			else:
				self.error=fakeSerialNumberError
				cursor.close()
				return False
			cursor.close()
			return True
		except:
			raise
			if cursor:
				cursor.close()
			self.error=serverFailureError
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
					self.error=fakeCookieError
					cursor.close()
					return False
			except:
				raise
				if cursor:
					cursor.close()
				self.error=serverFailureError
				return False;
		else:
			return True
			
	def outputDefinition(self, definition):
		self.outputField(formatVersionField, str(definitionFormatVersion))
		self.outputField(definitionForField, self.term)
		self.outputPayloadField(definitionField, definition)
		
	def findDefinition(self, db, cursor):
		finished = False
		result=None
		history=[self.term]
		while not finished:
#			query="""select id, term, definition from definitions where term='%s' """ % db.escape_string(self.term.replace(' ', '_'))
			query="""select 1, cur_title, cur_text from enwiki.cur where cur_title='%s' order by cur_timestamp desc limit 1""" % db.escape_string(self.term.replace(' ', '_'))
			cursor.execute(query)
			row=cursor.fetchone()
			if row:
				result=row[2]
				if startsWithIgnoreCase(result, redirectCommand):
					termStart=result.find(termStartDelimiter)+2
					termEnd=result.find(termEndDelimiter)
					self.term=result[termStart:termEnd]
				else:
					result=result.replace('\r', '')  
					self.term=row[1].replace('_', ' ');
					finished=True
			else: 
				result=None				
				finished = True
			if not finished:
				if history.count(self.term)>0:
					print "--------------------------------------------------------------------------------"
					print "WARNING! Circular reference: "+self.term
					print "--------------------------------------------------------------------------------"
					result=None
					finished=True
				else:
					history.append(self.term)
		return result
		
	def handleDefinitionRequest(self):
		cursor=None
		definition=None
		try:
			db=self.getDatabase()
			cursor=db.cursor()
			definition=self.findDefinition(db, cursor)
			cursor.close()
		except:
			raise
			if cursor:
				cursor.close()
			self.error=serverFailureError
			return False;
			
		if definition:
			self.outputDefinition(definition)
		else:
			self.outputField(definitionNotFoundField)
		return True
						
	def answer(self):
	
		print ""
		print "--------------------------------------------------------------------------------"
		print ""
		
		if self.transactionId:
				self.outputField(transactionIdField, self.transactionId)
		else:
			return self.finish()
		
		if self.deviceInfoToken and not self.handleGetCookieRequest():
			return self.finish()
	
		if self.serialNumber and not self.handleRegisterRequest():
			return self.finish()
			
		if self.cookie:
			if not self.handleCookieRequest():
				return self.finish()
		else:
			self.error=malformedRequestError
			return self.finish()
		
		if self.term and not self.handleDefinitionRequest():
			return self.finish()								

		self.finish()

	def extractFieldValue(self, line):
		index=line.find(fieldSeparator)
		if (index!=-1):
			return line[index+2:]
		else:
			return None
			
	def lineReceived(self, request):
		if (request == ""):
			self.answer()
		else:
			print request
			print ""
			
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
				self.term=self.extractFieldValue(request)
			
			elif request.startswith(registerField):
				self.serialNumber=self.extractFieldValue(request)

		
class iPediaFactory(protocol.ServerFactory):
	protocol = iPediaProtocol
	
reactor.listenTCP(9000, iPediaFactory())
reactor.run()

