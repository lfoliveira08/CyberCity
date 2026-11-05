# Campus_Map -- top-level HMI

Both buildings side by side, all four systems each, with per-system status and an event log panel.

Color coding (from `Piranga_Workflow_v2.md` Section 10):

| Color | Meaning |
|---|---|
| Green | Normal operation |
| Yellow | Warning / degraded communication |
| Red | Intrusion / compromised |
| Blue | Active communication |

Heartbeat rule: frozen about 3 s = yellow, about 5 s = red, resumed = blue briefly, then green. Log every lost/reconnected event with timestamp and downtime; use `RECONNECT_COUNT` to tell a WiFi drop from a reboot.

Status: see ../../STATUS.md.

## Layout idea (Nov 2, 2026, mockup only, nothing built in Ignition)

A web (Perspective-style) campus map: an image of each building with one dot per system; the dot color follows the `Status` member of that system's `Device` instance (0 green, 1 yellow, 2 red; see `../Templates/README.md`); clicking a dot opens that system's detail view. Perspective pieces read in the manual: Image (`appendix/components/perspective-components/perspective-display-palette/perspective-image.md`), Coordinate Container for overlaying the dots (`.../perspective-container-palette/perspective-coordinate-container.md`, percent mode), Popup Views with parameters (`ignition-modules/perspective/views-in-perspective/popup-views.md`, `system.perspective.openPopup`). **Not read:** the Image Management Tool upload steps, and the component for the dot. **Decided Nov 2, 2026 (Luis): Perspective.** Vision may be considered after the Dec 11 demo; its screens would be rebuilt by hand, tags and UDT carry over.

Dot positions chosen by Luis (Nov 2, 2026) by dragging dots on a **stylised redraw** of each building made in a chat mockup, not on his real images. They are rough and must be placed again on the real images. The real images were seen in chat (3D grey models of BSH, three floors, and FH, about nine floors); they are not in this folder.

| Building | System | x, y on a 380 x 330 sketch | As a share of the sketch |
|---|---|---|---|
| BSH | BSH-Elevator | 63, 87 | 17 % , 26 % |
| BSH | BSH-FireAlarm | 87, 276 | 23 % , 84 % |
| BSH | BSH-Door | 321, 264 | 84 % , 80 % |
| BSH | BSH-PowerBox | 199, 186 | 52 % , 56 % |
| FH | FH-HVAC | 64, 238 | 17 % , 72 % |
| FH | FH-FireAlarm | 309, 101 | 81 % , 31 % |
| FH | FH-Door | 295, 284 | 78 % , 86 % |
| FH | FH-PowerBox | 175, 191 | 46 % , 58 % |
