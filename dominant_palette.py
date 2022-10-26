import numpy as np

class Dominant_Palette:
    """ extract the dominant color of an image
        Attributes:
            img: numay array RGB img
            level: division accuracy.
    """
    def __init__(self,img,level):
        self.img = img
        self.level = level

    def get_max_palette(self):
        self.rgb_groups = self.__group_colors(self.img,self.level)
        self.max_palette = self.__search_max_rgb(self.img,self.rgb_groups)
        return self.max_palette

    def __group_colors(self, img, level):
        h,w,c = img.shape
        r = img[:,:,0]
        g = img[:,:,1]
        b = img[:,:,2]

        r_min = np.min(r)
        r_max = np.max(r)
        g_min = np.min(g)
        g_max = np.max(g)
        b_min = np.min(b)
        b_max = np.max(b)

        r_step = int((r_max-r_min) / level)
        g_step = int((g_max - g_min) / level)
        b_step = int((b_max - b_min) / level)

        if r_step ==0:
            r_step = level+1
        if g_step ==0:
            g_step = level+1
        if b_step ==0:
            b_step = level+1

        '''  c style
        rgb_groups = [[] for _ in range(level*level*level)]
        for row in range(h): 
            for col in range(w):
                r = img[row][col][0]
                g = img[row][col][1]
                b = img[row][col][2]


                r_idx = int((r-r_min) / r_step) if int((r-r_min) / r_step)<level else level - 1
                g_idx = int((g-g_min) / g_step) if int((g-g_min) / g_step)<level else level - 1
                b_idx = int((b-b_min) / b_step) if int((b-b_min) / b_step)<level else level - 1

                grid_idx = r_idx*level*level+g_idx*level+b_idx
                image_idx = row*w+col
                rgb_groups[grid_idx].append(image_idx)
                

        return rgb_groups
        '''
        r_idx = ((r-r_min) // r_step).astype(np.uint8)
        g_idx = ((g-g_min) // g_step).astype(np.uint8)
        b_idx = ((b-b_min) // b_step).astype(np.uint8)
        r_idx[r_idx == level] = level - 1
        g_idx[g_idx == level] = level - 1
        b_idx[b_idx == level] = level - 1
        rgb_groups = np.stack((r_idx,g_idx,b_idx),axis =2)
        return rgb_groups

    def __search_max_rgb(self, img, rgb_groups,mode = "pixel_prior"): # mode pixel_prior  ch_prior
        h, w, c = img.shape
        r = img[:, :, 0]
        g = img[:, :, 1]
        b = img[:, :, 2]
        '''c style  pixel_prior
        max_idx = 0
        for idx in range(len(rgb_groups)):
            if len(rgb_groups[idx]) > len(rgb_groups[max_idx]):
                max_idx = idx

        sum_r = 0
        sum_g = 0
        sum_b = 0
        for item in rgb_groups[max_idx]:  
             y = item // w
             x = item % w
             r,g,b = img[y,x]

             sum_r += r
             sum_g += g
             sum_b += b
        pixel_num = len(rgb_groups[max_idx])
        max_rgb = [int(sum_r/pixel_num),int(sum_g/pixel_num),int(sum_b/pixel_num)]
        return max_rgb
        '''
        r_idx = rgb_groups[:, :, 0]
        g_idx = rgb_groups[:, :, 1]
        b_idx = rgb_groups[:, :, 2]

        r_select = np.argmax(np.bincount(r_idx.ravel()))
        r_mask = r_idx == r_select

        g_num = g_idx[ r_mask]
        g_select = np.argmax(np.bincount(g_num))
        g_mask =  g_idx==g_select

        rg_mask = np.logical_and(g_mask,r_mask)
        b_num = b_idx[rg_mask]
        b_select = np.argmax(np.bincount(b_num))
        b_mask = b_idx == b_select
        rgb_mask = np.logical_and(g_mask, r_mask, b_mask)

        # 对所有在该范围内的各通道信息进行平均
        r_res = g_res = b_res = 0
        if mode == "ch_prior":
            r_res = int(np.mean(r[r_idx == r_select]))
            g_res = int(np.mean(g[g_idx == g_select]))
            b_res = int(np.mean(b[b_idx == b_select]))

        # 对符合分组的像素点的rgb信息进行平均
        elif mode =="pixel_prior":
            r_res = int(np.mean(r[rgb_mask]))
            g_res = int(np.mean(g[rgb_mask]))
            b_res = int(np.mean(b[rgb_mask]))

        return [r_res,g_res,b_res]




if __name__=="__main__":
    import cv2
    import os
    img = cv2.imread("./imgs/diablo_lake_north_cascades_71713xz.jpg")
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    solition = Dominant_Palette(img,3)
    print(solition.get_max_palette())





