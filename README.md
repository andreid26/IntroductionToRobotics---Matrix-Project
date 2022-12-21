## Snake Game
# Introduction
This repository contains the matrix project for the 'Introduction to Robotics' course taken in the third year at the Faculty of Mathematics and Informatics, University of Bucharest.

# Game description
The main idea of the game is a simple one: the player uses a joystick to move a snake made of dots around. As the snake finds food, it eats the food and thereby grows larger. The game ends when the snake either moves off the screen or moves into itself. The goal is to make the snake as large as possible before that happens.

# How to play
When powering up the game, a greeting message is shown until the joystick button is pressed.

The main menu contains:

    1. When powering up the game, a greeting message is shown until the joystick button is pressed
    2. Contains the following categories:
            (a) Start game - when pressed, the game starts with the default difficulty (LOW)
            (b) Highscore:
                    - initially, it prints a default value
                    - when a player makes a new one, the highscore is updated
            (c) Settings:
                    - Name: when locked (i.e the joystick button was pressed) reads the value from Serial and store it
                    - Difficuly: there are 3 different values (LOW, MEDIUM, HIGH)
                    - Sound: ON or OFF (sounds upon interaction - moving the joystick, pressing the button, snake eats food)
                    - Matrix brightness - from 0 to 15
                    - LCD brightness - from 0 to 255
                    - Back - when pressed, returns to the main menu
            (d) Info: includes details about the github profile
            (e) How to play: short and informative description
    3. The navigation through the menu is done by moving the joystick UP and DOWN
    
# Used components:

    - Arduino uno main board
    - 8x8 LED Matrix
    - LCD
    - Joystick
    - Buzzer
    - Ceramic capacitor of 104 pF
    - Electrolytic capacitor of 10 µF
    - Potentiometer (controlling the LCD contrast)
    - Resistors & cables
    
# Picture of the setup

<img src="https://user-images.githubusercontent.com/63780942/208868914-badb3f88-ea25-4eb3-8beb-22307ddec848.png" style="width: 50%;"/>

# Demo video

<a href="https://youtu.be/0qX3Yy0WwqM">Click here</a>
