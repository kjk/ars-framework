# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk

# Purpose:
#  Unit testing for python code
#  see http://diveintopython.org/unit_testing/index.html for more info

import unittest
import arsutils,wikipediasql
from articleconvert import *

# tests for functions in arsutils module
class ArsUtils(unittest.TestCase):
    def test_fIsBzipFile(self):
        self.assertEqual(arsutils.fIsBzipFile("me.bz2"), True)
        self.assertEqual(arsutils.fIsBzipFile("me.bz"), False)
        self.assertEqual(arsutils.fIsBzipFile("me.bz2.txt"), False)

class ArticleConvert(unittest.TestCase):
    def test_WikipediaLinkEq(self):
        # testing if I got __eq__ and __ne__ right
        wl1 = WikipediaLink(None,None)
        wl2 = WikipediaLink(None,None)
        self.assertEqual( wl1==wl2, True )
        self.assertEqual( wl1!=wl2, False )
        self.assertEqual( wl1, wl2)
        wl3 = WikipediaLink("me", None)
        self.assertEqual( wl1==wl3, False)
        self.assertEqual( wl2!=wl3, True)

    def test_ExtractLinks(self):
        testData = [ ["[[me|him]]", WikipediaLink("me", "him")],
            ["[[foo]]", WikipediaLink("foo", None)],
            ["[[orb_here|]]", WikipediaLink("orb_here", "")],
            ["no links here", None],
            ["me[[link|b]]and[][[another link|link]]", WikipediaLink("link","b"), WikipediaLink("another link","link")]]
        for t in testData:
            txt = t[0]
            expectedLinks = t[1:]
            links = articleExtractLinks(txt)
            if None == links:
                self.assertEqual(None,expectedLinks[0])
                continue
            self.assertEqual(len(links),len(expectedLinks))
            for (link,expectedLink) in zip(links,expectedLinks):
                if link != expectedLink:
                    print "got:"
                    link.dump()
                    print "expected:"
                    expectedLink.dump()
            self.assertEqual(link,expectedLink)

    def test_removeInvalidLinks(self):
        txt = removeInvalidLinks("be[[hello|rep]]",{},{})
        self.assertEqual(txt, "berep")
        txt = removeInvalidLinks("[[he_ro]]", {"me_bo":1}, {})
        self.assertEqual(txt, "he ro")
        txt = removeInvalidLinks("[[me_bo]]", {"me_bo":1}, {})
        self.assertEqual(txt, None)
        txt = removeInvalidLinks("al [[aj_ol|lo]] go", {}, {"aj_op":1})
        self.assertEqual(txt, "al lo go")
        txt = removeInvalidLinks("al[[me_ko]] [[l2_oj|gol]] a", {"me_me":1}, {"la_ha":1})
        self.assertEqual(txt,"alme ko gol a")

class WikipediaSql(unittest.TestCase):

    def test_getRedirectFromText(self):
        testData = [
                #["#redirect [[Qing Dynasty]", "Qing Dynasty"],
                ["#REDIRECT[[football (soccer)|football]]", "football (soccer)"],
                ["#REDIRECT [[linkA_bro]]", "linkA_bro"],
                ["#REDIRECT [[]]", ""],
                ["#REDIREC [[foo]]", None],
                ["asdfasdfawer", None],
                ["#redirect [[blast]]", "blast"],
                ["#reDIRect [[glas]]", "glas"],
                ["#REDIRECT [[History of Algeria]] \nsee also:", "History of Algeria"],
                ["#REDIRECT[[blast]]", "blast"],
                ["#REDIRECT [[ab]] [[cd]]", "ab"],
                ["#REDIRECT [[mest|b]]", "mest"]
                ]
        for t in testData:
            got = wikipediasql.getRedirectFromText(t[0])
            expected = t[1]
            self.assertEqual(got,expected)

    def test_fIsRedirectLine(self):
        testData = [ ["^r as", True], ["^r ", False], ["^r", False], ["^report", False]]
        for t in testData:
            self.assertEqual(wikipediasql.fIsRedirectLine(t[0]), t[1])

if __name__ == "__main__":
    unittest.main()
