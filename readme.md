## Playing DMX

## Installation

Run the install script. This will put the scripts in your `$PATH`.

Script are written for Python 3.6 or above.

### `vezer2csv`

e.g.
```sh
vezer2csv exports/vezer_export.xml
```

This will create a csv for each compostion in the export.


#### `csv2raw`

Usage:

```sh
cv2raw filenames...
```

Do create raws for every csv in the current directory, do:

e.g.
```sh
csv2raw *.csv
```
