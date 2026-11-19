monopoly-banker/
├── platformio.ini        # env:uno (real board) + env:native (laptop tests)
├── .clang-format         # one shared formatting style
├── .github/workflows/ci.yml
├── include/config/
│   ├── pins.h            # every pin number, only here (Zen owns)
│   └── calibration.h     # colour thresholds, slot angles/steps, roller timings
├── lib/
│   ├── core/             # plain C++, NO Arduino.h (Aung)
│   │   ├── Money.h       # Denom enum, values, slot mapping
│   │   ├── ChangeMaker.* # stock-aware change algorithm
│   │   ├── Ledger.*      # bill count per slot, total balance
│   │   └── Transaction.* # deposit / withdraw / exchange rules
│   ├── hal/              # one class per hardware part, behind an interface
│   │   ├── IColorSensor.h  + Apds9960Sensor.*   (Osy)
│   │   ├── ICarousel.h     + StepperCarousel.*  (Milo)
│   │   ├── IRoller.h       + DcRoller.*         (Milo)
│   │   ├── IKeypad.h, IDisplay.h + impls        (Minh)
│   │   └── BillClassifier.*  # RGB reading → Denom or Unknown (Osy)
│   └── app/
│       └── BankerStateMachine.*  # Idle → EnterAmount → Sorting → Dispensing → Error
├── src/main.cpp          # wiring only: build objects, call update() in loop()
└── test/
    ├── test_core/        # native unit tests (you + Aung)
    ├── mocks/            # FakeCarousel, FakeSensor, etc.
    └── hardware/         # one small test sketch per part (servo/stepper sweep, roller feed)

Hello