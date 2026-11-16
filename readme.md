# Monopoly Automated Banker

An Arduino-powered banker for Monopoly. It takes in paper money, sorts each bill by colour into a rotating carousel, and pays out change, so nobody has to sit out as the banker.

ENSC 100 project, Fall 2026.

## Must knows

Four rules with no exceptions. If you only read one part of this README, read this.

1. **Pull from `main` before you start, and at least once a day while you work.** Run `git checkout main` then `git pull`. If you're partway through a task, switch back to your branch and run `git merge main` to bring it up to date. The longer you go without pulling, the bigger the merge conflicts get.
2. **Never commit or push directly to `main`.** Every change goes on its own branch and in through a pull request, even a one-line fix.
3. **Never push code you haven't built and tested.** Run `pio test -e native` and `pio run` first, and never merge a pull request while CI shows a red ✗.
4. **Never change a frozen header without asking first.** The `I*.h` files in `lib/hal/` and the headers in `lib/core/` are shared by the whole team. Talk to the file's owner and Felix before you edit one.

**Contents**

1. [Team and who owns what](#1-team-and-who-owns-what)
2. [Code structure](#2-code-structure)
3. [One-time setup](#3-one-time-setup)
4. [Your daily workflow (step by step)](#4-your-daily-workflow-step-by-step)
5. [Pull request rules](#5-pull-request-rules)
6. [Coding rules](#6-coding-rules)
7. [Glossary](#7-glossary)

---

## 1. Team and who owns what

| Person | Area | Code they own |
|---|---|---|
| Aung | Backend logic | `lib/core/` (ChangeMaker, Ledger, transactions), Pass GO button |
| Minh | Player interface | Keypad and LCD drivers, menu screens |
| Osy | Sensors | Colour sensor driver, bill classifier |
| Milo | Motors and power | Carousel stepper, input and output rollers |
| Zen | Electrical integration | `include/config/pins.h`, wiring, `src/main.cpp` |
| Felix | Software lead and testing | Reviews, tests, CI, frozen headers |
| Crystal, Kaitlyn | CAD and casing | (no code) |

If you own a file, you write it and you review changes other people make to it.

---

## 2. Code structure

Files marked *(planned)* don't exist yet. They show where new code should go.

```
monopolydealer/
├── platformio.ini            # build settings: "uno" (real board) and "native" (laptop tests)
├── readme.md                 # this file
├── .gitignore                # files Git should never upload
├── .github/
│   ├── CODEOWNERS            # who must review changes to which files
│   └── workflows/ci.yml      # GitHub builds and tests every pull request
├── include/config/           # (planned)
│   ├── pins.h                # every pin number lives here and only here (Zen)
│   └── calibration.h         # colour thresholds, slot positions, roller timings
├── lib/
│   ├── core/                 # pure logic, NO Arduino.h, runs on a laptop
│   │   ├── Types.h           # Result error codes, ColorReading
│   │   ├── Money.h           # bill types ($1 to $500), values, BillCounts
│   │   ├── ChangeMaker.h     # works out which bills to pay out (Aung)
│   │   ├── Ledger.h          # how many bills are in each slot (Aung)
│   │   └── *.cpp             # (planned) the code behind those headers
│   ├── hal/                  # one class per hardware part
│   │   ├── IColorSensor.h    # interface (Osy)
│   │   ├── ICarousel.h       # interface (Milo)
│   │   ├── IRoller.h         # interface (Milo)
│   │   ├── IKeypad.h         # interface (Minh)
│   │   ├── IDisplay.h        # interface (Minh)
│   │   ├── IButton.h         # interface (Aung)
│   │   └── ...               # (planned) the real drivers, e.g. StepperCarousel.cpp
│   └── app/                  # (planned)
│       └── BankerStateMachine.*   # Idle -> Enter amount -> Sort -> Pay out
├── src/
│   └── main.cpp              # creates every part and calls update() in loop()
└── test/
    ├── test_core/            # unit tests that run on your laptop
    ├── mocks/                # (planned) fake hardware for testing
    └── hardware/             # (planned) small test programs for each real part
```

### How the pieces fit together

The code has three layers. Each layer only talks to the one below it.

| Layer | Folder | What it does | Example |
|---|---|---|---|
| App | `lib/app/` | Decides what happens next | "Player pressed #, so start paying out $236" |
| Hardware (HAL) | `lib/hal/` | Makes one physical part do one thing | "Turn the carousel to slot 3" |
| Core | `lib/core/` | Maths and bookkeeping, no hardware | "$236 = 2 x $100 + 1 x $20 + 1 x $10 + 1 x $5 + 1 x $1" |

### What the `I` files are (interfaces)

A file like `IRoller.h` is an **interface**. It lists *what* a part can do (function names and rules) but has no code for *how*. Milo writes the "how" in a separate class, such as `DcRoller`, that follows those rules.

This means nobody has to wait for anyone else. Aung can test the banker logic with a *fake* roller before Milo's motor works, and Minh can build the screens before the backend is finished.

**The interface headers are frozen.** Everyone's code depends on them, so they only change through a pull request that the affected owners approve. The `CODEOWNERS` file makes GitHub request those reviews automatically.

---

## 3. One-time setup

Do this once on your own laptop.

**1. Install the tools**

- **Git:** https://git-scm.com (keep the default options)
- **An editor with PlatformIO.** Pick one:
  - **VS Code** + the **PlatformIO IDE** extension. Easiest if you're new.
  - **CLion** + the **PlatformIO for CLion** plugin. Free for students.

  The Arduino IDE can't open this project, so please use one of these two.

**2. Get a copy of the code (clone)**

Open a terminal in the folder where you keep school projects, then run:

```bash
git clone https://github.com/FelixKongyuy/monopolydealer.git
cd monopolydealer
```

**3. Open the `monopolydealer` folder** in your editor (the folder that holds `platformio.ini`, not the one above it).

**4. Check that everything works**

```bash
pio test -e native
```

---

## 4. Your daily workflow (step by step)

Never edit `main` directly. `main` is the shared copy that always works. You work on your own **branch** (a private copy), then ask to add your changes to `main` with a **pull request**.

```
main ──────────────●──────────────●──────▶
                   │              ▲
                   └─ your branch ┘
                     (commit, commit, push, pull request, review, merge)
```

### Step 1: Start from the latest `main`

```bash
git checkout main
git pull
```

This downloads everyone else's merged work so you don't start from an old copy.

### Step 2: Make a branch for your task

```bash
git checkout -b feat/stepper-homing
```

Name it `type/short-description`:

One branch = one task. Small branches get reviewed faster.

### Step 3: Write your code, then build and test it

```bash
pio test -e native     # runs the unit tests on your laptop
pio run                # checks that it builds for the Uno
pio run -t upload      # (optional) uploads to the board plugged in by USB
```

Fix anything red before moving on.

### Step 4: Save your work (commit)

```bash
git status                       # see what you changed
git add lib/hal/StepperCarousel.cpp lib/hal/StepperCarousel.h
git commit -m "Add stepper homing with home switch"
```

- Add files by name rather than `git add .`, so you don't commit something by accident.
- Write the message as a short instruction: "Add ...", "Fix ...", "Test ...".
- Commit often. Each commit is a save point you can go back to.

### Step 5: Upload your branch (push)

The first time you push a new branch:

```bash
git push -u origin feat/stepper-homing
```

After that, `git push` is enough.

### Step 6: Open a pull request

1. Go to the repo on GitHub. A yellow banner says **"Compare & pull request"**. Click it.
2. Make sure it says **base: `main`** and **compare: your branch**.
3. Fill in the description (see [what to write](#what-to-write-in-the-description)).
4. Click **Create pull request**.

Or from the terminal: `gh pr create --fill`

### Step 7: Wait for CI and a review

**CI** is the automatic check. GitHub builds your code for the Uno and runs every unit test, usually in about a minute.

- **Where to see it:** at the bottom of your pull request, next to "All checks have passed" or "Some checks were not successful". The **Actions** tab at the top of the repo lists every run.
- **Green ✓:** it passed.
- **Red ✗:** something failed. Click **Details** to see the error, fix it on your branch, then commit and push again. CI runs again by itself. Your pull request isn't closed or deleted when CI fails. It just can't be merged until CI is green.

A **reviewer** reads your code and either approves it or leaves comments. Reply to each comment, push your fixes, and ask them to look again.

### Step 8: Merge and clean up

Once CI is green and you have an approval, click **Merge pull request** on GitHub. Then on your laptop:

```bash
git checkout main
git pull
git branch -d feat/stepper-homing
```

You're back at Step 1, ready for the next task.

### The whole loop on one screen

```bash
git checkout main
git pull
git checkout -b feat/my-task
# ...write code...
pio test -e native
pio run
git add <files>
git commit -m "Add my thing"
git push -u origin feat/my-task
# open the pull request on GitHub, wait for ✓ and an approval, merge
```

---

## 5. Pull request rules

`main` is protected, so GitHub enforces these:

- Every change to `main` goes through a pull request.
- At least **1 approval** from a teammate.
- **CI must pass** (green ✓).
- Changes to a frozen header need approval from its **code owner**.
- You can't approve your own pull request.

### What to write in the description

```markdown
## What this does
Adds homing to the carousel: it turns until the home switch clicks, then calls that slot 0.

## How I tested it
- pio test -e native passes
- On the real board: homed 10 times from random positions, stopped on slot 0 every time

## Anything reviewers should know
Homing gives up after 1 full turn and returns HardwareFault.
```

### Before you ask for a review

- [ ] `pio test -e native` passes
- [ ] `pio run` builds with no errors
- [ ] New logic in `lib/core/` has a test in `test/test_core/`
- [ ] No pin numbers outside `include/config/pins.h`
- [ ] No `delay()`, `String` or `new` (see [Coding rules](#6-coding-rules))
- [ ] The pull request does one thing

### When you're the reviewer

- Check that it does what the description says.
- Check the [coding rules](#6-coding-rules).
- Be specific and kind: "This could overflow if amount > 32,767, can we use int32_t?" helps more than "wrong type".
- Approve when you'd be happy for it to run during the demo.

---

## 6. Coding rules

The Uno has only **2 KB of RAM**, which is less than this README. Most of these rules come from that.

| Rule | Why |
|---|---|
| Money is a whole number in `int32_t`. Never `float`. | Floats round, so $0.01 errors creep in. `int` on the Uno is only 16 bits and breaks past 32,767. |
| No `String`, `new` or `malloc`. Use fixed `char` arrays. | They break RAM into pieces until the board freezes mid-game. |
| Wrap printed text in `F("...")`. | Keeps the text in flash instead of using up RAM. |
| No `delay()` outside `setup()`. Use `millis()` and an `update()` function. | `delay()` freezes everything, including the keypad, while a motor runs. |
| Pin numbers only in `include/config/pins.h`. | One place to check when the wiring changes. |
| Functions that can fail return a `Result`. | The caller always knows if something went wrong. |
| Unknown bill colour means reject or re-scan, never guess. | A wrong guess puts a bill in the wrong slot and the bank's totals go wrong. |
| `lib/core/` never includes `Arduino.h`. | Core code has to run on a laptop for the unit tests. |

### Naming

| What | Style | Example |
|---|---|---|
| Classes, structs, enums | `PascalCase` | `StepperCarousel`, `BillCounts` |
| Functions, variables | `camelCase` | `moveToSlot()`, `billCount` |
| Constants | `k` + `PascalCase` | `kSlotCapacity` |
| Interfaces | `I` + `PascalCase` | `IRoller` |
| Private member variables | trailing `_` | `counts_` |

### Every hardware class looks like this

```cpp
class DcRoller : public IRoller {
 public:
  Result begin() override;          // once, in setup()
  void update() override;           // every loop(), returns quickly
  Result startFeedOne() override;   // starts the job and returns right away
  bool isBusy() const override;     // the app checks this to know when it's done
  // ...
};
```

---

## 7. Glossary

| Term | Meaning |
|---|---|
| **Repository (repo)** | The project folder plus its full history, stored on GitHub. |
| **Clone** | Download a copy of the repo to your laptop. |
| **Branch** | Your own copy of the code to work on without affecting anyone else. |
| **Commit** | A saved snapshot of your changes, with a message. |
| **Push** | Upload your commits to GitHub. |
| **Pull** | Download other people's commits from GitHub. |
| **Pull request (PR)** | A request to merge your branch into `main`, where teammates review it. |
| **Merge** | Combine a branch's changes into another branch. |
| **CI** | Continuous Integration: GitHub automatically builds and tests every pull request. |
| **Merge conflict** | Two people changed the same lines and Git needs a human to pick. |
| **Interface** | A header that lists what a part does, with no code for how. |
| **HAL** | Hardware Abstraction Layer: the code that talks to real hardware. |
| **I2C** | A two-wire connection (SDA and SCL) that several devices share. |
| **PlatformIO** | The tool that builds, uploads and tests the code. |
