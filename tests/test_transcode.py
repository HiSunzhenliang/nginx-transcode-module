#!/usr/bin/env python3
# @file test_transcode.py
# @author SunZhenliang

import os
import urllib.request
import subprocess

PORT = 8080
HOST = "127.0.0.1"

# SOURCE = [(CODING_FORMAT, CONTAINER_FORMAT)]
# TARGET = [(CODING_FORMAT, CONTAINER_FORMAT)]

WORK_DIR = "work_dir"
ORIGIN_DIR = WORK_DIR + "/" + "origin"
TARGET_DIR = WORK_DIR + "/" + "target"

SOURCE = [
    ("signed-integer", "wav"),
    ("a-law", "wav"),
    ("u-law", "wav"),
    ("amrnb", "amr"),
    ("amrwb", "awb"),
    ("MPEG", "mp3")
]

TARGET = [
    ("signed-integer", "wav"),
    ("MPEG", "mp3")
]


def listdir(path, list_name):
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        if os.path.isdir(file_path):
            listdir(file_path, list_name)
        else:
            list_name.append(file_path)


def download_target(target_list):
    os.mkdir(TARGET_DIR)
    urls = [f"http://{HOST}:{PORT}/{i}" for i in target_list]
    targets = []
    for i in urls:
        try:
            file_path = TARGET_DIR + "/" + os.path.basename(i)
            print(f"Downloading {i}")
            urllib.request.urlretrieve(i, file_path)
            targets.append(file_path)
        except Exception as e:
            print(e, i)
    return targets


def generate_targetlist(origin_list):
    namebases = [os.path.splitext(i)[0] for i in origin_list]
    target_list = [namebase + "." + exten
                   for namebase in namebases for _, exten in TARGET]
    return target_list

def callcmd(cmd):
    return subprocess.check_output(cmd,shell=True,stderr=subprocess.PIPE,encoding="utf-8",timeout=1)

def main():
    origin_list = []
    listdir(ORIGIN_DIR, origin_list)
    origin_list = [os.path.basename(i) for i in origin_list]
    target_list = generate_targetlist(origin_list)
    targets = download_target(target_list)
    print(f"\nTargets:")
    for i in targets:
        print(f"{i}\n{callcmd(f'soxi -e {i}')}{callcmd(f'soxi -d {i}')}")


if __name__ == '__main__':
    main()
