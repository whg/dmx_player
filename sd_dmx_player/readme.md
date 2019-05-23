
The top of the file looks like:

```c++
//#define DEBUG
#define FILENAME "fog.raw"
#define INITIAL_DELAY 60 // seconds
#define TRIGGER_PIN A0
#define NUM_RUNS 3
#define TIMEOUT_SECONDS 3.2
```

These are the only things you should need to change.

For the bottom 3, if you comment them out (or remove them), you will remove their functionality.

So, with the following:

```c++
#define FILENAME "fog.raw"
#define INITIAL_DELAY 60 // seconds
```

After 60 seconds, the player will loop `fog.raw` forever.
x
### TRIGGER_PIN

The trigger pin is configured to be active low, i.e. it enables the pull up resistor. To use it, `#define TRIGGER_PIN` with some pin number.

When pressed this pauses or plays the file.

### NUM_RUNS

If this is `#define`d then the player will stop after that number. If `TRIGGER_PIN` is defined then this it the player will be able to start again from beginning if it receives a trigger after it's stopped.

### TIMEOUT_SECONDS

This is related to `TRIGGER_PIN` and has no functionality unless it is also `#define`d. With this enabled, the program looks for changes on the `TRIGGER_PIN`, if there is no change within `TIMEOUT_SECONDS` the player is paused. A trigger will resume playback.
