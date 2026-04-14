# UltraWire

**Advanced Graph Editor Visual Customization Plugin for Unreal Engine 5**

*By StraySpark*

---

UltraWire is an all-in-one graph editor visual overhaul plugin for Unreal Engine 5 that combines wire customization, node theming, smart routing, a graph minimap, performance heatmaps, and team collaboration into a single plugin. It replaces and extends what previously required multiple separate plugins, while adding features that no existing plugin offers.

---

## Table of Contents

- [Features](#features)
- [Supported Engine Versions](#supported-engine-versions)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Wire Routing Styles](#wire-routing-styles)
- [Corner Styles](#corner-styles)
- [Wire Ribbons (Stacking)](#wire-ribbons-stacking)
- [Crossing Symbols](#crossing-symbols)
- [Moving Bubbles](#moving-bubbles)
- [Wire Glow and Neon Mode](#wire-glow-and-neon-mode)
- [Smart Auto-Routing (A* Pathfinding)](#smart-auto-routing-a-pathfinding)
- [Node Theming](#node-theming)
- [Graph Minimap](#graph-minimap)
- [Execution Heatmap](#execution-heatmap)
- [Wire Labels](#wire-labels)
- [Per-Graph-Type Profiles](#per-graph-type-profiles)
- [Built-in Presets](#built-in-presets)
- [Preset Management](#preset-management)
- [Team Collaboration and Project Defaults](#team-collaboration-and-project-defaults)
- [Supported Graph Types](#supported-graph-types)
- [Settings Reference](#settings-reference)
- [Performance](#performance)
- [Troubleshooting](#troubleshooting)
- [Technical Architecture](#technical-architecture)
- [File Structure](#file-structure)
- [License](#license)
- [Support](#support)

---

## Features

| Feature | Description |
|---------|-------------|
| **4 Wire Routing Styles** | Default (Bezier), Manhattan (90°), Subway (45°), Freeform (custom angle) |
| **3 Corner Styles** | Sharp, Rounded (arc), Chamfered (diagonal cut) |
| **Wire Ribbons** | Multiple wires between the same nodes stack into clean ribbon bundles |
| **Crossing Symbols** | Gap, Arc, or Circle indicators where wires cross each other |
| **Moving Bubbles** | Animated dots flowing along wires to show data direction |
| **Wire Glow / Neon** | Additive-blend glow effect with optional pulse animation |
| **Smart Auto-Routing** | A*-based pathfinding that routes wires around node bodies |
| **Node Theming** | Customize node headers, body opacity, pin shapes, and comment boxes |
| **Graph Minimap** | Picture-in-picture minimap overlay with click-to-navigate |
| **Execution Heatmap** | Wires display real-time execution frequency as a color gradient |
| **Wire Labels** | Inline text annotations on wire segments |
| **Per-Graph Profiles** | Different visual settings for each graph type |
| **5 Built-in Presets** | Professional theme presets ready to use |
| **Team Sharing** | Export/import presets, project-level defaults via INI config |
| **12+ Graph Types** | Blueprint, Material, Niagara, Animation, Behavior Tree, PCG, and more |
| **Hot Reload** | All visual changes apply instantly without restarting the editor |
| **ALT+LMB Wire Slice** | Full compatibility with UE 5.5+ wire slicing |

---

## Supported Engine Versions

| Engine Version | Status |
|----------------|--------|
| Unreal Engine 5.7 | Primary (built & tested against) |
| Unreal Engine 5.6 | Supported |
| Unreal Engine 5.5 | Supported |

**Platforms:** Win64, Mac, Linux (Editor only — this plugin does not affect runtime builds)

---

## Installation

### From Fab

1. Purchase and download **UltraWire** from the Fab marketplace.
2. The plugin will be installed to your engine's `Plugins/Fab` directory automatically.
3. Open any Unreal Engine project. The plugin is enabled by default.
4. Verify installation: **Edit > Plugins**, search for "UltraWire" — it should appear under the **Editor** category with a checkmark.

### Manual Installation

1. Copy the `UltraWire` folder into your project's `Plugins` directory:
   ```
   YourProject/
   └── Plugins/
       └── UltraWire/
           ├── UltraWire.uplugin
           ├── Source/
           ├── Config/
           ├── Content/
           └── Resources/
   ```
2. Regenerate project files (right-click `.uproject` > **Generate Visual Studio project files**).
3. Build and open your project.

---

## Quick Start

1. **Open Settings:** Go to **Edit > Editor Preferences**, then navigate to **Plugins > UltraWire** in the left panel.
2. **Choose a Wire Style:** Under **Default Profile > Wire Style**, select **Manhattan (90°)** for clean right-angle wires.
3. **Apply a Preset:** Scroll to the **Built-in Presets** section, select **"Neon Cyberpunk"** from the dropdown, and click **Apply** to instantly transform your graph editor.
4. **Open a Blueprint:** Navigate to any Blueprint graph. Your wires will now render with the selected style.

All changes take effect immediately — no editor restart required.

---

## Wire Routing Styles

UltraWire provides four wire routing algorithms. Select your preferred style in **Editor Preferences > Plugins > UltraWire > Default Profile > Wire Style**.

### Default (Bezier)

The standard Unreal Engine curved wire. UltraWire passes through to the engine's default Bezier spline renderer, but still applies glow, bubbles, and other effects on top.

### Manhattan (90°)

Wires route exclusively with horizontal and vertical segments, creating clean right-angle paths similar to PCB traces or circuit diagrams. The routing algorithm:

- Exits horizontally from the output pin.
- Turns 90° to traverse vertically.
- Turns 90° again to enter the input pin horizontally.
- Handles **backward connections** (input is to the left of output) with an automatic loop-around path.
- Handles **same-Y pins** with a direct horizontal segment.

### Subway (45°)

Wires exit the output pin horizontally, transition through a 45-degree diagonal segment, then enter the input pin horizontally. This creates a sleek, angled aesthetic with three segments per wire. The algorithm handles backward connections with extended routing segments.

### Freeform (Custom Angle)

Similar to Subway, but the diagonal angle is user-configurable from **5° to 85°**. Set the angle with the **Freeform Angle** slider in settings. This allows unique looks like 30° shallow angles or 60° steep angles.

---

## Corner Styles

When using Manhattan, Subway, or Freeform routing, wire corners can be styled three ways:

| Style | Description |
|-------|-------------|
| **Sharp** | Hard 90° or angled corners with no rounding. Crisp PCB look. |
| **Rounded** | Circular arc approximation at each corner. Configurable radius (0–50 px). The arc is subdivided into 8 segments for smooth curves. |
| **Chamfered** | Diagonal cut at corners. A 45° line replaces the corner point, with the cut size controlled by the corner radius setting. |

**Setting:** `Corner Style` and `Corner Radius` under **Default Profile > Wire Style**.

---

## Wire Ribbons (Stacking)

When multiple wires connect from one node to another (or from the same output pin to different inputs), enabling **Ribbons** offsets them perpendicular to the wire path so they run in parallel like a ribbon cable.

- **Enable:** Check `Enable Ribbons` in settings.
- **Ribbon Offset:** Controls the spacing between parallel wires (1–20 px, default 4).
- The offset is calculated per pin index: the first wire stays centered, subsequent wires offset above and below.

---

## Crossing Symbols

When two wires cross each other on the graph canvas, UltraWire can draw an indicator at the intersection point to visually disambiguate the crossing.

| Style | Visual |
|-------|--------|
| **None** | No indicator (wires simply overlap) |
| **Gap** | A small break in the wire that passes behind, creating the illusion it goes "under" |
| **Arc** | A semicircular bump in one wire, suggesting it jumps over the other |
| **Circle** | A filled circle at the intersection point (solder joint aesthetic) |

- **Setting:** `Crossing Style` and `Crossing Size` (3–20 px).
- Crossing detection runs after all wires are drawn using an O(W²·S²) segment intersection test, where W is the number of wires and S is segments per wire.

---

## Moving Bubbles

Animated dots that flow along wires to indicate data flow direction. Bubbles travel from the output pin toward the input pin.

| Setting | Range | Default | Description |
|---------|-------|---------|-------------|
| `Enable Bubbles` | on/off | off | Master toggle |
| `Bubble Speed` | 10–500 | 100 | Travel speed in pixels per second |
| `Bubble Size` | 1–10 | 3 | Radius of each bubble dot |
| `Bubble Spacing` | 10–200 | 50 | Distance between consecutive bubbles |

Bubble positions are computed by evaluating the wire path at regular intervals offset by `Time * Speed`, creating smooth flowing animation along the exact wire geometry (including around corners).

---

## Wire Glow and Neon Mode

An additive-blend glow effect renders a luminous halo around each wire. The glow is drawn as 4 concentric layers with increasing width and decreasing opacity, producing a natural bloom falloff.

| Setting | Range | Default | Description |
|---------|-------|---------|-------------|
| `Enable Glow` | on/off | off | Master toggle |
| `Glow Intensity` | 0–5 | 1.0 | Brightness multiplier |
| `Glow Width` | 1–20 | 6.0 | Width of the outermost glow layer in pixels |
| `Glow Pulse` | on/off | off | Enable sinusoidal intensity modulation |
| `Glow Pulse Speed` | 0.1–5.0 | 1.0 | Pulse frequency in Hz |

**Pulse animation** modulates the glow intensity with a sine wave mapped to the range [0.25, 1.0], so the glow dims and brightens but never fully disappears.

The glow color is automatically derived from the wire's color. Combined with the Neon Cyberpunk preset, this creates a dramatic, futuristic aesthetic ideal for streams, tutorials, and showcases.

---

## Smart Auto-Routing (A* Pathfinding)

UltraWire's flagship feature. When enabled, wires are routed around node bodies using an A* pathfinding algorithm instead of simple geometric calculation.

### How It Works

1. **Grid Construction:** The graph canvas is discretized into a grid of cells (configurable size, default 20 px).
2. **Occupancy Marking:** Node bounding rectangles are expanded by a padding value and rasterized onto the grid, marking those cells as blocked.
3. **A* Search:** For each wire, A* finds the shortest unblocked path from the output pin cell to the input pin cell.
   - **Manhattan style** uses 4-directional movement with Manhattan heuristic.
   - **Subway/Freeform** uses 8-directional movement with Octile heuristic, with diagonal moves validated to prevent corner-cutting through node edges.
4. **Path Smoothing:** The grid path is converted to world coordinates, and collinear points are collapsed to produce clean line segments.
5. **Corner Styling:** The smoothed path is then processed through the corner rounding system (Rounded/Chamfered/Sharp).

### Settings

| Setting | Range | Default | Description |
|---------|-------|---------|-------------|
| `Enable Smart Routing` | on/off | off | Master toggle |
| `Routing Grid Size` | 5–50 | 20 | Cell size in pixels. Smaller = more precise but slower. |
| `Routing Node Padding` | 0–50 | 10.0 | Clearance around nodes that wires must respect |

### Performance Safeguards

- **50 ms Timeout:** If A* cannot find a path within 50 ms wall-clock time, it aborts and the wire falls back to simple geometric routing. This prevents freezes on very large graphs.
- **Route Caching:** Computed routes are cached with a hash key derived from start/end positions and node rectangles. Routes are only recomputed when nodes move.
- **Incremental Invalidation:** When a single node is moved, only routes that intersect that node's bounds are invalidated — not the entire cache.
- **Grid Size Cap:** The grid is capped at 512×512 cells to prevent excessive memory use on very large canvases.

---

## Node Theming

UltraWire includes an integrated node appearance customization engine that modifies Slate styles to change how graph nodes look. This eliminates the need for a separate theming plugin.

| Setting | Range | Default | Description |
|---------|-------|---------|-------------|
| `Enable Node Theming` | on/off | off | Master toggle |
| `Node Body Opacity` | 0–1 | 1.0 | Transparency of the node body background |
| `Node Header Tint Color` | Color | White | Tint applied to node title bar |
| `Node Corner Radius` | 0–16 | 4.0 | Rounding of node body corners |
| `Pin Shape` | Circle/Diamond/Square/Arrow | Circle | Shape of pin connection points |

### What Gets Themed

The theme engine modifies these Slate style keys:

- `Graph.Node.Body` — Node body background
- `Graph.Node.TitleGloss` — Title bar gloss overlay
- `Graph.Node.ColorSpill` — Color spill behind the title
- `Graph.Node.TitleHighlight` — Title highlight accent
- `Graph.Node.Shadow` / `Graph.Node.ShadowSize` — Drop shadow
- `Graph.Node.PinPoint` — Pin connection point shape
- `Graph.CommentBubble.Body` / `Graph.CommentBubble.Border` — Comment box appearance

### Reversibility

The theme engine snapshots all original Slate brushes before modifying them. When UltraWire is disabled or theming is turned off, all styles are restored to their original values. The plugin never permanently modifies engine files.

---

## Graph Minimap

A picture-in-picture minimap overlay that appears in the corner of every graph editor, showing the full graph topology at a glance.

### Features

- **Node visualization:** Nodes appear as solid colored rectangles, colored by their title color (which corresponds to their category — functions, events, math, etc.).
- **Wire visualization:** Wires render as thin colored lines between node positions.
- **Viewport indicator:** A translucent outlined rectangle shows your current visible area.
- **Click-to-navigate:** Click anywhere on the minimap to instantly pan the graph editor to that location.
- **Comment boxes:** Comment nodes render as outlined rectangles with a distinct yellow-green color.

### Settings

| Setting | Range | Default | Description |
|---------|-------|---------|-------------|
| `Enable Minimap` | on/off | off | Master toggle |
| `Minimap Position` | TopLeft / TopRight / BottomLeft / BottomRight | BottomRight | Which corner of the graph editor |
| `Minimap Size` | 100–500 | 200 | Width and height of the minimap in pixels |
| `Minimap Opacity` | 0.1–1.0 | 0.8 | Overall opacity of the minimap overlay |

### Performance

The minimap caches the graph topology (node positions, sizes, colors, and wire connections) and refreshes it at approximately **10 fps** (every 100 ms). This avoids per-frame iteration over graph nodes while keeping the display responsive to node movement.

---

## Execution Heatmap

A world-first feature: wires can display real-time execution frequency and timing data as a color gradient overlay. Hot paths glow red, cold paths stay blue. This provides at-a-glance performance insights directly on the graph canvas.

### How It Works

1. **Start the Blueprint Profiler** in UE (via the **Blueprint Debugger** toolbar or **Window > Developer Tools > Blueprint Profiler**).
2. **Enable Heatmap** in UltraWire settings.
3. Play in Editor (PIE) or run a Blueprint debugging session.
4. UltraWire's heatmap bridge polls the Blueprint profiler at ~4 Hz, reads execution counts per node/pin, and normalizes them to a 0–1 heat value.
5. The heat value is mapped to a color gradient between your configured Cold Color and Hot Color using perceptually smooth HSV interpolation.
6. Wire colors are modulated by the heat value during rendering.

### Settings

| Setting | Default | Description |
|---------|---------|-------------|
| `Enable Heatmap` | off | Master toggle |
| `Heatmap Cold Color` | Blue (0, 0.2, 1) | Color for rarely-executed wires |
| `Heatmap Hot Color` | Red (1, 0.1, 0) | Color for frequently-executed wires |

### Zero-Overhead Design

When the Blueprint profiler is not running, the heatmap system is completely inactive — it adds zero overhead. The profiler bridge only activates in response to `FBlueprintCoreDelegates` profiling callbacks.

---

## Wire Labels

Add inline text labels to any wire to create self-documenting graphs. Labels render at the midpoint of the wire path and scale with the graph zoom level.

### Auto-Labels

When `Enable Auto Labels` is turned on, wires connected to Get/Set variable nodes automatically display the variable name as a label. This turns spaghetti graphs into readable diagrams without needing comment boxes everywhere.

Labels render with:
- A semi-transparent dark background for readability
- White text by default
- Padding around the text for clean appearance
- Proper positioning at the wire midpoint with configurable vertical offset

---

## Per-Graph-Type Profiles

UltraWire allows you to configure completely different visual profiles for each type of graph editor. For example:

- **Blueprint graphs:** Manhattan 90° wires with thick ribbons and gap crossings
- **Material graphs:** Thin Subway 45° wires with no effects
- **Niagara graphs:** Glow-enabled wires with pulse animation
- **Animation Blueprints:** Clean Professional preset with smart routing

### How to Configure

1. Go to **Editor Preferences > Plugins > UltraWire > Graph Type Profiles**.
2. For each graph type, select a preset from the dropdown or configure custom settings.
3. If a graph type has no override, it uses the **Default Profile**.

The following graph types can be independently configured:
- Blueprint
- Material
- Niagara
- Animation Blueprint
- Behavior Tree
- Control Rig
- PCG (Procedural Content Generation)
- Sound Cue
- MetaSound
- Environment Query
- Gameplay Ability
- Other (fallback for unrecognized graph types)

---

## Built-in Presets

UltraWire ships with 5 professionally designed presets:

### Default

Subtle improvements over vanilla Unreal Engine. Clean Manhattan routing with rounded corners at radius 8. No animations or effects. Node theming at 92% body opacity for a slightly more refined look. Ideal for users who want cleaner wires without a dramatic visual change.

### Circuit Board

A PCB-inspired aesthetic. Sharp Manhattan routing with square pin shapes, gap crossings, and ribbon stacking. Green-tinted node headers. Smart routing and auto-labels enabled. Designed for users who appreciate the look of electronic circuit boards.

### Neon Cyberpunk

A dramatic dark theme with bright glow accents. Default Bezier routing with high-intensity pulsing glow (2.5 intensity, 1.5 Hz pulse), animated bubbles, arc crossings, and diamond pin shapes. Rounded corners at 14 px radius. Node bodies at 85% opacity. This preset is inherently eye-catching on video — ideal for streams, tutorials, and showcases.

### Clean Professional

Minimal and high-contrast. No animations, no glow, no visual noise. Designed for documentation screenshots, code reviews, and presentations. Uses Subway routing for clean diagonal lines. Full node body opacity for maximum readability.

### Retro Terminal

An amber and green monochrome terminal aesthetic. Freeform routing with custom angles, slow-moving bubbles, and a soft glow. Square pin shapes and sharp corners evoke the look of old-school terminal displays and early computer graphics.

---

## Preset Management

### Saving Custom Presets

After configuring your ideal visual profile, save it as a named preset from the settings panel. Custom presets are stored in your Editor Preferences and persist across sessions.

### Exporting and Importing Presets

- **Export:** Click the **Export** button in the Profile section. Choose a save location. The preset is written as a `.ultrawire` JSON file.
- **Import:** Click the **Import** button. Select a `.ultrawire` file. The preset is loaded and added to your saved presets.

The `.ultrawire` file format is a simple JSON structure:

```json
{
    "PresetName": "My Custom Style",
    "Description": "Manhattan with soft glow",
    "Author": "YourName",
    "Profile": {
        "WireStyle": "Manhattan",
        "CornerStyle": "Rounded",
        "CornerRadius": 12.0,
        "WireThickness": 2.0,
        "bEnableGlow": true,
        "GlowIntensity": 1.5,
        ...
    }
}
```

---

## Team Collaboration and Project Defaults

### Project-Level Defaults

UltraWire supports a `DefaultUltraWire.ini` configuration file in your project's `Config` directory. This file sets team-wide visual defaults that all developers inherit when they open the project.

**Location:** `YourProject/Config/DefaultUltraWire.ini` (or `YourProject/Plugins/UltraWire/Config/DefaultUltraWire.ini`)

When a new team member opens the project with UltraWire installed, they automatically get the team's visual standard. Individual developers can override settings locally in their `EditorPerProjectUserSettings.ini`.

### Sharing Workflow

1. Configure your preferred visual settings in Editor Preferences.
2. Export the preset as a `.ultrawire` file.
3. Share the file with your team via version control, Slack, email, etc.
4. Team members import the file and apply it.

Alternatively, configure `DefaultUltraWire.ini` directly and commit it to your project's source control. This is the recommended approach for teams that want enforced visual standards.

---

## Supported Graph Types

UltraWire registers a custom wire drawing factory for each of these graph editors:

| Graph Type | Editor | Status |
|------------|--------|--------|
| Blueprint | Blueprint Editor | Full support |
| Material | Material Editor | Full support |
| Niagara | Niagara Editor | Full support |
| Animation Blueprint | AnimGraph | Full support |
| Behavior Tree | BT Editor | Full support |
| Control Rig | Control Rig Editor | Full support |
| PCG | PCG Graph | Full support |
| Sound Cue | Sound Cue Editor | Full support |
| MetaSound | MetaSound Editor | Full support |
| Environment Query | EQS Editor | Full support |
| Gameplay Ability | GA Editor | Full support |
| Other | Any unrecognized graph | Fallback support |

Each graph type can be assigned its own visual profile or use the default.

---

## Settings Reference

All settings are accessible at **Edit > Editor Preferences > Plugins > UltraWire**.

### General

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| Enabled | bool | true | Master enable/disable for the entire plugin |

### Wire Style

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Wire Style | Enum | Manhattan | — | Default, Manhattan, Subway, Freeform |
| Freeform Angle | float | 30.0 | 5–85° | Angle for Freeform style |
| Corner Style | Enum | Rounded | — | Sharp, Rounded, Chamfered |
| Corner Radius | float | 10.0 | 0–50 | Radius for Rounded/Chamfered corners |
| Wire Thickness | float | 1.5 | 0.5–10 | Base wire thickness in pixels |

### Ribbons

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Enable Ribbons | bool | false | — | Stack multiple wires as parallel ribbons |
| Ribbon Offset | float | 4.0 | 1–20 | Spacing between parallel wires |

### Crossings

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Crossing Style | Enum | None | — | None, Gap, Arc, Circle |
| Crossing Size | float | 8.0 | 3–20 | Size of crossing indicator |

### Bubbles

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Enable Bubbles | bool | false | — | Animated data flow dots |
| Bubble Speed | float | 100.0 | 10–500 | Travel speed (px/sec) |
| Bubble Size | float | 3.0 | 1–10 | Dot radius (px) |
| Bubble Spacing | float | 50.0 | 10–200 | Distance between dots (px) |

### Glow

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Enable Glow | bool | false | — | Additive glow around wires |
| Glow Intensity | float | 1.0 | 0–5 | Brightness multiplier |
| Glow Width | float | 6.0 | 1–20 | Outer glow radius (px) |
| Glow Pulse | bool | false | — | Sinusoidal brightness animation |
| Glow Pulse Speed | float | 1.0 | 0.1–5 | Pulse frequency (Hz) |

### Smart Routing

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Enable Smart Routing | bool | false | — | A* pathfinding around nodes |
| Routing Grid Size | int | 20 | 5–50 | Grid cell size (px) |
| Routing Node Padding | float | 10.0 | 0–50 | Clearance around nodes (px) |

### Heatmap

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| Enable Heatmap | bool | false | Execution frequency visualization |
| Heatmap Cold Color | Color | Blue (0, 0.2, 1) | Rarely-executed wire color |
| Heatmap Hot Color | Color | Red (1, 0.1, 0) | Frequently-executed wire color |

### Node Theme

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Enable Node Theming | bool | false | — | Override node Slate styles |
| Node Body Opacity | float | 1.0 | 0–1 | Node background transparency |
| Node Header Tint Color | Color | White | — | Title bar color tint |
| Node Corner Radius | float | 4.0 | 0–16 | Node body corner rounding |
| Pin Shape | Enum | Circle | — | Circle, Diamond, Square, Arrow |

### Labels

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| Enable Auto Labels | bool | false | Auto-display variable names on Get/Set wires |

### Minimap

| Setting | Type | Default | Range | Description |
|---------|------|---------|-------|-------------|
| Enable Minimap | bool | false | — | Show minimap overlay |
| Minimap Position | Enum | BottomRight | — | TopLeft, TopRight, BottomLeft, BottomRight |
| Minimap Size | float | 200.0 | 100–500 | Minimap width/height (px) |
| Minimap Opacity | float | 0.8 | 0.1–1 | Minimap overall transparency |

---

## Performance

UltraWire is designed with performance as a primary concern:

- **Simple routing** (Manhattan/Subway/Freeform without Smart Routing): Negligible overhead. Pure geometry calculation with no allocation per frame.
- **Smart Routing:** Routes are cached and only recomputed when nodes move. A* has a 50 ms wall-clock timeout. Grid is capped at 512×512 cells.
- **Glow rendering:** Only visible wires are processed. The glow pass draws 4 additional line layers per wire segment.
- **Bubble animation:** Positions computed via path evaluation — no per-frame allocation.
- **Crossing detection:** O(W²·S²) with AABB pre-culling. Runs once after all wires are drawn.
- **Minimap:** Topology cached at ~10 fps. Rendering is lightweight (rectangles and lines).
- **Heatmap:** Profiler data polled at ~4 Hz. Zero overhead when the Blueprint Profiler is not running.
- **Node theming:** One-time Slate style modification on settings change. No per-frame cost.

### Recommendations for Large Graphs (200+ Nodes)

- Keep Smart Routing grid size at 20+ for reasonable computation time.
- Disable crossing detection on graphs with 100+ wires if you experience stutter.
- The minimap handles 500+ nodes at its 10 fps update rate without issues.

---

## Troubleshooting

### Wires Look the Same as Vanilla UE

- Verify the plugin is enabled: **Edit > Plugins**, search "UltraWire".
- Check **Edit > Editor Preferences > Plugins > UltraWire** and ensure `Enabled` is checked.
- Ensure `Wire Style` is not set to `Default (Bezier)` — that passes through to vanilla rendering.

### Settings Changes Don't Apply

- UltraWire uses hot-reload. Changes should apply instantly. If they don't, try closing and reopening the Blueprint graph tab.
- Verify you're editing the correct profile — if you have a graph-type override for Blueprints, changes to the Default Profile won't affect Blueprint graphs.

### Performance Issues on Large Graphs

- Disable **Smart Routing** or increase the **Routing Grid Size**.
- Disable **Crossing Symbols** — crossing detection is the most expensive per-frame operation.
- Disable **Glow** on graphs with many visible wires.
- The **Minimap** and **Bubbles** have minimal performance impact.

### Node Theming Not Visible

- Ensure `Enable Node Theming` is checked in your active profile.
- Theme changes require an `FSlateApplication::InvalidateAllWidgets()` call, which UltraWire triggers automatically. If a graph doesn't update, close and reopen the tab.

### ALT+LMB Wire Slicing Not Working

- UltraWire is designed to be fully compatible with wire slicing in UE 5.5+. If slicing doesn't work, please report it as a bug.

---

## Technical Architecture

UltraWire is organized into 6 editor modules:

| Module | Purpose |
|--------|---------|
| **UltraWireCore** | Shared types, enums, settings system (UDeveloperSettings), preset I/O, JSON serialization |
| **UltraWireRenderer** | Drawing policies, geometry engine, A* routing, bubble/glow/crossing/label renderers, pin connection factories |
| **UltraWireTheme** | Node theming engine, Slate style overrides, 5 built-in preset definitions |
| **UltraWireMinimap** | Minimap Slate widget, topology caching, click-to-navigate |
| **UltraWireProfiler** | Blueprint profiler heatmap bridge, execution data normalization |
| **UltraWireSettings** | Editor Preferences UI, detail customization, preset browser |

### Dependency Graph

```
UltraWireCore (no plugin dependencies)
    ├── UltraWireRenderer  (depends on Core)
    ├── UltraWireTheme     (depends on Core)
    ├── UltraWireMinimap   (depends on Core)
    ├── UltraWireProfiler  (depends on Core)
    └── UltraWireSettings  (depends on Core)
```

### Rendering Pipeline

For each wire drawn in the graph editor:

1. **Route Computation** — Geometry engine or A* router computes the wire path.
2. **Geometry Generation** — Path points are converted to drawable line segments with corner rounding and ribbon offsets.
3. **Style Application** — Wire colors, thickness, and heatmap modulation are applied.
4. **Glow Pass** (optional) — 4-layer additive blend halo.
5. **Bubble Animation** (optional) — Animated dots along the wire path.
6. **Crossing Detection** (post-draw) — Intersection symbols drawn after all wires complete.

---

## File Structure

```
UltraWire/
├── UltraWire.uplugin
├── Config/
│   └── DefaultUltraWire.ini
├── Content/
│   └── Presets/
│       ├── Default.ultrawire
│       ├── CircuitBoard.ultrawire
│       ├── NeonCyberpunk.ultrawire
│       ├── CleanProfessional.ultrawire
│       └── RetroTerminal.ultrawire
├── Resources/
│   └── Icons/
└── Source/
    ├── UltraWireCore/
    │   ├── Public/
    │   │   ├── UltraWireCoreModule.h
    │   │   ├── UltraWireTypes.h
    │   │   └── UltraWireSettings.h
    │   ├── Private/
    │   │   ├── UltraWireCoreModule.cpp
    │   │   └── UltraWireSettings.cpp
    │   └── UltraWireCore.Build.cs
    ├── UltraWireRenderer/
    │   ├── Public/
    │   │   ├── UltraWireRendererModule.h
    │   │   ├── UltraWireGeometry.h
    │   │   ├── UltraWireDrawingPolicy.h
    │   │   ├── UltraWirePinConnectionFactory.h
    │   │   ├── UltraWireRouteEngine.h
    │   │   ├── UltraWireBubbleSystem.h
    │   │   ├── UltraWireCrossingDetector.h
    │   │   ├── UltraWireGlowRenderer.h
    │   │   └── UltraWireLabelRenderer.h
    │   ├── Private/
    │   │   ├── UltraWireRendererModule.cpp
    │   │   ├── UltraWireGeometry.cpp
    │   │   ├── UltraWireDrawingPolicy.cpp
    │   │   ├── UltraWirePinConnectionFactory.cpp
    │   │   ├── UltraWireRouteEngine.cpp
    │   │   ├── UltraWireBubbleSystem.cpp
    │   │   ├── UltraWireCrossingDetector.cpp
    │   │   ├── UltraWireGlowRenderer.cpp
    │   │   └── UltraWireLabelRenderer.cpp
    │   └── UltraWireRenderer.Build.cs
    ├── UltraWireTheme/
    │   ├── Public/
    │   │   ├── UltraWireThemeModule.h
    │   │   ├── UltraWireThemeEngine.h
    │   │   └── UltraWirePresetLibrary.h
    │   ├── Private/
    │   │   ├── UltraWireThemeModule.cpp
    │   │   ├── UltraWireThemeEngine.cpp
    │   │   └── UltraWirePresetLibrary.cpp
    │   └── UltraWireTheme.Build.cs
    ├── UltraWireMinimap/
    │   ├── Public/
    │   │   ├── UltraWireMinimapModule.h
    │   │   └── SUltraWireMinimap.h
    │   ├── Private/
    │   │   ├── UltraWireMinimapModule.cpp
    │   │   └── SUltraWireMinimap.cpp
    │   └── UltraWireMinimap.Build.cs
    ├── UltraWireProfiler/
    │   ├── Public/
    │   │   ├── UltraWireProfilerModule.h
    │   │   └── UltraWireHeatmapBridge.h
    │   ├── Private/
    │   │   ├── UltraWireProfilerModule.cpp
    │   │   └── UltraWireHeatmapBridge.cpp
    │   └── UltraWireProfiler.Build.cs
    └── UltraWireSettings/
        ├── Public/
        │   ├── UltraWireSettingsModule.h
        │   └── UltraWireSettingsCustomization.h
        ├── Private/
        │   ├── UltraWireSettingsModule.cpp
        │   └── UltraWireSettingsCustomization.cpp
        └── UltraWireSettings.Build.cs
```


---

## Support

- **Publisher:** StraySpark
- **Email:** (configured in Fab Publisher Profile)
- **Bug Reports:** Please include your UE version, OS, a description of the issue, and steps to reproduce.
- **Feature Requests:** We welcome feedback and feature suggestions.

---

*Copyright 2026 StraySpark. All Rights Reserved.*
*UltraWire v1.0.0 for Unreal Engine 5.7*
