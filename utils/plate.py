#!/usr/bin/env python
# -*- coding: utf-8 -*-
import math as m
import sys

def s2b(x):
    return (1+x)/2

def b2s(x):
    return 2*x-1


def index1(s1, s2, s3):
    return 1 + s2b(s1) + s2b(s2)*2 + s2b(s3)*4

def index2(x, s1, s2):
    return 9 + 4*x + s2b(s1) + s2b(s2)*2

def nindex(x, s1, s2):
    return 1 + 4*x + s2b(s1) + s2b(s2)*2

class vertex(object):
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

class normal(object):
    def __init__(self, x, y, z):
        norm = m.sqrt(x*x + y*y + z*z)
        self.x = x / norm
        self.y = y / norm
        self.z = z / norm

def dotn(v1, v2, n):
    return (v2.x-v1.x)*n.x + (v2.y-v1.y)*n.y + (v2.z-v1.z)*n.z

def dot(v1, v2):
    return v2.x*v1.x + v2.y*v1.y + v2.z*v1.z

def sub(v2, v1):
    return vertex(v2.x-v1.x, v2.y-v1.y, v2.z-v1.z)

def cross(v1, v2):
    return vertex(v1.y * v2.z - v1.z * v2.y,
                  v1.z * v2.x - v1.x * v2.z,
                  v1.x * v2.y - v1.y * v2.x)


def vol(v0, v1, v2, n):
    return dot(cross(sub(v1, v0), sub(v2, v0)), n)



def n_bueno(v, n):
    return v.x*n.x + v.y*n.y + v.z*n.z > 0

def main(detailX = 128, detailY = 128):

    verts = []
    texes = []
    faces = []
    dx = 1. / (detailX - 1)
    dy = 1. / (detailY - 1)
    for a in range(0, detailX):
        x = -1 + 2 * a * dx
        tx = a * dx
        for b in range(0, detailY):
            y = -1 + 2 * b * dy
            ty = b * dy
            verts.append(vertex(x, y, 1))
            texes.append(vertex(tx, ty, 0))

    for a in range(0, detailX - 1):
        for b in range(0, detailY - 1):
            x00 = a * detailX + b + 1
            x01 = a * detailX + b + 1 + 1
            x10 = (a + 1) * detailX + b + 1
            x11 = (a + 1) * detailX + b + 1 + 1
            faces.append((x00, x10, x11, x01))

    for v in verts:
        print 'v {} {} {}'.format(v.x, v.y, v.z)

    print('\nvn 0 0 1\n')

    for t in texes:
        print 'vt {} {}'.format(t.x, t.y)


    for f1, f2, f3, f4 in faces:
        print 'f {0}/{0}/1 {1}/{1}/1 {2}/{2}/1 {3}/{3}/1'.format(f1, f2, f3, f4)


if __name__ == '__main__':
    args = map(int, sys.argv[1:])
    main(*args)
