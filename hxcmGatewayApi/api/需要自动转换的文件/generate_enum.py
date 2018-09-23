__author__ = '刘军'
#!/usr/bin/env python
import sys
import os
import string


def main():
    '''主函数'''
    fcpp = open('O2GEnum.h','r')
    fpy = open('fxcm_enum1.py','w',encoding='utf-8')
    lines = []
    line = fcpp.readline()
    lines = fcpp.readlines()

    lines.reverse()

    fpy.write('# encoding: UTF-8\n')
    fpy.write('\n')
    fpy.write('enumDict = {}\n')
    fpy.write('\n')

    for line in lines:
        # enum 结束
        if '}' in line:
            content = line.split(' ')
            name = content[1].replace(';\n', '')
            py_line = 'enumDict["%s"] = %s\n' % (name, name)
        # enum申明
        elif 'enum' in line:
            py_line = '\n'
        # enum 变量
        elif '=' in line:
            content = line.split(' ')
            enumName = content[0]
            enumValue = content[2].replace('\n','')
            py_line = '%s[%s] = %s\n' % (name, enumName,enumValue)
        elif '{' in line:
            py_line = '\n'

if __name__ == '__main__':
    main()

