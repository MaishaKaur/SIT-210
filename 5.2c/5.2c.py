from tkinter import *
import tkinter.font
from gpiozero import PWMLED

# GPIO pin setup
led_red = PWMLED(18)     
led_green = PWMLED(14)   
led_white = PWMLED(15) 
# GUI setup
win = Tk()
win.title("LED Brightness Control")
myFont = tkinter.font.Font(family='Helvetica', size=12, weight='bold')

# Functions to update LED brightness
def update_red(val):
    brightness = int(val) / 100
    led_red.value = brightness

def update_green(val):
    brightness = int(val) / 100
    led_green.value = brightness

def update_white(val):
    brightness = int(val) / 100
    led_white.value = brightness

# Sliders for each LED
red_slider = Scale(win, from_=0, to=100, orient=HORIZONTAL, 
                   label="Red LED Brightness", command=update_red, length=300)
red_slider.grid(row=0, column=0, padx=20, pady=20)

green_slider = Scale(win, from_=0, to=100, orient=HORIZONTAL, 
                     label="Green LED Brightness", command=update_green, length=300)
green_slider.grid(row=1, column=0, padx=20, pady=20)

white_slider = Scale(win, from_=0, to=100, orient=HORIZONTAL, 
                     label="White LED Brightness", command=update_white, length=300)
white_slider.grid(row=2, column=0, padx=20, pady=20)

# Exit button
exit_button = Button(win, text="Exit", font=myFont, command=win.destroy)
exit_button.grid(row=3, column=0, pady=20)

win.mainloop()
