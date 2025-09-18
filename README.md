## dansweeper

beta state experimental minesweeper initially intended for testing various solver implementations, but have deviated to release as a just a game for players. Solver implementations and machine learning version can be found [here](https://github.com/denialpan/dansweeper-ml).

### features

![image](https://github.com/user-attachments/assets/0fa369c5-a810-47c6-aabc-21447ffefd7a)

texture credit: https://github.com/Minesweeper-World/MS-Texture

---

#### this game features:

- custom 5x5 to 250x250 board dimensions and presets
- deterministic seed hashing
- safe first click _(seeded input generation does not use first click for replayability)_
- chording
- endgame stats with millisecond timer
- panning and pixel perfect zooming (no texture blurring)

#### controls:

- `left mouse` - reveal tile
- `right mouse` - flag tile
- `middle mouse` - chord
- `middle mouse drag` - pan grid around window
- `middle mouse scroll` - zoom
- `f3` - debug ahh minecraft screen
- _note: board seed is automatically copied to clipboard when clicking_

#### showcase:

##### basic gameplay
https://github.com/user-attachments/assets/efa116e9-f89b-4106-a741-9f2f353f57a9
##### extreme board generation
https://github.com/user-attachments/assets/b593123b-cc41-415e-aedb-1b8697e8611c
##### first click guaranteed, then not in seed
https://github.com/user-attachments/assets/731caf56-2ff0-424a-8f4f-6ac0402d2a4c

---

#### download:

https://github.com/denialpan/dansweeper/releases

---

#### some maybe todo or future ideas:

- control rebinding
- demo playback
- clean up ui

#### building or modifying

f5 in vscode. there was a starter template that i built this off that had `tasks.json` properly set up. elsewhere in other IDEs im not too sure.
