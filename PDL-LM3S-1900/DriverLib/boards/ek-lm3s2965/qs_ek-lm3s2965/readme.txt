EK-LM3S2965 Quickstart Application

A game in which a blob-like character tries to find its way out of a maze.  The
character starts in the middle of the maze and must find the exit, which will
always be located at one of the four corners of the maze.  Once the exit to the
maze is located, the character is placed into the middle of a new maze and must
find the exit to that maze; this repeats endlessly.

The game is started by pressing the select push button on the right side of the
board.  During game play, the select push button will fire a bullet in the
direction the character is currently facing, and the navigation push buttons on
the left side of the board will cuase the character to walk in the
corresponding direction.

Populating the maze are a hundred spinning stars that mindlessly attack the
character.  Contact with one of these stars results in the game ending, but the
stars go away when shot.

Score is accumulated for shooting the stars and for finding the exit to the
maze.  The game lasts for only one character, and the score is displayed on the
virtual UART at 115,200, 8-N-1 during game play and will be displayed on the
screen at the end of the game.

If the CAN Device Board is attached and is running the can_device_qs
application, the volume of the music and sound effects can be adjusted over CAN
with the two push buttons on the target board.  The LED on the CAN device board
will track the state of the LED on the main board via CAN messages and during
game play it will also flash when the select button is pressed to fire a
bullet. The operation of the game will not be affected by the absence of the
CAN device board.

Since the OLED display on the evaluation board has burn-in characteristics
similar to a CRT, the application also contains a screen saver.  The screen
saver will only become active if two minutes have passed without the user push
button being pressed while waiting to start the game (that is, it will never
come on during game play).  Qix-style bouncing lines are drawn on the display
by the screen saver.

After two minutes of running the screen saver, the display will be turned off
and the user LED will blink.  Either mode of screen saver (bouncing lines or
blank display) will be exited by pressing the select push button.  The select
push button will then need to be pressed again to start the game.

-------------------------------------------------------------------------------

Copyright (c) 2007 Luminary Micro, Inc.  All rights reserved.

Software License Agreement

Luminary Micro, Inc. (LMI) is supplying this software for use solely and
exclusively on LMI's microcontroller products.

The software is owned by LMI and/or its suppliers, and is protected under
applicable copyright laws.  All rights are reserved.  You may not combine
this software with "viral" open-source software in order to form a larger
program.  Any use in violation of the foregoing restrictions may subject
the user to criminal sanctions under applicable laws, as well as to civil
liability for the breach of the terms and conditions of this license.

THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

This is part of revision 1900 of the Stellaris Peripheral Driver Library.
