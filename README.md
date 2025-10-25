#Camsynth Sender
This is a C++ app works in conjunction with the Camsynth Player or Camsynth Filter to control a filter based on the ratio of the amplitudes of high and low frequencies of an outline.

For a full demonstration and short walkthrough of how it works, check out my website: https://devynoh.dev/portfolio/camsynth/

###Compiling and Running
This does not have a compiled version on github. To run this, run main.cpp in a C++ ide or compile and run it like any other C++ program. There are settings you can change in main.cpp. Look for the OptionsSnapshot snapshot, starting near the top of the file. Used snapshot.port.pushback() to connect to multiple plugins with multiple ports at once.

###See also:
Camsynth Player: A synth that uses an outline to generate sound using all of the partials of its fourier series.
Camsynth Filter: A plugin that uses the ratio between the amplitudes of high and low frequencies of an outline to control a filter.
