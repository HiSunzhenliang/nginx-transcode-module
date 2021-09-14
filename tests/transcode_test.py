#!/usr/bin/env python3

# @file transcode_test.py
# @author SunZhenliang

import os
import subprocess
import urllib.request
import json

# SOURCE = [(CODING_FORMAT, CONTAINER_FORMAT)]
# TARGET = [(CODING_FORMAT, CONTAINER_FORMAT)]

WORK_DIR = "work_dir"
ORIGIN_DIR = WORK_DIR + "/" + "origin"
TARGET_DIR = WORK_DIR + "/" + "target"

ORIGIN_LIST = "origin.list"

SOURCE = [
    ("signed-integer","wav"),
    ("a-law", "wav"),
    ("u-law", "wav"),
    ("amrnb", "amr"),
    ("amrwb", "awb"),
    ("MPEG", "mp3")
]

TARGET = [
    ("signed-integer","wav"),
    ("MPEG", "mp3")
]


def listdir(path, list_name):
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        if os.path.isdir(file_path):
            listdir(file_path, list_name)
        else:
            list_name.append(file_path)


def callcmd(cmd):
    ret = subprocess.call(cmd, shell=True)
    if ret != 0:
        print(f'ERROR cmd: {cmd}')


def download_origin():
    os.mkdir(ORIGIN_DIR)
    origin_list = []
    with open(ORIGIN_LIST,"r") as f:
        urls = json.load(f)
        for i in urls['origin']:
            try:
                file_path = ORIGIN_DIR + "/" + os.path.basename(i)
                print(f"Downloading {i}")
                urllib.request.urlretrieve(i, file_path)
                origin_list.append(file_path)
            except Exception as e:
                print(e,i)
    return origin_list

def main():
    os.mkdir(WORK_DIR)
    origin_list = download_origin()
    print(f"Inputs:")
    for i in origin_list:
        print(i)


if __name__ == '__main__':
    main()
