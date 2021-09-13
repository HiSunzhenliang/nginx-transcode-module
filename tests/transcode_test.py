#!/usr/bin/env python3

# @file transcode_test.py
# @author SunZhenliang


# SOURCE = [(CODING_FORMAT, CONTAINER_FORMAT)]
# TARGET = [(CODING_FORMAT, CONTAINER_FORMAT)]

SOURCE = [
    ("pcm","wav"),
    ("a-law", "wav"),
    ("u-law", "wav"),
    ("amrnb", "amr"),
    ("amrwb", "awb"),
    ("MPEG", "mp3")
]

TARGET = [
    ("pcm","wav"),
    ("MPEG", "mp3")
]

