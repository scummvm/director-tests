on buildText
  t = new(#text)
  t.name = "sample_text"
  t.text = "The quick brown fox"
  b1 = new(#button)
  b1.name = "dup_button"
  b1.text = "Duplicate"
  b2 = new(#button)
  b2.name = "save_button"
  b2.text = "Save"
  beginRecording
  the frameTempo = 30
  repeat with f = 1 to 8
    sprite(1).member = t
    sprite(1).loc = point(60, 80 + 5 * min(f, 2))
    sprite(2).member = b1
    sprite(2).loc = point(100, 200)
    sprite(3).member = b2
    sprite(3).loc = point(220, 200)
    updateFrame
  end repeat
  endRecording
  saveMovie
end
