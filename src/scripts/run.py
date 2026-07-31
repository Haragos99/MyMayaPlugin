import sys
import importlib

DEV_SRC = r"C:/Users/Geri/Documents/Projects/CG/MyMayaPlugin/src"

if DEV_SRC not in sys.path:
    sys.path.append(DEV_SRC)

import scripts.analayze.log_parser
import scripts.analayze.charts_plotter
import scripts.analayze.main_window
import scripts.analayze.statistics

importlib.reload(scripts.analayze.log_parser)
importlib.reload(scripts.analayze.charts_plotter)
importlib.reload(scripts.analayze.main_window)
importlib.reload(scripts.analayze.statistics)

scripts.analayze.main_window.LogVisualizerWindow.show_window()