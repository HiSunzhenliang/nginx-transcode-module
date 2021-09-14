#!/usr/bin/env python3
# @file test_origin.py
# @author SunZhenliang

import os
import urllib.request
import json

WORK_DIR = "work_dir"
ORIGIN_DIR = WORK_DIR + "/" + "origin"

ORIGIN_LIST = "origin.list"

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
    print(f"\nOrigins:")
    for i in origin_list:
        print(i)

    path = os.getcwd() + "/" + ORIGIN_DIR
    print(f"\nAdd this in nginx.conf: \ntranscode_root \"{path}\";\n")


if __name__ == '__main__':
    main()
