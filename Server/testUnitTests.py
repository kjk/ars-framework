# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk

# Purpose:
#  Unit testing for python code
#  see http://diveintopython.org/unit_testing/index.html for more info

import arsutils
import unittest

# tests for functions in arsutils module
class ArsUtils(unittest.TestCase):
    def test_fIsBzipFile(self):
        self.assertEqual(arsutils.fIsBzipFile("me.bz2"), True)
        self.assertEqual(arsutils.fIsBzipFile("me.bz"), False)
        self.assertEqual(arsutils.fIsBzipFile("me.bz2.txt"), False)

if __name__ == "__main__":
    unittest.main()
