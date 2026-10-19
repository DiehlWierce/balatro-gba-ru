import sys
from PIL import Image
import numpy as np
from scipy import ndimage
def boxes(path,y0,y1,x0,x1,white=None,minpal=240,gap=1):
    im=Image.open(path); pal=im.getpalette(); px=np.array(im)
    wmask=np.array([min(pal[i*3:i*3+3])>=minpal for i in range(len(pal)//3)]+[False]*(256-len(pal)//3))
    m=wmask[px[y0:y1,x0:x1]] if white is None else (px[y0:y1,x0:x1]==white)
    m2=ndimage.binary_dilation(m,iterations=gap)
    lab,k=ndimage.label(m2)
    out=[]
    for i,sl in enumerate(ndimage.find_objects(lab)):
        sub=m[sl]; 
        ys,xs=np.nonzero(sub)
        if len(ys)==0: continue
        out.append((y0+sl[0].start+ys.min(),x0+sl[1].start+xs.min(),x0+sl[1].start+xs.max()+1,y0+sl[0].start+ys.max()+1))
    return out
if __name__=='__main__':
    p=sys.argv[1]; a=list(map(int,sys.argv[2:6])); g=int(sys.argv[6]) if len(sys.argv)>6 else 1
    for b in boxes(p,*a,gap=g): print(b)
