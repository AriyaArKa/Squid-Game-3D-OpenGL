"""Builds the report's figures from the game's own screenshots.

    build/bin/squid_game_opengl.exe --capture docs/report/captures
    python tools/make_report_figures.py                (writes docs/report/source/figures/)

Everything shown in the report therefore comes straight out of the real program.
"""
import os
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SRC = os.path.join(ROOT, "docs", "report", "captures")
OUT = os.path.join(ROOT, "docs", "report", "source", "figures")

BG = (244, 245, 247)
INK = (34, 38, 46)
MUTED = (110, 116, 128)

# Same colors as GLCompat::shapeColor()
SHAPES = [
    ("Box (cube)", "Box", (0.22, 0.50, 0.95)),
    ("Sphere", "Sphere", (0.98, 0.55, 0.12)),
    ("Cylinder", "Cylinder", (0.18, 0.74, 0.36)),
    ("Cone", "Cone", (0.96, 0.82, 0.10)),
    ("Disk / ring", "Disk", (0.62, 0.34, 0.90)),
    ("Torus", "Torus", (0.92, 0.22, 0.40)),
    ("Custom polygon", "Polygon", (0.10, 0.74, 0.80)),
    ("Line loop", "Line", (0.20, 0.20, 0.22)),
]


def font(size, bold=False):
    names = ["seguisb.ttf" if bold else "segoeui.ttf", "arial.ttf"]
    for n in names:
        try:
            return ImageFont.truetype(os.path.join("C:/Windows/Fonts", n), size)
        except OSError:
            pass
    return ImageFont.load_default()


def load(name):
    return Image.open(os.path.join(SRC, name + ".bmp")).convert("RGB")


def save(img, name, quality=90):
    path = os.path.join(OUT, name)
    if name.endswith(".jpg"):
        img.save(path, quality=quality, optimize=True)
    else:
        img.save(path, optimize=True)
    print("wrote", name, img.size)


def read_counts(name):
    counts = {}
    path = os.path.join(SRC, name + ".txt")
    if os.path.exists(path):
        for line in open(path, encoding="utf-8"):
            if "=" in line:
                k, v = line.strip().split("=")
                counts[k] = int(v)
    return counts


def rgb255(c):
    return tuple(int(round(x * 255)) for x in c)


def legend_strip(counts, width, height=64):
    """Colored chips: one per shape kind used, with how many parts of that kind."""
    strip = Image.new("RGB", (width, height), BG)
    d = ImageDraw.Draw(strip)
    f = font(26)
    items = [(short, col, counts[key]) for key, short, col in SHAPES if counts.get(key)]
    total = sum(n for _, _, n in items)
    texts = [f"{short} \u00d7 {n}" for short, _, n in items] + [f"Total parts: {total}"]
    widths = [d.textlength(t, font=f) + (44 if i < len(items) else 0) for i, t in enumerate(texts)]
    gap = 34
    x = max(10, (width - (sum(widths) + gap * (len(texts) - 1))) / 2)
    y = height / 2
    for i, t in enumerate(texts):
        if i < len(items):
            d.rounded_rectangle([x, y - 14, x + 28, y + 14], radius=6, fill=rgb255(items[i][1]))
            d.text((x + 40, y), t, fill=INK, font=f, anchor="lm")
        else:
            d.text((x, y), t, fill=MUTED, font=font(26, True), anchor="lm")
        x += widths[i] + gap
    return strip


def trim(img, pad=24):
    """Crop the empty background around a showcase render (keeps a small margin)."""
    bg = Image.new("RGB", img.size, BG)
    from PIL import ImageChops
    diff = ImageChops.difference(img, bg).convert("L").point(lambda v: 255 if v > 10 else 0)
    box = diff.getbbox()
    if not box:
        return img
    l, t, r, b = box
    return img.crop((max(0, l - pad), max(0, t - pad), min(img.width, r + pad), min(img.height, b + pad)))


def fit(img, w, h):
    """Scale img to fit inside w x h and center it on the background color."""
    s = min(w / img.width, h / img.height)
    im = img.resize((max(1, int(img.width * s)), max(1, int(img.height * s))), Image.LANCZOS)
    canvas = Image.new("RGB", (w, h), BG)
    canvas.paste(im, ((w - im.width) // 2, (h - im.height) // 2))
    return canvas


def decomposition(item, out_name, cell=820):
    """[full model] [model split into basic shapes] + legend."""
    full = trim(load(f"obj_{item}_full"))
    parts = trim(load(f"obj_{item}_parts"))
    counts = read_counts(f"obj_{item}_parts")
    title_h, legend_h = 58, 64
    # wide objects (clock, bridge) get shorter cells instead of empty space
    need = max(cell * full.height / full.width, cell * parts.height / parts.width)
    cell_h = int(min(cell, max(cell * 0.5, need)))
    W = cell * 2 + 30
    img = Image.new("RGB", (W, title_h + cell_h + legend_h), BG)
    d = ImageDraw.Draw(img)
    f = font(30, True)
    d.text((cell / 2, title_h / 2), "Full model", fill=INK, font=f, anchor="mm")
    d.text((cell + 30 + cell / 2, title_h / 2), "Made of basic shapes (pulled apart)", fill=INK, font=f, anchor="mm")
    img.paste(fit(full, cell, cell_h), (0, title_h))
    img.paste(fit(parts, cell, cell_h), (cell + 30, title_h))
    d.line([(cell + 15, title_h + 20), (cell + 15, title_h + cell_h - 20)], fill=(205, 208, 214), width=3)
    img.paste(legend_strip(counts, W, legend_h), (0, title_h + cell_h))
    save(img, out_name)
    return counts


def scene_pair(left, right, out_name, labels=("Normal view", "Shape view (I key)")):
    a, b = load(left), load(right)
    w, h = 1100, int(1100 * a.height / a.width)
    title_h = 56
    img = Image.new("RGB", (w * 2 + 24, h + title_h), BG)
    d = ImageDraw.Draw(img)
    f = font(30, True)
    for i, (im, lab) in enumerate(((a, labels[0]), (b, labels[1]))):
        x = i * (w + 24)
        img.paste(im.resize((w, h), Image.LANCZOS), (x, title_h))
        d.text((x + w / 2, title_h / 2), lab, fill=INK, font=f, anchor="mm")
    save(img, out_name, quality=88)


def grid(names, labels, cols, out_name, cell_w=800):
    ims = [load(n) for n in names]
    cell_h = int(cell_w * ims[0].height / ims[0].width)
    label_h = 48
    rows = (len(ims) + cols - 1) // cols
    img = Image.new("RGB", (cols * cell_w + (cols - 1) * 16, rows * (cell_h + label_h) + (rows - 1) * 12), BG)
    d = ImageDraw.Draw(img)
    f = font(28, True)
    for i, (im, lab) in enumerate(zip(ims, labels)):
        r, c = divmod(i, cols)
        x = c * (cell_w + 16)
        y = r * (cell_h + label_h + 12)
        d.text((x + cell_w / 2, y + label_h / 2), lab, fill=INK, font=f, anchor="mm")
        img.paste(im.resize((cell_w, cell_h), Image.LANCZOS), (x, y + label_h))
    save(img, out_name, quality=88)


def basic_shapes():
    tiles = [("prim_box", "Box (cube)", "drawBox"),
             ("prim_sphere", "Sphere", "drawSphere"),
             ("prim_cylinder", "Cylinder", "drawCylinder"),
             ("prim_cone", "Cone", "drawCone"),
             ("prim_disk", "Disk / ring", "drawDisk"),
             ("prim_torus", "Torus", "drawTorus"),
             ("prim_polygon", "Triangle & quad", "glBegin ... glEnd")]
    cell, label_h = 420, 92
    cols = 4
    img = Image.new("RGB", (cols * cell, 2 * (cell + label_h)), BG)
    d = ImageDraw.Draw(img)
    for i, (name, title, fn) in enumerate(tiles):
        r, c = divmod(i, cols)
        x, y = c * cell, r * (cell + label_h)
        img.paste(fit(trim(load(f"obj_{name}_parts"), 30), cell - 40, cell - 40), (x + 20, y + 20))
        d.text((x + cell / 2, y + cell + 22), title, fill=INK, font=font(32, True), anchor="mm")
        d.text((x + cell / 2, y + cell + 62), fn, fill=MUTED, font=font(25), anchor="mm")
    # last cell: a short note
    x, y = 3 * cell, cell + label_h
    note = ["Thin dark lines show", "the triangles the GPU", "really draws.", "", "Every model in the game",
            "is built only from", "these 7 shapes."]
    for k, line in enumerate(note):
        d.text((x + cell / 2, y + 70 + k * 40), line, fill=MUTED, font=font(28), anchor="mm")
    save(img, "basic_shapes.png")


def textures():
    names = [("sand", "Sand  (Round 1 floor)"), ("bark", "Bark  (dead tree)"), ("concrete", "Concrete  (bridge hall)")]
    cell, label_h = 520, 56
    img = Image.new("RGB", (3 * cell + 2 * 24, cell + label_h), BG)
    d = ImageDraw.Draw(img)
    for i, (n, lab) in enumerate(names):
        tex = Image.open(os.path.join(ROOT, "assets", "textures", n + ".png")).convert("RGB")
        x = i * (cell + 24)
        d.text((x + cell / 2, label_h / 2), lab, fill=INK, font=font(30, True), anchor="mm")
        img.paste(tex.resize((cell, cell), Image.LANCZOS), (x, label_h))
    save(img, "textures.png")


def multiview_grid(item, out_name):
    views = ["front", "side", "top", "iso"]
    imgs = {}
    for v in views:
        p = os.path.join(SRC, f"view_{item}_{v}.bmp")
        if os.path.exists(p):
            im = Image.open(p).convert("RGB")
            imgs[v] = trim(im, pad=16)
            save(imgs[v], f"view_{item}_{v}.png")
    if len(imgs) == 4:
        cell_w, cell_h = 420, 380
        label_h = 44
        total_w = cell_w * 4
        comp = Image.new("RGB", (total_w, cell_h + label_h), BG)
        d = ImageDraw.Draw(comp)
        f = font(26, bold=True)
        labels = ["(a) Front", "(b) Side", "(c) Top", "(d) Isometric"]
        for i, (v, lab) in enumerate(zip(views, labels)):
            fitted = fit(imgs[v], cell_w - 24, cell_h - 20)
            x = i * cell_w
            comp.paste(fitted, (x + 12, 10))
            d.text((x + cell_w // 2, cell_h + 12), lab, fill=INK, font=f, anchor="mt")
        save(comp, out_name)


def main():
    os.makedirs(OUT, exist_ok=True)

    # --- plain screenshots (HUD shots keep full size so the text stays readable)
    for n in ["ui_rlgl_start", "ui_rlgl_green", "ui_rlgl_red", "ui_rlgl_won", "ui_gb_turn", "ui_gb_won"]:
        save(load(n), n + ".jpg", quality=92)
    for n in ["gb_shatter", "gb_hall", "gb_sides", "rlgl_wide", "phong_doll", "phong_players",
              "tex_sand", "tex_tree", "tex_wall"]:
        save(load(n).resize((1280, 720), Image.LANCZOS), n + ".jpg")

    # --- basic shapes, textures
    basic_shapes()
    textures()

    # --- object decompositions and multi-view projections
    summary = {}
    for item in ["player", "guard", "gbguard", "doll", "dollhead", "tree", "house", "clock",
                 "bridge", "platform", "floodlight", "chandelier", "spotlight", "gantry", "walllamp"]:
        summary[item] = decomposition(item, f"decomp_{item}.png")
        multiview_grid(item, f"views_{item}.png")
    scene_pair("rlgl_wide", "shapes_rlgl", "decomp_arena.jpg")
    scene_pair("gb_hall", "shapes_gb", "decomp_hall.jpg")

    # --- lighting and cameras
    grid(["light_gb_dir", "light_gb_point", "light_gb_spot", "light_gb_all"],
         ["Directional light only", "Point lights only", "Spot light only", "All lights on"], 2, "lights_gb.jpg")
    grid(["light_rlgl_dir", "light_rlgl_point", "light_rlgl_spot", "light_rlgl_all"],
         ["Directional light only", "Point light only", "Spot light only", "All lights on"], 2, "lights_rlgl.jpg")
    grid([f"rlgl_cam{i}" for i in range(1, 7)],
         ["[1] Third person", "[2] Free orbit", "[3] Top view", "[4] Doll orbit", "[5] Left broadcast",
          "[6] Right broadcast"], 3, "cameras_rlgl.jpg", cell_w=640)
    grid([f"gb_cam{i}" for i in range(1, 6)] + ["rlgl_cam7"],
         ["[1] Follow", "[2] Free orbit", "[3] Top-down", "[4] Side profile", "[5] Destination",
          "Round 1 [7] Finish line"], 3, "cameras_gb.jpg", cell_w=640)

    # --- part counts for the summary table
    with open(os.path.join(OUT, "part_counts.txt"), "w", encoding="utf-8") as f:
        for item, counts in summary.items():
            f.write(item + ": " + ", ".join(f"{k}={v}" for k, v in counts.items()) +
                    f", total={sum(counts.values())}\n")
    print(open(os.path.join(OUT, "part_counts.txt"), encoding="utf-8").read())


if __name__ == "__main__":
    main()
