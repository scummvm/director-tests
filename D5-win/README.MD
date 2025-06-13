Unit tests for Director 5
=========================

The objective of these tests is to provide regression coverage for the undocumented parts of the Macromedia Director engine as implemented in ScummVM.

`switch_movie_test.dir`: On Startup load an external movie `switch_movie_test_movie1.dir` onto the stage. Meant to check if external movie castmembers can switch to different external movies.

`switch_movie_test_movie1.dir`: Load a recognizable (blue colored solid circle) bitmap image and a button `Switch Movie` upon pressing which the movie switches to a different movie `switch_movie_test_movie2.dir`, not meant to be played separately.

`switch_movie_test_movie2.dir`: Load a recognizable (orange colored solid circle) bitmap image and a button `Switch Movie` upon pressing which the movie switches to a different movie `switch_movie_test_movie1.dir`, not meant to be played separately.
