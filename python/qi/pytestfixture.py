##
## Author(s):
##  - Cedric GESTES <cgestes@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics
##

# -*- coding: utf-8 -*-

import pytest
import qi

def pytest_addoption(parser):
    parser.addoption('--url', action='store', default='tcp://127.0.0.1:9559',
                     help='NAOqi Url')

@pytest.fixture
def url(request):
    """ Url of the NAOqi to connect to """
    return request.config.getoption('--url')

@pytest.fixture
def session(url):
    """ Connected session to a NAOqi """
    ses = qi.Session()
    ses.connect(url)
    return ses

__all__ = ('session', 'url', 'pytest_addoption')
