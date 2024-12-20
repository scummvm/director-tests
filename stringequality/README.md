# String equalitytables

Different version of director have different ideas of what characters are equal.
These tables are created in their respective Director versions and platforms.
The original files and tools to generate the tables are stored in this directory 
to make the generation reproducable.

## Director side of things

For the windows versions, the tables are generated as a global variable called table.
In the Message window the tables are printed with `put table` and copy-pasted into a text file. 
For the mac versions the output is different and also copy-pasted into a text file.
For D3 mac and D4 mac the tables are the same.

## ScummVM side of things

The tables can be read and outputted with the corresponding python files.
For `createtable-win.py` append the filename of the table.

These string equality tables are used in `engines/director/util.cpp`.
