## ClockSim

Rubik's Clock simulator written by Mattia Furlan (2016).

![Clock simulator](https://github.com/mattia-furlan/clocksim/blob/main/imgs/clocksim.png)


## SOLVING
Press the key G to scramble the clock.
Press any key related to the pins or to the wheels to start the timer.
If you rotate the clock (y2, x2, z, z', z2) during the inspection the timer will not start.
The timer stops automatically when you finish the solve, so there are no DNFs.

To see the stats about the current single or a current avgX (time, movecount, tps, scramble, date and time), just click on the time/movecount/tps with the mouse and a new window will be displayed.

When you press a key to rotate a wheel, the program will automatically rotate a wheel adjacent to a pin in "up" state (if there are any, otherwise just a random wheel).

## CONTROLS
These are the default controls, you can change them in Settings -> Controls.
To do that click on the control you want to change anche press the desired key (combinations work too).
`
    Flip pins UL/UR/DL/DR: Q/W/A/S
    1+ turn: K
    1- turn: J
    6+ turn: L
    y2 rotation: Space
    z rotation: V
    Scramble: G
    Clear the session: Esc`

When you close the simulator, the controls are automatically stored in a settings file so you don't have to set them again the next time you open the simulator.

## COLORS
You can personalize the style in Settings -> Control.
Click on the colored rectangles to change them.

As with the controls, the colors are saved in a settings file too.


![Settings](https://github.com/mattia-furlan/clocksim/blob/main/imgs/clocksim_settings.png)

## STATISTICS AND PBs 
The program saves the PBs on a file called 'clockPBs.bin' in you home folder.
Don't try to change it otherwise it will be corrupted and you will lose your PBs.

When you quit the sim the session does not get reset because the simulator automatically saves the solves on a file called 'clocksession.bin' in your home folder.
Don't try to change it otherwise it will be corrupted and you will lose your solves.


![PBs](https://github.com/mattia-furlan/clocksim/blob/main/imgs/clocksim_pbs.png)
