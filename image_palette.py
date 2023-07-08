#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author : wangke
# @Mail :   wangkechn@163.com

import numpy as np

class ImagePalette:
    """ extract the palette color of an image.
        Attributes:
            img: numay array RGB img
            level: division accuracy
            mode: color sort style   # Support pixel_prior<recommend>  ch_prior
    """
    def __init__(self, level, mode="pixel_prior"):
            self.img = None
            self.level = level
            self.mode = mode
            self.rgb_groups = None
            
    def load_image(self, image):
        '''
        iamge: numpy array RGB image
        '''
        self.img = image

    def get_max_palette(self):
        '''
        return the dominant color values in the specified mode
        '''
        self.rgb_groups = self.__group_colors()
        max_palette = self.__search_max_rgb(self.mode)
        return max_palette

    def get_palette(self):
        '''
        return the palette value at the specified level
        '''
        self.rgb_groups = self.__group_colors()
        cube_palette_sort = self.__sort_rgb(self.mode)
        return cube_palette_sort

    def __group_colors(self):
        h, w, _ = self.img.shape
        r = self.img[:, :, 0]
        g = self.img[:, :, 1]
        b = self.img[:, :, 2]

        r_min = np.min(r)
        r_max = np.max(r)
        g_min = np.min(g)
        g_max = np.max(g)
        b_min = np.min(b)
        b_max = np.max(b)

        # such as R: color number (r_max - r_min + 1)

        '''
        # r_step = int((r_max - r_min + 1) / level)
        # g_step = int((g_max - g_min + 1) / level)
        # b_step = int((b_max - b_min + 1) / level)
        
        # case: max - min < level
        # if r_step == 0:
        #     r_step = level+1
        # if g_step == 0:
        #     g_step = level+1
        # if b_step == 0:
        #     b_step = level+1
        
        #
        # c style
        # rgb_groups = [[] for _ in range(level*level*level)]
        # for row in range(h): 
        #     for col in range(w):
        #         r = img[row][col][0]
        #         g = img[row][col][1]
        #         b = img[row][col][2]
        #         r_idx = int((r-r_min) / r_step) if int((r-r_min) / r_step)<level else level - 1
        #         g_idx = int((g-g_min) / g_step) if int((g-g_min) / g_step)<level else level - 1
        #         b_idx = int((b-b_min) / b_step) if int((b-b_min) / b_step)<level else level - 1
        #         grid_idx = r_idx*level*level+g_idx*level+b_idx
        #         image_idx = row*w+col
        #         rgb_groups[grid_idx].append(image_idx)
        # return rgb_groups
        
        #

        # r_idx = ((r-r_min) // r_step).astype(np.uint8)
        # g_idx = ((g-g_min) // g_step).astype(np.uint8)
        # b_idx = ((b-b_min) // b_step).astype(np.uint8)
        # r_idx[r_idx == level] = level - 1
        # g_idx[g_idx == level] = level - 1
        # b_idx[b_idx == level] = level - 1
        # rgb_groups = np.stack((r_idx,g_idx,b_idx),axis =2)

        '''

        r_step = (r_max - r_min + 1) / self.level
        g_step = (g_max - g_min + 1) / self.level
        b_step = (b_max - b_min + 1) / self.level
        
        r_idx = np.zeros(r.shape)
        g_idx = np.zeros(g.shape)
        b_idx = np.zeros(b.shape)

        for idx in range(self.level):
            r_mask_pre = idx*r_step <= r - r_min 
            r_mask_cur = r-r_min < (idx+1)*r_step
            r_mask = np.logical_and(r_mask_pre, r_mask_cur)
            r_idx[r_mask] = idx # level mask From 0 To level-1

            g_mask_pre = idx*g_step <= g - g_min 
            g_mask_cur = g-g_min < (idx+1)*g_step
            g_mask = np.logical_and(g_mask_pre, g_mask_cur)
            g_idx[g_mask] = idx

            b_mask_pre = idx*b_step <= b - b_min 
            b_mask_cur = b-b_min < (idx+1)*b_step
            b_mask = np.logical_and(b_mask_pre, b_mask_cur)
            b_idx[b_mask] = idx


        r_idx = r_idx.astype(np.uint8)
        g_idx = g_idx.astype(np.uint8)
        b_idx = b_idx.astype(np.uint8)
        rgb_groups = np.stack((r_idx, g_idx, b_idx), axis=2)
        return rgb_groups

    def __sort_rgb(self, mode="pixel_prior"):
        assert mode == "pixel_prior", "only support pixel mode"
        r = self.img[:, :, 0]
        g = self.img[:, :, 1]
        b = self.img[:, :, 2]

        r_idx = self.rgb_groups[:, :, 0]
        g_idx = self.rgb_groups[:, :, 1]
        b_idx = self.rgb_groups[:, :, 2]

        cube_palette = []

        # count the number of pixels in each color cube, sort the number of pixels and calculate the  mean in each cube
        for r_ in range(self.level):
            for g_ in range(self.level):
                for b_ in range(self.level):
                    r_mask = r_idx == r_
                    g_mask = g_idx == g_
                    b_mask = b_idx == b_
                    rgb_mask = np.logical_and(np.logical_and(r_mask, g_mask), b_mask)
                    #  calculates the mean and number of pixels for the selected pixels
                    pixel_num  = np.sum(rgb_mask)
                    if pixel_num == 0:
                        r_res, g_res, b_res = 0, 0, 0
                    else:
                        r_res = int(np.mean(r[rgb_mask]))
                        g_res = int(np.mean(g[rgb_mask]))
                        b_res = int(np.mean(b[rgb_mask]))

                    cube_palette.append([r_res,g_res,b_res,pixel_num])
        # sort by pixel number
        pix_nums = [elem[3] for elem in cube_palette]
        idx = np.argsort(pix_nums)[::-1]
        cube_palette_sort = np.asarray(cube_palette)[idx]

        return cube_palette_sort

    def __search_max_rgb(self, mode="pixel_prior"): # mode pixel_prior  ch_prior
        r = self.img[:, :, 0]
        g = self.img[:, :, 1]
        b = self.img[:, :, 2]

        r_idx = self.rgb_groups[:, :, 0]
        g_idx = self.rgb_groups[:, :, 1]
        b_idx = self.rgb_groups[:, :, 2]

        r_select = np.argmax(np.bincount(r_idx.ravel()))
        r_mask = r_idx == r_select

        g_num = g_idx[ r_mask]
        g_select = np.argmax(np.bincount(g_num))
        g_mask =  g_idx==g_select

        rg_mask = np.logical_and(g_mask,r_mask)
        b_num = b_idx[rg_mask]
        b_select = np.argmax(np.bincount(b_num))
        b_mask = b_idx == b_select
        rgb_mask = np.logical_and(np.logical_and(g_mask, r_mask), b_mask)

        # average all channel information within that range
        r_res = g_res = b_res = 0
        if mode == "ch_prior":
            r_res = int(np.mean(r[r_idx == r_select]))
            g_res = int(np.mean(g[g_idx == g_select]))
            b_res = int(np.mean(b[b_idx == b_select]))

        # average the RGB information of pixels that match the grouping
        elif mode =="pixel_prior":
            r_res = int(np.mean(r[rgb_mask]))
            g_res = int(np.mean(g[rgb_mask]))
            b_res = int(np.mean(b[rgb_mask]))

        return [r_res,g_res,b_res]


if __name__=="__main__":
    from PIL import Image

    img_path = "./images/test.jpg"
    image = Image.open(img_path)

    solution = ImagePalette(3) 
    solution.load_image(np.array(image))

    print(solution.get_max_palette())
    print(solution.get_palette())