#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##


class BadSignatureException(Exception):
    """Custom exception"""
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return "error:", self.message


Bool   = "b"
Char   = "c"
Int    = "i"
Float  = "f"
String = "s"
Void   = "v"

def Protobuf(Name):
    """
    >>> Protobuf("toto")
    '@toto@'
    """
    return "@%s@" % (Name)

def Map(Key, Value):
    """
    >>> Map(String, String)
    '[ss]'
    >>> Map(Map(String, String), Int)
    '[[ss]i]'
    """
    return "[%s%s]" % (Key, Value)

def Function(Return=None, *args):
    """
    >>> Function (String, String)
    's:s'
    """
    if not Return:
        Return = ''
    return "%s:%s" % (Return, "".join(args))

def Signature(Name, Return=None, *args):
    """
    >>> Signature("motion", Void, Int, Int)
    'motion::v:ii'
    """
    return "%s::%s" % (Name, Function(Return, *args))

def _search_closing_char(sig, start, cstart, cend):
    """
    search for the closing cend matching the number of cstart, starting from start in sig.

    >>> _search_closing_char("sssss[sssss[[[[[]]]]]]", 5, '[', ']')
    16
    >>> len("sssss[[[[[]]]]]]")
    16
    """

    bo      = 1
    be      = 0
    i       = start
    if sig[start] != cstart:
        raise BadSignatureException("sig[start] != cstart")

    i += 1
    siglen = len(sig)
    while i < siglen:
        current = sig[i]
        if current == cstart:
            bo += 1
        if current == cend:
            be += 1
        if be == bo:
            break
        i += 1
    if be != bo:
        raise BadSignatureException("no matching %s for %s in %s at %d" % (cend, cstart, sig, start))
    advance = i - start
    if advance < 0:
        raise BadSignatureException("what?")
    return advance

def split(sig):
    """ Split a signature in individual type. Warning it is not recursive.
        for example [[s]]s will become [[s]], s

        >>> split("is[]{}")
        ['i', 's', '[]', '{}']
        >>> split("iis[s]")
        ['i', 'i', 's', '[s]']
        >>> split("iis{}[[[s]]]")
        ['i', 'i', 's', '{}', '[[[s]]]']
        >>> split("iidffd[ss]{s[s]}")
        ['i', 'i', 'd', 'f', 'f', 'd', '[ss]', '{s[s]}']
        >>> split("i*id*ffd[s*s]*{s[s]}*")
        ['i*', 'i', 'd*', 'f', 'f', 'd', '[s*s]*', '{s[s]}*']
    """

    ret = list()

    siglen = len(sig)
    if siglen == 0:
        return ret
    i = 0
    while i < siglen:
        current = sig[i]
        # vector
        if current == '[':
            advance = _search_closing_char(sig, i, '[', ']')
            ret.append(sig[i:i + advance + 1])
            i += advance
        # map
        elif current == '{':
            advance = _search_closing_char(sig, i, '{', '}')
            ret.append(sig[i:i + advance + 1])
            i += advance
        # simple type
        elif current in 'bcifdvs':
            ret.append(current)
        elif current == '*':
            if len(ret) == 0:
                raise BadSignatureException("Pointer should refer to a type for : %s" % (sig))
            ret[-1] += '*'
        else:
            raise BadSignatureException("Unknown signature")
        i += 1
    return ret

if __name__ == "__main__":
    print "Test signature:"
    import doctest
    doctest.testmod()
