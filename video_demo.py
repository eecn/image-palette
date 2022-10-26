import datetime
import sys
import cv2
import numpy as np
from dominant_palette import Dominant_Palette
sys.path.insert(0, "..")


def split_img_region(img,led_w,led_h):

    size = img.shape
    h = size[0]
    w = size[1]
    # 先不做图像缩放
    # 对图像进行区域拆分 这里直接向下取整 不做四舍五入 否则 + led_w/2

    w_stride = w//led_w
    h_stride = h//led_h

    sub_w_stride = int(w * 0.2)
    sub_h_stride = int(h * 0.2)

    # 左上角开始 顺时针排序

    imgs = []
    # 上侧 从左到右
    start_w = 0
    while (start_w+w_stride<=w):
        sub_img = img[0:sub_h_stride,start_w:start_w+w_stride]
        imgs.append(sub_img)
        start_w += w_stride

    start_h=0
    while(start_h+h_stride<=h):
        sub_img = img[start_h:start_h+h_stride, w-sub_h_stride:w]
        imgs.append(sub_img)
        start_h += h_stride

    start_w=w
    while(start_w-w_stride>=0):
        sub_img = img[h-sub_h_stride:h, start_w-w_stride:start_w]
        imgs.append(sub_img)
        start_w -= w_stride

    start_h=h
    while(start_h-h_stride>=0):
        sub_img = img[start_h-h_stride:start_h, 0:sub_w_stride]
        imgs.append(sub_img)
        start_h -= h_stride

    return imgs



if __name__ == "__main__":
    video_path = r"./imgs/video.mp4"
    cap = cv2.VideoCapture(video_path)

    w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = cap.get(cv2.CAP_PROP_FPS)

    led_h = 9
    led_w = 16
    img_margin = 0.2  # 默认  根据实际测试调整 Maybe更智能的语义分割进行区域分割



    w_stride = int(w // led_w)
    h_stride = int(h // led_h)
    sub_w_stride = int(w * img_margin)
    sub_h_stride = int(h * img_margin)

    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter('video_res.mp4', fourcc, fps, (int(w*(1+2*img_margin)), int(h * (1+2*img_margin))), True)
    average_time = []
    while cap.isOpened():
        ret, frame = cap.read()  # 先不做图像缩放
        if not ret:
            break
        # frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        start = datetime.datetime.now()
        # 对图像进行区域拆分 这里直接向下取整 不做四舍五入 否则 + led_w/2
        sub_imgs = split_img_region(frame,led_w,led_h)
        palettes = []
        for img in sub_imgs:
            solution = Dominant_Palette(img, 3)
            # cv2.imwrite("img___.png",img)
            palette = solution.get_max_palette()  # 返回的是RGB 转为BGR
            # palette = [palette[2], palette[1], palette[0]]
            palettes.append(palette)
        end = datetime.datetime.now()
        time_epoch = (end - start).microseconds
        average_time.append(time_epoch)


        up_imgs = []
        right_imgs = []
        down_imgs=[]
        left_imgs=[]
        for idx, palette in enumerate(palettes):
            if idx<led_w:
                img = np.zeros((sub_h_stride, w_stride, 3), np.uint8)
                img_rgb = img.copy()
                img_rgb[:, :, :] = [palette[0], palette[1], palette[2]]
                up_imgs.append(img_rgb)
            elif idx<led_w+led_h:
                img = np.zeros((h_stride, sub_w_stride, 3), np.uint8)
                img_rgb = img.copy()
                img_rgb[:, :, :] = [palette[0], palette[1], palette[2]]
                right_imgs.append(img_rgb)
            elif idx<led_w+led_w+led_h:
                img = np.zeros((sub_h_stride, w_stride, 3), np.uint8)
                img_rgb = img.copy()
                img_rgb[:, :, :] = [palette[0], palette[1], palette[2]]
                down_imgs.append(img_rgb)
            else:
                img = np.zeros((h_stride, sub_w_stride, 3), np.uint8)
                img_rgb = img.copy()
                img_rgb[:, :, :] = [palette[0], palette[1], palette[2]]
                left_imgs.append(img_rgb)



        # hstack 上边缘顺序
        base_img_up = up_imgs[0]
        for i in range(1,len(up_imgs)):
            base_img_up  = np.hstack((base_img_up,up_imgs[i]))
        # 上边缘
        base_img_up.resize(sub_h_stride,w,3)
        show_img = np.vstack((base_img_up,frame))

        # hstack 下边缘反序
        down_imgs.reverse()
        base_img_down = down_imgs[0]
        for i in range(1,len(down_imgs)):
            base_img_down = np.hstack((base_img_down, down_imgs[i]))
        # 下边缘
        base_img_down.resize(sub_h_stride, w, 3)
        show_img = np.vstack((show_img,base_img_down))

        new_h = show_img.shape[0]

        # 填边图像
        matgin_h = int((new_h-h)//2)
        margin_img = np.zeros((sub_h_stride, sub_w_stride, 3), np.uint8)

        # 左右两侧
        left_imgs.reverse()
        base_img_left = margin_img
        for i in range(0, len(left_imgs)):
            base_img_left = np.vstack((base_img_left, left_imgs[i]))
        # 左边缘
        base_img_left = np.vstack((base_img_left,margin_img))
        base_img_left.resize(new_h, sub_w_stride, 3,refcheck=False)
        show_img = np.hstack((base_img_left, show_img))

        base_img_right = margin_img
        for i in range(0, len(right_imgs)):
            base_img_right = np.vstack((base_img_right, right_imgs[i]))
        # 右边缘
        base_img_right = np.vstack((base_img_right, margin_img))
        base_img_right.resize(new_h, sub_w_stride, 3,refcheck=False)
        show_img = np.hstack((show_img, base_img_right))
        #cv2.imwrite("res.png",show_img)

        out.write(show_img)
    cap.release()
    if len(average_time):
        print("Average time process per frame:",end=" ")
        print(sum(average_time)/len(average_time))
