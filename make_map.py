#!/usr/bin/env python3
"""Make the radar's terrain background (hills + contours, with water and parks).

Run on your Mac (needs internet, Pillow and numpy):
    pip3 install pillow numpy
    python3 make_map.py

Outputs (next to this script):
    radar_map_data.cpp / radar_map_data.h   -> copy into the firmware
    preview_*.png                           -> look at these to check the result

Data: elevation from the open AWS Terrain Tiles; water and parks recognised from
OpenStreetMap tiles. Please keep the credit "(c) OpenStreetMap contributors".
Downloads are cached in ./map_cache so re-running does not download again.
"""
import argparse, io, math, os, sys, time, urllib.request
import numpy as np
from PIL import Image, ImageFilter

ap = argparse.ArgumentParser()
ap.add_argument('--lat', type=float, default=51.351246644627366)
ap.add_argument('--lon', type=float, default=-0.21796408932666123)
# Outer ring distance (km) of each range setting. Must match the radar:
# ring-3 of 5/10/15/25 km  ->  outer ring about 6.67/13.33/20/33.33 km.
ap.add_argument('--outer-km', default='6.667,13.333,20,33.333')
ap.add_argument('--no-swap', action='store_true', help='do not swap red/blue')
ap.add_argument('--quality', type=int, default=82)
args = ap.parse_args()

HERE = os.path.dirname(os.path.abspath(__file__))
CACHE = os.path.join(HERE, 'map_cache')
os.makedirs(CACHE, exist_ok=True)
UA = 'ESP32-Plane-Radar-map-script/1.0 (hobby project; one-off download)'

S = 240            # screen size (px)
C = 120            # centre pixel
GRID_R = 107       # px radius of the outer ring (radar_theme.h kGridOuterRadius)
SS = 2             # supersampling
KM_PER_DEG = 111.0 # same flat projection as the radar firmware


def fetch(url, name):
    path = os.path.join(CACHE, name)
    if os.path.exists(path):
        return open(path, 'rb').read()
    req = urllib.request.Request(url, headers={'User-Agent': UA})
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=30) as r:
                data = r.read()
            open(path, 'wb').write(data)
            time.sleep(0.4)  # be polite to the tile servers
            return data
        except Exception as e:
            if attempt == 2:
                sys.exit(f'Download failed: {url}\n{e}')
            time.sleep(2)


def tile_bytes(kind, z, x, y):
    if kind == 'terrain':
        url = f'https://s3.amazonaws.com/elevation-tiles-prod/terrarium/{z}/{x}/{y}.png'
    else:
        url = f'https://tile.openstreetmap.org/{z}/{x}/{y}.png'
    return fetch(url, f'{kind}_{z}_{x}_{y}.png')


def merc(lat, lon, z):
    n = 2.0 ** z
    x = (lon + 180.0) / 360.0 * n
    y = (1.0 - np.arcsinh(np.tan(np.radians(lat))) / math.pi) / 2.0 * n
    return x * 256.0, y * 256.0


def mosaic(kind, z, x0, x1, y0, y1):
    w = (x1 - x0 + 1) * 256
    h = (y1 - y0 + 1) * 256
    out = np.zeros((h, w, 3), np.uint8)
    for ty in range(y0, y1 + 1):
        for tx in range(x0, x1 + 1):
            im = Image.open(io.BytesIO(tile_bytes(kind, z, tx, ty))).convert('RGB')
            out[(ty - y0) * 256:(ty - y0 + 1) * 256, (tx - x0) * 256:(tx - x0 + 1) * 256] = np.asarray(im)
    return out


def bilinear(arr, px, py):
    h, w = arr.shape[:2]
    px = np.clip(px, 0, w - 1.001)
    py = np.clip(py, 0, h - 1.001)
    x0 = np.floor(px).astype(int); y0 = np.floor(py).astype(int)
    fx = (px - x0)[..., None] if arr.ndim == 3 else (px - x0)
    fy = (py - y0)[..., None] if arr.ndim == 3 else (py - y0)
    a = arr[y0, x0].astype(np.float32); b = arr[y0, x0 + 1].astype(np.float32)
    c = arr[y0 + 1, x0].astype(np.float32); d = arr[y0 + 1, x0 + 1].astype(np.float32)
    return (a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy


def blur(a, sigma):
    """Separable Gaussian blur of a 2-D float array (numpy only)."""
    a = a.astype(np.float32)
    r = max(1, int(math.ceil(sigma * 3)))
    k = np.exp(-0.5 * (np.arange(-r, r + 1) / sigma) ** 2).astype(np.float32)
    k /= k.sum()
    for axis in (0, 1):
        pad = [(0, 0), (0, 0)]
        pad[axis] = (r, r)
        p = np.pad(a, pad, mode='edge')
        out = np.zeros_like(a)
        n = a.shape[axis]
        for i, w in enumerate(k):
            sl = [slice(None), slice(None)]
            sl[axis] = slice(i, i + n)
            out += w * p[tuple(sl)]
        a = out
    return a


def class_mask(rgb, colours, tol):
    rgbf = rgb.astype(np.float32)
    m = np.zeros(rgb.shape[:2], np.float32)
    for c in colours:
        d = np.sqrt(((rgbf - np.array(c, np.float32)) ** 2).sum(-1))
        m = np.maximum(m, (d <= tol).astype(np.float32))
    return m

WATER = [(170, 211, 223)]
# parks, gardens, recreation grounds, woods, grass / village greens
GREEN = [(200, 250, 204), (223, 252, 226), (173, 209, 158), (205, 235, 176),
         (206, 235, 177), (200, 215, 171)]


def render(lat0, lon0, outer_km):
    ppk = GRID_R / outer_km                       # screen px per km
    half_km = (C + 2) / ppk
    span_m = 2 * half_km * 1000.0
    target = span_m / (S * SS)                    # wanted metres per source pixel
    z = int(math.ceil(math.log2(156543.03 * math.cos(math.radians(lat0)) / target)))
    z = max(7, min(z, 13))

    n = SS * S
    i = (np.arange(n) + 0.5) / SS - 0.5           # screen pixel coordinates
    sx, sy = np.meshgrid(i, i)
    lat = lat0 + (C - sy) / ppk / KM_PER_DEG
    lon = lon0 + (sx - C) / ppk / (KM_PER_DEG * math.cos(math.radians(lat0)))
    mx, my = merc(lat, lon, z)
    x0, x1 = int(mx.min() // 256) - 0, int(mx.max() // 256)
    y0, y1 = int(my.min() // 256) - 0, int(my.max() // 256)
    print(f'  outer {outer_km:g} km: zoom {z}, {(x1-x0+1)*(y1-y0+1)} tiles x2')

    # ---- elevation -------------------------------------------------------
    terr = mosaic('terrain', z, x0, x1, y0, y1).astype(np.float32)
    elev_src = terr[..., 0] * 256.0 + terr[..., 1] + terr[..., 2] / 256.0 - 32768.0
    elev = bilinear(elev_src, mx - x0 * 256, my - y0 * 256)
    elev = blur(elev, 1.6 * SS)                    # remove DEM noise

    gy, gx = np.gradient(elev)
    inside = ((sx - C) ** 2 + (sy - C) ** 2) <= (C - 2) ** 2
    mag = np.hypot(gx, gy)
    scale = np.percentile(mag[inside], 90) + 1e-6
    # light from the north-west; hills facing it are brighter
    shade = np.clip(((-gx) * 0.7 + (gy) * 0.7) / scale, -1.0, 1.0)
    shade = 0.5 + 0.38 * shade
    base = np.stack([shade * 40 + 8, shade * 50 + 12, shade * 38 + 10], -1)

    # ---- contour lines ---------------------------------------------------
    lo, hi = np.percentile(elev[inside], [2, 98])
    rng = max(hi - lo, 1.0)
    steps = [1, 2, 5, 10, 20, 25, 50, 100, 200]
    step = next((s for s in steps if rng / s <= 14), steps[-1])
    lv = np.floor(elev / step)
    edge = (np.abs(np.diff(lv, axis=0, prepend=lv[:1])) + np.abs(np.diff(lv, axis=1, prepend=lv[:, :1])) > 0)
    ct = blur(edge.astype(np.float32), 0.7 * SS)
    ct = np.clip(ct * 1.6, 0, 1)
    print(f'    contours every {step:g} m (area spans {rng:.0f} m)')

    # ---- water and green space from OpenStreetMap tiles -----------------------
    osm = mosaic('osm', z, x0, x1, y0, y1)
    osm_s = bilinear(osm, mx - x0 * 256, my - y0 * 256)
    water = class_mask(osm_s, WATER, 14)
    green = class_mask(osm_s, GREEN, 12)
    # close thin gaps (roads/labels cutting through) and soften edges
    water = np.clip((blur(water, 1.4 * SS) - 0.30) / 0.4, 0, 1)
    green = np.clip((blur(green, 1.6 * SS) - 0.30) / 0.4, 0, 1)

    def mix(img, col, m, a):
        col = np.array(col, np.float32)
        return img * (1 - (m * a)[..., None]) + col * (m * a)[..., None]

    img = base
    img = mix(img, (95, 70, 45), ct, 0.55)        # contours
    img = mix(img, (20, 52, 30), green, 0.60)     # parks / woods
    img = mix(img, (22, 60, 110), water, 0.90)    # rivers, lakes
    img = np.clip(img, 0, 255).astype(np.uint8)
    out = Image.fromarray(img).resize((S, S), Image.LANCZOS)
    # black outside the round screen
    yy, xx = np.mgrid[0:S, 0:S]
    mask = ((xx - C) ** 2 + (yy - C) ** 2) > (C - 0.5) ** 2
    a = np.asarray(out).copy(); a[mask] = 0
    return Image.fromarray(a)


outers = [float(v) for v in args.outer_km.split(',')]
blobs = []
for k, ok in enumerate(outers):
    im = render(args.lat, args.lon, ok)
    im.save(os.path.join(HERE, f'preview_{k}_{ok:g}km.png'))
    if not args.no_swap:
        r, g, b = im.split()
        im = Image.merge('RGB', (b, g, r))   # this display shows red and blue swapped
    buf = io.BytesIO()
    im.save(buf, 'JPEG', quality=args.quality, progressive=False, subsampling=2)
    blobs.append(buf.getvalue())
    print(f'    saved {len(blobs[-1])} bytes')

with open(os.path.join(HERE, 'radar_map_data.h'), 'w') as f:
    f.write('#pragma once\n\n#include <cstddef>\n#include <cstdint>\n\nnamespace ui {\n\n'
            'struct RadarMapImage {\n  float outer_km;\n  const uint8_t* data;\n  size_t len;\n};\n\n'
            f'inline constexpr double kRadarMapLat = {args.lat!r};\n'
            f'inline constexpr double kRadarMapLon = {args.lon!r};\n'
            f'inline constexpr size_t kRadarMapCount = {len(blobs)};\n'
            'extern const RadarMapImage kRadarMaps[kRadarMapCount];\n\n}  // namespace ui\n')
with open(os.path.join(HERE, 'radar_map_data.cpp'), 'w') as f:
    f.write('// Generated by make_map.py - do not edit by hand.\n'
            '// Map data (c) OpenStreetMap contributors (ODbL); elevation: AWS Terrain Tiles.\n'
            '#include "ui/radar_map_data.h"\n\nnamespace ui {\nnamespace {\n')
    for i, b in enumerate(blobs):
        f.write(f'const uint8_t kMap{i}[] = {{\n')
        for j in range(0, len(b), 24):
            f.write('    ' + ','.join(str(x) for x in b[j:j + 24]) + ',\n')
        f.write('};\n')
    f.write('}  // namespace\n\nconst RadarMapImage kRadarMaps[kRadarMapCount] = {\n')
    for i, (b, ok) in enumerate(zip(blobs, outers)):
        f.write(f'    {{{ok}f, kMap{i}, sizeof(kMap{i})}},\n')
    f.write('};\n\n}  // namespace ui\n')
print('done, total', sum(len(b) for b in blobs), 'bytes')
