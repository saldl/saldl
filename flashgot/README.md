# Using saldl with flashgot

## Scripts in this directory

 Note that these scripts assume everything is in `PATH`.

 They also assume that `konsole` is your terminal emulator of choice.
 Feel free to change it to whatever emulator you like (as long as it
 supports the `-e` option).

### saldl-get

 This is the script you should point `flashgot` to. It depends on
 `saldl-no-exit` which in turn depends on `zsh`.

### saldl-no-exit

 A `zsh` script responsible for parsing command line arguments passed
 by `flashgot`. Then passing the parsed arguments in turn to `saldl`.

 This template (or a variation of it) should be set in `flashgot`:

```
-r -VVV [;;;URL;;;] [-u ;;;UA;;;] [-e ;;;REFERER;;;] [-D ;;;FOLDER;;;] [-o ;;;FNAME;;;] [-k ;;;COOKIE;;;] [-p ;;;POST;;;]
```

 Parsing code depends on `;;;` being present before and after placeholder names.

### saldl-get-tmux

 An alternative to `saldl-get` that creates an empty `tmux` session
 an runs `saldl` inside it.

 It depends on `saldl-tmux` and `saldl-no-exit`.

### saldl-tmux

 A script used by `saldl-get-tmux`.
