"""Generates the three seamless textures used by the game (assets/textures/*.png).

    python tools/make_textures.py

Noise is made in the frequency domain (FFT), so every image tiles without seams.
"""
import os
import numpy as np
from PIL import Image

SIZE = 512
OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "textures")
rng = np.random.default_rng(456)


def noise(fx_scale, fy_scale=None, power=2.0):
    """Tileable noise in [0,1]; larger *_scale keeps finer detail along that axis."""
    fy_scale = fx_scale if fy_scale is None else fy_scale
    fx = np.fft.fftfreq(SIZE)[None, :] * SIZE
    fy = np.fft.fftfreq(SIZE)[:, None] * SIZE
    r = np.sqrt((fx / fx_scale) ** 2 + (fy / fy_scale) ** 2)
    r[0, 0] = 1.0
    spectrum = np.exp(2j * np.pi * rng.random((SIZE, SIZE))) / (1.0 + r ** power)
    spectrum[0, 0] = 0
    n = np.real(np.fft.ifft2(spectrum))
    return (n - n.min()) / (n.max() - n.min())


def save(name, rgb):
    img = (np.clip(rgb, 0, 1) * 255).astype(np.uint8)
    Image.fromarray(img, "RGB").save(os.path.join(OUT, name))
    print("wrote", name, "mean", rgb.reshape(-1, 3).mean(0).round(3))


def wrap_dist(cx, cy):
    y, x = np.mgrid[0:SIZE, 0:SIZE]
    dx = np.minimum(abs(x - cx), SIZE - abs(x - cx))
    dy = np.minimum(abs(y - cy), SIZE - abs(y - cy))
    return np.sqrt(dx ** 2 + dy ** 2)


def sand():
    base = np.array([0.80, 0.62, 0.40])
    blotch = noise(3) - 0.5          # large soft patches
    ripple = noise(40, 10) - 0.5      # wind ripples
    grain = noise(220, power=1.2) - 0.5
    shade = 1.0 + 0.22 * blotch + 0.10 * ripple + 0.26 * grain
    rgb = base[None, None, :] * shade[..., None]
    # scattered pebbles
    for _ in range(240):
        cx, cy = rng.integers(0, SIZE, 2)
        d = wrap_dist(cx, cy)
        r = rng.uniform(1.2, 3.2)
        tone = rng.choice([0.72, 0.82, 1.12])
        mask = np.clip(1.5 - d / r, 0, 1)[..., None]
        rgb = rgb * (1 - mask) + rgb * tone * mask
    save("sand.png", rgb)


def bark():
    fibers = noise(90, 4, power=1.6)              # long vertical fibers
    ridges = noise(24, 3, power=2.0)
    crack = np.clip((np.abs(ridges - 0.5) * 2.0 - 0.06) * 3.0, 0, 1)  # dark crevices
    knots = noise(6, power=2.5)
    light = np.array([0.46, 0.38, 0.30])
    dark = np.array([0.13, 0.10, 0.08])
    t = np.clip(0.25 + 0.55 * crack + 0.30 * (fibers - 0.5) + 0.15 * (knots - 0.5), 0, 1)
    rgb = dark[None, None, :] * (1 - t[..., None]) + light[None, None, :] * t[..., None]
    save("bark.png", rgb)


def concrete():
    base = np.array([0.74, 0.75, 0.76])
    stains = noise(4) - 0.5
    mottle = noise(30) - 0.5
    speck = noise(250, power=1.0) - 0.5
    shade = 1.0 + 0.14 * stains + 0.06 * mottle + 0.10 * speck
    rgb = base[None, None, :] * shade[..., None]
    y, x = np.mgrid[0:SIZE, 0:SIZE]
    # 2 x 2 formwork panels with thin dark seams (seams sit on the tile edges and centre)
    seam = np.minimum(np.minimum(x % 256, 256 - x % 256), np.minimum(y % 256, 256 - y % 256))
    rgb *= (1.0 - 0.32 * np.clip(1.6 - seam, 0, 1))[..., None]
    # four tie-holes per panel
    for px in (0, 256):
        for py in (0, 256):
            for hx in (64, 192):
                for hy in (64, 192):
                    d = wrap_dist(px + hx, py + hy)
                    rim = np.clip(1.0 - abs(d - 7.5) / 1.6, 0, 1)
                    hole = np.clip(6.5 - d, 0, 1)
                    rgb *= (1.0 + 0.10 * rim - 0.55 * hole)[..., None]
    save("concrete.png", rgb)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    sand()
    bark()
    concrete()
