Unit tests for Director 5
=========================

The objective of these tests is to provide regression coverage for the undocumented parts of the Macromedia Director engine as implemented in ScummVM.

`switch_movie_test.dir`: On Startup load an external movie `switch_movie_test_movie1.dir` onto the stage. Meant to check if external movie castmembers can switch to different external movies.

`switch_movie_test_movie1.dir`: Load a recognizable (blue colored solid circle) bitmap image and a button `Switch Movie` upon pressing which the movie switches to a different movie `switch_movie_test_movie2.dir`, not meant to be played separately.

`switch_movie_test_movie2.dir`: Load a recognizable (orange colored solid circle) bitmap image and a button `Switch Movie` upon pressing which the movie switches to a different movie `switch_movie_test_movie1.dir`, not meant to be played separately.

`palette_duplication_test_d5.dir`: Load an empty canvas with two buttons, `Duplicate Cast` (for duplicating the palette cast member present) and `Save Movie` (which attempts to save the movie). No visual effect to be noticed except Cast has an extra castmember.

`movie_castmember_test_d5.dir`: Load a movie cast member onto the stage, two buttons, one sets the value of a global variable `gNodeNow` to 1 and the other to 3, depending upon the value of the node, the movie cast member should react.

`10PLAN_M.dir`: Holds the plan data for Captain's Chembers in trektech-win, meant only as support for `movie_castmember_test_d5.dir`, not meant to be loaded separately.

`4PLAN_M.dir`: Holds the plan data for one of the floors in trektech-win, meant only as support for `movie_castmember_test_d5.dir`, not meant to be loaded separately.
