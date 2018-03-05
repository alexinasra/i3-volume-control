# i3wm Volume Control

## Usage :
```
  -i [n], --increment [n] - increment volume by n .
  -d [n], --decrement [n] - decrement volume by n .
  -s [n] --set [n] - set volume to n .
  -m --mute - toggle mute .
  -v --verbose - explain what is being done .
  -h --help - display this help and exit .
```
## Build :
```bash
$>make all
```

## Install :
```bash
$>sudo cp -v i3-volume-control /usr[/local]/bin
```
## Configuration :
This is an example of key bindings in the i3config file, open ~/.config/i3/config with your favorite Editor, and add the following.

```
# sound volume control
bindsym $mod+F12 exec i3-volume-control -i
bindsym $mod+F11 exec i3-volume-control -d
bindsym $mod+F10 exec i3-volume-control -m
```

.
