# BonceKeeb v3.0

It's a Keyboard! And my online name is Bonce!
..I had a brief moment of existential doubt creep in there after re-writing that line too many times and had to google how to spell 'keyboard' because the word utterly lost meaning to me.

Anyway, in other news, I guess this is Version 3!
Not because I made such great strides in versions 1 and 2, but because of entirely poor reasons that I don't feel great about.

Version 1 was written in Circuit Python for the following reasons:
 - I had started to write Python for work and really liked it
 - I had started to write Micro Python on Raspberry Pico's and ESP8266's and really liked it
 - I was fully aware of the pitfalls of using Python on a embedded platform, but also knew where and how to make performance savings when I needed to
 - I couldn't use Micro Python for this so had to use Circuit Python because the HID layer had not yet been implemented into Micro Python 

This resulted in an *almost* fully working implementation, it gave me:
 - All individually addressable RGB LED's
 - Functioning keypress/release on each key
 - When a key is pressed it triggered a corresponding HID keypress report and that keypress happened in the OS
 - The front button presses registered, although the functionality to do anything with them was not implemented
 - The SSD1306 screen displayed the current keymap overlay using bitmap image sprites
 - It knew about keyboard and LED overlays! I mean there was only one of them, but it knew there at least could be more!

Then Version 2 happened, this is the follow reasons why Version 1 was dropped:
 - Circuit Python for me didn't work, I want to version control my changes, and without setting up an unatural toolchain this wasn't feasible
 - When a key was pressed I would either lock the HID report open so it was never issued to the OS, or it went into a race-conditioning loop where that key then kept being added into subsequent reports, this got worse as time went by
 - I wanted to at least try and do it in C++, to see if I knew enough C++ to get there

Version 2 was written in C++ for the following reasons:
 - It's C++, there isn't much further down I can go, it's seemingly perfect for embedded
 - The documentation and getting started support from the Raspberry Foundation is really rich, I should be able to 'do something'
 - Initally all the big problems look to be initially solved, I can find online instances of simple buttons, Neopixels, SSD1306 screens, HID devices and keyboard matrices eitgher directly within the Raspberry Pico C++ documentation, or close enough that I could get a working version into a Pico
 - The challenge

This resulted in a *somewhat* functioning implementation, it gave me:
 - All individually addressable RGB LED's
 - Functioning keypress/release on each key
 - The front button presses registered, although the functionality to do anything with them was not implemented
 - The SSD1306 screen on thr front displayed text

As the astute amongst you will note, Version 1 never **worked**. It's supposed to be a keyboard first, the pretty lights, the overlays selection buttons on the front, the Screen to tell you which overlay you are in, all these things are secondary to being a keyboard, and a keyboard that either repeatedly machine guns the buttons you press, or just stops sending keypresses is not a keyboard. I'm not sure what it *is* but a keyboard is definitely not that.

Saying that, Version 1 was the closest and to be honest if it was in Micro Python I likely would have stuck with it as Source Control and IDE choice would have been natural, but the further I used Circuit Python the weirder it was for me. Even getting the right 3rd party Libs was a trial. This is not me bashing it, it just doesn't work for my process because I'm old and stuck in my ways.

Version 2 quickly did what C++ has done to me every time, the broad strokes are fine, the big chonky boy lumps all work wonderfully, but purely because I have no idea what I'm doing, the fine detail turns into a bramble patch and because it's such trivial bug fixes that are obvious in retrospect, it doesn't feel like I'm learning anything or getting any better.

:) Which brings us to Version 3. Rust.
So a pal of mine mentioned that he's been learning Rust. This chap is a C guy, he can C++ but doesn't like it. He's **really** good at C.
..and he's saying that he likes Rust.
That goes into the back of my mind and sits there until last week when I found a amazing walkthrough on setting up a Rust project to register a switch press on a Pico and then issue  the key `i`. When released it issues a `ESC` (yes, for Vim ;)).
Well, that's interesting, HID is really the one thing that's been holding me up, if I have a HID implementation that is otherwise a `hello world` project for a Pico then this is likely most of the hard work, so I jumped on PluralSight and watched a deprecated Rust video and I really like the language.. It's got all the things in it that I like from C++ (object orientation, albeit functionally, type safety as first party), some of the things I like about C# (lazy man's helper functions **everywhere**) and even some human readablity things from Python (pretty loops, truthiness).

So, that brings you up-to-date with why this has leaped to a whole new major version. I'm going to try doing this in Rust. It can't go any worse than my C++ attempt, frankly and even if it does, if I keep circling the drain on this, the longer that takes, the more likely that Micro Python has decent HID support for Pico's and I take the extremely lazy route out ;)  