import sys
import importlib

DEV_SRC = r"C:/Users/Geri/Documents/Projects/CG/MyMayaPlugin/src/scripts/"

if DEV_SRC not in sys.path:
    sys.path.append(DEV_SRC)

import analayze.log_parser
import analayze.charts_plotter
import analayze.main_window

importlib.reload(analayze.log_parser)
importlib.reload(analayze.charts_plotter)
importlib.reload(analayze.main_window)

analayze.main_window.LogVisualizerWindow.show_window()