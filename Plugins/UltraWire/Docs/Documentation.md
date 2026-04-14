# UltraWire Documentation

**Version 1.0.0 | StraySpark | Unreal Engine 5.7**

---

# 1. Introduction

UltraWire is an editor-only code plugin for Unreal Engine 5 that completely overhauls how graph editors look and feel. It customizes wire routing, node appearance, and provides productivity tools like a minimap and execution heatmap — all from a single, unified settings panel.

### What UltraWire Does

- Replaces Unreal's default curved (Bezier) wires with clean geometric routing (Manhattan 90°, Subway 45°, or custom angles).
- Routes wires intelligently around node bodies using A* pathfinding.
- Adds visual effects: glow, animated bubbles, crossing indicators.
- Themes graph nodes: header colors, body opacity, pin shapes, comment boxes.
- Provides a minimap overlay for navigating large graphs.
- Visualizes Blueprint execution frequency as a wire color heatmap.
- Supports different visual profiles per graph type.
- Enables team-wide visual standards via shareable presets and project config files.

### What UltraWire Does NOT Do

- It does not modify runtime behavior. No game code is affected.
- It does not change Blueprint logic or node functionality.
- It does not modify any engine source files — all changes are reversible.
- It does not affect shipping builds. All modules are Editor-only.

---

# 2. Installation

## 2.1 From Fab (Recommended)

1. Purchase UltraWire on the Fab marketplace.
2. In the Epic Games Launcher, navigate to your engine version's plugin library.
3. UltraWire will appear under installed plugins. Enable it for your project.
4. Launch your project. The plugin is active immediately.

## 2.2 Manual Installation

1. Extract the `UltraWire` folder into your project's `Plugins` directory.
2. Your folder structure should be:
   ```
   YourProject/
   ├── Config/
   ├── Content/
   ├── Plugins/
   │   └── UltraWire/
   │       ├── UltraWire.uplugin
   │       ├── Source/
   │       ├── Config/
   │       ├── Content/
   │       └── Resources/
   └── YourProject.uproject
   ```
3. Right-click your `.uproject` file and select **Generate Visual Studio project files**.
4. Open the project in Unreal Editor or build from your IDE.

## 2.3 Verifying Installation

1. Open your project in Unreal Editor.
2. Go to **Edit > Plugins**.
3. Search for "UltraWire".
4. Confirm it appears under the **Editor** category with a checkmark.
5. Go to **Edit > Editor Preferences > Plugins > UltraWire** to access settings.

---

# 3. Getting Started

## 3.1 Opening the Settings Panel

Navigate to **Edit > Editor Preferences**, then in the left sidebar expand **Plugins** and click **UltraWire**.

The settings panel is organized into these sections:

- **General** — Master enable toggle
- **Default Profile** — Visual configuration applied to all graph types
- **Graph Type Profiles** — Per-graph-type overrides
- **Presets** — Save, load, import, and export presets
- **Built-in Presets** — Apply one of 5 factory presets

## 3.2 Applying Your First Preset

The fastest way to see UltraWire in action:

1. Open the UltraWire settings panel.
2. Scroll to the **Built-in Presets** section.
3. Select **"Neon Cyberpunk"** from the dropdown.
4. Click **Apply**.
5. Open any Blueprint graph.

You will immediately see glowing, pulsing wires with animated bubbles.

## 3.3 Choosing a Wire Style

Under **Default Profile > Wire Style**, select one of:

| Style | Look |
|-------|------|
| **Default (Bezier)** | Standard UE curves. UltraWire effects still apply (glow, bubbles, etc.). |
| **Manhattan (90°)** | Right-angle segments like PCB traces. Clean and orderly. |
| **Subway (45°)** | Diagonal segments. A balance between curves and straight lines. |
| **Freeform** | Custom exit angle (5°–85°). Set with the **Freeform Angle** slider. |

## 3.4 Hot Reload

All settings changes apply instantly. You do not need to restart the editor, close tabs, or recompile. Simply change a value and look at your graph — it updates in real time.

---

# 4. Wire Routing

## 4.1 Manhattan Routing (90°)

Wires travel exclusively along horizontal and vertical paths with 90° turns. The routing algorithm determines optimal bend placement:

- **Normal connection** (output is left of input): Exit horizontally from the output pin, bend vertically, bend horizontally into the input pin.
- **Backward connection** (output is right of input): The wire extends beyond both pins, loops around with vertical segments, and enters from the correct side.
- **Same-height pins:** A single horizontal segment connects them directly.
- **Zero-length wire:** Handled gracefully as a degenerate case.

## 4.2 Subway Routing (45°)

Wires exit horizontally, transition through a 45° diagonal, then enter horizontally. This creates exactly 3 segments per wire in the normal case.

- The horizontal "exit stub" is proportional to the horizontal distance, giving a balanced look.
- Backward connections use extended geometry to maintain the 45° aesthetic.

## 4.3 Freeform Routing

Identical to Subway routing, but the diagonal angle is user-defined:

- **Freeform Angle** slider: 5° (nearly horizontal) to 85° (nearly vertical).
- The angle is clamped to prevent degenerate geometry.
- At 45°, Freeform is identical to Subway.

## 4.4 Corner Styles

Applies to Manhattan, Subway, and Freeform routing:

### Sharp Corners

No modification to corner points. Produces crisp geometric intersections. Best for PCB/Circuit Board aesthetics.

### Rounded Corners

Each corner is replaced by a circular arc. The **Corner Radius** setting controls the arc size (0–50 px). Each arc is approximated with 8 line segments for smooth rendering. If the corner radius exceeds the available segment length, it is automatically clamped.

### Chamfered Corners

Each corner is replaced by a diagonal cut. The **Corner Radius** setting controls the chamfer size. This creates a beveled appearance.

## 4.5 Smart Auto-Routing (A* Pathfinding)

When **Enable Smart Routing** is checked, UltraWire replaces the simple geometric routing with an A* pathfinding algorithm that routes wires around node bodies.

### Configuration

| Setting | Description | Performance Impact |
|---------|-------------|-------------------|
| **Routing Grid Size** | Cell size for the pathfinding grid (5–50 px). Smaller values give more precise routing but are slower. Default 20 px is recommended. | Smaller = slower |
| **Routing Node Padding** | Clearance around node bodies (0–50 px). Wires will maintain at least this distance from any node edge. | Minimal impact |

### Algorithm Details

1. A bounding box is computed over all visible nodes, expanded by a border margin.
2. The bounding box is divided into a grid of square cells.
3. Cells overlapping node rectangles (plus padding) are marked as blocked.
4. A* search finds the shortest path from output pin to input pin through unblocked cells.
5. For Manhattan style: only 4-directional movement is allowed.
6. For Subway/Freeform: 8-directional movement is allowed (with corner-cutting prevention).
7. The grid path is smoothed by removing collinear points.
8. Corner styling is applied to the smoothed path.

### Failsafe Mechanisms

- **50 ms timeout:** If A* cannot find a path within 50 ms, it silently falls back to simple geometric routing. You will never experience a freeze.
- **Grid cap:** The grid is limited to 512×512 cells maximum. If the graph canvas would require a larger grid, cell size is automatically increased.
- **Route caching:** Computed routes are stored in a hash-based cache. Routes are only recomputed when nodes move. Moving a single node only invalidates routes connected to that node.

### When to Use Smart Routing

Smart Routing works best on medium-sized graphs (20–200 nodes) where wire-node overlap is a readability problem. On very small graphs, the visual difference is minimal. On very large graphs (500+), consider increasing the grid size to 30–50 to maintain performance.

---

# 5. Wire Effects

## 5.1 Wire Ribbons

When multiple output pins on one node connect to input pins on another node, enabling **Ribbons** offsets the wires perpendicular to their path so they run in parallel, like a ribbon cable.

**Settings:**
- `Enable Ribbons` — on/off
- `Ribbon Offset` — spacing between wires (1–20 px)

The offset is calculated per pin index relative to the center:
- 1 wire: no offset (centered)
- 2 wires: offset ±half the ribbon offset
- N wires: evenly distributed around the center path

## 5.2 Crossing Symbols

When two wires cross, an indicator can be drawn at the intersection point:

- **None:** Wires simply overlap. No indicator.
- **Gap:** A break is drawn in the background wire, creating the visual illusion that it passes underneath.
- **Arc:** A semicircular bump is drawn on one wire, as if it jumps over the other.
- **Circle:** A filled circle is drawn at the intersection, like a solder joint on a PCB.

**Settings:**
- `Crossing Style` — None / Gap / Arc / Circle
- `Crossing Size` — diameter of the symbol (3–20 px)

Crossing detection uses a segment-by-segment intersection test across all wire pairs. An AABB (axis-aligned bounding box) pre-cull reduces unnecessary segment comparisons.

## 5.3 Moving Bubbles

Animated dots that travel along wires from output to input, indicating data flow direction.

**Settings:**
- `Enable Bubbles` — on/off
- `Bubble Speed` — 10–500 px/sec. Higher = faster animation.
- `Bubble Size` — 1–10 px radius. The dots are drawn as filled circles.
- `Bubble Spacing` — 10–200 px. Distance between consecutive dots along the wire path.

Bubble positions are computed by evaluating the wire's polyline path at regular parameter intervals, offset by `CurrentTime * Speed`. This means bubbles follow the exact wire geometry, including around corners and through bends.

## 5.4 Wire Glow and Neon

An additive-blend glow effect creates a luminous halo around wires.

**How it renders:** Four concentric line layers are drawn on top of the wire, each progressively wider and more transparent. The innermost layer is 25% wider than the wire and nearly opaque; the outermost layer is the full glow width and nearly transparent. This produces a natural bloom-like falloff.

**Settings:**
- `Enable Glow` — on/off
- `Glow Intensity` — 0–5. Multiplies the alpha of all glow layers.
- `Glow Width` — 1–20 px. The width of the outermost glow layer.
- `Glow Pulse` — on/off. Enables sinusoidal intensity modulation.
- `Glow Pulse Speed` — 0.1–5 Hz. How fast the glow pulses.

The pulse animation maps a sine wave to the range [0.25, 1.0], so the glow dims and brightens rhythmically but never fully disappears.

The glow color is automatically derived from the wire's base color. If heatmap mode is active, the glow color reflects the heat gradient.

---

# 6. Node Theming

## 6.1 Overview

UltraWire can modify the visual appearance of graph nodes by overriding Unreal's Slate styles. This allows you to customize:

- Node body background color and opacity
- Node title bar (header) tint color
- Node corner rounding
- Pin connection point shapes
- Comment box colors

## 6.2 Settings

| Setting | Range | Description |
|---------|-------|-------------|
| Enable Node Theming | on/off | Master toggle. When off, nodes use vanilla UE appearance. |
| Node Body Opacity | 0–1 | Transparency of the node body. 0 = fully transparent, 1 = fully opaque. |
| Node Header Tint Color | Color picker | Tint applied to the node title bar. White = no change. |
| Node Corner Radius | 0–16 | Rounding of node body corners in pixels. |
| Pin Shape | Circle / Diamond / Square / Arrow | Shape of pin connection points. |

## 6.3 How Theming Works

The theme engine operates by:

1. **Snapshotting** original Slate brushes for all affected style keys the first time theming is applied. These originals are stored in memory.
2. **Creating modified copies** of each brush with the requested tint, opacity, or shape changes.
3. **Writing the modified brushes** into `FAppStyle` (the global Slate style registry).
4. **Invalidating all Slate widgets** to force a visual refresh.

When theming is disabled (or the plugin is unloaded), all original brushes are restored.

## 6.4 Affected Slate Style Keys

| Style Key | What It Controls |
|-----------|-----------------|
| `Graph.Node.Body` | Node body background |
| `Graph.Node.TitleGloss` | Glossy overlay on the title bar |
| `Graph.Node.ColorSpill` | Color region behind the title |
| `Graph.Node.TitleHighlight` | Highlight accent on the title |
| `Graph.Node.Shadow` | Drop shadow behind the node |
| `Graph.Node.ShadowSize` | Shadow size |
| `Graph.Node.PinPoint` | Pin connection point marker |
| `Graph.Node.PinDiffIndicator` | Pin diff indicator |
| `Graph.CommentBubble.Body` | Comment node body |
| `Graph.CommentBubble.Border` | Comment node border |

## 6.5 Per-Graph Theming

Since theming is part of the profile system, you can have different node themes per graph type. For example, you could have:
- Dark nodes with 70% opacity in Blueprint graphs
- Full opacity with green header tints in Niagara graphs
- Minimal theming in Material graphs

---

# 7. Graph Minimap

## 7.1 Overview

The minimap is a picture-in-picture overlay that appears in the corner of graph editor tabs. It shows a bird's-eye view of the entire graph with:

- **Nodes** as solid colored rectangles (colored by node category)
- **Wires** as thin colored lines
- **Your viewport** as a translucent outlined rectangle
- **Comment boxes** as outlined rectangles with a distinct color

## 7.2 Click-to-Navigate

Click anywhere on the minimap to instantly pan the graph editor to that location. The graph centers on the clicked point. This is invaluable for navigating large graphs without scrolling or zooming out manually.

## 7.3 Settings

| Setting | Range | Description |
|---------|-------|-------------|
| Enable Minimap | on/off | Show or hide the minimap |
| Minimap Position | Corner selector | TopLeft, TopRight, BottomLeft, BottomRight |
| Minimap Size | 100–500 px | Width and height of the minimap |
| Minimap Opacity | 0.1–1.0 | Overall transparency |

## 7.4 Node Colors on the Minimap

Nodes on the minimap are colored using their `GetNodeTitleColor()` value, which corresponds to their category:
- **Blue:** Function calls
- **Red:** Events
- **Green:** Flow control (Branch, Sequence, etc.)
- **Gray:** Default/uncategorized

Wire colors on the minimap are derived from pin type:
- **White:** Execution pins (PC_Exec)
- **Red:** Boolean
- **Blue:** Integer / Object
- **Green:** Float
- **Teal:** Struct

Comment nodes are rendered as outlined rectangles with a yellow-green color so they're visually distinct.

## 7.5 Performance

The minimap caches the graph topology and refreshes at approximately **10 fps** (every 100 ms). This means:
- Moving nodes updates on the minimap within 100 ms.
- Adding or removing nodes is reflected within 100 ms.
- The cache prevents expensive per-frame iteration over graph nodes.
- On graphs with 500+ nodes, the minimap remains responsive.

---

# 8. Execution Heatmap

## 8.1 Overview

The execution heatmap visualizes Blueprint execution frequency directly on the graph canvas. Wires connected to frequently-executed nodes glow hot (red), while rarely-executed paths stay cold (blue). This provides instant visual feedback about performance hotspots.

## 8.2 How to Use

1. Enable **Heatmap** in UltraWire settings.
2. Open a Blueprint graph.
3. Start the **Blueprint Profiler**: **Window > Developer Tools > Blueprint Profiler**, or use the profiler button in the Blueprint Debugger toolbar.
4. Start a **Play In Editor (PIE)** session or run a Blueprint debugging session.
5. As the game runs, watch the wires change color in real time.

## 8.3 How It Works

UltraWire integrates with Unreal's `FBlueprintCoreDelegates` profiling system:

1. When the Blueprint Profiler is enabled, UltraWire's heatmap bridge activates automatically.
2. The bridge polls profiler data at ~4 Hz (every 250 ms).
3. Raw execution counts per node/pin are accumulated across the session.
4. Counts are normalized to a 0–1 range (0 = coldest, 1 = hottest).
5. The normalized heat value is mapped to a color gradient using HSV interpolation.
6. During wire rendering, the heat color replaces or modulates the wire's base color.

## 8.4 Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Enable Heatmap | off | Master toggle |
| Heatmap Cold Color | Blue (0, 0.2, 1) | Color for minimum execution frequency |
| Heatmap Hot Color | Red (1, 0.1, 0) | Color for maximum execution frequency |

You can customize the gradient endpoints. For example:
- White-to-Red for a clean heat look
- Green-to-Yellow for a traffic light style
- Blue-to-White for a cool palette

## 8.5 Zero Overhead

When the Blueprint Profiler is not running, the heatmap system is completely dormant:
- No profiler delegates are active.
- No data polling occurs.
- `GetHeatForPin()` returns 0.0 immediately.
- There is zero CPU cost.

---

# 9. Wire Labels

## 9.1 Auto-Labels

When **Enable Auto Labels** is checked, UltraWire automatically adds text labels to wires connected to Get/Set variable nodes. The label displays the variable name, turning this:

```
[Get PlayerHealth] ──────────── [Branch]
```

Into this:

```
[Get PlayerHealth] ── PlayerHealth ── [Branch]
```

## 9.2 Label Rendering

Labels are rendered at the midpoint of the wire path with:
- A semi-transparent dark background (55% opacity) for readability.
- White text that scales with the graph zoom level.
- Horizontal/vertical padding around the text.
- An offset above the wire to avoid overlapping.

---

# 10. Per-Graph-Type Profiles

## 10.1 Concept

Every visual setting in UltraWire is stored in a **Profile** (`FUltraWireProfile`). The plugin maintains:

- One **Default Profile** that applies to all graph types.
- Optional **per-graph-type overrides** that replace the default for specific graph editors.

## 10.2 Supported Graph Types

| Graph Type | Unreal Editor |
|------------|---------------|
| Blueprint | Blueprint Editor (EventGraph, ConstructionScript, Functions, Macros) |
| Material | Material Editor |
| Niagara | Niagara System/Emitter Editor |
| Animation Blueprint | AnimGraph, State Machine |
| Behavior Tree | Behavior Tree Editor |
| Control Rig | Control Rig Editor |
| PCG | Procedural Content Generation Graph |
| Sound Cue | Sound Cue Editor |
| MetaSound | MetaSound Editor |
| Environment Query | EQS Editor |
| Gameplay Ability | Gameplay Ability Graph |
| Other | Any graph type not listed above |

## 10.3 Configuring Per-Graph Profiles

1. Go to **Editor Preferences > Plugins > UltraWire > Graph Type Profiles**.
2. Each graph type has a dropdown selector.
3. Select a preset to assign to that graph type, or leave it as "Default" to inherit from the Default Profile.
4. Changes apply immediately to any open graph of that type.

## 10.4 Example Configuration

| Graph Type | Assigned Preset | Rationale |
|------------|----------------|-----------|
| Blueprint | Circuit Board | Clean PCB-style routing for complex logic graphs |
| Material | Clean Professional | Minimal look for shader graphs that are usually simpler |
| Niagara | Neon Cyberpunk | Dramatic glow for particle system visualization |
| Animation Blueprint | Default | Subtle improvements for AnimGraphs |
| All Others | (inherit Default) | No override needed |

---

# 11. Presets

## 11.1 Built-in Presets

UltraWire ships with 5 presets:

### Default
- **Wire Style:** Manhattan (90°)
- **Corners:** Rounded, 8 px radius
- **Effects:** None (no glow, no bubbles)
- **Theming:** Subtle (92% body opacity)
- **Best for:** Users who want cleaner wires without dramatic changes

### Circuit Board
- **Wire Style:** Manhattan (90°), sharp corners
- **Effects:** Gap crossings, ribbon stacking
- **Theming:** Square pin shapes, green header tint
- **Extras:** Smart routing enabled, auto-labels on
- **Best for:** Users who like the look of electronic circuit boards

### Neon Cyberpunk
- **Wire Style:** Default (Bezier), rounded 14 px corners
- **Effects:** High-intensity pulsing glow (2.5), animated bubbles, arc crossings
- **Theming:** Diamond pins, 85% body opacity
- **Best for:** Streams, tutorials, showcases, YouTube content

### Clean Professional
- **Wire Style:** Subway (45°)
- **Effects:** None (no glow, no bubbles, no animations)
- **Theming:** Full opacity, clean appearance
- **Best for:** Documentation screenshots, code reviews, presentations

### Retro Terminal
- **Wire Style:** Freeform (custom angle)
- **Effects:** Soft glow (1.2), slow bubbles
- **Theming:** Square pins, sharp corners, amber/green palette
- **Best for:** Retro aesthetic enthusiasts

## 11.2 Saving Custom Presets

1. Configure your desired settings in the Default Profile.
2. In the **Presets** section of the settings panel, enter a name for your preset.
3. Click **Save**.
4. Your preset appears in the preset dropdown for future use.

## 11.3 Exporting Presets

1. Select the preset you want to export from the dropdown.
2. Click **Export**.
3. Choose a save location and filename.
4. The preset is saved as a `.ultrawire` JSON file.

## 11.4 Importing Presets

1. Click **Import** in the Presets section.
2. Select a `.ultrawire` file.
3. The preset is imported and added to your saved presets.

## 11.5 The `.ultrawire` File Format

Presets are stored as JSON files with the extension `.ultrawire`. The format is:

```json
{
    "PresetName": "My Custom Style",
    "Description": "A brief description of this preset",
    "Author": "YourName",
    "Profile": {
        "WireStyle": "Manhattan",
        "FreeformAngle": 30.0,
        "CornerStyle": "Rounded",
        "CornerRadius": 12.0,
        "WireThickness": 2.0,
        "bEnableRibbons": true,
        "RibbonOffset": 5.0,
        "CrossingStyle": "Gap",
        "CrossingSize": 8.0,
        "bEnableBubbles": false,
        "BubbleSpeed": 100.0,
        "BubbleSize": 3.0,
        "BubbleSpacing": 50.0,
        "bEnableGlow": true,
        "GlowIntensity": 1.5,
        "GlowWidth": 8.0,
        "bGlowPulse": false,
        "GlowPulseSpeed": 1.0,
        "bEnableSmartRouting": false,
        "RoutingGridSize": 20,
        "RoutingNodePadding": 10.0,
        "bEnableHeatmap": false,
        "HeatmapColdColor": {"R": 0.0, "G": 0.2, "B": 1.0, "A": 1.0},
        "HeatmapHotColor": {"R": 1.0, "G": 0.1, "B": 0.0, "A": 1.0},
        "bEnableNodeTheming": true,
        "NodeBodyOpacity": 0.9,
        "NodeHeaderTintColor": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0},
        "NodeCornerRadius": 6.0,
        "PinShape": "Diamond",
        "bEnableAutoLabels": false,
        "bEnableMinimap": false,
        "MinimapPosition": "BottomRight",
        "MinimapSize": 200.0,
        "MinimapOpacity": 0.8
    }
}
```

All profile fields are optional in the JSON — omitted fields use their default values.

---

# 12. Team Collaboration

## 12.1 Project-Level Defaults

UltraWire supports a project-level configuration file that sets team-wide defaults.

**File:** `YourProject/Config/DefaultUltraWire.ini`

When a developer opens the project with UltraWire installed, the plugin reads this INI file and applies the settings as defaults. Individual developers can then override settings locally.

### Setting Up Team Defaults

1. Configure UltraWire to your team's preferred visual standard.
2. Locate the generated settings in `YourProject/Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` under the `[/Script/UltraWireCore.UltraWireSettings]` section.
3. Copy that section into `YourProject/Config/DefaultUltraWire.ini`.
4. Commit the file to source control.

Alternatively, edit `DefaultUltraWire.ini` directly. An example file is included with the plugin at `UltraWire/Config/DefaultUltraWire.ini` with all available settings documented.

### INI File Format

```ini
[/Script/UltraWireCore.UltraWireSettings]
bEnabled=True
DefaultProfile.WireStyle=Manhattan
DefaultProfile.CornerStyle=Rounded
DefaultProfile.CornerRadius=10.0
DefaultProfile.WireThickness=1.5
DefaultProfile.bEnableRibbons=False
DefaultProfile.RibbonOffset=4.0
DefaultProfile.CrossingStyle=None
DefaultProfile.bEnableBubbles=False
DefaultProfile.bEnableGlow=False
DefaultProfile.bEnableSmartRouting=False
DefaultProfile.bEnableHeatmap=False
DefaultProfile.bEnableNodeTheming=False
DefaultProfile.bEnableMinimap=False
```

## 12.2 Sharing Presets via Source Control

The recommended workflow for teams:

1. Create a `Config/UltraWirePresets/` directory in your project.
2. Export your team's standard presets as `.ultrawire` files into this directory.
3. Commit the directory to source control.
4. New team members import the presets on first setup.

## 12.3 Local Overrides

Individual developers' personal preferences are stored in `EditorPerProjectUserSettings.ini`, which is typically excluded from source control (listed in `.gitignore`). This means:

- Team defaults propagate to everyone via `DefaultUltraWire.ini`.
- Individual preferences stay local and don't create merge conflicts.
- If a developer prefers a different style, they simply change their settings — it doesn't affect the team.

---

# 13. Complete Settings Reference

## 13.1 All Profile Properties

| Property | Type | Default | Range | Category |
|----------|------|---------|-------|----------|
| WireStyle | Enum | Manhattan | Default/Manhattan/Subway/Freeform | Wire Style |
| FreeformAngle | float | 30.0 | 5–85 | Wire Style |
| CornerStyle | Enum | Rounded | Sharp/Rounded/Chamfered | Wire Style |
| CornerRadius | float | 10.0 | 0–50 | Wire Style |
| WireThickness | float | 1.5 | 0.5–10 | Wire Style |
| bEnableRibbons | bool | false | — | Ribbons |
| RibbonOffset | float | 4.0 | 1–20 | Ribbons |
| CrossingStyle | Enum | None | None/Gap/Arc/Circle | Crossings |
| CrossingSize | float | 8.0 | 3–20 | Crossings |
| bEnableBubbles | bool | false | — | Bubbles |
| BubbleSpeed | float | 100.0 | 10–500 | Bubbles |
| BubbleSize | float | 3.0 | 1–10 | Bubbles |
| BubbleSpacing | float | 50.0 | 10–200 | Bubbles |
| bEnableGlow | bool | false | — | Glow |
| GlowIntensity | float | 1.0 | 0–5 | Glow |
| GlowWidth | float | 6.0 | 1–20 | Glow |
| bGlowPulse | bool | false | — | Glow |
| GlowPulseSpeed | float | 1.0 | 0.1–5 | Glow |
| bEnableSmartRouting | bool | false | — | Smart Routing |
| RoutingGridSize | int32 | 20 | 5–50 | Smart Routing |
| RoutingNodePadding | float | 10.0 | 0–50 | Smart Routing |
| bEnableHeatmap | bool | false | — | Heatmap |
| HeatmapColdColor | FLinearColor | (0, 0.2, 1, 1) | Color | Heatmap |
| HeatmapHotColor | FLinearColor | (1, 0.1, 0, 1) | Color | Heatmap |
| bEnableNodeTheming | bool | false | — | Node Theme |
| NodeBodyOpacity | float | 1.0 | 0–1 | Node Theme |
| NodeHeaderTintColor | FLinearColor | White | Color | Node Theme |
| NodeCornerRadius | float | 4.0 | 0–16 | Node Theme |
| PinShape | Enum | Circle | Circle/Diamond/Square/Arrow | Node Theme |
| bEnableAutoLabels | bool | false | — | Labels |
| bEnableMinimap | bool | false | — | Minimap |
| MinimapPosition | Enum | BottomRight | TL/TR/BL/BR | Minimap |
| MinimapSize | float | 200.0 | 100–500 | Minimap |
| MinimapOpacity | float | 0.8 | 0.1–1 | Minimap |

## 13.2 Global Settings

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| bEnabled | bool | true | Master enable for the entire plugin |
| DefaultProfile | FUltraWireProfile | (see above) | Default visual profile |
| GraphProfileOverrides | TMap | empty | Per-graph-type profile overrides |
| SavedPresets | TArray | empty | User-saved preset list |

---

# 14. Performance Guide

## 14.1 Feature Performance Matrix

| Feature | CPU Cost | When Active |
|---------|----------|-------------|
| Wire routing (Manhattan/Subway/Freeform) | Negligible | Always (when style is not Default) |
| Corner rounding | Negligible | Always (when corner style is not Sharp) |
| Wire ribbons | Negligible | When enabled |
| Crossing detection | Moderate (O(W^2 * S^2)) | When Crossing Style is not None |
| Moving bubbles | Low | When enabled |
| Wire glow | Low-Moderate (4x line draws) | When enabled |
| Smart routing (A*) | Moderate-High (cached) | When enabled |
| Node theming | Zero per-frame | Applied once on settings change |
| Minimap | Low (~10 fps cache) | When enabled |
| Heatmap | Low (~4 Hz polling) | Only when Blueprint Profiler is running |
| Wire labels | Low | When enabled |

## 14.2 Recommendations by Graph Size

### Small Graphs (< 50 nodes)
All features can be enabled simultaneously without noticeable performance impact.

### Medium Graphs (50–200 nodes)
- All features work well.
- Smart Routing is most beneficial here — it significantly improves readability.
- Default grid size (20) is appropriate.

### Large Graphs (200–500 nodes)
- Consider increasing **Routing Grid Size** to 30–40 if using Smart Routing.
- Consider disabling **Crossing Detection** if you have many wires (100+).
- The minimap remains responsive.
- Glow adds visible rendering cost with many wires.

### Very Large Graphs (500+ nodes)
- Disable **Smart Routing** or increase grid size to 40–50.
- Disable **Crossing Detection**.
- Disable **Glow** if too many wires are visible.
- The minimap handles this size at its 10 fps rate.
- Consider using simple Manhattan routing without effects.

---

# 15. Troubleshooting

## 15.1 Plugin Not Loading

| Symptom | Solution |
|---------|----------|
| Plugin not visible in Plugin Browser | Verify the `UltraWire` folder is in `Plugins/` with a valid `UltraWire.uplugin` file |
| Plugin visible but grayed out | Check the engine version — UltraWire requires UE 5.5+ |
| Compilation errors on startup | Ensure all 6 Source modules are present. Regenerate project files. |

## 15.2 Visual Issues

| Symptom | Solution |
|---------|----------|
| Wires look unchanged | Check `bEnabled` is true. Check wire style is not "Default (Bezier)". |
| Settings not applying | Verify you're editing the correct profile (default vs. graph-type override). |
| Node theming not visible | Enable `bEnableNodeTheming`. Close and reopen the graph tab if needed. |
| Minimap not showing | Enable `bEnableMinimap`. The minimap injects into graph tabs when they gain focus — try clicking on the graph tab. |
| Heatmap not coloring wires | Ensure the Blueprint Profiler is running and PIE is active. Enable `bEnableHeatmap`. |

## 15.3 Performance Issues

| Symptom | Solution |
|---------|----------|
| Graph feels sluggish | Disable Smart Routing or increase grid size. Disable crossing detection. |
| Brief freeze when opening large graph | Smart Routing computing initial routes. This only happens once; routes are then cached. |
| Minimap causing stutter | Should not happen (10 fps cache). If it does, increase `MinimapSize` (reduces resolution complexity). |

## 15.4 Compatibility

| Issue | Notes |
|-------|-------|
| ALT+LMB wire slicing | Fully supported in UE 5.5+. If broken, report as a bug. |
| Third-party graph editor plugins | UltraWire uses the standard `FGraphPanelPinConnectionFactory` system. It should coexist with plugins that don't also override wire drawing. Conflicts may occur with Electronic Nodes if both are installed. |
| Custom graph types (from other plugins) | UltraWire's "Other" factory provides fallback support. Wire routing will work, but the heatmap and some graph-specific features may not apply. |

---

# 16. Uninstallation

## From Fab

1. Open the Epic Games Launcher.
2. Navigate to your library.
3. Remove UltraWire from your installed plugins.

## Manual

1. Delete the `Plugins/UltraWire` folder from your project.
2. Regenerate project files.
3. All visual changes revert automatically — no engine files are modified.

If you had `DefaultUltraWire.ini` in your project's Config folder, you may optionally delete it. It has no effect without the plugin installed.

---

# 17. Version History

## v1.0.0 (Initial Release)

- 4 wire routing styles: Default (Bezier), Manhattan (90°), Subway (45°), Freeform (custom angle)
- 3 corner styles: Sharp, Rounded, Chamfered
- Wire ribbons for multi-wire stacking
- 3 crossing symbol styles: Gap, Arc, Circle
- Moving bubble animation
- 4-layer additive glow with pulse animation
- A* smart auto-routing with 50 ms timeout and route caching
- Node theming engine with header/body/pin/comment customization
- Graph minimap overlay with click-to-navigate
- Execution heatmap via Blueprint Profiler integration
- Wire auto-labels for Get/Set variable nodes
- Per-graph-type visual profiles for 12+ graph types
- 5 built-in presets: Default, Circuit Board, Neon Cyberpunk, Clean Professional, Retro Terminal
- Preset save/load/import/export as .ultrawire JSON files
- Team collaboration via DefaultUltraWire.ini project defaults
- Hot reload — all changes apply instantly
- Full ALT+LMB wire slicing support in UE 5.5+
- Support for UE 5.5, 5.6, 5.7
- Platforms: Win64, Mac, Linux

---

*Copyright 2026 StraySpark. All Rights Reserved.*
*UltraWire v1.0.0 for Unreal Engine 5.7*
