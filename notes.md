# Bonce Keeb ToDo

- [X]  01 - Simple NeoPixel rigging
- [X]  02 - NeoPixel address to button mapper
- [ ]  03 - Keyboard binding maps
- [ ]  04 - NeoPixel maps for Keyboard bindings
- [ ]  05 - Button Matrix code to recognise keypresses
- [ ]  06 - Button Matrix for assigning given keypresses to button presses
- [ ]  07 - Neopixel animations on button press event
- [ ]  08 - OLED library support
- [ ]  09 - Tile bitmaps to code
- [ ]  10 - Assign each 'tile' to a Button in the Button Matrix
- [ ]  11 - Front Button code
- [ ]  12 - Event receiver for Front Button presses

-----

## Notes
### 03 - Keyboard Binding Maps
These are in the Circuit Python code

### 04 - NeoPixel colour map
These are in the Circuit Python code
neopixels

```python
pixel_count = 20
pixels = pretty_pixels(board.GP0, pixel_count, 3, 0.5, False, None)
```

## Original Python rigging


```python
# ====== Screen Setup ====== #
display = ssd_1306s(board.GP2, board.GP3, 128, 64)

# ====== Spritesheets Setup ====== #
# key index mappings:
# 0 : key name
# 1 : key code
# 2 : key colour
# 3 : ?
# 4 : ?

dev_sprite_details = [
    ["Esc", Keycode.ESCAPE, 0xff0000, 0, 17],
    ["Sh-Del", [Keycode.SHIFT, Keycode.DELETE], 0xff0000, 16, 16],
    ["F9", Keycode.F9, 0xed1351, 12, 9],
    ["F5", Keycode.F5, 0x00ff6e, 8, 8],
    ["Sh-F5", [Keycode.SHIFT, Keycode.F5], 0xff0000, 9, 1],

    ["empty", None, 0x000000, 14, 18],
    ["empty", None, 0x000000, 14, 15],
    ["F12", Keycode.F12, 0x00ccff, 13, 10],
    ["F10", Keycode.F10, 0x00ccff, 10, 7],
    ["F11", Keycode.F11, 0x4374b5, 11, 2],

    ["empty", None, 0x000000, 14, 19],
    ["Pg-Up", Keycode.PAGE_UP, 0xf782ff, 7, 14],
    ["Home", Keycode.HOME, 0x6af011, 1, 11],
    ["Up", Keycode.UP_ARROW, 0x00aaff, 2, 6],
    ["End", Keycode.END, 0xf782ff, 6, 3],

    ["empty", None, 0x000000, 14, 20],
    ["Pg-Dn", Keycode.PAGE_DOWN, 0xf782ff, 15, 13],
    ["Left", Keycode.LEFT_ARROW, 0x00aaff, 4, 12],
    ["Down", Keycode.DOWN_ARROW, 0x00aaff, 3, 5],
    ["Right", Keycode.RIGHT_ARROW, 0x00aaff, 5, 4]
]

dev_sprites = sprite_sheet(display, 
                        "boncekeeb/tiles/dev_tiles.bmp", 
                        16, 16, dev_sprite_details)


# ====== Front Buttons Setup ====== #
names_and_pins = [ ["Blue", board.GP21, 0x0000ff],
                   ["Green", board.GP20, 0x00ff00],
                   ["Red", board.GP19, 0xff0000],
                   ["Yellow", board.GP18, 0xffff00] ]
front_buttons = front_buttons(names_and_pins)
selected_color = "Red"

# ====== Keyboard Setup ====== #
cols = (board.GP13, board.GP12, board.GP11, board.GP10, board.GP9)
rows = (board.GP22, board.GP26, board.GP27, board.GP28)
keys = [["1", "2", "3", "4", "5"],
        ["6", "7", "8", "9", "10"],
        ["11", "12", "13", "14", "15"],
        ["16", "17", "18", "19", "20"]]
keypad = keypads(cols, rows, keys)

# ====== Keyboard Pixel Map ====== #
key_pixel_map = [
    [5, 1], [10, 2], [15, 3], [20, 4], [19, 5], [14, 6],
    [9, 7], [4, 8], [3, 9], [8, 10], [13,11], [18,12],
    [17,13], [12,14], [7,15], [2,16], [1,17], [6,18],
    [11,19], [16,20]
]
```

