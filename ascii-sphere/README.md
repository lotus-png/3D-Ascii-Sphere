# ASCII Sphere

A small C++ practice project that renders a shaded 3D sphere as ASCII art in your terminal. No libraries, just the standard library and some math.

It was built for fun and for practice with C++ and basic 3D graphics ideas, not to solve a particular problem.

## What it demonstrates

- **Parametric surface sampling**: points on the sphere are generated with spherical coordinates (`theta`, `phi`)
- **3D rotation**: rotation around the X, Y, and Z axes applied to both positions and surface normals
- **Perspective projection**: dividing by depth, with a 2x horizontal correction because terminal characters are taller than they are wide
- **Z-buffering**: the closest surface point wins at each character cell
- **Lambertian shading**: brightness comes from the dot product of the surface normal and the light direction, mapped onto a character ramp (`.,-~:;=!*#$@`)
- **Flicker-free terminal animation**: ANSI escape codes move the cursor home each frame, and the whole frame is written in one `fwrite`

## Build and run

You need a C++ compiler (g++ or clang++).

```bash
g++ -O2 3dcircle.cpp -o sphere
./sphere
```

On Windows (PowerShell), run `.\sphere.exe` after compiling.

Press `Ctrl+C` to quit.

**Note:** The program uses ANSI escape codes. They work out of the box on Linux, macOS, and Windows Terminal. The VS Code integrated terminal also works. The legacy `cmd.exe` console may not render them correctly.

## Configuration

Constants at the top of `3dcircle.cpp`:

| Constant | Purpose |
|---|---|
| `WIDTH`, `HEIGHT` | Size of the character grid. Adjust to fit your terminal window |
| `RADIUS` | Sphere radius in 3D units |
| `DISTANCE_FROM_CAM` | How far the camera is from the sphere's center |
| `K1` | Projection scale (effectively zoom) |
| `THETA_STEP`, `PHI_STEP` | Sampling density. Smaller values give a smoother sphere but cost more CPU |
| `SHADE_CHARS` | Brightness ramp, darkest to brightest |
| `LIGHT_X/Y/Z` | Light direction |
| `angleX/Y/Z` increments | Rotation speed per frame (in `main`) |

## How it works

1. Each frame, loop over `theta` (0 to 2π) and `phi` (0 to π) to sample points on the sphere.
2. For a sphere, the outward normal at a point is simply that point's direction from the center.
3. Rotate the point and its normal, then project onto the 2D character grid.
4. Compare depth against the z-buffer and keep only the nearest point per cell.
5. Compute brightness as `max(0, normal · light)` and pick a character from the shade ramp.
6. Print the whole buffer in one write and sleep ~30 ms.

## Known limitations and ideas

- **The rotation isn't visible.** A perfectly smooth sphere looks identical from every angle, and the light is fixed relative to the screen, so the image barely changes between frames. Ideas to make motion visible:
  - Add a texture (stripes or a checkerboard based on `theta` and `phi`) so there is something to see rotating
  - Rotate the light around the sphere instead
  - Swap in a different shape such as a torus
- The cursor is hidden at startup and not restored on exit. Add a `SIGINT` handler that prints `\x1b[?25h`, or run `reset` or `tput cnorm` in your terminal afterward.
- Colour output using ANSI 256-colour codes
- Read terminal size at runtime instead of hard-coding `WIDTH` and `HEIGHT`
- Adjustable light, speed, and zoom from the keyboard

## License

MIT, or whatever you prefer. Add a `LICENSE` file if you want one.
