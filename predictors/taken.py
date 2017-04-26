"""
__name__ = taken.py
__author__ = Yash Patel
__description__ = Taken enumeration class, which defines a 
saturating taken status in some of the dynamic predictors that
have been programmer (i.e. gshare and bimodal)
"""

from enum import Enum

class Taken(Enum):
    StronglyNotTaken = 1
    WeaklyNotTaken   = 2
    WeaklyTaken      = 3
    StronglyTaken    = 4

    def incr(taken_status):
        if taken_status == Taken.StronglyNotTaken:
            return Taken.WeaklyNotTaken
        elif taken_status == Taken.WeaklyNotTaken:
            return Taken.WeaklyTaken
        elif taken_status == Taken.WeaklyTaken:
            return Taken.StronglyTaken
        return Taken.StronglyTaken # staturating state for increasing case

    def decr(taken_status):
        if taken_status == Taken.StronglyTaken:
            return Taken.WeaklyTaken
        elif taken_status == Taken.WeaklyTaken:
            return Taken.WeaklyNotTaken
        elif taken_status == Taken.WeaklyNotTaken:
            return Taken.StronglyNotTaken
        return Taken.StronglyNotTaken # staturating state for decreasing case
