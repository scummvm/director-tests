# director-tests
Testing movies for Director development


#### D3-mac/D3-Events-NonShared
Test for understanding event passing and script execution in D3 - Without a SharedCast

#### D3-mac/D3-Events-NonShared-Macros
Test for understanding event passing and script execution in D3 along with Macros - Without a SharedCast

#### D4-mac/D4-Events-NonShared
Test for understanding event passing and script execution in D4 - Without a SharedCast

#### buildbot-tests/

Tools for creating repeatable movies that can be run with Buildbot.

An introduction.

The Shared Cast in D3-mac-record has a macro called `scummVMLog`.
Log things you're interested in by calling `scummVMLog importantVAR`.
The macro stores this log as text in the cast B11.
Save the movie after a run so that the text in cast B11 gets saved as well.
B11 was used as to not interfere with regular casts.

The Shared Cast in D3-mac-replay also has a macro called `scummVMLog`.
This time it checks if the emitted log line is the same as the line stored in B11.
It will emit a 'BUILDBOT: incorrect check for line: ...' when the emitted log
is not the same as the stored log. 

Since this all happens in Lingo lots of interesting things can by logging them.

As an example check the movie putter in buildbot-tests/D3-mac. 

For reference, the recorder and replay macros:
-- scummVM logger
macro scummVMLog logLine
  global logText, nextLogLine
  if logText = 0 then
    set logText to logLine
    set nextLogLine to 2
  else
    put logLine after line nextLogLine of logText
    set nextLogLine to nextLogLine + 1
  end if
  put logText into cast B11

-- scummVM check if log in cast B11 is the same
macro scummVMLog logLine
  global  currentLogLine
  if currentLogLine = 0 then
    -- globals were previously undefined
    set currentLogLine to 1
  else
    set currentLogLine to currentLogLine + 1
  end if
  set lineToCheck to line currentLogLine of the text of cast B11
  if lineToCheck = logLine then
    put "Line is correct:" && currentLogLine
  else
    put "BUILDBOT: incorect check for line:" && currentLogLine
    put "Got:" && logLine
    put "Expected:" && lineToCheck
  end if

