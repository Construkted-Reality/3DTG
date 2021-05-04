import sys
import os
import subprocess
import time

HYTHON = 'C:/Program Files/Side Effects Software/Houdini 18.5.351/bin/hython.exe'
TOOL = 'lod_processor_v2.2.py'
FILE = sys.argv[1]
TESTMODE = str(0)
if len(sys.argv)>2:
    TESTMODE = sys.argv[2]


def run_lod_tool():
	dir_path = os.path.dirname(os.path.realpath(__file__))
	tool = dir_path+'/'+TOOL

	cmd = HYTHON+' '+tool+' '+FILE+' '+dir_path +' '+TESTMODE
	subprocess.call(cmd)

run_lod_tool()


