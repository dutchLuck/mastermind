# mastermind
Text mode version of the Mastermind game, with letters instead of colours.
This coding effort was inspired by the lack of a plain old c version of
mastermind in the lineup of
<a href="https://rosettacode.org/wiki/Mastermind">Rosetta Code</a>
program examples.

There are some choices that the game player can make that change the setup
configuration of the game. 
```
User choices; -
  choose the number of code letters available in the game (2 - 20)
  choose the letter code length of the secret code (4 - 10)
  choose the maximum number of guesses the player has (7 - 20)
  choose whether or not letters may be repeated in the code
```
An example of playing the game is; -
```
% ./mastermind 

Hello, Welcome to the game of Mastermind
This version of the game uses letter codes instead of colours.

The game settings are; -
7 code letters in use & they are: ABCDEFG
The secret code is 4 letters long
7 attempts to guess the secret code are avaiable
Code letters are repeated in the code

Do you wish to change the Game settings?: yes or no (defaults to no) : no

Round number 1: Please enter 4 letters from the set "ABCDEFG": aaaa
 1.   AAAA   |   X---
Round number 2: Please enter 4 letters from the set "ABCDEFG": abbb
 1.   AAAA   |   X---
 2.   ABBB   |   X---
Round number 3: Please enter 4 letters from the set "ABCDEFG": accc
 1.   AAAA   |   X---
 2.   ABBB   |   X---
 3.   ACCC   |   XXX-
Round number 4: Please enter 4 letters from the set "ABCDEFG": accd
Congratulations, you succeeded in specifying "ACCD", which was the secret code!

Do you wish to play again?: yes or no (defaults to yes) : no

Thank you for playing Mastermind
You played 1 game and were succesful on 1 occasion.
%
```
