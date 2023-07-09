#!/usr/bin/env python
# -*- coding: utf-8 -*-

# @Author : wangke
# @Mail :   wangkechn@163.com

import numpy as np

def rgb_to_hsl(rgb):
    r, g, b = rgb / 255.0  

    cmax = np.max(rgb) / 255.0
    cmin = np.min(rgb) / 255.0
    delta = cmax - cmin

    l = (cmax + cmin) / 2.0

    if delta == 0:
        h = 0
        s = 0
    else:
        if l <= 0.5:
            s = delta / (cmax + cmin)
        else:
            s = delta / (2 - cmax - cmin)

        if cmax == r:
            h = (g - b) / delta
        elif cmax == g:
            h = 2 + (b - r) / delta
        else:
            h = 4 + (r - g) / delta

        h = (h / 6.0) % 1

    return [h, s, l]

class Mode:
    ''' 
    Python implements Android's Palette different mode image function
    
    '''
    __mode_name = ["lightVibrant", "vibrant", "darkVibrant", "lightMuted", "muted", "darkMuted"]
    __mode_lium = [[0.3, 0.5, 0.7], [0.55, 0.74, 1.0], [0.0, 0.26, 0.44],
                 [0.3, 0.5, 0.7], [0.55, 0.74, 1.0], [0.0, 0.26, 0.44]]
    
    __mode_satu = [[0.35, 1.0, 1.0], [0.35, 1.0, 1.0], [0.35, 1.0, 1.0], 
                   [0.0, 0.3, 0.4],  [0.0, 0.3, 0.4], [0.0, 0.3, 0.4]]
    
    __mode_weight = [[0.24, 0.52, 0.24]] * 6
    def __init__(self,mode):
        self.mode = mode
        self.palette = None
        
    def load_palette(self, palette):
        self.palette = palette


    def get_best(self):
        name = Mode.__mode_name[self.mode]
        lium = Mode.__mode_lium[self.mode]
        weight = Mode.__mode_weight[self.mode]
        satu = Mode.__mode_satu[self.mode]

        palette_hsl = []
        for elem in self.palette:
            rgb = np.array([elem[0], elem[1], elem[2]])
            hsl = rgb_to_hsl(rgb)
            palette_hsl.append(hsl)
        max_pix = self.palette[-1][-1]

        best_score = -1
        best_score_idx = -1

        for idx in range(len(palette_hsl)):
            hsl = palette_hsl[idx]
            num_pix = self.palette[idx][-1] / max_pix
            s, l = hsl[1], hsl[2]

            if not (s >= satu[0] and s<= satu[2]  and l >= lium[0] and l<=lium[2]):
                continue
            saturation_score = weight[0] * (1.0 - np.abs(s- satu[1]))
            luminance_score = weight[1] * (1.0 - np.abs(l- lium[1]))
            population_score = weight[2] * num_pix
            total_score = saturation_score + luminance_score + population_score
            if total_score > best_score:
                best_score = total_score
                best_score_idx = idx
        return best_score_idx


    def get_best_rgb(self):
        ret_idx = self.get_best()
        # If the mode requirements are not met, returns the dominant color # ascending order
        #if ret_idx == -1:
        #    return (self.palette[-1][0], self.palette[-1][1], self.palette[-1][2])
        return (self.palette[ret_idx][0], self.palette[ret_idx][1], self.palette[ret_idx][2])


if __name__=="__main__":
    rgb = np.array([128, 64, 192])
    hsl = rgb_to_hsl(rgb)
    print(hsl)





