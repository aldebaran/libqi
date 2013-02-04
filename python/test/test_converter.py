##
## Copyright (C) 2012 Aldebaran Robotics
##

from qimessagingswig import converterTest

def test_converterUTF8():
    cv = converterTest()
    s1 = u"CHICHE"
    for i in range(0, 100):
      s2 = cv.TestObjectConversion(s1)
      assert s1 == s2

def test_converter():
    cv = converterTest()

    print "\nTest #1 : Convert bool (True)"
    ret = cv.TestObjectConversion(True)
    assert ret == True

    print "\nTest #2 : Convert bool (False)"
    ret = cv.TestObjectConversion(False)
    assert ret == False

    print "\nTest #3 : Convert None"
    ret = cv.TestObjectConversion(None)
    assert ret == None

    print "\nTest #4 : Convert char"
    ret = cv.TestObjectConversion(-42)
    assert ret == -42

    print "\nTest #5 : Convert uchar"
    ret = cv.TestObjectConversion(42)
    assert ret == 42

    print "\nTest #6 : Convert short"
    ret = cv.TestObjectConversion(-420)
    assert ret == -420

    print "\nTest #7 : Convert ushort"
    ret = cv.TestObjectConversion(420)
    assert ret == 420

    print "\nTest #8 : Convert int"
    ret = cv.TestObjectConversion(-42000)
    assert ret == -42000

    print "\nTest #9 : Convert uint"
    ret = cv.TestObjectConversion(42000)
    assert ret == 42000

    print "\nTest #10 : Convert long"
    ret = cv.TestObjectConversion(-4200000000)
    assert ret == -4200000000

    print "\nTest #11 : Convert ulong"
    ret = cv.TestObjectConversion(4200000000)
    assert ret == 4200000000

    tolerance = 0.0000001
    print "\nTest #12 : Convert float"
    ret = cv.TestObjectConversion(0.42)
    assert abs(ret - 0.42) < tolerance

    tolerance = 0.000000000000001
    print "\nTest #13 : Convert double"
    ret = cv.TestObjectConversion(0.0000000042)
    assert abs(ret - 0.0000000042) < tolerance

    print "\nTest #14 : Convert List (Full int)"
    ret = cv.TestObjectConversion([0,1,2])
    assert ret == [0,1,2]

    print "\nTest #15 : Convert List (mixed)"
    ret = cv.TestObjectConversion([0,"lol", True, 0.42])
    assert ret == [0, "lol", True, 0.42]

    print "\nTest #16 : Convert Map"
    ret = cv.TestObjectConversion({ "0": 0, "1": 1})
    assert ret == {"0":0,"1":1}

    print "\nTest #17 : Convert Map (mixed)"
    ret = cv.TestObjectConversion({ "0": "test", "1": True})
    assert ret == {"0": "test", "1" : True}

    print "\nTest #18 : Convert Tuple 1"
    ret = cv.TestObjectConversion(("hello", "world"))
    assert ret == ("hello", "world")

    print "\nTest #19 : Convert Tuple 2"
    ret = cv.TestObjectConversion((1, 0.5, "lol", (0,1, "nao")))
    assert ret == (1, 0.5, "lol", (0,1, "nao"))

    print "\nTest #20 : Convert String"
    ret = cv.TestObjectConversion("Hello World !")
    assert ret == "Hello World !"

if __name__ == "__main__":
    test_converter()
    test_converterUTF8()
