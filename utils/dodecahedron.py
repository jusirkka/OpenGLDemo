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

def dot(v1, v2, n):
    return (v2.x-v1.x)*n.x + (v2.y-v1.y)*n.y + (v2.z-v1.z)*n.z

def n_bueno(v, n):
    return v.x*n.x + v.y*n.y + v.z*n.z > 0

def main(testing):
    vertices = {}
    normals = {}
    texes = {}
    faces = {}
    p1 = (m.sqrt(5) + 1) / 2
    p2 = (m.sqrt(5) - 1) / 2
    for x in [-1, 1]:
        for y in [-1, 1]:
            for z in [-1, 1]:
                vertices[index1(x, y, z)] = vertex(x, y, z)
    for a1 in [-1, 1]:
        for a2 in [-1, 1]:
            normals[nindex(2, a1, a2)] = normal(a1, p2 * a2, 0)
            vertices[index2(2, a1, a2)] = vertex(p2 * a1, p1 * a2, 0)
    for a2 in [-1, 1]:
        for a3 in [-1, 1]:
            normals[nindex(0, a2, a3)] = normal(0, a2, p2 * a3)
            vertices[index2(0, a2, a3)] = vertex(0, p2 * a2, p1 * a3)
    for a3 in [-1, 1]:
        for a1 in [-1, 1]:
            normals[nindex(1, a3, a1)] = normal(p2 * a1, 0, a3)
            vertices[index2(1, a3, a1)] = vertex(p1 * a1, 0, p2 * a3)

    for n in range(1, 6):
        texes[n] = vertex(m.cos(2*m.pi*n/5), m.sin(2*m.pi*n/5), 0)

    for a1 in [-1, 1]:
        for a2 in [-1, 1]:
            faces[nindex(2, a1, a2)] = (index2(1, -1, a1),
                                        index1(a1, a2, -1),
                                        index2(2, a1, a2),
                                        index1(a1, a2, 1),
                                        index2(1, 1, a1))
    for a2 in [-1, 1]:
        for a3 in [-1, 1]:
            faces[nindex(0, a2, a3)] = (index2(2, -1, a2),
                                        index1(-1, a2, a3),
                                        index2(0, a2, a3),
                                        index1(1, a2, a3),
                                        index2(2, 1, a2))
    for a3 in [-1, 1]:
        for a1 in [-1, 1]:
            faces[nindex(1, a3, a1)] = (index2(0, -1, a3),
                                        index1(a1, -1, a3),
                                        index2(1, a3, a1),
                                        index1(a1, 1, a3),
                                        index2(0, 1, a3))

    keys = sorted(vertices.keys())
    for key in keys:
        v = vertices[key]
        print 'v {} {} {}'.format(v.x, v.y, v.z)

    keys = sorted(normals.keys())
    for key in keys:
        n = normals[key]
        print 'vn {} {} {}'.format(n.x, n.y, n.z)

    keys = sorted(texes.keys())
    for key in keys:
        t = texes[key]
        print 'vt {} {}'.format(t.x, t.y)

    keys = sorted(faces.keys())
    for key in keys:
        f1, f2, f3, f4, f5 = faces[key]
        print 'f {1}/1/{0} {2}/2/{0} {3}/3/{0} {4}/4/{0} {5}/5/{0}'.format(key, f1, f2, f3, f4, f5)

    if testing:
        for key in keys:
            print key
            f1, f2, f3, f4, f5 = faces[key]
            print dot(vertices[f1], vertices[f2], normals[key])
            print dot(vertices[f1], vertices[f3], normals[key])
            print dot(vertices[f1], vertices[f4], normals[key])
            print dot(vertices[f1], vertices[f5], normals[key])
            print n_bueno(vertices[f1], normals[key])
            print n_bueno(vertices[f2], normals[key])
            print n_bueno(vertices[f3], normals[key])
            print n_bueno(vertices[f4], normals[key])
            print n_bueno(vertices[f5], normals[key])


if __name__ == '__main__':
    main(''.join(sys.argv[1:]))
