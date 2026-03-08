from PIL import Image

# Zkopírujte sem obsah pole font_glyphs z vašeho font_basic.h
font_glyphs = [ [0]*8 for _ in range(128) ]
font_glyphs[ord('A')] = [0b00111100, 0b01100110, 0b01100110, 0b01111110, 0b01100110, 0b01100110, 0b01100110, 0b00000000]
font_glyphs[ord('B')] = [0b01111100, 0b01100110, 0b01100110, 0b01111100, 0b01100110, 0b01100110, 0b01111100, 0b00000000]
# ... (doplňte všechny znaky z vaší tabulky) ...

glyph_width = 8
glyph_height = 8

# Najdi všechny znaky, které mají nějaký obsah (první řádek není 0)
used_glyphs = [i for i in range(128) if any(font_glyphs[i])]

img_width = glyph_width * len(used_glyphs)
img_height = glyph_height

img = Image.new('1', (img_width, img_height), color=1)  # 1 = černobílý, 1 = bílá

for idx, glyph_idx in enumerate(used_glyphs):
    for y in range(glyph_height):
        row = font_glyphs[glyph_idx][y]
        for x in range(glyph_width):
            if row & (1 << (7 - x)):
                img.putpixel((idx * glyph_width + x, y), 0)  # 0 = černá

img.save('font_basic.bmp')
print('font_basic.bmp uložen.')