import sys, shutil, os
sys.path.insert(0,'/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/ru')
from PIL import Image
from relabel import *
from tinyfont import F5, F4, F5N, F8
G='/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/bg23/graphics/'
ORIG='/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/ru/o23/'
def load(name): return Image.open(ORIG+name)
def save(img,name): img.save(G+name, optimize=False)

def score_label(im):
    px=im.load(); pal=im.getpalette()
    # индексы берём из оригинала: белый, тень "#d", фон бара
    white=px[7,48]; shadow=px[8,49]; bg=px[10,47]
    assert min(pal[white*3:white*3+3])>=240, white
    for y in range(47,58):
        for x in range(7,25):
            if px[x,y] in (white,shadow): px[x,y]=bg
    rows,w=text_mask('ОЧКИ',F8,gap=1)
    x0=7
    for r,line in enumerate(rows):
        for c,ch in enumerate(line):
            if ch=='#': px[x0+c,48+r]=white
    for r,line in enumerate(rows):
        for c,ch in enumerate(line):
            if ch=='#' and (r+1>=len(rows) or rows[r+1][c]!='#') and px[x0+c,48+r+1]==bg: px[x0+c,48+r+1]=shadow

def panel(img):
    score_label(img)
    for (y0,x0,x1,y1),t in [((98,12,27,102),'РУКИ'),((98,40,63,102),'СБРОС'),((138,13,28,142),'АНТЕ'),((138,42,61,142),'РАУНД')]:
        relabel(img,(y0,x0,x1,y1),t,F4)

# ================= background_gfx =================
im=load('background_gfx.png'); panel(im)
relabel(im,(130,93,108,135),'ИГРАТЬ',F5)
relabel(im,(137,93,108,142),'РУКУ',F5)
relabel(im,(133,168,191,138),'СБРОС',F5)
relabel(im,(169,19,54,174),'МАЛЫЙ БЛАЙНД',F5)
relabel(im,(179,34,62,183),'ЦЕЛЬ',F4,align='right')
relabel(im,(209,26,54,214),'БИГ БЛАЙНД',F5)
relabel(im,(217,22,54,222),'БОСС БЛАЙНД',F5)
relabel(im,(234,2,30,238),'ЦЕЛЬ',F4,align='left')
relabel(im,(244,8,32,249),'ЗАНОВО',F5)
F5b=dict(F5); F5b['М']=['#..#','####','####','#..#','#..#']
relabel(im,(139,122,137,144),'РАНГ',F5)
relabel(im,(139,145,157,144),'МАСТ',F5b)
save(im,'background_gfx.png')

# ================= background_blind_select_gfx =================
im=load('background_blind_select_gfx.png'); panel(im)
names=[('МАЛЫЙ','БЛАЙНД'),('БИГ','БЛАЙНД'),('БОСС','БЛАЙНД')]
l1=[(170,84,100,175),(170,127,136,175),(170,165,178,175)]
l2=[(177,84,100,182),(177,124,140,182),(177,164,180,182)]
sel=[(162,83,101,167),(162,123,141,167),(162,163,181,167)]
sc =[(210,78,106,214),(210,118,146,214),(210,158,186,214)]
for p in range(3):
    relabel(im,sel[p],'ВЫБОР',F5,shadow=None,bg=15)
    relabel(im,sc[p],'ЦЕЛЬ',F4)
    i1=relabel(im,l1[p],'',F5N,defer=True,shadow=None)
    i2=relabel(im,l2[p],'',F5N,defer=True,shadow=None)
    cx=91+40*p
    #    widen(im,168,184,81+40*p,102+40*p,cx,3,3,6)
    c=91.5+40*p
    draw_text(im,i1,names[p][0],F5N,cx=c,shadow=False)
    draw_text(im,i2,names[p][1],F5N,cx=c,shadow=False)
relabel(im,(241,92,104,246),'ПАС',F5)
relabel(im,(241,132,144,246),'ПАС',F5)
# серые метки состояния (NEXT / SKIP / DEFEATED) и серые SKIP
gray=lambda p: p==20
relabel(im,(162,5,19,167),'ДАЛЕЕ',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(162,30,43,167),'ПАС',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(162,50,70,167),'ВЗЯТ',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(201,19,33,206),'ПАС',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(233,20,32,238),'ПАС',F5,white_fn=gray,bg=24,shadow=None)
save(im,'background_blind_select_gfx.png')

# ================= background_shop_gfx =================
im=load('background_shop_gfx.png'); panel(im)
relabel(im,(168,89,104,172),'ДАЛЕЕ',F4)
relabel(im,(174,87,106,178),'РАУНД',F4)
relabel(im,(187,86,109,191),'РЕРОЛЛ',F4)
save(im,'background_shop_gfx.png')

# ================= main menu =================
im=load('background_main_menu_gfx.png')
relabel(im,(129,96,110,134),'ИГРАТЬ',F5)
relabel(im,(129,125,149,134),'ОПЦИИ',F5)
save(im,'background_main_menu_gfx.png')
print('done')


# ================= вывеска SHOP -> ЛАВКА =================
BIG = {
'Л': ['..#####','..#####','..##.##','..##.##','..##.##','.##..##','.##..##','.##..##','##...##','##...##','##...##','##...##','##...##','##...##'],
'А': ['..###..','..###..','.##.##.','.##.##.','.##.##.','##...##','##...##','#######','#######','##...##','##...##','##...##','##...##','##...##'],
'В': ['#####.','#####.','##..##','##..##','##..##','##..##','#####.','#####.','##..##','##..##','##..##','##..##','#####.','#####.'],
'К': ['##...##','##...##','##..##.','##..##.','##.##..','##.##..','#####..','#####..','##.##..','##.##..','##..##.','##..##.','##...##','##...##'],
}
def shop_sign(word):
    im=Image.open(G+'background_shop_gfx.png'); px=im.load()
    for y in range(175,193):
        for x in range(15,58): px[x,y]=23
    gl=[BIG[c] for c in word]
    gap=2
    W=sum(len(g[0]) for g in gl)+gap*(len(gl)-1)
    x0=int(round((15+58)/2 - W/2))
    y0=177
    mask=set(); cx=x0
    for g in gl:
        for r,line in enumerate(g):
            for c,ch in enumerate(line):
                if ch=='#': mask.add((cx+c,y0+r))
        cx+=len(g[0])+gap
    for (x,y) in mask: px[x,y]=18
    for (x,y) in mask:
        if (x,y-1) not in mask or (x-1,y) not in mask: px[x,y]=12
    for (x,y) in mask:
        if (x,y+1) not in mask or (x+1,y) not in mask:
            px[x,y]= 11 if px[x,y]==18 else 15
    for (x,y) in mask:
        for dx,dy in ((1,1),(0,1),(1,0)):
            q=(x+dx,y+dy)
            if q not in mask and px[q]==23: px[q]=20
    im.save(G+'background_shop_gfx.png')
shop_sign('ЛАВКА')

# ================= токены малого/большого блайнда (blind_gfx0: 64x32 P) =================
def chips23():
    im = Image.open(ORIG+'blind_gfx0.png'); px = im.load()
    W,H = im.size
    import numpy as np
    arr = np.array(im)
    for half,(l1,l2,textidx,body) in enumerate((('МАЛ','БЛ.',1,4),('БИГ','БЛ.',9,12))):
        x0h=half*32
        # границы фишки: непрозрачные пиксели в этой половине
        xs=[x for x in range(x0h,x0h+32) for y in range(32) if px[x,y]!=0]
        cx=(min(xs)+max(xs))/2
        for y in range(5,22):
            for x in range(x0h+3,x0h+29):
                if px[x,y]==textidx: px[x,y]=body
        for txt,y0 in ((l1,6),(l2,13)):
            rows,w=text_mask(txt,F5N)
            tx=int(round(cx-w/2+0.5))
            for r,line in enumerate(rows):
                for c,ch in enumerate(line):
                    if ch=='#': px[tx+c,y0+r]=textidx
    im.save(G+'blind_gfx0.png')
chips23()
print('chips ok')

# ================= общие помощники: шрифт 8x8 игры и «большие» буквы =================
FONT8 = Image.open('/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/bg23/font/gbalatro_sys8.png').convert('L')
def glyph8(ch):
    idx = ch.encode('cp1251')[0] - 32
    r, c = divmod(idx, 16)
    return [[FONT8.getpixel((c*8+x, r*8+y)) < 100 for x in range(8)] for y in range(8)]
def draw8(im, text, x, y, ink):
    px = im.load()
    for i, ch in enumerate(text):
        g = glyph8(ch)
        for yy in range(8):
            for xx in range(8):
                if g[yy][xx]: px[x + i*8 + xx, y + yy] = ink
def erase_color(im, box, idx, bg):
    px = im.load(); x0, y0, x1, y1 = box
    for y in range(y0, y1):
        for x in range(x0, x1):
            if px[x, y] == idx: px[x, y] = bg
def big_mask(txt, gap=1):
    cw = {3: [2,1,2], 4: [2,1,1,2], 5: [2,1,1,1,2]}
    rh = [3,2,2,2,3]
    cols = []
    for ch in txt:
        gl = F5[ch]; n = len(gl[0]); w = cw[n]
        rows = []
        for r, line in enumerate(gl):
            row = ''.join(('#' if line[c]=='#' else '.') * w[c] for c in range(n))
            rows += [row] * rh[r]
        cols.append(rows)
    W = sum(len(c[0]) for c in cols) + gap*(len(cols)-1)
    out = [''] * 12
    for i, c in enumerate(cols):
        for r in range(12): out[r] += c[r] + ('.' * gap if i < len(cols)-1 else '')
    return out, W
def draw_big(im, mask, x, y, ink, shadow):
    px = im.load(); H = len(mask)
    for r, line in enumerate(mask):
        for c, ch in enumerate(line):
            if ch == '#': px[x+c, y+r] = ink
    for r, line in enumerate(mask):
        for c, ch in enumerate(line):
            if ch == '#' and (r+1 >= H or mask[r+1][c] != '#') and px[x+c, y+r+1] != ink: px[x+c, y+r+1] = shadow

# ================= background_options_menu_gfx =================
im = load('background_options_menu_gfx.png'); px = im.load()
bgi = px[47, 63]
erase_color(im, (44, 56, 172, 72), 6 if False else px[60, 62] if False else 1, bgi) if False else None
pal = im.getpalette()
def white_p(p): return min(pal[p*3:p*3+3]) >= 240
for y in range(56, 72):
    for x in range(44, 172):
        if white_p(px[x, y]): px[x, y] = bgi
inkp = [p for p in range(len(pal)//3) if white_p(p)][0]
# индекс «белого» берём из оригинального текста
from collections import Counter
inkp = Counter(Image.open(ORIG+'background_options_menu_gfx.png').getpixel((x,y)) for y in range(60,67) for x in range(49,110) if white_p(Image.open(ORIG+'background_options_menu_gfx.png').getpixel((x,y)))).most_common(1)[0][0]
draw8(im, 'Контраст', 48, 60, inkp)
draw8(im, 'Круп', 136, 60, inkp)
save(im, 'background_options_menu_gfx.png')

# ================= background_run_setup_gfx =================
im = load('background_run_setup_gfx.png')
relabel(im, (182,122,142,187), 'ЗАГРУЗ', F5)
relabel(im, (182,163,182,187), 'ЗАГРУЗ', F5, white_fn=lambda p: p==7, bg=11, shadow=None)
relabel(im, (242,93,115,248), 'СЛУЧ.', F5)
save(im, 'background_run_setup_gfx.png')

# ================= background_game_over_gfx =================
im = load('background_game_over_gfx.png'); px = im.load()
# подписи на серых плашках
for box, txt, ox in (((36,190,104,202),'Рекорд',47), ((36,206,108,218),'Частая',47), ((56,222,92,234),'Сид',62)):
    erase_color(im, box[:2]+(box[2],box[3]), 1, 2)
y_best, y_most, y_seed = 192, 208, 224
draw8(im, 'Рекорд', 47, y_best, 1)
draw8(im, 'Частая', 47, y_most, 1)
draw8(im, 'Сид', 62, y_seed, 1)
# метки DEFEATED: панель (победа) и тайлы (поражение)
def two_lines(cx, y1, y2, a, b):
    for y, t in ((y1,a),(y2,b)):
        rows, w = text_mask(t, F5)
        x0 = int(round(cx - w/2))
        for r, line in enumerate(rows):
            for c, ch in enumerate(line):
                if ch == '#': px[x0+c, y+r] = 1
erase_color(im, (176,190,208,204), 1, 2)
two_lines(191.5, 192, 198, 'ФИНАЛ', 'ПРОЙДЕН')
erase_color(im, (224,240,256,254), 1, 2)
two_lines(239.5, 241, 247, 'ВАС', 'ПОБЕДИЛ')
# YOU WIN -> ПОБЕДА (синий, индекс 7, тень 5, фон 4)
for y in range(166, 184):
    for x in range(86, 154):
        if px[x, y] in (7, 5): px[x, y] = 4
mk, w = big_mask('ПОБЕДА')
draw_big(im, mk, int(round(119.5 - w/2)), 169, 7, 5)
# GAME / OVER -> ПОРАЖ / ЕНИЕ (красный 6, тень 5, фон 4)
for (sx, sy) in ((216,192),(216,208)):
    for y in range(sy, sy+16):
        for x in range(sx, sx+40):
            if px[x, y] in (6, 5): px[x, y] = 4
mk1, w1 = big_mask('ПОРАЖ'); mk2, w2 = big_mask('ЕНИЕ')
draw_big(im, mk1, 216 + 40 - w1, 193, 6, 5)
draw_big(im, mk2, 216 + 1, 209, 6, 5)
save(im, 'background_game_over_gfx.png')
print('go/opts/rs ok')
