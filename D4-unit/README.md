Unit tests for Director 4
=========================

The objective of these tests is to provide regression coverage for the undocumented parts of the Macromedia Director engine as implemented in ScummVM. These cases are impossible to isolate as a pure Lingo unit test; for example, the execution order of the event handler system, and the precise output of the sprite blitter.

These tests are based on the UnitTest framework; with code in SHARED.DIR, and an XObject DLL UNITTEST.DLL (source in xobjdev/unittest) used for injecting input events and taking screenshots. 

Running the tests in Director
-----------------------------

RUNTESTS.EXE is a Director for Windows 4.0.4 projector. It is an empty movie, except for a single startMovie handler which calls `UTRunAllTests()`. RUNTESTS.DIR is the same movie, without the projector.

Starting RUNTESTS.EXE in a Windows 3.1 VM will run all of the tests in sequence. The test report will be appended to UTLOG.TXT, and screenshots will be saved to the SCRBASE folder in BMP format. Goes without saying, but these tests should all pass!

RUNTESTS.EXE will run every test (i.e. movie beginning with `T_`) in order and exit. Opening and running an individual test in the Director editor will pause the move when the test is completed. 

Running the tests in ScummVM
----------------------------

Running the full test suite:

```sh
scummvm --start-movie=RUNTESTS.DIR director
```

The test report will be sent to stdout. It is possible to run individual tests in a similar way, however as above they will pause the movie instead of exiting when completed.

UnitTest Lingo API
------------------

`UTBeginTest(name)` - Start the current test. Ideally call this in the `startMovie` handler. `name` is a short description about the contents of the test.

`UTEndTest(count)` - End the current test. Ideally call this in a handler at the end of the movie, and ensure that it gets called even if the test fails. `count` is an optional check for the number of `UTAssert*` statements that should have been called during this test.

`UTAssertTrue(value, desc)` - Check and record that `value` is equal to TRUE. `desc` is a short description about the position in the code or the behaviour under test.

`UTAssertEqual(valueA, valueB, desc)` - Check and record that `valueA` is equal to `valueB`. `desc` is a short description about the position in the code or the behaviour under test.

`UTMoveMouse(x, y)` - Move the mouse to these window coordinates.

`UTLeftMouseDown()` - Press the left mouse button.

`UTLeftMouseUp()` - Release the left mouse button.

`UTScreenshot(name)` - Save the contents of the window to a file for visual regression testing. `name` can be a file name, or an integer (in which case the file name will be `{moviename}.{%03d}`). Screenshots will be saved to the `SCRBASE` folder for native Director, and the `SCRTEST` folder for ScummVM.

`UTLog(message)` - Write a timestamped message to the Message window/stdout, and (when running the tests from a projector) to the file UTLOG.TXT.

Tips for making event system tests
----------------------------------

- Leave frame 1 blank; you will not get the `enterFrame` event when running the test from the editor.
- Create a global variable `evCount` that starts from 1 and counts upward.
- Create event handlers for every possible outcome with a call to `UTAssertEqual(1, evCount, "useful comment about where and what is being called")`. Afterwards, increment `evCount` by 1.
- Run the test from the beginning. See which handlers get called and in what order.
- Go through and update the 1 in each `UTAssertEqual` call to match the order returned by running the test.
- For any handlers that are _not_ called, replace this with a `UTAssertTrue(FALSE, "comment")` call to indicate this code should never be reached.
- Ensure that your test runs reliably, both in the editor and the projector.
