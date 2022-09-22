#!/bin/bash

#reference: script argument parsing: https://unix.stackexchange.com/questions/31414/how-can-i-pass-a-command-line-argument-into-a-shell-script
#and https://stackoverflow.com/questions/4341630/checking-for-the-correct-number-of-arguments

if [ "$#" -ne 1 ] ; then
  echo "Usage: $0 [source abc notation file]"
  echo "The source abc notation file can be a URL link"
  echo "Using default abc notation file."
  abc_file="dist/sounds/default_abc.txt"
else
  abc_file=$1
fi

mkdir dist/sounds
python3 PySynth/abc_to_note_list.py "$abc_file" "dist/sounds/note_list.txt"
python3 PySynth/note_list_to_assets.py dist/sounds/note_list.txt --syn_s