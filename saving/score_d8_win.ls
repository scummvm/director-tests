on buildIt
  bm = new(#bitmap)
  bm.name = "box"
  img = image(32, 32, 16)
  img.fill(img.rect, rgb(255, 0, 0))
  bm.image = img
  b1 = new(#button)
  b1.name = "dup_button"
  b1.text = "Duplicate"
  b2 = new(#button)
  b2.name = "save_button"
  b2.text = "Save"
  beginRecording
  the frameTempo = 30
  repeat with f = 1 to 8
    sprite(1).member = bm
    sprite(1).loc = point(50 + 10 * min(f, 3), 100)
    sprite(2).member = b1
    sprite(2).loc = point(100, 200)
    sprite(3).member = b2
    sprite(3).loc = point(220, 200)
    updateFrame
  end repeat
  endRecording
  saveMovie
end
