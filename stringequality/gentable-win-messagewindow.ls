global table
set table to []
repeat with x = 0 to 255
  add table, x
end repeat
repeat with x = 0 to 255
  set nx to numToChar(x)
  repeat with y = 0 to 255
    set ny to numToChar(y)
    if x <> y and nx = ny and y < x then
      setAt table, x + 1, y
      exit repeat
    end if
  end repeat
end repeat
set outObj to new(xtra "fileio")
createFile(outObj, "C:\table-d8-en.txt")
openFile(outObj, "C:\table-d8-en.txt", 2)
writeString(outObj, string(table))
closeFile(outObj)
put "DONE"
