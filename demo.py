from image_palette import ImagePalette
from color_mode import Mode

import numpy as np
from PIL import Image

if __name__=="__main__":

    img_path = "./images/test.jpg"
    image = Image.open(img_path)

    solution = ImagePalette(3) 
    solution.load_image(np.array(image))
    # print(solution.get_max_palette())
    # print(solution.get_palette())

    palette = solution.get_palette()
    mode = Mode(4)
    mode.load_palette(palette)
    mode4 = mode.get_best_rgb()
    print(mode4)


    
