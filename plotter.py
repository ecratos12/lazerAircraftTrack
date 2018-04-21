import tkinter as tk
from tkinter import *

import matplotlib
matplotlib.use("TkAgg")

font = {'family' : 'monospace',
        'weight' : 'normal',
        'size'   : 10}

matplotlib.rc('font', **font)

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure
import matplotlib.pyplot as plt

import numpy as np
from math import radians

import csv
from decimal import Decimal
import time
from datetime import datetime

def r2el(r):
    return 90-r


def d2r(theta):
    return theta*np.pi/180


# def plot():
#     r = np.arange(0, 90, 0.5)
#     theta = r*9

#     fig = plt.figure(figsize=(10,10))
#     ax = fig.add_axes([0.1,0.1,0.8,0.8], polar=True)

#     #ax = plt.subplot(111, projection='polar')
#     ax.set_theta_zero_location('S')
#     ax.set_theta_direction(-1)
#     ax.set_rmax(90)
#     ax.set_rticks(np.arange(0, 91, 10))  # less radial ticks
#     ax.set_yticklabels(ax.get_yticks()[::-1])
#     ax.set_rlabel_position(0)
#     ax.grid(True)

#     sat_positions = [["sat1", 30, 0], ["sat2", 60, 90], ["sat3", 30, 180], ["sat4", 50, 270]]
#     for (PRN, E, Az) in sat_positions:
#         ax.annotate(str(PRN),
#                     xy=(radians(Az), r2el(E)),  # theta, radius
#                     bbox=dict(boxstyle="circle", fc = 'red', alpha = 0.3),
#                     horizontalalignment='center',
#                     verticalalignment='center')
#     ax.plot(d2r(theta), r2el(r))

#     plt.show()
#     plt.savefig('radar.png')


class mclass:
    def __init__(self,  window):
        self.window = window
        self.window.resizable(width=False, height=False)

        self.fig = Figure(figsize=(7.2,7.2))
        self.ax = self.fig.add_subplot(111, projection='polar')

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.window)
        self.canvas.get_tk_widget().pack(side=RIGHT, padx=0,pady=0)
        self.canvas.show()

        self.statusCanvas = Canvas(self.window)
        self.statusCanvas.pack(side=LEFT, padx=5, pady=5, fill=BOTH, expand=True)
        self.label = Label(self.statusCanvas, text="LAT = 50.3633 \n LON = 30.4961 \n alt = 211.8")
        self.label.pack(side=LEFT, padx=5, pady=5)

        self.plot()

    def plot(self):
        self.window.title("Sky Safety Manager >>>>>> UTC: " + datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"))
        
        with open("current_status", 'r') as status_file:
            status = status_file.readline()
        self.statusLable = self.statusCanvas.create_rectangle(0,0,120,300, 
                                                fill = 'red' if status=="Danger" else "green")


        self.ax.clear()
        self.ax.set_theta_zero_location('S')
        self.ax.set_theta_direction(-1)
        self.ax.set_rmax(90)
        self.ax.set_rticks(np.arange(0, 91, 10))  # less radial ticks
        self.ax.set_yticklabels(self.ax.get_yticks()[::-1])
        self.ax.set_rlabel_position(0)
        self.ax.grid(True)

        r = np.arange(0, 90, 1)
        theta = r

        positions = []
        with open("current_positions.csv") as csv_file:
            read = csv.reader(csv_file, delimiter=',')
            for row in read:
                row[2] = Decimal(row[2])
                row[3] = Decimal(row[3])
                positions.append(row[0:])
        # sat_positions = [["sat1", 30, 0], ["sat2", 60, 90], ["sat3", 30, 180], ["sat4", 50, 270]]
        for (type, PRN, Az, E) in positions:
            if type == "SAT":
                self.ax.annotate(str(PRN),
                            xy=(radians(Az), r2el(E)),  # theta, radius
                            bbox=dict(boxstyle="circle", fc = 'red', alpha = 0.3, pad = 1.25),
                            horizontalalignment='center',
                            verticalalignment='center')
            else:
                self.ax.annotate(str(hex(int(PRN)))[2:], xy=(radians(Az), r2el(E)))
            self.ax.plot(radians(Az), r2el(E), '.')

        self.ax.plot(d2r(theta), r2el(r), linewidth=0)
        self.fig.tight_layout()
        self.canvas.draw()

        window.after(1200, self.plot)

if __name__ == '__main__':
    window = Tk()
    start = mclass(window)
    window.mainloop()