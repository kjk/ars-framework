# Copyright: Krzysztof Kowalczyk
# Owner: Andrzej Ciarkowski
#
# Purpose:
#  Convert the body of wikipedia article from original wikipedia
#  format to our format

import re,unicodedata,entities

def stripBlocks(text, startPattern, endPattern):
    opened=0
    spanStart=-1
    spans=[]
    pattern=r"(%s)|(%s)" % (startPattern, endPattern)
    blockRe=re.compile(pattern, re.I+re.S)
    for match in blockRe.finditer(text):
        if match.lastindex==1: # This means it's a start tag
            if not opened:
                spanStart=match.start(match.lastindex)
            opened+=1
        else:                       # This is end tag
            if opened==1:
                spans.append((spanStart, match.end(match.lastindex)))
            opened-=1;
            if opened<0:
                opened=0
    if opened:
        spans.append((spanStart, len(text)))
    spans.reverse()             # Iterate from end so that text indices remain valid when we slice and concatenate text
    for span in spans:
        start, end=span
        text=text[:start]+text[end:]
    return text

def stripTagBlocks(text, blockElem):
    return stripBlocks(text, '<%s.*?>' % blockElem, '</%s>' % blockElem)

def replaceRegExp(text, regExp, repl):
    match=regExp.search(text)
    while match:
        #print "Replacing: ", text[match.start():match.end()], " with: ", repl
        text=text[0:match.start()]+repl+text[match.end():]
        match=regExp.search(text)
    return text

def replaceTagList(text, tagList, repl):
    for tag in tagList:
        text=replaceRegExp(text, re.compile(r'<(/)?%s(\s+.*?)?>' % tag, re.I), repl)
    return text

commentRe=re.compile("<!--.*?-->", re.S)
divRe=re.compile("<div.*?</div>", re.I+re.S)
tableRe=re.compile("<table.*?</table>", re.I+re.S)
captionRe=re.compile("<caption.*?</caption>", re.I+re.S)
tbodyRe=re.compile("<tbody.*?</tbody>", re.I+re.S)
theadRe=re.compile("<thead.*?</thead>", re.I+re.S)
trRe=re.compile("<tr.*?</tr>", re.I+re.S)
tdRe=re.compile("<td.*?</td>", re.I+re.S)
scriptRe=re.compile("<script.*?</script>", re.I+re.S)
badLinkRe=re.compile(r"\[\[((\w\w\w?(-\w\w)?)|(simple)|(image)|(media)):.*?\]\]", re.I+re.S)
#numEntityRe=re.compile(r'&#(\d+);')

multipleLinesRe=re.compile("\n{3,100}")
# replace multiple (1+) empty lines with just one empty line.
def stripMultipleNewLines(txt):
    txt=replaceRegExp(txt,multipleLinesRe,"\n\n")
    return txt

wikiMacrosReplacements = {
    "{{msg:stub}}"     : "",
    "{{msg:spoiler}}"  : "'''Warning:''' Plot details follow.",
    "{{msg:disambig}}" : "This is a disambiguation page; that is, one that points to other pages that might otherwise have the same name.",
#    "{{msg:copyvio1}}"   : "",
#    "{{msg:copyvio2}}"   : "",
#    "{{msg:NPOV}}"       : "",
#    "{{msg:disputed}}"   : "",
#    "{{msg:inclusion}}"  : "",
#    "{{msg:protected}}"  : "",
#    "{{msg:inuse}}"      : "",
#    "{{msg:controversial}}" : "",
}

# TODO: old code, remove
#wikiStubText=""                                                                                    
#wikiSpoilerText="'''Warning:''' Plot details follow."
#wikiDisambigText="This is a disambiguation page; that is, one that points to other pages that might otherwise have the same name."
#wikiCopyVio1Text=""
#wikiCopyVio2Text=""
#wikiNPOVText=""
#wikiDisputedText=""
#wikiInclusionText=""
#wikiProtectedText=""
#wikiInUseText=""
#wikiControversialText=""
wikiMacroRe=re.compile("\{\{(msg)|(subst)\:.*?\}\}", re.I)

def replaceWikiMacros(term, text):

    for (macro,replacement) in wikiMacrosReplacements.items():
        text = text.replace(macro, replacement)
# TODO: old code, remove
#    text=text.replace("{{msg:stub}}", wikiStubText)
#    text=text.replace("{{msg:spoiler}}", wikiSpoilerText)
#    text=text.replace("{{msg:disambig}}", wikiDisambigText)
#    text=text.replace("{{msg:copyvio1}}", wikiCopyVio1Text)
#    text=text.replace("{{msg:copyvio2}}", wikiCopyVio2Text)
#    text=text.replace("{{msg:NPOV}}", wikiNPOVText)
#    text=text.replace("{{msg:disputed}}", wikiDisputedText)
#    text=text.replace("{{msg:inclusion}}", wikiInclusionText)
#    text=text.replace("{{msg:protected}}", wikiProtectedText)
#    text=text.replace("{{msg:inuse}}", wikiInUseText)
#    text=text.replace("{{msg:controversial}}", wikiControversialText)
    text=replaceRegExp(text, wikiMacroRe, "")
    return text

#def convertEntities(text):
#    matches=[]
#    for iter in numEntityRe.finditer(text):
#        matches.append(iter)
#    matches.reverse()
#    for match in matches:
#        num=int(text[match.start(1):match.end(1)])
#        if num>255:
#            char=unichr(num)
#            decomposed=unicodedata.normalize('NFKD', char)
#            valid=''
#            for char in decomposed:
#                if ord(char)<256:
#                    valid+=chr(ord(char))
#            if len(valid):
#                text=text[:match.start()]+valid+text[match.end():]
#    return text

# main function: given the text of wikipedia article in original wikipedia
# format, return the article in our own format
def convertArticle(term, text):
    try:
        text=text.replace('__NOTOC__', '')
        text=replaceWikiMacros(term, text)
        text=text.replace('\r','')
        text=replaceRegExp(text, commentRe, '')     # This should be safe, as it's illegal in html to nest comments
    
        text=stripTagBlocks(text, 'div')
        text=stripTagBlocks(text, 'table')
        text=stripBlocks(text, r'\{\|', r'\|\}')
    
        text=replaceRegExp(text, scriptRe, '')
    
        text=replaceTagList(text, ['b', 'strong'], "'''")
        text=replaceTagList(text, ['em', 'i', 'cite'], "''")
        text=replaceTagList(text, ['hr'], '----')
        text=replaceTagList(text, ['p'], '<br>')
        text=replaceTagList(text, ['dfn', 'code', 'samp', 'kbd', 'var', 'abbr', 'acronym', 'blockquote', 'q', 'pre', 'ins', 'del', 'dir', 'menu', 'img', 'object', 'big', 'span', 'applet', 'font', 'basefont', 'tr', 'td', 'table', 'center', 'div'], '')
        text=replaceRegExp(text, badLinkRe, '')
        text=entities.convertNamedEntities(term, text)
        text=entities.convertNumberedEntities(term, text)
        text=stripMultipleNewLines(text)
        text=text.strip()
        text+='\n'
        return text
    except:
        print "Exception while converting term: ", term
        return ''
  
