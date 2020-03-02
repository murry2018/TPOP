# markov.py: 두 단어 접두어를 사용하는 마르코프 체인 알고리즘
from collections import defaultdict
import random
import sys

MAXGEN = 1000
NONWORD = '\n'
w1 = w2 = NONWORD

statetab = defaultdict(lambda: list())
for line in sys.stdin:
    for item in line.split():
        statetab[(w1, w2)].append(item)
        (w1, w2) = (w2, item)
statetab[(w1, w2)].append(NONWORD)

w1 = w2 = NONWORD
for i in range(MAXGEN):
    sufs = statetab[(w1, w2)]
    suf = random.choice(sufs)
    if suf == NONWORD:
        break
    print(suf, end=' ')
    (w1, w2) = (w2, suf)
