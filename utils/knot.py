#!/usr/bin/env python

from copy import deepcopy
from math import sqrt

class V(object):
    def __init__(self, x, y, z):
        self.comp = [x, y, z]

    @property
    def x(self):
        return self.comp[0]

    @property
    def y(self):
        return self.comp[1]

    @property
    def z(self):
        return self.comp[2]

    @z.setter
    def z(self, v):
        self.comp[2] = v

    def add(self, other):
        for i in range(0, 3):
            self.comp[i] += other.comp[i]

    def mul(self, s):
        for i in range(0, 3):
            self.comp[i] *= s

def add(v1, v2):
    v = V(v1.x, v1.y, v1.z)
    v.add(v2)
    return v

def mul(v1, s):
    v = V(v1.x, v1.y, v1.z)
    v.mul(s)
    return v

class B(object):
    def __init__(self, v0, v3):
        self.p0 = v0
        self.p3 = v3
        self.p1 = V(0, 0, 0)
        self.p2 = V(0, 0, 0)

    def add(self, dir, n):
        if dir.ud != 0:
            self.p3.z += dir.ud * n
        else:
            index = abs(dir.rl) - 1
            sgn = dir.rl / abs(dir.rl)
            self.p3.comp[index] += sgn * n

        # print(int(self.p3.x), int(self.p3.y), int(self.p3.z))


class Dir(object):
    def __init__(self, rl, ud = 0):
        self.rl = rl
        self.ud = ud

rmap = {1: -2, -1: 2, 2: 1, -2: -1}

def parse(s):

    int(s[0]) # raise exception if not integer
    int(s[-1]) # raise exception if not integer

    start = V(0, 0, 0)
    last = B(deepcopy(start), deepcopy(start))
    dir = Dir(1)

    n = ''
    res = []
    for t in s:
        if t in 'RLUD':
            last.add(dir, int(n))
            if t in 'RL':
                res.append(deepcopy(last))
                last = B(deepcopy(last.p3), deepcopy(last.p3))
            n = ''
            if t == 'R':
                dir = Dir(rmap[dir.rl])
            elif t == 'L':
                dir = Dir(-rmap[dir.rl])
            elif t == 'U':
                dir = Dir(dir.rl, 1)
            else:
                dir = Dir(dir.rl, -1)
        else:
            n += t

    last.add(dir, int(n))
    res.append(deepcopy(last))

    return res

def solve(points):

    n = len(points)
    x1 = -2 + sqrt(3)
    x2 = -2 - sqrt(3)
    d0 = (x1, x2)
    dx = x1 - x2
    q0 = [V(0, 0, 0), V(0, 0, 0)]
    di = (1 / (1 - x1 ** n), 1 / (1 - x2 ** n))
    v1 = (-1, -1)
    v2 = (x1, x2)
    z0 = ((2*x2 + 4) / dx, (-2*x1 - 4) / dx)
    for k, p in enumerate(points):
        for i in range(0, 2):
            s = di[i] * d0[i] ** (n - k) * z0[i]
            q0[i].add(mul(p.p0, s))

    points[0].p1 = add(mul(q0[0], v1[0]), mul(q0[1], v1[1]))
    points[0].p2 = add(mul(q0[0], v2[0]), mul(q0[1], v2[1]))

    for i in range(1, n):
        points[i].p1 = add(mul(points[i].p0, 2), mul(points[i-1].p2, -1))
        points[i].p2 = add(add(mul(points[i].p0, 4), points[i-1].p1), mul(points[i-1].p2, -4))


    return points


from sys import stdin, argv
from struct import pack

if __name__ == '__main__':
    tokens = ''
    for line in stdin:
        tokens += line.strip()
    patches = solve(parse(tokens))
    if argv[1] == '-f':
        fmt = argv[2]
        for k, p in enumerate(patches):
            v = [p.p0, p.p1, p.p2]
            for i in range(0, 3):
                s = fmt.format(3 * k + i, v[i].x , v[i].y, v[i].z)
                print(s)
        v = patches[-1].p3
        s = fmt.format(3 * len(patches), v.x , v.y, v.z)
        print(s)
    elif argv[1] == '-o':
        f = open(argv[2], 'wb')
        f.write(pack('>i', 4 * (3 * len(patches) + 1)))
        for p in patches:
            v = [p.p0, p.p1, p.p2]
            for i in range(0, 3):
                f.write(pack('>d', v[i].x))
                f.write(pack('>d', v[i].y))
                f.write(pack('>d', v[i].z))
                f.write(pack('>d', 1.0))
        v = patches[-1].p3
        f.write(pack('>d', v.x))
        f.write(pack('>d', v.y))
        f.write(pack('>d', v.z))
        f.write(pack('>d', 1.0))
        f.close()


